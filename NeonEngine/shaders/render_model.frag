#version 330 core
out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

#define SHININESS 32.0f

struct PointLight {
    vec3 position;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;   

    float constant;
    float linear;
    float quadratic;

    float inner_cut_off;
    float outer_cut_off;    
};

#define MAX_POINT_LIGHTS 50
#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_SPOT_LIGHTS 50

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 model_color;
uniform vec3 viewPos;
uniform int num_point_lights;
uniform int num_directional_lights;
uniform int num_spot_lights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform int render_ambient_one_color;
uniform int render_with_texture;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() { 
	if (render_ambient_one_color == 1) {
        FragColor = vec4(model_color, 1.0);
	}
	else {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);

        vec3 result = vec3(0.0);

        for (int i = 0; i < num_point_lights; i++) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        }
        for (int i = 0; i < num_directional_lights; i++) {
            result += CalcDirectionalLight(directionalLights[i], norm, viewDir);
        }
        for (int i = 0; i < num_spot_lights; i++) {
            result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
        }

		FragColor = vec4(result, 1.0);
	}
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_with_texture == 1) {
        ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    }
    else {
        ambient = light.ambient * model_color;
        diffuse = light.diffuse * diff * model_color;
        specular = light.specular * spec * vec3(0.5);
    }
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a directional light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_with_texture == 1) {
        ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    }
    else {
        ambient = light.ambient * model_color;
        diffuse = light.diffuse * diff * model_color;
        specular = light.specular * spec * vec3(0.5);
    }
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.inner_cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_with_texture == 1) {
        ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    }
    else {
        ambient = light.ambient * model_color;
        diffuse = light.diffuse * diff * model_color;
        specular = light.specular * spec * vec3(0.5);
    }
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}