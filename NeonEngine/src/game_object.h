#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    GameObject() {}

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

class Light : public GameObject {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light() {}

    Light(int idx_loaded_models, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& axis_rot, float angle, bool is_selected,
          const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
          : GameObject(idx_loaded_models, pos, scale, axis_rot, angle, is_selected) {
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
    }
};

class PointLight : public Light {
public:
    float constant;
    float linear;
    float quadratic;

    PointLight(int idx_loaded_models, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& axis_rot, float angle, bool is_selected,
               const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic)
               : Light(idx_loaded_models, pos, scale, axis_rot, angle, is_selected, ambient, diffuse, specular) {
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(int idx_loaded_models, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& axis_rot, float angle, bool is_selected,
                     const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction)
                     : Light(idx_loaded_models, pos, scale, axis_rot, angle, is_selected, ambient, diffuse, specular) {
        this->direction = direction;
    }
};

class SpotLight : public Light {
public:
    glm::vec3 direction;

    float inner_cut_off;
    float outer_cut_off;

    float constant;
    float linear;
    float quadratic;

    SpotLight(int idx_loaded_models, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& axis_rot, float angle, bool is_selected,
              const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction, float inner_cut_off, float outer_cut_off,
              float constant, float linear, float quadratic)
              : Light(idx_loaded_models, pos, scale, axis_rot, angle, is_selected, ambient, diffuse, specular) {
        this->direction = direction;

        this->inner_cut_off = inner_cut_off;
        this->outer_cut_off = inner_cut_off;

        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};