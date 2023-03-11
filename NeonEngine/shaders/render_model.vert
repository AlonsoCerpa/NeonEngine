#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int expand_vertices;

void main()
{
    TexCoords = aTexCoords;
    vec3 pos = aPos;
    if (expand_vertices == 1) {
        pos += 0.05 * aNormal;
    }
    gl_Position = projection * view * model * vec4(pos, 1.0);
}