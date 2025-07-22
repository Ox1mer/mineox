#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
flat in int TextureIndex;

uniform sampler2D textures[32];
uniform vec3 lightDir;
uniform vec3 lightColor;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(textures[TextureIndex], TexCoords);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}
