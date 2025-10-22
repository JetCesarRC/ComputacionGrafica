#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <glew.h>

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

class Shader
{
public:
    Shader();

    void CreateFromString(const char* vertexCode, const char* fragmentCode);
    void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);

    std::string ReadFile(const char* fileLocation);

    // Getters tradicionales de locations (mantener compatibilidad)
    GLuint GetProjectionLocation();
    GLuint GetModelLocation();
    GLuint GetViewLocation();
    GLuint GetAmbientIntensityLocation();
    GLuint GetAmbientcolorLocation();
    GLuint GetDiffuseIntensityLocation();
    GLuint GetDirectionLocation();
    GLuint GetSpecularIntensityLocation();
    GLuint GetShininessLocation();
    GLuint GetEyePositionLocation();
    GLuint getColorLocation();

    // Texturizado
    GLuint getTextureLocation();      // sampler2D theTexture
    GLuint getUseTextureLocation();   // int/bool useTexture

    // Setters de luces (ya existentes)
    void SetDirectionalLight(DirectionalLight* dLight);
    void SetPointLights(PointLight* pLight, unsigned int lightCount);
    void SetSpotLights(SpotLight* sLight, unsigned int lightCount);

    // --- NUEVO: Setters de alto nivel para limpiar main() ---
    void SetMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
    void SetEyePosition(const glm::vec3& eye);
    void SetMaterial(float specIntensity, float shininess);
    void SetUseTexture(bool use);
    void SetColor(const glm::vec3& c);

    void UseShader();
    void ClearShader();

    ~Shader();

private:
    void CompileShader(const char* vertexCode, const char* fragmentCode);
    void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);

    int pointLightCount;
    int spotLightCount;

    GLuint shaderID;

    // Uniforms principales
    GLuint uniformProjection;
    GLuint uniformModel;
    GLuint uniformColor;
    GLuint uniformView;
    GLuint uniformEyePosition;
    GLuint uniformSpecularIntensity;
    GLuint uniformShininess;

    // Texturas
    GLuint uniformTexture;     // sampler2D theTexture
    GLuint uniformUseTexture;  // int/bool useTexture

    // Direccional
    struct {
        GLuint uniformcolor;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;
        GLuint uniformDirection;
    } uniformDirectionalLight;

    // Point lights
    GLuint uniformPointLightCount;
    struct {
        GLuint uniformcolor;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;
        GLuint uniformPosition;
        GLuint uniformConstant;
        GLuint uniformLinear;
        GLuint uniformExponent;
    } uniformPointLight[MAX_POINT_LIGHTS];

    // Spot lights
    GLuint uniformSpotLightCount;
    struct {
        GLuint uniformColour;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;
        GLuint uniformPosition;
        GLuint uniformConstant;
        GLuint uniformLinear;
        GLuint uniformExponent;
        GLuint uniformDirection;
        GLuint uniformEdge;
    } uniformSpotLight[MAX_SPOT_LIGHTS];
};