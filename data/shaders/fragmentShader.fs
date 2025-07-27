#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
flat in int TextureIndex;
in vec4 FragPosLightSpace;

uniform sampler2D textures[32];
uniform sampler2D shadowMap;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform float shadowMapSize;
uniform float lightIntensity;

out vec4 FragColor;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{   
    float ndotl = max(dot(normal, -lightDir), 0.0);
    float normalOffsetStrength = 0.001 + 0.003 * (1.0 - ndotl);
    float lightSize = 0.05f;
    vec4 fragPosLS = fragPosLightSpace + vec4(normal * normalOffsetStrength, 0.0);
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    vec3 lightDir = normalize(lightDir);

    float currentDepth = projCoords.z;
    float bias = 0.0015 * tan(acos(ndotl)); 
    bias = clamp(bias, 0.0006, 0.001);
    float texelSize = 1.0 / shadowMapSize;

    float avgBlockerDepth = 0.0;
    int numBlockers = 0;

    int searchSamples = 4;

    for (int x = -searchSamples; x <= searchSamples; ++x)
    {
        for (int y = -searchSamples; y <= searchSamples; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize * 2.0;
            float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;

            if (sampleDepth < currentDepth - bias) {
                avgBlockerDepth += sampleDepth;
                numBlockers++;
            }
        }
    }

    if (numBlockers == 0) return 0.0;

    avgBlockerDepth /= numBlockers;

    float penumbra = (currentDepth - avgBlockerDepth) / avgBlockerDepth;
    float filterRadius = penumbra * lightSize * shadowMapSize / 2.0f;
    filterRadius = max(filterRadius, 1.0);

    vec2 poissonDisk[16] = vec2[](
        vec2(-0.94201624, -0.39906216),
        vec2( 0.94558609, -0.76890725),
        vec2(-0.094184101, -0.92938870),
        vec2( 0.34495938,  0.29387760),
        vec2(-0.91588581,  0.45771432),
        vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543,  0.27676845),
        vec2( 0.97484398,  0.75648379),
        vec2( 0.44323325, -0.97511554),
        vec2( 0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023),
        vec2( 0.79197514,  0.19090188),
        vec2(-0.24188840,  0.99706507),
        vec2(-0.81409955,  0.91437590),
        vec2( 0.19984126,  0.78641367),
        vec2( 0.14383161, -0.14100790)
    );

    float shadow = 0.0;
    float noise = rand(projCoords.xy);

    for (int i = 0; i < 16; i++)
    {
        vec2 randomShift = vec2(
        rand(vec2(i, noise)),
        rand(vec2(noise, i))
        ) * 0.4 - 0.2; // [-0.2, 0.2]
        vec2 offset = (poissonDisk[i] + randomShift) * texelSize * filterRadius;
        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;
        shadow += (currentDepth > sampleDepth + bias) ? 1.0 : 0.0;
    }
    shadow /= 16.0;

    penumbra = max(penumbra, 0.01);
    float penumbraFade = clamp(1.0 - penumbra * 3.0, 0.0, 1.0);
    shadow *= penumbraFade;

    float fadeStart = 0.7;
    float fadeEnd = 1.0;
    float fade = 1.0 - smoothstep(fadeStart, fadeEnd, currentDepth);
    shadow *= fade;

    return shadow;
}

void main()
{
    vec4 texColor = texture(textures[TextureIndex], TexCoords);

    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor * lightIntensity;

    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    float shadow = ShadowCalculation(FragPosLightSpace, norm);

    vec3 lighting = ambient + (1.0 - shadow) * diffuse;

    vec3 result = lighting * texColor.rgb;

    FragColor = vec4(result, texColor.a);
}