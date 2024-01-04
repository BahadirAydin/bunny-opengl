#version 330 core

in vec2 TexCoords; // Input from the vertex shader

uniform sampler2D text;
uniform vec3 textColor;

out vec4 FragColor; // Output to the framebuffer

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    FragColor = vec4(textColor, 1.0) * sampled;
}

