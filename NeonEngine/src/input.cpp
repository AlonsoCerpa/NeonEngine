#include "input.h"

#include "neon_engine.h"
#include "rendering.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <mutex>
#include <iostream>

Input* Input::instance = nullptr;
std::mutex Input::input_mutex;

Input::Input() {
    neon_engine = nullptr;
    rendering = nullptr;
}

Input::~Input() {

}

Input* Input::get_instance()
{
    std::lock_guard<std::mutex> lock(input_mutex);
    if (instance == nullptr) {
        instance = new Input();
    }
    return instance;
}

void Input::initialize() {
    neon_engine = NeonEngine::get_instance();
    rendering = Rendering::get_instance();
}

/*
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Input::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}*/

void Input::process_viewport_input() {
    Camera* camera_viewport = rendering->camera_viewport;
    float& deltaTime = neon_engine->deltaTime;
    bool& firstMouse = neon_engine->firstMouse;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        int game_object_idx = rendering->check_mouse_over_models();
        if (rendering->idx_selected_object != -1) {
            rendering->game_objects[rendering->idx_selected_object].is_selected = false;
        }
        if (game_object_idx != -1) {
            rendering->game_objects[game_object_idx].is_selected = true;
            rendering->idx_selected_object = game_object_idx;
            std::cout << "INTERSECTION DETECTED: " << game_object_idx << std::endl;
        }
        else {
            std::cout << "NOT INTERSECTION DETECTED" << std::endl;
        }
    }
    if (!firstMouse || (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
        if (firstMouse) {
            glfwSetInputMode(neon_engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        if (ImGui::IsKeyDown(ImGuiKey_W)) {
            camera_viewport->ProcessKeyboard(FORWARD, deltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_S)) {
            camera_viewport->ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_A)) {
            camera_viewport->ProcessKeyboard(LEFT, deltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_D)) {
            camera_viewport->ProcessKeyboard(RIGHT, deltaTime);
        }
        mouse_rotate_camera();
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        glfwSetInputMode(neon_engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void Input::mouse_rotate_camera() {
    Camera* camera_viewport = rendering->camera_viewport;
    bool& firstMouse = neon_engine->firstMouse;
    float& lastX = neon_engine->lastX;
    float& lastY = neon_engine->lastY;

    double mouse_pos_x, mouse_pos_y;
    glfwGetCursorPos(neon_engine->window, &mouse_pos_x, &mouse_pos_y);
    if (firstMouse)
    {
        lastX = mouse_pos_x;
        lastY = mouse_pos_y;
        firstMouse = false;
    }

    float xoffset = mouse_pos_x - lastX;
    float yoffset = lastY - mouse_pos_y; // reversed since y-coordinates go from bottom to top

    lastX = mouse_pos_x;
    lastY = mouse_pos_y;

    camera_viewport->ProcessMouseMovement(xoffset, yoffset);
}