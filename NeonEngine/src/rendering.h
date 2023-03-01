#pragma once

#include <mutex>

class Camera;
class Shader;
class Model;
class UserInterface;

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);
void create_and_set_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer, unsigned int* rboDepthStencil, int width, int height);

class Rendering {
public:
    static Rendering* get_instance();
    void initialize();

    Rendering(Rendering& other) = delete;
    void operator=(const Rendering&) = delete;

    void render_viewport();
    void set_viewport_shaders();
    void set_viewport_models();
    void create_and_set_viewport_framebuffer();
    void clean();
    void clean_viewport_framebuffer();

    Camera* camera_viewport;
    unsigned int framebuffer, textureColorbuffer, rboDepthStencil;
    Shader* ourShader;
    Model* ourModel;
    UserInterface* user_interface;

private:
    Rendering();
    ~Rendering();

    static Rendering* instance;
    static std::mutex rendering_mutex;
};