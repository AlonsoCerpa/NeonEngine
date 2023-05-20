#include "input.h"

#include "neon_engine.h"
#include "rendering.h"
#include "camera.h"
#include "game_object.h"
#include "transform3d.h"
#include "user_interface.h"

#include <GLFW/glfw3.h>
#include <mutex>
#include <iostream>

Input* Input::instance = nullptr;
std::mutex Input::input_mutex;

Input::Input() {
    neon_engine = nullptr;
    rendering = nullptr;
    last_mouse_pos_selecting = ImVec2(0.0f, 0.0f);
    last_mouse_pos_transforming = ImVec2(0.0f, 0.0f);
    transforming_selected_object = false;
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
    UserInterface* user_interface = UserInterface::get_instance();
    Camera* camera_viewport = rendering->camera_viewport;
    float& deltaTime = neon_engine->delta_time_seconds;
    bool& firstMouse = neon_engine->firstMouse;
    //ImVec2 current_mouse_pos = ImGui::GetMousePos();
    double current_mouse_pos_x, current_mouse_pos_y;
    glfwGetCursorPos(neon_engine->window, &current_mouse_pos_x, &current_mouse_pos_y);
    ImVec2 current_mouse_pos(current_mouse_pos_x, current_mouse_pos_y);
    ImVec2 imgui_mouse_pos = ImGui::GetIO().MousePos;

    if (transforming_selected_object) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            glm::vec2 transform_vector(current_mouse_pos.x - last_mouse_pos_transforming.x, last_mouse_pos_transforming.y - current_mouse_pos.y);
            rendering->transform3d->transform(transform_vector);
            last_mouse_pos_transforming = current_mouse_pos;
        }
        else {
            if (rendering->transform3d->type != TRANSLATION) {
                glfwSetInputMode(neon_engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            transforming_selected_object = false;
        }
    }
    else {
        if (rendering->last_selected_object_transform3d != nullptr) {
            rendering->transform3d->set_highlight(false);
            rendering->last_selected_object_transform3d = nullptr;
        }
        GameObject* selected_object_transform3d = rendering->check_mouse_over_transform3d();
        if (selected_object_transform3d != nullptr) {
            rendering->last_selected_object_transform3d = selected_object_transform3d;
            rendering->transform3d->set_highlight(true);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
                if (rendering->transform3d->type != TRANSLATION) {
                    glfwSetInputMode(neon_engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                transforming_selected_object = true;
                last_mouse_pos_transforming = current_mouse_pos;
            }
        }
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
        last_mouse_pos_selecting = current_mouse_pos;
    }
    float squared_dist = (last_mouse_pos_selecting.x - current_mouse_pos.x) * (last_mouse_pos_selecting.x - current_mouse_pos.x);
    squared_dist += (last_mouse_pos_selecting.y - current_mouse_pos.y) * (last_mouse_pos_selecting.y - current_mouse_pos.y);

    ImVec2 mouse_pos_in_window = ImVec2(imgui_mouse_pos.x - user_interface->viewport_window_pos.x, imgui_mouse_pos.y - user_interface->viewport_window_pos.y);

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && squared_dist <= 25.0f && mouse_pos_in_window.x >= 0 && mouse_pos_in_window.y >= 0 &&
        mouse_pos_in_window.x <= user_interface->window_viewport_width && mouse_pos_in_window.y <= user_interface->window_viewport_height) {
        if (rendering->last_selected_object != nullptr) {
            rendering->last_selected_object->set_select_state(false);
            rendering->last_selected_object = nullptr;
        }
        GameObject* selected_object = rendering->check_mouse_over_models();
        if (selected_object != nullptr) {
            selected_object->set_select_state(true);
            rendering->last_selected_object = selected_object;
        }
        else {
            selected_object = rendering->game_objects["skybox"];
            selected_object->set_select_state(true);
            rendering->last_selected_object = selected_object;
        }
    }

    if (!transforming_selected_object) {
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
        else if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_E) || ImGui::IsKeyDown(ImGuiKey_R)) {
            TransformType old_type = rendering->transform3d->type;
            if (ImGui::IsKeyDown(ImGuiKey_W)) {
                rendering->transform3d->type = TRANSLATION;
            }
            else if (ImGui::IsKeyDown(ImGuiKey_E)) {
                rendering->transform3d->type = ROTATION;
            }
            else if (ImGui::IsKeyDown(ImGuiKey_R)) {
                rendering->transform3d->type = SCALING;
            }
            if (rendering->last_selected_object_transform3d != nullptr && old_type != rendering->transform3d->type) {
                TransformType new_type = rendering->transform3d->type;
                rendering->transform3d->type = old_type;
                rendering->transform3d->set_highlight(false);
                rendering->transform3d->type = new_type;
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            glfwSetInputMode(neon_engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
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