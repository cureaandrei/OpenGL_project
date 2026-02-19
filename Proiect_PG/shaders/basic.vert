#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 FragPosWorld;
out vec3 NormalWorld;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 worldPos = model * vec4(vPosition, 1.0);
    FragPosWorld = worldPos.xyz;

    mat3 normalMatWorld = mat3(transpose(inverse(model)));
    NormalWorld = normalize(normalMatWorld * vNormal);

    TexCoord = vTexCoords;

    FragPosLightSpace = lightSpaceMatrix * worldPos;

    gl_Position = projection * view * worldPos;
}
