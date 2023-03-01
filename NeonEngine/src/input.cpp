#include "input.h"

#include "neon_engine.h"
#include "rendering.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <mutex>

Input* Input::instance = nullptr;
std::mutex Input::input_mutex;

Input::Input() {
    
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
    if (ImGui::IsWindowHovered()) {
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
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            mouse_rotate_camera();
        }
    }
    if (!firstMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        mouse_rotate_camera();
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        firstMouse = true;
    }
}

void Input::mouse_rotate_camera() {
    Camera* camera_viewport = rendering->camera_viewport;
    bool& firstMouse = neon_engine->firstMouse;
    float& lastX = neon_engine->lastX;
    float& lastY = neon_engine->lastY;

    ImVec2 mouse_pos = ImGui::GetMousePos();
    if (firstMouse)
    {
        lastX = mouse_pos.x;
        lastY = mouse_pos.y;
        firstMouse = false;
    }

    float xoffset = mouse_pos.x - lastX;
    float yoffset = lastY - mouse_pos.y; // reversed since y-coordinates go from bottom to top

    lastX = mouse_pos.x;
    lastY = mouse_pos.y;

    camera_viewport->ProcessMouseMovement(xoffset, yoffset);
}