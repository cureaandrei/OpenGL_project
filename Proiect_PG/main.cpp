//USER MODE:
// PRESS DIFFERENT KEYS FOR ALL THE VIEW MODES
//1:DIRECTIONAL LIGHT
//2:LIGHT POINT(LOCAL LIGHT SOURCE)
//3:FLASHLIGHT
//4:SOLID VIEW
//5:WIREFRAME VIEW
//6:POLYGONAL VIEW
//7:SMOOTH VIEW
//N:ENABLE/DISABLE LIGHT
//H:ENABLE/DISABLE SHADOWS
//F:ENABLE/DISABLE FOG 
//M:CONTROL THE LEVEL OF FOG EXPOSURE

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <string>

gps::Window myWindow;

glm::mat4 view;
glm::mat4 projection;

int gWinW = 1024, gWinH = 768;
int gFbW = 1024, gFbH = 768;

glm::vec3 lightDir;   
glm::vec3 lightColor;

GLint modelLoc = -1, viewLoc = -1, projectionLoc = -1, normalMatrixLoc = -1;
GLint lightDirLoc = -1, lightColorLoc = -1;

GLint ambientStrengthLoc = -1;
GLint specularStrengthLoc = -1;
GLint shininessLoc = -1;
GLint lightingEnabledLoc = -1;

GLint lightModeLoc = -1;

GLint pointLightPosLoc = -1;
GLint pointLightColorLoc = -1;

GLint spotLightColorLoc = -1;
GLint spotInnerCutoffLoc = -1;
GLint spotOuterCutoffLoc = -1;
GLint spotLightPosLoc = -1;
GLint spotLightDirLoc = -1;

GLint attConstantLoc = -1;
GLint attLinearLoc = -1;
GLint attQuadraticLoc = -1;

GLint shadingModeLoc = -1;            
GLint overrideColorEnabledLoc = -1;   
GLint overrideColorLoc = -1;          

GLint lightSpaceMatrixLoc = -1;   
GLint shadowMapLoc = -1;          
GLint useShadowsLoc = -1;         

GLint cameraPosLoc = -1;          


GLint fogEnabledLoc = -1;   
GLint fogModeLoc = -1;      
GLint fogColorLoc = -1;     
GLint fogDensityLoc = -1;   
GLint fogStartLoc = -1;     
GLint fogEndLoc = -1;       

GLboolean pressedKeys[1024];

float lastFrame = 0.0f;
float deltaTime = 0.0f;

float mouseSensitivity = 0.12f;
float cameraMoveSpeed = 15.0f;
float verticalMoveSpeed = 10.0f;
float fov = 45.0f;

bool firstMouse = true;
float lastX = 512.0f, lastY = 384.0f;

gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 8.0f),
    glm::vec3(0.0f, 2.0f, 7.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);


gps::Model3D ground;   
glm::mat4 groundModel;
gps::Model3D solaire;  
glm::mat4 solaireModel;
gps::Model3D forest;   
glm::mat4 forestModel;
gps::Model3D bonefire; 
glm::mat4 bonefireModel;
glm::mat4 bonefireBaseModel; 
gps::Model3D artorias; 
glm::mat4 artoriasModel;
gps::Model3D gwyn;     
glm::mat4 gwynModel;
gps::Model3D town;     
glm::mat4 townModel;
gps::Model3D castle;   
glm::mat4 castleModel;
gps::Model3D dragon;   
glm::mat4 dragonModel;
gps::Model3D windmill; 
glm::mat4 windmillModel;

gps::Shader myBasicShader;
gps::Shader skyboxShader;

gps::Shader depthShader;
GLint depthModelLoc = -1;
GLint depthLightSpaceLoc = -1;

GLuint skyboxVAO = 0, skyboxVBO = 0;
GLuint cubemapTexture = 0;

bool lightingEnabled = true;
bool useShadows = true;

bool fogEnabled = true;
int fogMode = 2; 
glm::vec3 fogColor = glm::vec3(0.70f, 0.75f, 0.80f);
float fogDensity = 0.02f;
float fogStart = 20.0f;
float fogEnd = 140.0f;

static bool fWasPressed = false;
static bool mWasPressed = false;


enum LightMode { LIGHT_DIR = 0, LIGHT_POINT = 1, LIGHT_SPOT = 2 };
int lightMode = LIGHT_DIR;

glm::vec3 pointLightPos = glm::vec3(0.8f, 1.3f, -0.8f);
glm::vec3 pointLightColor = glm::vec3(2.5f, 1.6f, 0.9f);

