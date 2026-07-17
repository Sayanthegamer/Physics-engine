#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace GearEngine {

class EditorCamera {
public:
    EditorCamera() {
        UpdateVectors();
    }

    void Update(GLFWwindow* window, float /*dt*/) {
        ImGuiIO& io = ImGui::GetIO();
        
        // Crucial: Don't capture inputs if ImGui wants them (e.g. clicking buttons)
        if (io.WantCaptureMouse) return;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (first_mouse_) {
            last_x_ = xpos;
            last_y_ = ypos;
            first_mouse_ = false;
        }

        float xoffset = static_cast<float>(xpos - last_x_);
        float yoffset = static_cast<float>(last_y_ - ypos);
        last_x_ = xpos;
        last_y_ = ypos;

        int state_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        int state_middle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

        if (state_right == GLFW_PRESS) {
            // Orbit
            yaw_ += xoffset * mouse_sensitivity_;
            pitch_ += yoffset * mouse_sensitivity_;

            if (pitch_ > 89.0f) pitch_ = 89.0f;
            if (pitch_ < -89.0f) pitch_ = -89.0f;
            
            UpdateVectors();
        }

        if (state_middle == GLFW_PRESS) {
            // Pan
            glm::vec3 right = glm::normalize(glm::cross(front_, up_));
            glm::vec3 up = glm::normalize(glm::cross(right, front_));
            target_ -= (right * xoffset + up * yoffset) * pan_speed_;
            UpdateVectors(); // Update position based on new target
        }
    }

    void ProcessScroll(float yoffset) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        distance_ -= yoffset * zoom_speed_;
        if (distance_ < 1.0f) distance_ = 1.0f;
        if (distance_ > 1000.0f) distance_ = 1000.0f;
        UpdateVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(position_, target_, up_);
    }

    glm::mat4 GetProjectionMatrix(float aspect_ratio) const {
        return glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 1000.0f);
    }

private:
    void UpdateVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = glm::normalize(front);
        
        // Orbit position around the target
        position_ = target_ - front_ * distance_;
    }

    glm::vec3 position_{0.0f, 0.0f, 10.0f};
    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    glm::vec3 front_{0.0f, 0.0f, -1.0f};
    glm::vec3 up_{0.0f, 1.0f, 0.0f};

    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float distance_ = 10.0f;

    float mouse_sensitivity_ = 0.5f;
    float zoom_speed_ = 1.0f;
    float pan_speed_ = 0.02f;

    double last_x_ = 0.0;
    double last_y_ = 0.0;
    bool first_mouse_ = true;
};

} // namespace GearEngine
