#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
flat in int TextureIndex;
in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;
uniform sampler2D atlasTexture;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform float shadowMapSize;
uniform float lightIntensity;

out vec4 FragColor;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const float TWO_PI = 6.2831853;
const int BLOCKER_SAMPLES = 16;
const int PCF_SAMPLES = 16;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;

    float texelSize = 1.0 / shadowMapSize;

    vec3 L = normalize(-lightDir);
    float NdotL = max(dot(normal, L), 0.0);

    float slopeScaleBias = 0.0005;
    float minBias = 0.0001;
    float maxBias = 0.001;
    float bias = clamp(slopeScaleBias * (1.0 - NdotL), minBias, maxBias);

    float currentDepth = projCoords.z;

    float avgBlockerDepth = 0.0;
    int blockers = 0;

    vec2 noise = vec2(rand(projCoords.xy), rand(projCoords.yx));
    float searchRadius = 10.0 * texelSize;

    for(int i = 0; i < BLOCKER_SAMPLES; ++i)
    {
        float angle = TWO_PI * (float(i) / float(BLOCKER_SAMPLES)) + noise.x * TWO_PI;
        float radius = searchRadius * sqrt(float(i) / float(BLOCKER_SAMPLES));
        vec2 offset = vec2(cos(angle), sin(angle)) * radius + noise * texelSize;

        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;

        if(sampleDepth < currentDepth - bias)
        {
            avgBlockerDepth += sampleDepth;
            blockers++;
        }
    }

    if(blockers == 0) return 0.0;

    avgBlockerDepth /= float(blockers);

    float penumbraRatio = (currentDepth - avgBlockerDepth) / avgBlockerDepth;

    float maxBlurRadius = 400.0 * texelSize;
    float filterRadius = clamp(penumbraRatio * maxBlurRadius, texelSize, maxBlurRadius);

    float shadow = 0.0;
    float totalWeight = 0.0;

    float sigma = filterRadius / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;

    for(int i = 0; i < PCF_SAMPLES; ++i)
    {
        float angle = TWO_PI * (float(i) / float(PCF_SAMPLES)) + noise.y * TWO_PI;
        float normRadius = sqrt(float(i) / float(PCF_SAMPLES));
        float radius = filterRadius * normRadius;

        vec2 offset = vec2(cos(angle), sin(angle)) * radius + noise * texelSize;

        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;

        float weight = exp(- (radius * radius) / (twoSigmaSq));

        if(currentDepth - bias > sampleDepth)
            shadow += weight;

        totalWeight += weight;
    }

    shadow /= totalWeight;

    return shadow;
}

void main()
{
    vec4 texColor = texture(atlasTexture, TexCoords);

    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor * lightIntensity;

    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    float shadow = ShadowCalculation(FragPosLightSpace, norm);

    vec3 lighting = ambient + (1.0 - shadow) * diffuse;

    vec3 result = lighting * texColor.rgb;

    FragColor = vec4(result, texColor.a);
}