glm::vec3 spotLightColor = glm::vec3(2.0f, 2.0f, 2.0f);
float spotInnerCutoff = glm::cos(glm::radians(10.0f));
float spotOuterCutoff = glm::cos(glm::radians(16.0f));

float attConstant = 1.0f;
float attLinear = 0.09f;
float attQuadratic = 0.032f;

float bonfireBaseIntensity = 1.6f;
float bonfireFlickerAmp = 0.6f;
float bonfireFlickerSpeed = 8.0f;

float bonfireJitterAmp = 0.06f;
float bonfireJitterSpeed = 12.0f;


enum ViewMode { VIEW_SOLID = 0, VIEW_WIREFRAME = 1, VIEW_POLYGONAL = 2, VIEW_SMOOTH = 3 };
int viewMode = VIEW_SMOOTH;

int gOverrideColorEnabled = 0;
glm::vec3 gOverrideColor = glm::vec3(0.0f);

static bool v4 = false, v5 = false, v6 = false, v7 = false;

static bool hWasPressed = false;


const GLuint SHADOW_W = 2048;
const GLuint SHADOW_H = 2048;

GLuint shadowFBO = 0;
GLuint shadowDepthTex = 0;

glm::mat4 lightSpaceMatrix(1.0f);


static glm::vec3 extractCameraPosFromView(const glm::mat4& viewMat)
{
    glm::mat4 invV = glm::inverse(viewMat);
    return glm::vec3(invV[3]);
}

static glm::vec3 extractCameraFrontFromView(const glm::mat4& viewMat)
{
    glm::mat4 invV = glm::inverse(viewMat);
    glm::vec3 zAxis = glm::vec3(invV[2]); 
    return glm::normalize(-zAxis);
}

static glm::vec3 extractTranslation(const glm::mat4& m)
{
    return glm::vec3(m[3]);
}

static void syncView()
{
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

static void updateBonfireLight(float t)
{
    glm::vec3 basePos = extractTranslation(bonefireBaseModel);
    basePos.y += 1.2f;

    glm::vec3 jitter(0.0f);
    jitter.x = bonfireJitterAmp * sin(t * bonfireJitterSpeed);
    jitter.z = bonfireJitterAmp * cos(t * bonfireJitterSpeed * 0.9f);

    pointLightPos = basePos + jitter;

    float f1 = 0.5f + 0.5f * sin(t * bonfireFlickerSpeed);
    float f2 = 0.5f + 0.5f * sin(t * bonfireFlickerSpeed * 1.7f + 1.3f);
    float flicker = (f1 * 0.6f + f2 * 0.4f);

    float intensity = bonfireBaseIntensity + bonfireFlickerAmp * flicker;
    pointLightColor = intensity * glm::vec3(1.0f, 0.55f, 0.20f);

    float pulse = 1.0f + 0.03f * flicker;
    bonefireModel = bonefireBaseModel * glm::scale(glm::mat4(1.0f), glm::vec3(pulse));
}

static void drawModelWithTransform(gps::Model3D& mdl, const glm::mat4& modelMat)
{
    myBasicShader.useShaderProgram();

    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projectionLoc != -1) glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    if (normalMatrixLoc != -1) {
        glm::mat3 nm = glm::mat3(glm::transpose(glm::inverse(view * modelMat)));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(nm));
    }

    if (lightDirLoc != -1) glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    if (lightColorLoc != -1) glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    if (lightingEnabledLoc != -1) glUniform1i(lightingEnabledLoc, lightingEnabled ? 1 : 0);
    if (lightModeLoc != -1) glUniform1i(lightModeLoc, lightMode);

    if (pointLightPosLoc != -1) glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
    if (pointLightColorLoc != -1) glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    if (spotLightColorLoc != -1) glUniform3fv(spotLightColorLoc, 1, glm::value_ptr(spotLightColor));
    if (spotInnerCutoffLoc != -1) glUniform1f(spotInnerCutoffLoc, spotInnerCutoff);
    if (spotOuterCutoffLoc != -1) glUniform1f(spotOuterCutoffLoc, spotOuterCutoff);

    if (attConstantLoc != -1) glUniform1f(attConstantLoc, attConstant);
    if (attLinearLoc != -1) glUniform1f(attLinearLoc, attLinear);
    if (attQuadraticLoc != -1) glUniform1f(attQuadraticLoc, attQuadratic);

    if (shadingModeLoc != -1) {
        int shadingMode = 0; 
        if (viewMode == VIEW_SOLID || viewMode == VIEW_POLYGONAL) shadingMode = 1; 
        if (viewMode == VIEW_SMOOTH) shadingMode = 0;
        glUniform1i(shadingModeLoc, shadingMode);
    }

    if (overrideColorEnabledLoc != -1) glUniform1i(overrideColorEnabledLoc, gOverrideColorEnabled);
    if (overrideColorLoc != -1) glUniform3fv(overrideColorLoc, 1, glm::value_ptr(gOverrideColor));

    if (lightSpaceMatrixLoc != -1) glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    if (useShadowsLoc != -1) glUniform1i(useShadowsLoc, useShadows ? 1 : 0);

    if (cameraPosLoc != -1) {
        glm::vec3 camPos = extractCameraPosFromView(view);
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camPos));
    }

    glm::vec3 camPos = extractCameraPosFromView(view);
    glm::vec3 camDir = extractCameraFrontFromView(view);

    if (spotLightPosLoc != -1) glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(camPos));
    if (spotLightDirLoc != -1) glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(camDir));

    
    if (fogEnabledLoc != -1) glUniform1i(fogEnabledLoc, fogEnabled ? 1 : 0);
    if (fogModeLoc != -1) glUniform1i(fogModeLoc, fogMode);
    if (fogColorLoc != -1) glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));
    if (fogDensityLoc != -1) glUniform1f(fogDensityLoc, fogDensity);
    if (fogStartLoc != -1) glUniform1f(fogStartLoc, fogStart);
    if (fogEndLoc != -1) glUniform1f(fogEndLoc, fogEnd);

    mdl.Draw(myBasicShader);
}

