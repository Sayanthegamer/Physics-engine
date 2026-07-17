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
    std::vector<uint32_t> selected_gears;
    float motor_speed = 5.0f;
    
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

        // UI Panel
        ImGui::Begin("Engine Statistics");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        
        int active_count = 0;
        for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
            if (engine_state.IsIndexActive(i)) active_count++;
        }
        ImGui::Text("Active Bodies: %d (Max %d)", active_count, GearEngine::kMaxBodies); 
        
        ImGui::Separator();
        
        if (ImGui::Button("Add Gear")) {
            GearEngine::AddBodyCommand cmd;
            cmd.position = glm::vec3((rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f, 0.0f);
            cmd.radius = (rand() % 20 + 5) / 10.0f; // Random radius between 0.5 and 2.5
            command_queue.PushCommand(cmd);
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Clear All")) {
            for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
                if (engine_state.IsIndexActive(i)) {
                    GearEngine::RemoveBodyCommand cmd;
                    cmd.handle = {i, engine_state.GetGeneration(i)};
                    command_queue.PushCommand(cmd);
                }
            }
            selected_gears.clear();
            constraint_arrays.gears.clear(); // Clear constraints too
        }

        ImGui::Separator();
        ImGui::Text("Gear Linker:");
        
        // Remove stale selections
        selected_gears.erase(std::remove_if(selected_gears.begin(), selected_gears.end(), 
            [&](uint32_t id) { return !engine_state.IsIndexActive(id); }), selected_gears.end());

        ImGui::BeginChild("GearList", ImVec2(0, 150), true);
        for (uint32_t i = 1; i < engine_state.GetCapacity(); ++i) {
            if (engine_state.IsIndexActive(i)) {
                bool is_selected = std::find(selected_gears.begin(), selected_gears.end(), i) != selected_gears.end();
                if (ImGui::Selectable(("Gear ID " + std::to_string(i)).c_str(), is_selected)) {
                    if (is_selected) {
                        selected_gears.erase(std::remove(selected_gears.begin(), selected_gears.end(), i), selected_gears.end());
                    } else {
                        selected_gears.push_back(i);
                    }
                }
            }
        }
        ImGui::EndChild();

        if (selected_gears.size() == 2) {
            if (ImGui::Button("Link Selected Gears")) {
                GearEngine::AddGearConstraintCommand cmd;
                cmd.constraint.body_a = {selected_gears[0], engine_state.GetGeneration(selected_gears[0])};
                cmd.constraint.body_b = {selected_gears[1], engine_state.GetGeneration(selected_gears[1])};
                
                float r_a = engine_state.GetBodies().radii[selected_gears[0]];
                float r_b = engine_state.GetBodies().radii[selected_gears[1]];
                cmd.constraint.ratio = r_a / r_b; // Standard gear ratio
                
                command_queue.PushCommand(cmd);
                selected_gears.clear();
            }
        } else if (selected_gears.size() == 1) {
            ImGui::SliderFloat("Speed", &motor_speed, -20.0f, 20.0f);
            if (ImGui::Button("Apply Motor")) {
                uint32_t a = selected_gears[0];
                engine_state.GetBodies().angular_velocities[a].z = motor_speed;
            }
        } else {
            ImGui::TextDisabled("Select 1 gear for Motor, 2 to Link.");
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw Debug Renderer
        renderer.Draw(engine_state, constraint_arrays, camera, display_w, display_h);
        
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
