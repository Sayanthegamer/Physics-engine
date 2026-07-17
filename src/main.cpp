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

    // Fixed timestep settings
    const float kFixedDt = 1.0f / 60.0f;
    float accumulator = 0.0f;
    
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
            // Sequential Impulse Solver
            solver.Solve(engine_state, constraint_arrays, kFixedDt, 8);
            
            accumulator -= kFixedDt;
        }

        // Poll Events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI Panel
        ImGui::Begin("Engine Statistics");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Active Bodies: %d (Max %d)", 0, GearEngine::kMaxBodies); 
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
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
