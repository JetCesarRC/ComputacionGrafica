/*
Practica 7: Iluminacion 1
- Elementos: Helicoptero, Agave (quad con alpha), Lampara y Foco
- Luces: Direccional, Spotlight de camara, Spotlight de helicoptero, Punto en el foco
- Controles: Z/X elevan la camara, Flechas mueven el helicoptero mirando a -X
*/

#include <cstdio>
#include <cstring>
#include <vector>
#include <cmath>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

// Iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

// Texturas
Texture pisoTexture;
Texture AgaveTexture;
Texture LamparaTexture;
Texture FocoTexture;

// Modelos
Model Blackhawk_M;
Model Lampara_M;
Model Foco_M;

Skybox skybox;

// Materiales (coeficientes especulares)
Material Material_brillante;  // alto brillo
Material Material_opaco;      // bajo brillo

// Timing
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Luces
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight  spotLights[MAX_SPOT_LIGHTS];

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Transformaciones locales de lampara/foco (espacio de Lampara.obj)
const float       LAMP_SCALE = 1.0f;
const float       LAMP_ROT_Y = 0.0f;
const glm::vec3   BULB_OFFSET_L(0.0f, 0.0f, 0.0f);  // el foco ya esta en el punto correcto
const float       BULB_SCALE = 1.0f;

// Promedio de normales para mallas simples (piso y vegetacion)
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount,
    GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;

        glm::vec3 v1(vertices[in1] - vertices[in0],
            vertices[in1 + 1] - vertices[in0 + 1],
            vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0],
            vertices[in2 + 1] - vertices[in0 + 1],
            vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x;
        vertices[nOffset + 1] = vec.y;
        vertices[nOffset + 2] = vec.z;
    }
}

// Piso y vegetacion (dos quads cruzados para alpha)
void CreateObjects()
{
    unsigned int floorIndices[] = { 0, 2, 1, 1, 2, 3 };
    GLfloat floorVertices[] = {
        // x      y      z       u     v      nx    ny   nz
        -10.0f, 0.0f, -10.0f,   0.0f,  0.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f,  0.0f,  0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,  0.0f, -1.0f, 0.0f
    };

    unsigned int vegetacionIndices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7 };
    GLfloat vegetacionVertices[] = {
        // quad 1
        -0.5f, -0.5f,  0.0f,    0.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.0f,    1.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.0f,    1.0f, 1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.0f,    0.0f, 1.0f,  0.0f, 0.0f, 0.0f,
        // quad 2
         0.0f, -0.5f, -0.5f,    0.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.0f, -0.5f,  0.5f,    1.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.0f,  0.5f,  0.5f,    1.0f, 1.0f,  0.0f, 0.0f, 0.0f,
         0.0f,  0.5f, -0.5f,    0.0f, 1.0f,  0.0f, 0.0f, 0.0f,
    };

    calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

    Mesh* piso = new Mesh();  piso->CreateMesh(floorVertices, floorIndices, 32, 6);               meshList.push_back(piso);   // 0
    Mesh* agave = new Mesh(); agave->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);   meshList.push_back(agave);  // 1
}

// Carga de shader y uniform por defecto del sampler
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    shaderList[0].UseShader();
    glUniform1i(shaderList[0].getTextureLocation(), 0);
    shaderList[0].SetUseTexture(false);
}

