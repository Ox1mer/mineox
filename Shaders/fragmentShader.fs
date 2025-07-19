#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
flat in int TextureIndex;

uniform sampler2D textures[32];

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(textures[TextureIndex], TexCoords);
    //FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = texColor;
}