static void drawModelDepth(gps::Model3D& mdl, const glm::mat4& modelMat)
{
    depthShader.useShaderProgram();
    if (depthModelLoc != -1) glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    if (depthLightSpaceLoc != -1) glUniformMatrix4fv(depthLightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    mdl.Draw(depthShader);
}

static void drawAllObjects()
{
    drawModelWithTransform(ground, groundModel);
    drawModelWithTransform(solaire, solaireModel);
    drawModelWithTransform(forest, forestModel);

    drawModelWithTransform(bonefire, bonefireModel);

    drawModelWithTransform(artorias, artoriasModel);
    drawModelWithTransform(gwyn, gwynModel);

    drawModelWithTransform(town, townModel);
    drawModelWithTransform(castle, castleModel);
    drawModelWithTransform(dragon, dragonModel);
    drawModelWithTransform(windmill, windmillModel);
}

static void drawAllObjectsDepth()
{
    drawModelDepth(ground, groundModel);
    drawModelDepth(solaire, solaireModel);
    drawModelDepth(forest, forestModel);

    drawModelDepth(bonefire, bonefireModel);

    drawModelDepth(artorias, artoriasModel);
    drawModelDepth(gwyn, gwynModel);

    drawModelDepth(town, townModel);
    drawModelDepth(castle, castleModel);
    drawModelDepth(dragon, dragonModel);
    drawModelDepth(windmill, windmillModel);
}


GLuint loadCubemap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load cubemap face: " << faces[i] << "\n";
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return textureID;
}

void initSkybox()
{
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag");

    std::vector<std::string> faces = {
        "skybox/midnight-silence_rt.tga",
        "skybox/midnight-silence_lf.tga",
        "skybox/midnight-silence_up.tga",
        "skybox/midnight-silence_dn.tga",
        "skybox/midnight-silence_ft.tga",
        "skybox/midnight-silence_bk.tga"
    };

    cubemapTexture = loadCubemap(faces);

    skyboxShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "skybox"), 0);
}

static void drawSkybox()
{
    glDepthFunc(GL_LEQUAL);
    skyboxShader.useShaderProgram();

    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"),
        1, GL_FALSE, glm::value_ptr(viewNoTrans));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void initShadowMap()
{
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Shadow framebuffer not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void updateLightSpaceMatrix()
{
    glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lightPos = sceneCenter - lightDir * 80.0f;

    glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0, 1, 0));

    float orthoSize = 90.0f;
    glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 200.0f);

    lightSpaceMatrix = lightProj * lightView;
}

static void renderShadowPass()
{
    updateLightSpaceMatrix();

    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);

    depthShader.useShaderProgram();
    if (depthLightSpaceLoc != -1)
        glUniformMatrix4fv(depthLightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    drawAllObjectsDepth();

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, gFbW, gFbH);
}



