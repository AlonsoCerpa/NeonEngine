#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform int render_one_color;

void main()
{   
	if (render_one_color == 0) {
		FragColor = texture(texture_diffuse1, TexCoords);
	}
	else {
		FragColor = vec4(0.04, 0.28, 0.26, 1.0);
	}
}