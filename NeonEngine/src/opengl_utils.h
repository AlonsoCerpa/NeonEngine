#pragma once

#include <string>
#include <vector>

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data);

// renderCube() renders a 2x2x2 3D cube in NDC
void renderCube();

// renderQuad() renders a 2x2 XY quad in 
void renderQuad();

// renders (and builds at first invocation) a sphere
void renderSphere();

// utility function for loading a 2D texture from file
unsigned int load_texture(const std::string& path, int& num_channels);

unsigned int load_hdr_texture(char const* path);
unsigned int load_hdr_file_to_cubemap(const std::vector<std::string>& paths_to_mipmap_files, int base_width, int base_height);
void save_texture_to_png_file(unsigned int texture_id, int num_channels, int width, int height, const std::string& path_to_file);
void save_texture_to_hdr_file(unsigned int texture_id, int num_channels, int width, int height, const std::string& path_to_file);
void save_cubemap_to_hdr_file(unsigned int cubemap_texture_id, int num_channels, const std::vector<int>& widths, const std::vector<int>& heights, const std::vector<std::string>& paths_to_mipmap_files);