int main()
{
    mainWindow = Window(1366, 768);
    if (mainWindow.Initialise() != 0) return -1;

    // Framebuffer en espacio sRGB para tonos y brillos mas naturales
    glEnable(GL_FRAMEBUFFER_SRGB);

    CreateObjects();
    CreateShaders();

    // Camara libre con sensibilidad baja (suave)
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 0.5f);

    // Texturas
    pisoTexture = Texture("Textures/piso.tga");        pisoTexture.LoadTextureA();
    AgaveTexture = Texture("Textures/Agave.tga");       AgaveTexture.LoadTextureA();
    LamparaTexture = Texture("Textures/LamparaTex.tga");  LamparaTexture.LoadTextureA();
    FocoTexture = Texture("Textures/Foco.tga");        FocoTexture.LoadTextureA();

    // Modelos
    Blackhawk_M = Model(); Blackhawk_M.LoadModel("Models/uh60.obj");
    Lampara_M = Model(); Lampara_M.LoadModel("Models/Lampara.obj");
    Foco_M = Model(); Foco_M.LoadModel("Models/Foco.obj");

    // Skybox
    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    // Materiales (Ks e indice especular)
    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // Estado de escena
    float camLift = 0.0f;      const float LIFT_SPEED = 1.0f;

    // Helicoptero: orientado hacia -X (flecha arriba avanza en -X, flecha abajo retrocede)
    glm::vec3 heliPos(0.0f, 5.0f, 6.0f);
    const float HELI_SPEED = 3.0f;
    const float HELI_SCALE = 2.0f;

    // Lampara
    glm::vec3 lamparaPos(-5.0f, 0.0f, -2.0f);

    // Luz direccional (difusa controlada para no sobreexponer)
    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f
    );

    // Luz puntual del foco: se reubica cada frame con la matriz del foco
    unsigned int pointLightCount = 1;
    pointLights[0] = PointLight(
        1.0f, 0.95f, 0.85f,   // color blanco calido
        0.35f, 1.8f,          // intensidades base
        lamparaPos.x, lamparaPos.y, lamparaPos.z, // se actualiza mas abajo
        1.0f, 0.02f, 0.0005f  // atenuacion (alcance medio-alto)
    );

    // Spotlights
    unsigned int spotLightCount = 0;

    // Spotlight "linterna" de la camara
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f
    );
    spotLightCount++;

    // Spotlight del helicoptero: mas intenso, mayor alcance y haz mas amplio
    spotLights[1] = SpotLight(
        1.0f, 1.0f, 0.0f,     // amarillo
        1.2f, 2.8f,           // escala de intensidad
        0.0f, 0.0f, 0.0f,     // posicion (se actualiza cada frame)
        0.0f, -1.0f, 0.0f,    // direccion hacia el piso
        1.0f, 0.005f, 0.0001f,// atenuacion baja = gran radio
        50.0f                 // cono amplio
    );
    spotLightCount++;

    // Uniforms basicos
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0;
    GLuint uniformSpecularIntensity = 0, uniformShininess = 0;
    GLuint uniformColor = 0;

    // Proyeccion en perspectiva
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f);

    while (!mainWindow.getShouldClose())
    {
        // Delta de tiempo para animaciones y movimiento
        GLfloat now = static_cast<GLfloat>(glfwGetTime());
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / static_cast<GLfloat>(limitFPS);
        lastTime = now;

        // Entrada: camara (WASD/mouse) y elevacion Z/X; helicoptero con flechas (Window::applyP7Input)
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        mainWindow.applyP7Input(deltaTime, LIFT_SPEED, HELI_SPEED, camLift, heliPos);

        // Posicion del spotlight del helicoptero (ligeramente por debajo del modelo)
        {
            glm::vec3 spotPos = heliPos + glm::vec3(0.0f, -2.5f, 0.0f);
            glm::vec3 spotDir = glm::vec3(0.0f, -1.0f, 0.0f);
            spotLights[1].SetFlash(spotPos, spotDir);
        }

        // Spotlight de camara anclado a su posicion y direccion
        {
            glm::vec3 lowerLight = camera.getCameraPosition(); lowerLight.y -= 0.3f;
            spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());
        }

        // Limpieza y matrices de vista (con elevacion Z/X aplicada)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view = camera.calculateViewMatrix();
        const glm::mat4 viewLift = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -camLift, 0.0f)) * view;

        // Skybox primero (sin escribir profundidad de geometria)
        skybox.DrawSkybox(viewLift, projection);

        // Shader y uniform comunes
        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewLift));
        glUniform3f(uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Envio de luces al shader
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0f);
        glm::vec3 color(1.0f);

        // Piso
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glActiveTexture(GL_TEXTURE0);
        pisoTexture.UseTexture();
        shaderList[0].SetUseTexture(true);
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();
        shaderList[0].SetUseTexture(false);

        // Lampara (matriz base)
        glm::mat4 Mlamp(1.0f);
        Mlamp = glm::translate(Mlamp, lamparaPos);
        Mlamp = glm::rotate(Mlamp, glm::radians(LAMP_ROT_Y), glm::vec3(0, 1, 0));
        Mlamp = glm::scale(Mlamp, glm::vec3(LAMP_SCALE));

        // Malla de lampara
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(Mlamp));
        glActiveTexture(GL_TEXTURE0);
        LamparaTexture.UseTexture();
        shaderList[0].SetUseTexture(true);
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Lampara_M.RenderModel();
        shaderList[0].SetUseTexture(false);

        // Foco: misma base que Mlamp mas su offset y escala local
        glm::mat4 Mfoco = Mlamp;
        Mfoco = Mfoco * glm::translate(glm::mat4(1.0f), BULB_OFFSET_L);
        Mfoco = Mfoco * glm::scale(glm::mat4(1.0f), glm::vec3(BULB_SCALE));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(Mfoco));
        glActiveTexture(GL_TEXTURE0);
        FocoTexture.UseTexture();
        shaderList[0].SetUseTexture(true);
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Foco_M.RenderModel();
        shaderList[0].SetUseTexture(false);

        // Reubicar la luz puntual en el origen del Foco.obj en mundo (Mfoco * (0,0,0,1))
        {
            glm::vec3 bulbWorldPos = glm::vec3(Mfoco * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            pointLights[0] = PointLight(
                1.0f, 1.0f, 1.0f,    // blanco neutro 
                0.35f, 1.8f,         // intensidades
                bulbWorldPos.x, bulbWorldPos.y, bulbWorldPos.z,
                1.0f, 0.02f, 0.0005f // atenuacion
            );
        }
        shaderList[0].SetPointLights(pointLights, pointLightCount);

        // Vegetacion con alpha (dibujar despues de opacos)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
        AgaveTexture.UseTexture();
        shaderList[0].SetUseTexture(true);
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[1]->RenderMesh();
        shaderList[0].SetUseTexture(false);
        glDisable(GL_BLEND);

        // Helicoptero (orientado a -X)
        model = glm::mat4(1.0f);
        model = glm::translate(model, heliPos);
        model = glm::scale(model, glm::vec3(HELI_SCALE));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        shaderList[0].SetUseTexture(true);
        Blackhawk_M.RenderModel();
        shaderList[0].SetUseTexture(false);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
