#pragma once

#include <glm/glm.hpp>

#define IM_VEC4_CLASS_EXTRA \
        constexpr ImVec4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) { } \
        operator glm::vec4() const { return glm::vec4(x, y, z, w); }