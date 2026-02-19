#version 330 core

in vec3 FragPosWorld;
in vec3 NormalWorld;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D diffuseTexture;

uniform vec3 cameraPos;

uniform int lightingEnabled;

uniform int uShadingMode;              
uniform int uOverrideColorEnabled;
uniform vec3 uOverrideColor;

uniform int lightMode; 

uniform vec3 lightDir;   
uniform vec3 lightColor;

uniform vec3 pointLightPos;
uniform vec3 pointLightColor;

uniform float attConstant;
uniform float attLinear;
uniform float attQuadratic;

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotInnerCutoff; 
uniform float spotOuterCutoff; 

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

uniform int useShadows;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

uniform int fogEnabled;
uniform int fogMode;      
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;

float ShadowFactor(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 1.0;

    float closest = texture(shadowMap, projCoords.xy).r;
    float current = projCoords.z;

    float bias = max(0.0015 * (1.0 - dot(N, L)), 0.0008);

    return (current - bias > closest) ? 0.0 : 1.0;
}

vec3 ComputeDirectional(vec3 albedo, vec3 N, vec3 V)
{
    vec3 L = normalize(-lightDir);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    float shadow = 1.0;
    if (useShadows == 1) {
        vec4 fragLS = lightSpaceMatrix * vec4(FragPosWorld, 1.0);
        shadow = ShadowFactor(fragLS, N, L);
    }

    vec3 ambient  = ambientStrength * albedo * lightColor;
    vec3 diffuse  = diff * albedo * lightColor;
    vec3 specular = specularStrength * spec * lightColor;

    return ambient + shadow * (diffuse + specular);
}

vec3 ComputePoint(vec3 albedo, vec3 N, vec3 V)
{
    vec3 Ldir = pointLightPos - FragPosWorld;
    float dist = length(Ldir);
    vec3 L = normalize(Ldir);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    float att = 1.0 / (attConstant + attLinear * dist + attQuadratic * dist * dist);

    vec3 ambient  = ambientStrength * albedo * pointLightColor;
    vec3 diffuse  = diff * albedo * pointLightColor;
    vec3 specular = specularStrength * spec * pointLightColor;

    return (ambient + diffuse + specular) * att;
}

vec3 ComputeSpot(vec3 albedo, vec3 N, vec3 V)
{
    vec3 Ldir = spotLightPos - FragPosWorld;
    float dist = length(Ldir);
    vec3 L = normalize(Ldir);
    vec3 R = reflect(-L, N);

    float theta = dot(L, normalize(-spotLightDir));
    float eps = max(spotInnerCutoff - spotOuterCutoff, 0.0001);
    float intensity = clamp((theta - spotOuterCutoff) / eps, 0.0, 1.0);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    float att = 1.0 / (attConstant + attLinear * dist + attQuadratic * dist * dist);

    vec3 ambient  = ambientStrength * albedo * spotLightColor;
    vec3 diffuse  = diff * albedo * spotLightColor;
    vec3 specular = specularStrength * spec * spotLightColor;

    return (ambient + (diffuse + specular) * intensity) * att;
}

vec3 ApplyFog(vec3 col)
{
    if (fogEnabled == 0) return col;

    float dist = length(cameraPos - FragPosWorld);
    float fogT = 1.0;

    if (fogMode == 0) {
        fogT = (fogEnd - dist) / (fogEnd - fogStart);
        fogT = clamp(fogT, 0.0, 1.0);
    } else if (fogMode == 1) {
        fogT = exp(-fogDensity * dist);
        fogT = clamp(fogT, 0.0, 1.0);
    } else {
        float d = fogDensity * dist;
        fogT = exp(-(d * d));
        fogT = clamp(fogT, 0.0, 1.0);
    }

    return mix(fogColor, col, fogT);
}

void main()
{

    if (uOverrideColorEnabled == 1) {
        FragColor = vec4(uOverrideColor, 1.0);
        return;
    }

    vec3 albedo = texture(diffuseTexture, TexCoord).rgb;

    vec3 N = normalize(NormalWorld);

    if (uShadingMode == 1) {
        vec3 dx = dFdx(FragPosWorld);
        vec3 dy = dFdy(FragPosWorld);
        N = normalize(cross(dx, dy));
        if (dot(N, normalize(NormalWorld)) < 0.0) N = -N;
    }

    if (lightingEnabled == 0) {
        vec3 col = ApplyFog(albedo);
        FragColor = vec4(col, 1.0);
        return;
    }

    vec3 V = normalize(cameraPos - FragPosWorld);

    vec3 lit;
    if (lightMode == 0) lit = ComputeDirectional(albedo, N, V);
    else if (lightMode == 1) lit = ComputePoint(albedo, N, V);
    else lit = ComputeSpot(albedo, N, V);

    lit = ApplyFog(lit);

    FragColor = vec4(lit, 1.0);
}
