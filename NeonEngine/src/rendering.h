#pragma once

#include "bloom.h"

#include <mutex>
#include <vector>
#include <unordered_map>
#include <map>
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
class Cubemap;
class Model;
class Texture;
class Material;

enum CubemapTextureType;

class Rendering {
public:
    static Rendering* get_instance();
    void initialize();

    Rendering(Rendering& other) = delete;
    void operator=(const Rendering&) = delete;

    void set_opengl_state();
    void set_viewport_shaders();
    void add_model_to_loaded_data(Model* model);
    void load_cubemap(const std::string& cubemap_name, const std::vector<std::string>& cube_map_paths, bool is_hdri);
    void set_viewport_data();
    void initialize_game_objects();
    void set_pbr_shader();
    void set_time_before_rendering_loop();
    void render_viewport();
    void setup_framebuffer_and_textures();
    void resize_textures();
    void clean();
    void clean_viewport_framebuffer();
    GameObject* check_mouse_over_models();
    //std::string check_mouse_over_models2();
    GameObject* check_mouse_over_transform3d();

    void print_names_loaded_models();
    void print_names_loaded_materials();
    void print_names_loaded_textures();

    glm::mat4 view, projection;
    glm::mat4 view_projection;
    //glm::mat4 view_projection_inv;
    glm::mat4 view_skybox, view_projection_skybox;
    Camera* camera_viewport;
    Quad* screen_quad;
    float near_camera_viewport, far_camera_viewport;
    float exposure;
    unsigned int framebuffer, textureHDRColorbuffer, texture_id_colors, texture_selected_color_buffer, textureLDRColorbuffer;
    unsigned int textureHDRBrightColorbuffer, texture_id_colors_transform3d, rboDepthStencil;
    unsigned int brdfLUTTexture;
    std::chrono::time_point<std::chrono::system_clock> time_before_rendering;
    Shader* phong_shader;
    Shader* pbr_shader;
    Shader* selection_shader;
    Shader* outline_shader;
    Shader* skybox_shader;
    Shader* equirectangularToCubemapShader;
    Shader* irradianceShader;
    Shader* prefilterShader;
    Shader* brdfShader;
    Shader* bloom_downsample_shader;
    Shader* bloom_upsample_shader;
    Shader* hdr_to_ldr_shader;
    Cubemap* cubemap;
    CubemapTextureType cubemap_texture_type;
    float cubemap_texture_mipmap_level;
    unsigned int bloom_fbo;
    float bloom_filter_radius;
    float bloom_strength;
    bool bloom_activated;
    std::vector<TextureAndSize> bloom_textures;
    std::unordered_map<glm::u8vec3, GameObject*> id_color_to_game_object;
    std::unordered_map<glm::u8vec3, GameObject*> id_color_to_game_object_transform3d;
    std::map<std::string, BaseModel*> loaded_models;
    std::map<std::string, Texture*> loaded_textures;
    std::map<std::string, Material*> loaded_materials;
    std::map<std::string, PointLight*> point_lights;
    std::map<std::string, DirectionalLight*> directional_lights;
    std::map<std::string, SpotLight*> spot_lights;
    std::map<std::string, GameObject*> game_objects;
    Transform3D* transform3d;
    GameObject* last_selected_object;
    GameObject* last_selected_object_transform3d;
    glm::vec3 outline_color;

    // PBR parameters
    int ENVIRONMENT_MAP_WIDTH;
    int ENVIRONMENT_MAP_HEIGHT;
    int IRRADIANCE_MAP_WIDTH;
    int IRRADIANCE_MAP_HEIGHT;
    int PREFILTER_MAP_WIDTH;
    int PREFILTER_MAP_HEIGHT;
    int BRDF_LUT_MAP_WIDTH;
    int BRDF_LUT_MAP_HEIGHT;
    unsigned int captureFBO;
    glm::mat4 captureProjection;
    std::vector<glm::mat4> captureViews;

    UserInterface* user_interface;
    NeonEngine* neon_engine;

private:
    Rendering();
    ~Rendering();

    static Rendering* instance;
    static std::mutex rendering_mutex;
};