#pragma once

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

class GameObject {
public:
    std::string name;
    std::string model_name;
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::mat4 model;
    glm::mat4 model_inv;
    glm::mat3 model_normals;
    glm::vec3 color;
    glm::u8vec3 id_color;
    bool is_selected;
    bool render_only_ambient;
    bool render_one_color;
    int animation_id;

    static ColorGenerator* color_generator;

    GameObject(const std::string& name = "", const std::string& model_name = "", const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::quat& rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f), int animation_id = -1,  bool is_selected = false, bool render_only_ambient = false, bool render_one_color = false);
    ~GameObject();
    void set_model_matrices_standard();
    void draw(Shader* shader, bool disable_depth_test);
    bool intersected_ray(const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t);
    void set_select_state(bool is_game_obj_selected);

    static void clean();
};

class Light : public GameObject {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light() {}

    Light(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& color, int animation_id, bool is_selected, bool render_only_ambient, bool render_one_color,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
};

class PointLight : public Light {
public:
    float constant;
    float linear;
    float quadratic;

    PointLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& color, int animation_id, bool is_selected, bool render_only_ambient, bool render_one_color,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic);
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& color, int animation_id, bool is_selected, bool render_only_ambient, bool render_one_color,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction);
};

class SpotLight : public Light {
public:
    glm::vec3 direction;

    float inner_cut_off;
    float outer_cut_off;

    float constant;
    float linear;
    float quadratic;

    SpotLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& color, int animation_id, bool is_selected, bool render_only_ambient, bool render_one_color,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction, float inner_cut_off, float outer_cut_off,
        float constant, float linear, float quadratic);
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