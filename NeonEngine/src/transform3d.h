#pragma once

#include "game_object.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

enum TransformType {
    TRANSLATION,
    ROTATION,
    SCALING
};

class Transform3D {
public:
    TransformType type; // translation, rotation, scaling
    float scale_transform3d;
    std::vector<GameObject*> translation_game_objects;
    std::vector<GameObject*> rotation_game_objects;
    std::vector<GameObject*> scaling_game_objects;

    Transform3D(TransformType type = TRANSLATION, float scale_transform3d = 0.05) {
        this->type = type;
        this->scale_transform3d = scale_transform3d;

        GameObject* x_arrow_body = new GameObject("x_arrow_body", "cylinder", glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
        GameObject* x_arrow_head = new GameObject("x_arrow_head", "cone", glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
        GameObject* y_arrow_body = new GameObject("y_arrow_body", "cylinder", glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* y_arrow_head = new GameObject("y_arrow_head", "cone", glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* z_arrow_body = new GameObject("z_arrow_body", "cylinder", glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
        GameObject* z_arrow_head = new GameObject("z_arrow_head", "cone", glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
    
        translation_game_objects.push_back(x_arrow_body);
        translation_game_objects.push_back(x_arrow_head);
        translation_game_objects.push_back(y_arrow_body);
        translation_game_objects.push_back(y_arrow_head);
        translation_game_objects.push_back(z_arrow_body);
        translation_game_objects.push_back(z_arrow_head);
    }

    ~Transform3D() {
        for (int i = 0; i < translation_game_objects.size(); i++) {
            delete translation_game_objects[i];
        }
        for (int i = 0; i < rotation_game_objects.size(); i++) {
            delete rotation_game_objects[i];
        }
        for (int i = 0; i < scaling_game_objects.size(); i++) {
            delete scaling_game_objects[i];
        }
    }

    void update_model_matrices(Rendering* rendering, GameObject* parent) {
        std::vector<GameObject*>* transformation_game_objects;
        if (type == TRANSLATION) {
            transformation_game_objects = &translation_game_objects;
        }
        else if (type == ROTATION) {
            transformation_game_objects = &rotation_game_objects;
        }
        else { // SCALING
            transformation_game_objects = &scaling_game_objects;
        }
        for (int i = 0; i < transformation_game_objects->size(); i++) {
            GameObject* game_object = (*transformation_game_objects)[i];
            glm::mat4& model = game_object->model;
            glm::vec3& position = game_object->position;
            glm::vec3& rotation = game_object->rotation;
            glm::vec3& scale = game_object->scale;
            glm::mat4& model_inv = game_object->model_inv;
            glm::mat3& model_normals = game_object->model_normals;
            float distance_camera_to_parent_object = glm::length(rendering->camera_viewport->Position - parent->position);

            model = glm::mat4(1.0f);
            model = glm::translate(model, parent->position);
            model = glm::rotate(model, glm::radians(parent->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(parent->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(parent->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            model = glm::translate(model, position * distance_camera_to_parent_object * scale_transform3d);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale * distance_camera_to_parent_object * scale_transform3d);

            model_inv = glm::inverse(model);

            model_normals = glm::mat3(glm::transpose(model_inv));
        }
    }

    void draw(Shader* shader, Rendering* rendering) {
        if (type == TRANSLATION) { // Draw 3D translation axes
            for (int i = 0; i < translation_game_objects.size(); i++) {
                translation_game_objects[i]->draw(shader, rendering, true);
            }
        }
        else if (type == ROTATION) {
            for (int i = 0; i < rotation_game_objects.size(); i++) {
                rotation_game_objects[i]->draw(shader, rendering, true);
            }
        }
        else { // SCALING
            for (int i = 0; i < scaling_game_objects.size(); i++) {
                scaling_game_objects[i]->draw(shader, rendering, true);
            }
        }
    }
};