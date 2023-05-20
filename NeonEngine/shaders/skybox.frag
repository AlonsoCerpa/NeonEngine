#version 460 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 WorldPos;

uniform samplerCube cubemap;
uniform int is_hdri;
uniform float exposure;
uniform float mipmap_level;

vec3 aces_approx(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}

void main() {
    vec3 envColor = textureLod(cubemap, WorldPos, mipmap_level).rgb;
    
    // Exposure, HDR tone mapping and gamma correct
    /*
    if (is_hdri == 1) {
        envColor *= exposure;
        //envColor = envColor / (envColor + vec3(1.0));
        envColor = aces_approx(envColor);
        envColor = pow(envColor, vec3(1.0/2.2));
    }*/
    
    FragColor = vec4(envColor, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}