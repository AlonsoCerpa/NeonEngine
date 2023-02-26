#pragma once

#include <mutex>

class NeonEngine;
struct GLFWwindow;

class Input {
public:
    static Input* get_instance();
    void processInput();
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void initialize();

    Input(Input& other) = delete;
    void operator=(const Input&) = delete;

private:
    Input();
    ~Input();

    static bool firstMouse;
    static float lastX;
    static float lastY;
    static NeonEngine* neon_engine;

    static Input* instance;
    static std::mutex input_mutex;
};