void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    if (height == 0) height = 1;

    gFbW = width;
    gFbH = height;

    glViewport(0, 0, gFbW, gFbH);

    projection = glm::perspective(glm::radians(fov),
        (float)gFbW / (float)gFbH, 0.1f, 300.0f);

    myBasicShader.useShaderProgram();
    if (projectionLoc != -1)
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) pressedKeys[key] = true;
        if (action == GLFW_RELEASE) pressedKeys[key] = false;
    }

    static bool nWasPressed = false;
    if (key == GLFW_KEY_N) {
        if (action == GLFW_PRESS && !nWasPressed) {
            lightingEnabled = !lightingEnabled;
            nWasPressed = true;
            std::cout << (lightingEnabled ? "Lighting: ON\n" : "Lighting: OFF\n");
        }
        if (action == GLFW_RELEASE) nWasPressed = false;
    }

    if (key == GLFW_KEY_H) {
        if (action == GLFW_PRESS && !hWasPressed) {
            useShadows = !useShadows;
            hWasPressed = true;
            std::cout << (useShadows ? "Shadows: ON\n" : "Shadows: OFF\n");
        }
        if (action == GLFW_RELEASE) hWasPressed = false;
    }

    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS && !fWasPressed) {
            fogEnabled = !fogEnabled;
            fWasPressed = true;
            std::cout << (fogEnabled ? "Fog: ON\n" : "Fog: OFF\n");
        }
        if (action == GLFW_RELEASE) fWasPressed = false;
    }

    if (key == GLFW_KEY_M) {
        if (action == GLFW_PRESS && !mWasPressed) {
            fogMode = (fogMode + 1) % 3;
            mWasPressed = true;
            if (fogMode == 0) std::cout << "FogMode: LINEAR\n";
            if (fogMode == 1) std::cout << "FogMode: EXP\n";
            if (fogMode == 2) std::cout << "FogMode: EXP2\n";
        }
        if (action == GLFW_RELEASE) mWasPressed = false;
    }

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) { lightMode = LIGHT_DIR;   std::cout << "LightMode: Directional\n"; }
        if (key == GLFW_KEY_2) { lightMode = LIGHT_POINT; std::cout << "LightMode: Point (Bonfire)\n"; }
        if (key == GLFW_KEY_3) { lightMode = LIGHT_SPOT;  std::cout << "LightMode: Spot\n"; }
    }

    if (key == GLFW_KEY_4) {
        if (action == GLFW_PRESS && !v4) { viewMode = VIEW_SOLID; v4 = true; std::cout << "ViewMode: SOLID (flat/faceted)\n"; }
        if (action == GLFW_RELEASE) v4 = false;
    }
    if (key == GLFW_KEY_5) {
        if (action == GLFW_PRESS && !v5) { viewMode = VIEW_WIREFRAME; v5 = true; std::cout << "ViewMode: WIREFRAME\n"; }
        if (action == GLFW_RELEASE) v5 = false;
    }
    if (key == GLFW_KEY_6) {
        if (action == GLFW_PRESS && !v6) { viewMode = VIEW_POLYGONAL; v6 = true; std::cout << "ViewMode: POLYGONAL (fill + wire overlay)\n"; }
        if (action == GLFW_RELEASE) v6 = false;
    }
    if (key == GLFW_KEY_7) {
        if (action == GLFW_PRESS && !v7) { viewMode = VIEW_SMOOTH; v7 = true; std::cout << "ViewMode: SMOOTH\n"; }
        if (action == GLFW_RELEASE) v7 = false;
    }
}

void mouseCallback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
        return;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    myCamera.rotate(yoffset, xoffset);
    syncView();
}

void scrollCallback(GLFWwindow*, double, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 20.0f) fov = 20.0f;
    if (fov > 80.0f) fov = 80.0f;

    projection = glm::perspective(glm::radians(fov),
        (float)gFbW / (float)gFbH, 0.1f, 300.0f);

    myBasicShader.useShaderProgram();
    if (projectionLoc != -1)
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}



void initOpenGLWindow()
{
    myWindow.Create(gWinW, gWinH, "OpenGL Project");
}

