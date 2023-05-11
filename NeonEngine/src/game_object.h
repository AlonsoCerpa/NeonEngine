#pragma once

#include "mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <random>
#include <iostream>

class Rendering;
class Shader;
class KeyGenerator;
class ColorGenerator;
class Camera;

enum GameObjectType {
    TypeBaseModel,
    TypePointLight,
    TypeDirectionalLight,
    TypeSpotLight,
    TypeSkybox
};

std::string game_object_type_to_string(GameObjectType type);

class GameObject {
public:
    std::string name;
    std::string model_name;
    GameObjectType type;
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::mat4 model;
    glm::mat4 model_inv;
    glm::mat3 model_normals;
    glm::vec3 albedo;
    float metalness;
    float roughness;
    glm::vec3 emission;
    glm::u8vec3 id_color;
    bool is_selected;
    bool render_only_ambient;
    bool render_one_color;
    int animation_id;
    Material* material;

    static ColorGenerator* color_generator;

    GameObject(const std::string& name, const std::string& model_name);
    ~GameObject();
    void set_model_matrices_standard();
    void draw(Shader* shader, bool disable_depth_test);
    bool intersected_ray(const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t);
    void set_select_state(bool is_game_obj_selected);

    static void clean();
};

class Skybox : public GameObject {
public:
    std::string cubemap_name;

    Skybox(const std::string& name);
};

class Light : public GameObject {
public:
    glm::vec3 light_color;
    float intensity;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light(const std::string& name, const std::string& model_name);
};

class PointLight : public Light {
public:
    float constant;
    float linear;
    float quadratic;

    PointLight(const std::string& name, const std::string& model_name);
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const std::string& name, const std::string& model_name);
};

class SpotLight : public Light {
public:
    glm::vec3 direction;

    float inner_cut_off_angle;
    float outer_cut_off_angle;

    float constant;
    float linear;
    float quadratic;

    SpotLight(const std::string& name, const std::string& model_name);
    float get_inner_cut_off();
    float get_outer_cut_off();
};

class KeyGenerator {
public:
    KeyGenerator(int max_num_keys) {
        this->max_num_keys = max_num_keys;
        this->available_keys = std::vector<int>(max_num_keys);
        for (int i = max_num_keys - 1; i >= 0; i--) {
            this->available_keys[max_num_keys - i - 1] = i;
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


class ColorGenerator {
public:
    ColorGenerator(int max_num_colors) {
        this->max_num_colors = max_num_colors;
        for (int i = 0; i <= 255; i += 3) { // approx. 600 000 maximum possible colors
            for (int j = 0; j <= 255; j += 3) {
                for (int k = 0; k <= 255; k += 3) {
                    available_colors.push_back(glm::u8vec3(i, j, k));
                }
            }
        }
        std::random_device rd;
        //std::mt19937 g(rd());
        std::mt19937 g(0);
        std::shuffle(available_colors.begin(), available_colors.end(), g);
        if (max_num_colors > available_colors.size()) {
            std::cout << "Error: max_num_colors is too big, not enough colors to choose from" << std::endl;
        }
        available_colors.resize(max_num_colors);
    }

    glm::u8vec3 generate_color() {
        if (available_colors.empty()) {
            std::cout << "Error: no available colors!" << std::endl;
            return glm::u8vec3(0);
        }
        glm::u8vec3 color = available_colors.back();
        available_colors.pop_back();
        return color;
    }

    void return_color(const glm::u8vec3& color) {
        available_colors.push_back(color);
    }

private:
    int max_num_colors;
    std::vector<glm::u8vec3> available_colors;
};