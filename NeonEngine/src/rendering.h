#pragma once

#include <mutex>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera;
class Shader;
class Model;
class UserInterface;
class NeonEngine;
class Shape;

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);
void create_and_set_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer, unsigned int* rboDepthStencil, int width, int height);

class GameObject {
public:
    int idx_loaded_models = -1;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 axis_rotation;
    float angle_rotation_degrees;
    glm::mat4 model;
    glm::mat4 model_inv;
    bool is_selected;

    GameObject(int idx_loaded_models, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& axis_rot, float angle, bool is_selected) {
        this->idx_loaded_models = idx_loaded_models;
        this->position = pos;
        this->scale = scale;
        this->axis_rotation = axis_rot;
        this->angle_rotation_degrees = angle;
        this->is_selected = is_selected;

        set_model_and_model_inv();
    }

    void set_model_and_model_inv() {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(angle_rotation_degrees), axis_rotation);
        model = glm::scale(model, scale);
        
        model_inv = glm::inverse(model);
    }
};

class Rendering {
public:
    static Rendering* get_instance();
    void initialize();

    Rendering(Rendering& other) = delete;
    void operator=(const Rendering&) = delete;

    void set_opengl_state();
    void render_viewport();
    void set_viewport_shaders();
    void set_viewport_models();
    void create_and_set_viewport_framebuffer();
    void clean();
    void clean_viewport_framebuffer();
    int check_mouse_over_models();
    void initialize_game_objects();

    glm::mat4 view, projection;
    glm::mat4 view_projection, view_projection_inv;
    Camera* camera_viewport;
    float near_camera_viewport, far_camera_viewport;
    unsigned int framebuffer, textureColorbuffer, rboDepthStencil;
    Shader* ourShader;
    std::vector<Model*> loaded_models;
    std::vector<GameObject> game_objects;
    int idx_selected_object;
    UserInterface* user_interface;
    NeonEngine* neon_engine;

private:
    Rendering();
    ~Rendering();

    static Rendering* instance;
    static std::mutex rendering_mutex;
};