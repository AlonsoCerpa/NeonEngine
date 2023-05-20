#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdr_texture;
uniform sampler2D bloom_texture;
uniform float exposure;
uniform int bloom_activated;
uniform float bloomStrength;

// Approximated ACES (Academy Color Encoding System) tone mapping by Krzysztof Narkowicz
vec3 aces_approx(vec3 v) {
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}

void main() {
    vec3 hdr_color = texture(hdr_texture, TexCoords).rgb;
    vec3 final_color;
    if (bloom_activated == 1) {
        vec3 bloom_color = texture(bloom_texture, TexCoords).rgb;
        final_color = mix(hdr_color, bloom_color, bloomStrength); // linear interpolation
    }
    else {
        final_color = hdr_color;
    }
    
	// Exposure
    final_color *= exposure;
    // HDR tonemapping
    //color = color / (color + vec3(1.0)); // Reinhard tone mapping
    final_color = aces_approx(final_color); // Approximated ACES tone mapping
    // gamma correct
    final_color = pow(final_color, vec3(1.0/2.2));

    FragColor = vec4(final_color, 1.0);
}