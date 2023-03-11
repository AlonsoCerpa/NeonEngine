#pragma once

#include "glm/glm.hpp"

bool ray_triangle_intersection(const glm::vec3& orig, const glm::vec3& dir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);