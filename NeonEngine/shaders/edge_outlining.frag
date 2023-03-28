#version 330 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D screen_texture;

uniform vec2 pixel_size;
uniform vec3 outline_color;

in vec2 TexCoords;

void main() {
    const int WIDTH = 5;
    bool isInside = false;
    int count = 0;
    float coverage = 0.0;
    float dist = 1e6;
    for (int y = -WIDTH;  y <= WIDTH;  ++y) {
        for (int x = -WIDTH;  x <= WIDTH;  ++x) {
            vec2 dUV = vec2(float(x) * pixel_size.x, float(y) * pixel_size.y);
            float mask = texture2D(screen_texture, TexCoords + dUV).r;
            coverage += mask;
            if (mask >= 0.5) {
                dist = min(dist, sqrt(float(x * x + y * y)));
            }
            if (x == 0 && y == 0) {
                isInside = (mask >= 0.5);
            }
            count += 1;
        }
    }
    coverage /= float(count);
    float a;
    if (isInside) {
        a = min(1.0, (1.0 - coverage) / 0.75);
    } else {
        const float solid = 0.3 * float(WIDTH);
        const float fuzzy = float(WIDTH) - solid;
        a = 1.0 - min(1.0, max(0.0, dist - solid) / fuzzy);
    }

    if (a >= 0.001) {
        FragColor = vec4(outline_color, a);
    }
    else {
        discard;
    }
}