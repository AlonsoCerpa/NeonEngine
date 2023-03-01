#pragma once

#include <mutex>

class NeonEngine;
class Rendering;
struct GLFWwindow;

class Input {
public:
    static Input* get_instance();
    //void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void initialize();
    void process_viewport_input();

    Input(Input& other) = delete;
    void operator=(const Input&) = delete;

private:
    Input();
    ~Input();

    void mouse_rotate_camera();

    NeonEngine* neon_engine;
    Rendering* rendering;

    static Input* instance;
    static std::mutex input_mutex;
};