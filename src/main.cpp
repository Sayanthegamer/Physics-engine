#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>

#include "../include/GearEngine.hpp"
#include "../include/ConstraintSolver.hpp"
#include "../include/CommandQueue.hpp"
#include "../include/UI/Theme.hpp"
#include "../include/EditorCamera.hpp"
#include "../include/DebugRenderer.hpp"

// Standard main signature
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "GearEngine - DOD Mechanical Physics", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply custom UI theme
    UI::ApplyTheme();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Engine State Initialization
    GearEngine::EngineState engine_state;
    GearEngine::ConstraintArrays constraint_arrays;
    GearEngine::ConstraintSolver solver;
    GearEngine::CommandQueue command_queue;

    GearEngine::EditorCamera camera;
    GearEngine::DebugRenderer renderer; // Must be initialized AFTER ImGui OpenGL loader setup

    // Fixed timestep settings
    const float kFixedDt = 1.0f / 60.0f;
    float accumulator = 0.0f;
    
    // UI State
    int grabbed_gear = -1;
    int next_teeth = 8;
    const float kTeethDensity = 4.0f;
    float motor_speed = 5.0f;
    uint32_t selected_motor_gear = 0;
    
    auto previous_time = std::chrono::high_resolution_clock::now();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float frame_time = std::chrono::duration<float>(current_time - previous_time).count();
        previous_time = current_time;

        // Cap frame time to prevent spiral of death
        if (frame_time > 0.25f) frame_time = 0.25f;

        accumulator += frame_time;

        // Process any deferred commands before the physics steps
        command_queue.ApplyCommands(engine_state, constraint_arrays);

        // Fixed Timestep Physics Simulation
        while (accumulator >= kFixedDt) {
            // Solve velocity constraints first
            solver.Solve(engine_state, constraint_arrays, kFixedDt, 8);
            
            // Integrate velocities into positions
            solver.IntegratePositions(engine_state, kFixedDt);
            
            accumulator -= kFixedDt;
        }

        // Poll Events
        glfwPollEvents();
        
        // Update Camera
        camera.Update(window, frame_time);
        if (io.MouseWheel != 0.0f) {
            camera.ProcessScroll(io.MouseWheel);
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // UI Panel
        // --- 3D Interaction Logic ---
        glm::vec3 mouse_world = camera.GetMouseWorldPosition(window, display_w, display_h);
        
        if (!io.WantCaptureMouse && io.MouseWheel != 0.0f) {
            if (io.MouseWheel > 0) next_teeth++;
            if (io.MouseWheel < 0) next_teeth--;
            if (next_teeth < 4) next_teeth = 4;
            if (next_teeth > 64) next_teeth = 64;
        }
        
        float next_radius = (float)next_teeth / kTeethDensity;

        int nearest_gear = -1;
        float min_dist = 9999.0f;
        int hovered_gear = -1;
        const auto& bodies = engine_state.GetBodies();
        
        for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
            if (engine_state.IsIndexActive(i) && (int)i != grabbed_gear) {
                float dist = glm::length(bodies.positions[i] - mouse_world);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest_gear = i;
                }
                if (dist <= bodies.radii[i]) {
                    hovered_gear = i;
                }
            }
        }
        
        bool is_snapped = false;
        glm::vec3 placement_pos = mouse_world;
        int snap_target = -1;
        float current_radius = (grabbed_gear != -1) ? bodies.radii[grabbed_gear] : next_radius;
        
        // Only snap if we aren't directly hovering over a gear's body
        if (nearest_gear != -1 && hovered_gear == -1) {
            float target_dist = bodies.radii[nearest_gear] + current_radius;
            if (min_dist < target_dist + 2.0f) { // 2.0f snap tolerance
                glm::vec3 dir = glm::normalize(mouse_world - bodies.positions[nearest_gear]);
                placement_pos = bodies.positions[nearest_gear] + dir * target_dist;
                is_snapped = true;
                snap_target = nearest_gear;
            }
        }
        
        if (grabbed_gear != -1) {
            engine_state.GetBodies().positions[grabbed_gear] = placement_pos;
        }

        if (!io.WantCaptureMouse) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (hovered_gear != -1) {
                    grabbed_gear = hovered_gear;
                    selected_motor_gear = hovered_gear;
                } else {
                    // Spawn new gear
                    GearEngine::EntityHandle new_handle = engine_state.AllocateBody();
                    if (new_handle.IsValid()) {
                        uint32_t i = new_handle.index;
                        engine_state.GetBodies().positions[i] = placement_pos;
                        engine_state.GetBodies().radii[i] = next_radius;
                        engine_state.GetBodies().linear_velocities[i] = glm::vec3(0.0f);
                        engine_state.GetBodies().angular_velocities[i] = glm::vec3(0.0f);
                        engine_state.GetBodies().rotations[i] = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                        
                        float mass = 3.14159f * next_radius * next_radius;
                        engine_state.GetBodies().inverse_masses[i] = 1.0f / mass;
                        float I_zz = 0.5f * mass * next_radius * next_radius;
                        engine_state.GetBodies().inverse_inertias[i] = glm::mat3(0.0f);
                        engine_state.GetBodies().inverse_inertias[i][2][2] = 1.0f / I_zz;
                        
                        if (is_snapped) {
                            GearEngine::GearConstraint gc;
                            gc.body_a = new_handle;
                            gc.body_b = {(uint32_t)snap_target, engine_state.GetGeneration(snap_target)};
                            // FIX: Ratio must be Target_Radius / New_Radius 
                            // so that w_A + ratio*w_B = 0 enforces equal tangential velocity
                            gc.ratio = engine_state.GetBodies().radii[snap_target] / next_radius;
                            constraint_arrays.gears.push_back(gc);
                        }
                    }
                }
            }
            else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (grabbed_gear != -1 && is_snapped) {
                    GearEngine::GearConstraint gc;
                    gc.body_a = {(uint32_t)grabbed_gear, engine_state.GetGeneration(grabbed_gear)};
                    gc.body_b = {(uint32_t)snap_target, engine_state.GetGeneration(snap_target)};
                    // FIX: Ratio is Target_Radius / Grabbed_Radius
                    gc.ratio = engine_state.GetBodies().radii[snap_target] / engine_state.GetBodies().radii[grabbed_gear];
                    constraint_arrays.gears.push_back(gc);
                }
                grabbed_gear = -1;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                int target_to_delete = hovered_gear != -1 ? hovered_gear : (selected_motor_gear != 0 ? selected_motor_gear : -1);
                if (target_to_delete != -1) {
                    GearEngine::RemoveBodyCommand cmd;
                    cmd.handle = {(uint32_t)target_to_delete, engine_state.GetGeneration(target_to_delete)};
                    command_queue.PushCommand(cmd);
                    if (selected_motor_gear == (uint32_t)target_to_delete) selected_motor_gear = 0;
                    if (grabbed_gear == target_to_delete) grabbed_gear = -1;
                }
            }
        }
        // ------------------------------

        ImGui::Begin("Engine Statistics");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        
        int active_count = 0;
        for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
            if (engine_state.IsIndexActive(i)) active_count++;
        }
        ImGui::Text("Active Bodies: %d (Max %d)", active_count, GearEngine::kMaxBodies); 
        
        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::BulletText("Left-Click: Place Gear (Snaps to edges)");
        ImGui::BulletText("Left-Click & Drag: Move Gear (Auto-links)");
        ImGui::BulletText("Delete Key: Delete Hovered/Selected Gear");
        ImGui::BulletText("Scroll Wheel: Resize Brush");
        ImGui::Separator();
        
        if (ImGui::Button("Clear All")) {
            for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
                if (engine_state.IsIndexActive(i)) {
                    GearEngine::RemoveBodyCommand cmd;
                    cmd.handle = {i, engine_state.GetGeneration(i)};
                    command_queue.PushCommand(cmd);
                }
            }
            constraint_arrays.gears.clear();
            constraint_arrays.motors.clear();
            selected_motor_gear = 0;
            grabbed_gear = -1;
        }

        ImGui::Separator();
        if (selected_motor_gear != 0 && engine_state.IsIndexActive(selected_motor_gear)) {
            ImGui::Text("Motor Control (Gear %d)", selected_motor_gear);
            ImGui::SliderFloat("Speed", &motor_speed, -20.0f, 20.0f);
            if (ImGui::Button("Apply Motor")) {
                bool found = false;
                for (auto& mc : constraint_arrays.motors) {
                    if (mc.body.index == (uint32_t)selected_motor_gear) {
                        mc.target_speed = motor_speed;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    GearEngine::MotorConstraint mc;
                    mc.body = {(uint32_t)selected_motor_gear, engine_state.GetGeneration(selected_motor_gear)};
                    mc.target_speed = motor_speed;
                    constraint_arrays.motors.push_back(mc);
                }
            }
        } else {
            ImGui::TextDisabled("Click a gear to access motor controls.");
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw Debug Renderer
        renderer.Draw(engine_state, constraint_arrays, camera, display_w, display_h, grabbed_gear != -1 ? grabbed_gear : selected_motor_gear);
        
        if (grabbed_gear == -1 && !io.WantCaptureMouse && hovered_gear == -1) {
            renderer.DrawPreviewGear(placement_pos, next_radius, is_snapped, camera, display_w, display_h);
        }
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
