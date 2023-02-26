#include "input.h"

#include "neon_engine.h"

#include <GLFW/glfw3.h>
#include <mutex>

Input* Input::instance = nullptr;
std::mutex Input::input_mutex;
bool Input::firstMouse = true;
float Input::lastX = 0;
float Input::lastY = 0;
NeonEngine* Input::neon_engine = nullptr;

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
    lastX = (float)(neon_engine->viewport_width / 2.0);
    lastY = (float)(neon_engine->viewport_height / 2.0);
}

void Input::processInput()
{
    if (glfwGetKey(neon_engine->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(neon_engine->window, true);

    if (glfwGetKey(neon_engine->window, GLFW_KEY_W) == GLFW_PRESS)
        neon_engine->camera_viewport.ProcessKeyboard(FORWARD, neon_engine->deltaTime);
    if (glfwGetKey(neon_engine->window, GLFW_KEY_S) == GLFW_PRESS)
        neon_engine->camera_viewport.ProcessKeyboard(BACKWARD, neon_engine->deltaTime);
    if (glfwGetKey(neon_engine->window, GLFW_KEY_A) == GLFW_PRESS)
        neon_engine->camera_viewport.ProcessKeyboard(LEFT, neon_engine->deltaTime);
    if (glfwGetKey(neon_engine->window, GLFW_KEY_D) == GLFW_PRESS)
        neon_engine->camera_viewport.ProcessKeyboard(RIGHT, neon_engine->deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Input::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Input::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    neon_engine->camera_viewport.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    neon_engine->camera_viewport.ProcessMouseScroll(static_cast<float>(yoffset));
}