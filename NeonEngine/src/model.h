#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader;
class Rendering;

class Model {
public:
    std::string name;

    virtual void draw(Shader& shader, Rendering* rendering, bool is_selected, bool disable_depth_test) = 0;
    virtual bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) = 0;
};