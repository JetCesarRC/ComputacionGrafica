// práctica 3: Pyraminx mínima (colores visibles + fondo blanco)
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

#include <glew.h>
#include <glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Clases del proyecto
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

// Shaders
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert"; // no usado aquí

// ---- Pyraminx (Rubik) ----
int PYR_BASE = -1;              // índice de la base negra
int PYR_FACE_START[4] = { 0 };    // inicio de stickers por cara en meshList
int PYR_COUNT_PER_FACE = 0;     // # de stickers por cara

// -----------------------------------------------------------------------------
// Crear Pyraminx (tetraedro con "stickers")
// L: subdivisiones por lado (3 para clásica)
// S: escala global del tetraedro
// gap: borde negro (encoge el sticker)
// lift: elevación para evitar z-fighting
// -----------------------------------------------------------------------------
void CrearPyraminx(int L = 3, float S = 0.9f, float gap = 0.12f, float lift = 0.020f)
{
    using glm::vec3;

    // Tetraedro regular desde 4 esquinas del cubo
    vec3 v0 = S * vec3(1.0f, 1.0f, 1.0f);
    vec3 v1 = S * vec3(-1.0f, -1.0f, 1.0f);
    vec3 v2 = S * vec3(-1.0f, 1.0f, -1.0f);
    vec3 v3 = S * vec3(1.0f, -1.0f, -1.0f);

    glm::vec3 Tcenter = (v0 + v1 + v2 + v3) / 4.0f; // centro del tetraedro

    // Base negra (plástico)
    GLfloat V[] = {
        v0.x, v0.y, v0.z,
        v1.x, v1.y, v1.z,
        v2.x, v2.y, v2.z,
        v3.x, v3.y, v3.z
    };
    unsigned int I[] = { 0,1,2,  0,3,1,  0,2,3,  1,3,2 };

    Mesh* base = new Mesh();
    base->CreateMesh(V, I, 12, 12);
    meshList.push_back(base);
    PYR_BASE = (int)meshList.size() - 1;

    // Caras del tetraedro
    vec3 F[4][3] = {
        {v0, v1, v2},
        {v0, v3, v1},
        {v0, v2, v3},
        {v1, v3, v2}
    };

    PYR_COUNT_PER_FACE = L * L;      // stickers por cara
    unsigned int triI[] = { 0,1,2 };

    auto addTriangle = [&](const vec3& a, const vec3& b, const vec3& c, const vec3& n)
        {
            vec3 m = (a + b + c) / 3.0f;
            vec3 a2 = m + (a - m) * (1.0f - gap) + n * lift;
            vec3 b2 = m + (b - m) * (1.0f - gap) + n * lift;
            vec3 c2 = m + (c - m) * (1.0f - gap) + n * lift;

            GLfloat triV[] = {
                a2.x, a2.y, a2.z,
                b2.x, b2.y, b2.z,
                c2.x, c2.y, c2.z
            };
            Mesh* t = new Mesh();
            t->CreateMesh(triV, triI, 9, 3);
            meshList.push_back(t);
        };

    // Tesselado por malla baricéntrica
    for (int f = 0; f < 4; ++f)
    {
        PYR_FACE_START[f] = (int)meshList.size();

        vec3 A = F[f][0], B = F[f][1], C = F[f][2];
        vec3 N = glm::normalize(glm::cross(B - A, C - A));

        // Asegura que la normal apunte hacia afuera (lejos del centro del tetraedro)
        glm::vec3 Fc = (A + B + C) / 3.0f;              // centro de la cara
        if (glm::dot(N, Fc - Tcenter) < 0.0f) N = -N;   // invierte si apunta hacia adentro

        for (int i = 0; i < L; ++i)
        {
            for (int j = 0; j < L - i; ++j)
            {
                // Triángulo superior
                vec3 p1 = ((float)i / L) * A + ((float)j / L) * B + ((float)(L - i - j) / L) * C;
                vec3 p2 = ((float)(i + 1) / L) * A + ((float)j / L) * B + ((float)(L - i - j - 1) / L) * C;
                vec3 p3 = ((float)i / L) * A + ((float)(j + 1) / L) * B + ((float)(L - i - j - 1) / L) * C;
                addTriangle(p1, p2, p3, N);

                // Triángulo inferior (si existe)
                if (j < L - i - 1)
                {
                    vec3 q1 = ((float)(i + 1) / L) * A + ((float)j / L) * B + ((float)(L - i - j - 1) / L) * C;
                    vec3 q2 = ((float)(i + 1) / L) * A + ((float)(j + 1) / L) * B + ((float)(L - i - j - 2) / L) * C;
                    vec3 q3 = ((float)i / L) * A + ((float)(j + 1) / L) * B + ((float)(L - i - j - 1) / L) * C;
                    addTriangle(q1, q2, q3, N);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Shaders
// -----------------------------------------------------------------------------
void CreateShaders()
{
    Shader* shader1 = new Shader(); // vShader + fShader (con uniformColor)
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    Shader* shader2 = new Shader(); // vShaderColor (no usado aquí)
    shader2->CreateFromFiles(vShaderColor, fShader);
    shaderList.push_back(*shader2);
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main()
{
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST); // importante para 3D

    CreateShaders();
    CrearPyraminx(3, 0.9f, 0.12f, 0.030f); // levantar stickers para evitar z-fighting

    // Cámara
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 0.3f);

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(),
        0.1f, 100.0f
    );

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Fondo blanco
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        // Transform raíz (llevar a -Z y permitir rotaciones con E/R/T desde Window)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // =========================
        //     DIBUJO PYRAMINX
        // =========================
        const glm::vec3 ROJO(0.85f, 0.10f, 0.10f);
        const glm::vec3 AZUL(0.00f, 0.35f, 1.00f);
        const glm::vec3 VERDE(0.00f, 0.85f, 0.00f);
        const glm::vec3 AMARILLO(0.95f, 0.90f, 0.08f);
        const glm::vec3 NEGRO(0.05f, 0.05f, 0.05f);

        glm::mat4 m = model;
        // Opcional: orientarla para ver varias caras
        // m = glm::rotate(m, glm::radians(-25.0f), glm::vec3(1,0,0));
        // m = glm::rotate(m, glm::radians( 20.0f), glm::vec3(0,1,0));
        m = glm::scale(m, glm::vec3(0.60f));

        // 1) base negra: empujar en el z-buffer para que no pelee con stickers
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.5f, 1.0f);
        
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3fv(uniformColor, 1, glm::value_ptr(NEGRO));
        meshList[PYR_BASE]->RenderMesh();

        glDisable(GL_POLYGON_OFFSET_FILL);

        // 2) stickers por cara
        auto drawFace = [&](int f, const glm::vec3& col)
            {
                glUniform3fv(uniformColor, 1, glm::value_ptr(col));
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                for (int k = 0; k < PYR_COUNT_PER_FACE; ++k)
                    meshList[PYR_FACE_START[f] + k]->RenderMesh();
            };

        // orden de caras: 0,1,2,3
        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        drawFace(0, ROJO);
        drawFace(1, VERDE);
        drawFace(2, AZUL);
        drawFace(3, AMARILLO);

        if (wasCull) glEnable(GL_CULL_FACE);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}