void initOpenGLState()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_FRAMEBUFFER_SRGB);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void initModels()
{
    ground.LoadModel("models/ground/plane.obj");
    groundModel = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f));

    solaire.LoadModel("models/solaire/solaire.obj");
    solaireModel = glm::mat4(1.0f);
    solaireModel = glm::scale(solaireModel, glm::vec3(0.75f));

    forest.LoadModel("models/forest/forest.obj");
    forestModel = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f));

    bonefire.LoadModel("models/bonefire/bonefire.obj");
    bonefireModel = glm::mat4(1.0f);
    bonefireModel = glm::translate(bonefireModel, glm::vec3(0.8f, 0.0f, -0.8f));
    bonefireModel = glm::scale(bonefireModel, glm::vec3(10.0f));
    bonefireBaseModel = bonefireModel;

    artorias.LoadModel("models/artorias/artorias.obj");
    artoriasModel = glm::mat4(1.0f);
    artoriasModel = glm::translate(artoriasModel, glm::vec3(2.2f, 0.0f, -0.6f));
    artoriasModel = glm::scale(artoriasModel, glm::vec3(10.0f));

    gwyn.LoadModel("models/gwyn/gwyn.obj");
    gwynModel = glm::mat4(1.0f);
    gwynModel = glm::translate(gwynModel, glm::vec3(-0.6f, 0.8f, -2.0f));
    gwynModel = glm::scale(gwynModel, glm::vec3(10.0f));

    town.LoadModel("models/town/town.obj");
    townModel = glm::mat4(1.0f);
    townModel = glm::translate(townModel, glm::vec3(1.0f, 0.0f, -8.0f));
    townModel = glm::scale(townModel, glm::vec3(10.0f));

    castle.LoadModel("models/castle/castle.obj");
    castleModel = glm::mat4(1.0f);
    castleModel = glm::translate(castleModel, glm::vec3(-30.0f, 0.0f, -4.0f));
    castleModel = glm::scale(castleModel, glm::vec3(7.0f));

    dragon.LoadModel("models/dragon/dragon.obj");
    dragonModel = glm::mat4(1.0f);
    dragonModel = glm::translate(dragonModel, glm::vec3(-10.0f, 7.0f, 0.0f));
    dragonModel = glm::scale(dragonModel, glm::vec3(10.0f));

    windmill.LoadModel("models/windmill/windmill.obj");
    windmillModel = glm::mat4(1.0f);
    windmillModel = glm::translate(windmillModel, glm::vec3(10.0f, 0.0f, 60.0f));
    windmillModel = glm::scale(windmillModel, glm::vec3(10.0f));
    windmillModel = glm::rotate(windmillModel, glm::radians(-30.0f), glm::vec3(0, 1, 0));
}

void initShaders()
{
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();

    GLint dLoc = glGetUniformLocation(myBasicShader.shaderProgram, "diffuseTexture");
    GLint sLoc = glGetUniformLocation(myBasicShader.shaderProgram, "specularTexture");
    if (dLoc != -1) glUniform1i(dLoc, 0);
    if (sLoc != -1) glUniform1i(sLoc, 1);

    shadowMapLoc = glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap");
    if (shadowMapLoc != -1) glUniform1i(shadowMapLoc, 2);

    depthShader.loadShader("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");
}

