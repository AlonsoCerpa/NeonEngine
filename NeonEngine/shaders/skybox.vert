#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 view_projection;

void main() {
    WorldPos = aPos;
    vec4 pos = view_projection * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  