#pragma once

#include <imgui.h>
#include <mutex>

class UserInterface;
class Input;
class Rendering;
struct GLFWwindow;
class Shader;
class Model;

class NeonEngine {
public:
    static NeonEngine* get_instance();
    NeonEngine(NeonEngine& other) = delete;
    void operator=(const NeonEngine&) = delete;

    int run();

    GLFWwindow* window;
    const char* glsl_version;
    int glfw_major_version;
    int glfw_minor_version;
    ImVec4 clear_color;
    int window_width;
    int window_height;
    const char* window_title;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool firstMouse;
    float lastX, lastY;

private:
    NeonEngine();
    ~NeonEngine();

    void initialize_all_components();
    int setup_glfw();
    int setup_glad();
    void clean_gflw();

    UserInterface* user_interface;
    Input* input;
    Rendering* rendering;
    
    static NeonEngine* instance;
    static std::mutex neon_engine_mutex;
};