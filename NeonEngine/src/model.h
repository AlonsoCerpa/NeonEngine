#pragma once

#include <glm/glm.hpp>

class Shader;

class Model {
public:
    virtual void draw(Shader& shader, bool is_selected) = 0;
    virtual bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) = 0;
};