void initUniforms()
{
    myBasicShader.useShaderProgram();

    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");

    ambientStrengthLoc = glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength");
    specularStrengthLoc = glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength");
    shininessLoc = glGetUniformLocation(myBasicShader.shaderProgram, "shininess");
    lightingEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightingEnabled");

    lightModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightMode");

    pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
    pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");

    spotLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightColor");
    spotInnerCutoffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotInnerCutoff");
    spotOuterCutoffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotOuterCutoff");

    attConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attConstant");
    attLinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attLinear");
    attQuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attQuadratic");

    shadingModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "uShadingMode");
    overrideColorEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "uOverrideColorEnabled");
    overrideColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "uOverrideColor");

    lightSpaceMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceMatrix");
    useShadowsLoc = glGetUniformLocation(myBasicShader.shaderProgram, "useShadows");

    cameraPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cameraPos");

    fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled");
    fogModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogMode");
    fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    fogStartLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogStart");
    fogEndLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnd");
    spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos");
    spotLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir");

    view = myCamera.getViewMatrix();
    projection = glm::perspective(glm::radians(fov), (float)gFbW / (float)gFbH, 0.1f, 300.0f);

    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projectionLoc != -1) glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightDir = glm::normalize(glm::vec3(0.3f, -1.0f, 0.2f));
    lightColor = glm::vec3(1.0f);

    if (lightDirLoc != -1) glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    if (lightColorLoc != -1) glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    if (ambientStrengthLoc != -1)  glUniform1f(ambientStrengthLoc, 0.20f);
    if (specularStrengthLoc != -1) glUniform1f(specularStrengthLoc, 0.50f);
    if (shininessLoc != -1)        glUniform1f(shininessLoc, 32.0f);

    if (lightingEnabledLoc != -1) glUniform1i(lightingEnabledLoc, lightingEnabled ? 1 : 0);
    if (lightModeLoc != -1) glUniform1i(lightModeLoc, lightMode);

    if (pointLightPosLoc != -1) glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
    if (pointLightColorLoc != -1) glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    if (spotLightColorLoc != -1) glUniform3fv(spotLightColorLoc, 1, glm::value_ptr(spotLightColor));
    if (spotInnerCutoffLoc != -1) glUniform1f(spotInnerCutoffLoc, spotInnerCutoff);
    if (spotOuterCutoffLoc != -1) glUniform1f(spotOuterCutoffLoc, spotOuterCutoff);

    if (attConstantLoc != -1) glUniform1f(attConstantLoc, attConstant);
    if (attLinearLoc != -1) glUniform1f(attLinearLoc, attLinear);
    if (attQuadraticLoc != -1) glUniform1f(attQuadraticLoc, attQuadratic);

    if (shadingModeLoc != -1) glUniform1i(shadingModeLoc, 0);
    if (overrideColorEnabledLoc != -1) glUniform1i(overrideColorEnabledLoc, 0);
    if (overrideColorLoc != -1) glUniform3f(overrideColorLoc, 0.0f, 0.0f, 0.0f);

    if (useShadowsLoc != -1) glUniform1i(useShadowsLoc, useShadows ? 1 : 0);

    if (fogEnabledLoc != -1) glUniform1i(fogEnabledLoc, fogEnabled ? 1 : 0);
    if (fogModeLoc != -1) glUniform1i(fogModeLoc, fogMode);
    if (fogColorLoc != -1) glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));
    if (fogDensityLoc != -1) glUniform1f(fogDensityLoc, fogDensity);
    if (fogStartLoc != -1) glUniform1f(fogStartLoc, fogStart);
    if (fogEndLoc != -1) glUniform1f(fogEndLoc, fogEnd);

    depthShader.useShaderProgram();
    depthModelLoc = glGetUniformLocation(depthShader.shaderProgram, "model");
    depthLightSpaceLoc = glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix");
}


void processMovement()
{
    float speed = cameraMoveSpeed * deltaTime;

    if (pressedKeys[GLFW_KEY_W]) myCamera.move(gps::MOVE_FORWARD, speed);
    if (pressedKeys[GLFW_KEY_S]) myCamera.move(gps::MOVE_BACKWARD, speed);
    if (pressedKeys[GLFW_KEY_A]) myCamera.move(gps::MOVE_LEFT, speed);
    if (pressedKeys[GLFW_KEY_D]) myCamera.move(gps::MOVE_RIGHT, speed);

    if (pressedKeys[GLFW_KEY_SPACE] || pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
        view = myCamera.getViewMatrix();
        glm::vec3 pos = extractCameraPosFromView(view);
        glm::vec3 front = extractCameraFrontFromView(view);

        float vSpeed = verticalMoveSpeed * deltaTime;
        if (pressedKeys[GLFW_KEY_SPACE]) pos.y += vSpeed;
        if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) pos.y -= vSpeed;

        myCamera = gps::Camera(pos, pos + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    syncView();
}



void renderScene()
{
    
    if (useShadows && lightMode == LIGHT_DIR) {
        renderShadowPass();
    }
    else {
        updateLightSpaceMatrix();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

    if (viewMode == VIEW_WIREFRAME) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gOverrideColorEnabled = 0;
        drawAllObjects();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
    else if (viewMode == VIEW_POLYGONAL) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gOverrideColorEnabled = 0;
        drawAllObjects();

        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        gOverrideColorEnabled = 1;
        gOverrideColor = glm::vec3(0.0f, 0.0f, 0.0f);

        glDisable(GL_CULL_FACE);
        drawAllObjects();
        glEnable(GL_CULL_FACE);

        gOverrideColorEnabled = 0;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);

    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gOverrideColorEnabled = 0;
        drawAllObjects();
    }

    drawSkybox();
}



int main()
{
    initOpenGLWindow();

    GLFWwindow* window = myWindow.getWindow();
    glfwGetFramebufferSize(window, &gFbW, &gFbH);
    glViewport(0, 0, gFbW, gFbH);

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    initSkybox();
    initShadowMap();

    glfwSetKeyCallback(window, keyboardCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateBonfireLight(currentFrame);

        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    myWindow.Delete();
    return 0;
}
