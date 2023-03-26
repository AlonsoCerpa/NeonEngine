#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

class Rendering;
class Shader;

class GameObject {
public:
    std::string name;
    std::string model_name;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::mat4 model;
    glm::mat4 model_inv;
    glm::mat3 model_normals;
    glm::vec3 color;
    bool is_selected;

    GameObject(const std::string& name = "", const std::string& model_name = "", const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f), bool is_selected = false);
    ~GameObject();
    void set_model_matrices_standard();
    void set_model_matrices_type1(GameObject* parent);
    void draw(Shader* shader, Rendering* rendering, bool disable_depth_test);
    bool intersected_ray(Rendering* rendering, const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t);
    void set_select_state(bool is_game_obj_selected);
};

class Light : public GameObject {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light() {}

    Light(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
};

class PointLight : public Light {
public:
    float constant;
    float linear;
    float quadratic;

    PointLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic);
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
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

    SpotLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
        const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction, float inner_cut_off, float outer_cut_off,
        float constant, float linear, float quadratic);
};