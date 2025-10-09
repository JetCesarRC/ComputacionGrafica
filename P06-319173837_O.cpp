/*
Práctica 6: Texturizado (versión simplificada solo octaedro y piso)
- Includes portables GL/GLFW/GLM
- glm::perspective con radianes
- deltaTime corregido
- Control de elevación de cámara: Z (subir) / X (bajar)
- Normales corregidas en vegetación
- Normal del piso hacia +Y
*/

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Archivos locales
#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Shader> shaderList;
std::vector<Mesh*> meshList;

Camera camera;

// Velocidades originales de cámara
const float CAM_MOVE_SPEED = 0.3f;
const float CAM_TURN_SPEED = 0.5f;

// --- Elevación vertical con Z / X ---
float camLift = 0.0f;
const float LIFT_SPEED = CAM_MOVE_SPEED;

Texture pisoTexture;
Texture dado8Texture;

Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Vertex/Fragment Shader
static const char* vShader = "shaders/shader_texture.vert";
static const char* fShader = "shaders/shader_texture.frag";

// ------------------------------- Utilidades -----------------------------------
// cálculo del promedio de las normales para sombreado de Phong
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
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

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
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

// ------------------------------- Geometría ------------------------------------
void CreateObjects()
{
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    // Piso con normal hacia +Y
    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f,  0.0f,   0.0f, 1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f,  0.0f,   0.0f, 1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,   0.0f, 1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,   0.0f, 1.0f, 0.0f
    };

    Mesh* piso = new Mesh();
    piso->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(piso);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

// --------------------------- Octaedro -----------------------------------------
void CrearOctaedro()
{
    unsigned int octa_indices[] = {
        0, 1, 2,     3, 4, 5,     6, 7, 8,     9, 10, 11,
        12, 13, 14,  15, 16, 17,  18, 19, 20,  21, 22, 23
    };

    GLfloat octa_vertices[] = {
        // -------- Top (fila 1 -> números 1–4) --------
         0.0f,  0.7f,  0.0f,   0.00f, 0.96f, 0, 0, 0,
         0.7f,  0.0f,  0.0f,   0.25f, 0.96f,   0,0,0,
         0.0f,  0.0f,  0.7f,   0.125f,0.46f,   0,0,0,

          0.0f,  0.7f,  0.0f,   0.24f, 0.97f,   0,0,0,
          0.0f,  0.0f,  0.7f,   0.49f, 0.97f,   0,0,0,
         -0.7f,  0.0f,  0.0f,   0.365f,0.47f,   0,0,0,

          0.0f,  0.7f,  0.0f,   0.50f, 0.98f,   0,0,0,
         -0.7f,  0.0f,  0.0f,   0.75f, 0.98f,   0,0,0,
          0.0f,  0.0f, -0.7f,   0.625f,0.48f,   0,0,0,

           0.0f,  0.7f,  0.0f,   0.74f, 0.98f,   0,0,0,
           0.0f,  0.0f, -0.7f,   0.98f, 0.98f,   0,0,0,
           0.7f,  0.0f,  0.0f,   0.865f,0.48f,   0,0,0,

           // -------- Bottom (fila 0 -> números 5–8) --------
               0.0f, -0.7f,  0.0f,   0.00f, 0.48f,   0,0,0,
               0.0f,  0.0f,  0.7f,   0.25f, 0.48f,   0,0,0,
               0.7f,  0.0f,  0.0f,   0.125f,-0.02f,   0,0,0,

                0.0f, -0.7f,  0.0f,   0.25f, 0.48f,   0,0,0,
               -0.7f,  0.0f,  0.0f,   0.50f, 0.48f,   0,0,0,
                0.0f,  0.0f,  0.7f,   0.375f,-0.02f,   0,0,0,

                 0.0f, -0.7f,  0.0f,   0.50f, 0.50f,   0,0,0,
                 0.0f,  0.0f, -0.7f,   0.75f, 0.50f,   0,0,0,
                -0.7f,  0.0f,  0.0f,   0.625f,0.00f,   0,0,0,

                 0.0f, -0.7f,  0.0f,   0.75f, 0.48f,   0,0,0,
                 0.7f,  0.0f,  0.0f,   1.00f, 0.48f,   0,0,0,
                 0.0f,  0.0f, -0.7f,   0.875f,-0.02f,   0,0,0,
    };

    calcAverageNormals(octa_indices, 24, octa_vertices, 24 * 8, 8, 5);

    Mesh* octa = new Mesh();
    octa->CreateMesh(octa_vertices, octa_indices, 24 * 8, 24);
    meshList.push_back(octa);
}

// ----------------------------------- Main -------------------------------------
int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);

    CreateObjects();
    CrearOctaedro();
    CreateShaders();

    // Cámara
    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f,
        CAM_MOVE_SPEED, CAM_TURN_SPEED
    );

    stbi_set_flip_vertically_on_load(true);

    // Texturas
    pisoTexture = Texture("Textures/piso.tga");       pisoTexture.LoadTextureA();
    dado8Texture = Texture("Textures/dado8_numeros.png"); dado8Texture.LoadTextureA();

    // Skybox
    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
    skybox = Skybox(skyboxFaces);

    // Uniforms
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f
    );

    glm::mat4 model(1.0f);
    glm::vec3 color = glm::vec3(1.0f);

    // --------------------------- Loop principal -------------------------------
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        bool* keys = mainWindow.getsKeys();
        if (keys[GLFW_KEY_Z]) camLift += LIFT_SPEED * deltaTime;
        if (keys[GLFW_KEY_X]) camLift -= LIFT_SPEED * deltaTime;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.calculateViewMatrix();
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -camLift, 0.0f)) * view;

        skybox.DrawSkybox(view, projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));

        // --- Piso ---
        color = glm::vec3(1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        meshList[0]->RenderMesh(); // piso

        // --- Octaedro escalado ---
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.5f, -2.0f));
        model = glm::scale(model, glm::vec3(9.0f));   // escala x9
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f)));
        dado8Texture.UseTexture();
        meshList[1]->RenderMesh(); // octaedro

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
