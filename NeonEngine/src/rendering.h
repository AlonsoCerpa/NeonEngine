#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <chrono>

class Camera;
class Shader;
class BaseModel;
class UserInterface;
class NeonEngine;
class Shape;
class GameObject;
class PointLight;
class DirectionalLight;
class SpotLight;
class Transform3D;
class Quad;

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);
void create_and_set_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer, unsigned int* rboDepthStencil, int width, int height);

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
    void initialize_game_objects();
    void set_time_before_rendering_loop();
    void create_and_set_viewport_framebuffer();
    void clean();
    void clean_viewport_framebuffer();
    GameObject* check_mouse_over_models();
    std::string check_mouse_over_models2();
    GameObject* check_mouse_over_transform3d();

    glm::mat4 view, projection;
    glm::mat4 view_projection, view_projection_inv;
    Camera* camera_viewport;
    Quad* screen_quad;
    float near_camera_viewport, far_camera_viewport;
    unsigned int framebuffer, textureColorbuffer, texture_id_colors, texture_selected_color_buffer;
    unsigned int texture_id_colors_transform3d, rboDepthStencil;
    std::chrono::time_point<std::chrono::system_clock> time_before_rendering;
    Shader* phong_shader;
    Shader* selection_shader;
    Shader* outline_shader;
    std::unordered_map<glm::u8vec3, GameObject*> id_color_to_game_object;
    std::unordered_map<glm::u8vec3, GameObject*> id_color_to_game_object_transform3d;
    std::unordered_map<std::string, BaseModel*> loaded_models;
    std::unordered_map<std::string, PointLight*> point_lights;
    std::unordered_map<std::string, DirectionalLight*> directional_lights;
    std::unordered_map<std::string, SpotLight*> spot_lights;
    std::unordered_map<std::string, GameObject*> game_objects;
    Transform3D* transform3d;
    GameObject* last_selected_object;
    GameObject* last_selected_object_transform3d;
    glm::vec3 outline_color;
    UserInterface* user_interface;
    NeonEngine* neon_engine;

private:
    Rendering();
    ~Rendering();

    static Rendering* instance;
    static std::mutex rendering_mutex;
};