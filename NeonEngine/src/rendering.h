#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera;
class Shader;
class Model;
class UserInterface;
class NeonEngine;
class Shape;
class GameObject;
class PointLight;
class DirectionalLight;
class SpotLight;

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);
void create_and_set_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer, unsigned int* rboDepthStencil, int width, int height);

class KeyGenerator {
public:
    KeyGenerator(int max_num_keys) {
        this->max_num_keys = max_num_keys;
        this->available_keys = std::vector<int>(max_num_keys);
        for (int i = max_num_keys - 1; i >= 0; i--) {
            this->available_keys[max_num_keys-i-1] = i;
        }
    }

    int generate_key() {
        if (available_keys.empty()) {
            std::cout << "Error: no available keys!" << std::endl;
            return -1;
        }
        int key = available_keys.back();
        available_keys.pop_back();
        return key;
    }

    void return_key(int key) {
        if (key < 0 || key >= max_num_keys) {
            std::cout << "Error: invalid key!" << std::endl;
            return;
        }
        available_keys.push_back(key);
    }

private:
    int max_num_keys;
    std::vector<int> available_keys;
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

    KeyGenerator* key_generator;
    glm::mat4 view, projection;
    glm::mat4 view_projection, view_projection_inv;
    Camera* camera_viewport;
    float near_camera_viewport, far_camera_viewport;
    unsigned int framebuffer, textureColorbuffer, rboDepthStencil;
    Shader* ourShader;
    std::vector<Model*> loaded_models;
    std::unordered_map<int, PointLight*> point_lights;
    std::unordered_map<int, DirectionalLight*> directional_lights;
    std::unordered_map<int, SpotLight*> spot_lights;
    std::unordered_map<int, GameObject*> game_objects;
    int key_selected_object;
    UserInterface* user_interface;
    NeonEngine* neon_engine;

private:
    Rendering();
    ~Rendering();

    static Rendering* instance;
    static std::mutex rendering_mutex;
};