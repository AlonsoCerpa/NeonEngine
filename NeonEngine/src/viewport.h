#pragma once

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);
void render_to_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer);