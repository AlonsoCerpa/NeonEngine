#version 460 core
layout (location = 0) out vec4 SelectedColor;

uniform int paint_selected_texture;

void main() {
    if (paint_selected_texture == 1) {
        SelectedColor = vec4(1.0);
    }
    else {
        SelectedColor = vec4(0.0);
    }
}