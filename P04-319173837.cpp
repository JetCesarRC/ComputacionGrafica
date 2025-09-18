// Modelado jerárquico: “animal robot” con 4 patas (2 DOF por pata) y 2 orejas articuladas.
// Controles (incrementos continuos mientras se mantiene la tecla):
//   Pata frontal izquierda (FL):   U/Z = cadera ±     I/X = rodilla ±
//   Pata frontal derecha (FR):     O/C = cadera ±     P/V = rodilla ±
//   Pata trasera  izquierda (BL):  Q/G = cadera ±     Y/H = rodilla ±
//   Pata trasera  derecha (BR):    J/N = cadera ±     K/M = rodilla ±
//   Oreja izquierda:               7/8 = oreja ±
//   Oreja derecha:                 9/0 = oreja ±
// Teclas WASD + mouse (cámara), igual que en la práctica de la grúa.

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

const float PI = 3.14159265f;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(0.18, 16, 16); // esfera unitaria para articulaciones

// ------------------ Geometrías base (mismas que en la grúa) ------------------

void CrearCubo()
{
    unsigned int idx[] = {
        0,1,2, 2,3,0,   1,5,6, 6,2,1,
        7,6,5, 5,4,7,   4,0,3, 3,7,4,
        4,5,1, 1,0,4,   3,2,6, 6,7,3
    };
    GLfloat vtx[] = {
        -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f
    };
    Mesh* m = new Mesh();
    m->CreateMesh(vtx, idx, 24, 36);
    meshList.push_back(m);      // 0
}

void CrearPiramideTriangular()
{
    unsigned int idx[] = { 0,1,2, 1,3,2, 3,0,2, 1,0,3 };
    GLfloat vtx[] = {
        -0.5f,-0.5f, 0.0f,   0.5f,-0.5f, 0.0f,
         0.0f, 0.5f,-0.25f,  0.0f,-0.5f,-0.5f
    };
    Mesh* m = new Mesh();
    m->CreateMesh(vtx, idx, 12, 12);
    meshList.push_back(m);      // 1
}

void CrearCilindro(int res, float R)
{
    int n; GLfloat dt = 2 * PI / res, x, z;
    vector<GLfloat> vertices; vector<unsigned int> indices;

    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
        vertices.push_back(x); vertices.push_back(+0.5f); vertices.push_back(z);
    }
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
    }
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(+0.5f); vertices.push_back(z);
    }
    for (int i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* m = new Mesh();
    m->CreateMeshGeometry(vertices, indices, (unsigned int)vertices.size(), (unsigned int)indices.size());
    meshList.push_back(m);      // 2
}

void CrearCono(int res, float R)
{
    int n; GLfloat dt = 2 * PI / res, x, z;
    vector<GLfloat> vertices; vector<unsigned int> indices;

    vertices.push_back(0.0f); vertices.push_back(0.5f); vertices.push_back(0.0f); // vértice
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
    }
    vertices.push_back(R * cos(0)); vertices.push_back(-0.5f); vertices.push_back(R * sin(0));
    for (int i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* m = new Mesh();
    m->CreateMeshGeometry(vertices, indices, (unsigned int)vertices.size(), (unsigned int)indices.size());
    meshList.push_back(m);      // 3
}

void CrearPiramideCuadrangular()
{
    vector<unsigned int> idx = { 0,3,4, 3,2,4, 2,1,4, 1,0,4, 0,1,2, 0,2,3 };
    vector<GLfloat> vtx = {
         0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,
         0.0f, 0.5f, 0.0f
    };
    Mesh* m = new Mesh();
    m->CreateMeshGeometry(vtx, idx, 15, 18);
    meshList.push_back(m);      // 4
}

void CreateShaders()
{
    Shader* s = new Shader();
    s->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*s);
}

// ------------------ Estado de articulaciones (grados) ------------------
struct RobotJoints {
    // 4 patas × (cadera, rodilla)
    float flHip = 0, flKnee = 20;    // Front-Left
    float frHip = 0, frKnee = 20;    // Front-Right
    float blHip = 0, blKnee = 20;    // Back-Left
    float brHip = 0, brKnee = 20;    // Back-Right
    // orejas
    float earL = 10, earR = 10;
} J;

// Limita ángulos a rangos razonables
static inline float clampf(float v, float lo, float hi) { return fminf(hi, fmaxf(lo, v)); }

// Incrementa las articulaciones leyendo el teclado (keys) a velocidad “degps”
void actualizarArticulaciones(const bool* keys, float degps)
{
    // Hips (caderas): ±35°, Knees (rodillas): 0..85°, Ears: ±30°
    // FL (Front-Left)
    if (keys[GLFW_KEY_U]) J.flHip = clampf(J.flHip + degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_Z]) J.flHip = clampf(J.flHip - degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_I]) J.flKnee = clampf(J.flKnee + degps * deltaTime, 0.f, 85.f);
    if (keys[GLFW_KEY_X]) J.flKnee = clampf(J.flKnee - degps * deltaTime, 0.f, 85.f);

    // FR (Front-Right)
    if (keys[GLFW_KEY_O]) J.frHip = clampf(J.frHip + degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_C]) J.frHip = clampf(J.frHip - degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_P]) J.frKnee = clampf(J.frKnee + degps * deltaTime, 0.f, 85.f);
    if (keys[GLFW_KEY_V]) J.frKnee = clampf(J.frKnee - degps * deltaTime, 0.f, 85.f);

    // BL (Back-Left)  — 
    if (keys[GLFW_KEY_Q]) J.blHip = clampf(J.blHip + degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_G]) J.blHip = clampf(J.blHip - degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_Y]) J.blKnee = clampf(J.blKnee + degps * deltaTime, 0.f, 85.f);
    if (keys[GLFW_KEY_H]) J.blKnee = clampf(J.blKnee - degps * deltaTime, 0.f, 85.f);

    // BR (Back-Right)
    if (keys[GLFW_KEY_J]) J.brHip = clampf(J.brHip + degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_N]) J.brHip = clampf(J.brHip - degps * deltaTime, -35.f, 35.f);
    if (keys[GLFW_KEY_K]) J.brKnee = clampf(J.brKnee + degps * deltaTime, 0.f, 85.f);
    if (keys[GLFW_KEY_M]) J.brKnee = clampf(J.brKnee - degps * deltaTime, 0.f, 85.f);

    // Ears (ahora con 7/8/9/0;  
    if (keys[GLFW_KEY_7]) J.earL = clampf(J.earL + degps * deltaTime, -30.f, 30.f); // Left +
    if (keys[GLFW_KEY_8]) J.earL = clampf(J.earL - degps * deltaTime, -30.f, 30.f); // Left -
    if (keys[GLFW_KEY_9]) J.earR = clampf(J.earR + degps * deltaTime, -30.f, 30.f); // Right +
    if (keys[GLFW_KEY_0]) J.earR = clampf(J.earR - degps * deltaTime, -30.f, 30.f); // Right -
}

// Dibuja una pata de 2 segmentos a partir de un marco “base” (bajo el cuerpo)
// Rotación en Z (plano X–Y): hipDeg (cadera) y kneeDeg (rodilla)
void dibujaPata(glm::mat4 base, GLuint uModel, GLuint uColor,
    float hipDeg, float kneeDeg,
    float largoSuperior, float largoInferior,
    float anchoSup, float anchoInf)
{
    glm::vec3 rojo(1, 0, 0), gris(0.6f), negro(0.1f);

    // Cadera (esfera)
    glUniform3fv(uColor, 1, glm::value_ptr(rojo));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(base));
    sp.render();

    // Segmento superior
    glm::mat4 m = base;
    m = glm::rotate(m, glm::radians(hipDeg), glm::vec3(0, 0, 1));       // rotación de cadera
    m = glm::translate(m, glm::vec3(0.0f, -0.5f * largoSuperior, 0.0f));
    glm::mat4 upperCenter = m;                                        // centro del segmento
    m = glm::scale(m, glm::vec3(anchoSup, largoSuperior, anchoSup));
    glUniform3fv(uColor, 1, glm::value_ptr(gris));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m));
    meshList[0]->RenderMesh(); // cubo

    // Rodilla (esfera) en el extremo inferior del primer segmento
    glm::mat4 knee = upperCenter;
    knee = glm::translate(knee, glm::vec3(0.0f, -0.5f * largoSuperior, 0.0f));
    glUniform3fv(uColor, 1, glm::value_ptr(rojo));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(knee));
    sp.render();

    // Segmento inferior
    m = knee;
    m = glm::rotate(m, glm::radians(kneeDeg), glm::vec3(0, 0, 1));      // rotación de rodilla
    m = glm::translate(m, glm::vec3(0.0f, -0.5f * largoInferior, 0.0f));
    m = glm::scale(m, glm::vec3(anchoInf, largoInferior, anchoInf));
    glUniform3fv(uColor, 1, glm::value_ptr(negro));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m));
    meshList[0]->RenderMesh(); // cubo
}

int main()
{
    // Ventana + OpenGL
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    // Mallas base
    CrearCubo();                 // 0
    CrearPiramideTriangular();   // 1
    CrearCilindro(24, 1.0f);     // 2
    CrearCono(24, 1.0f);         // 3 (orejas)
    CrearPiramideCuadrangular(); // 4
    CreateShaders();

    // Cámara
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 0.3f);

    GLuint uProj = 0, uModel = 0, uView = 0, uColor = 0;
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(),
        0.1f, 100.0f
    );

    // Esfera para articulaciones
    sp.init(); sp.load();

    glm::mat4 model(1.0f), modelaux(1.0f), bodyFrame(1.0f);
    glm::vec3 color(0.0f);

    // Parámetros geométricos del cuerpo y cabeza
    const float bodyL = 4.5f, bodyH = 1.4f, bodyW = 2.0f;  // largo X, alto Y, ancho Z
    const float headL = 1.6f, headH = 1.2f, headW = 1.6f;

    // Patas (longitudes y grosores)
    const float upperLen = 1.6f, lowerLen = 1.6f;
    const float upperW = 0.35f, lowerW = 0.30f;

    while (!mainWindow.getShouldClose())
    {
        // Delta time
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        bool* keys = mainWindow.getsKeys();
        camera.keyControl(keys, deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Actualizar articulaciones desde teclado
        actualizarArticulaciones(keys, 70.0f); // 70°/s

        // Clear
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shader + uniforms
        shaderList[0].useShader();
        uModel = shaderList[0].getModelLocation();
        uProj = shaderList[0].getProjectLocation();
        uView = shaderList[0].getViewLocation();
        uColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // ----------------- Cuerpo (cubo alargado) -----------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.5f, -6.0f));           // ubicar el animal en la escena
        // (Opcional) rotaciones globales si Window las provee (E/R/T)
        model = glm::rotate(model, glm::radians(mainWindow.getrotax()), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(mainWindow.getrotay()), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(mainWindow.getrotaz()), glm::vec3(0, 0, 1));

        bodyFrame = model;                         // marco sin escala para anclar patas/cabeza
        modelaux = model;                         // copia para escalar el cuerpo
        model = glm::scale(model, glm::vec3(bodyL, bodyH, bodyW));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.35f, 0.55f, 0.85f);
        glUniform3fv(uColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // ----------------- Cabeza (cubo) -----------------
        model = bodyFrame;
        model = glm::translate(model, glm::vec3(+0.5f * bodyL + 0.5f * headL, +0.15f * bodyH, 0.0f));
        glm::mat4 headFrame = model;               // marco sin escala de la cabeza
        model = glm::scale(model, glm::vec3(headL, headH, headW));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.7f, 0.2f);
        glUniform3fv(uColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // ----------------- Orejas (conos) -----------------
        auto dibujaOreja = [&](float sideSign, float earDeg) {
            glm::mat4 m = headFrame;
            // base de la oreja en la “azotea” de la cabeza, hacia ±Z
            m = glm::translate(m, glm::vec3(-0.25f * headL, +0.5f * headH, sideSign * (0.35f * headW)));
            // inclinación alrededor de Z (abre/cierra hacia los lados)
            m = glm::rotate(m, glm::radians(earDeg), glm::vec3(0, 0, 1));
            // orientar el cono apuntando hacia +Y
            m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            m = glm::scale(m, glm::vec3(0.3f, 0.3f, 0.8f)); // radio y “alto” de la oreja
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.9f, 0.5f, 0.2f);
            glUniform3fv(uColor, 1, glm::value_ptr(color));
            meshList[3]->RenderMeshGeometry(); // cono
            };
        dibujaOreja(+1.f, J.earR); // derecha
        dibujaOreja(-1.f, J.earL); // izquierda

        // ----------------- Patas: posiciones de caderas -----------------
        // Caderas en la “panza” del cuerpo (y = -0.5*bodyH)
        const float yHip = -0.5f * bodyH;
        const float xFront = +0.5f * bodyL - 0.35f;
        const float xBack = -0.5f * bodyL + 0.35f;
        const float zLeft = +0.5f * bodyW - 0.25f;
        const float zRight = -0.5f * bodyW + 0.25f;

        auto hipFrame = [&](float x, float z) {
            glm::mat4 m = bodyFrame;
            m = glm::translate(m, glm::vec3(x, yHip, z));
            return m;
            };

        // FL (front-left = +x, +z)
        dibujaPata(hipFrame(xFront, zLeft), uModel, uColor, J.flHip, J.flKnee, upperLen, lowerLen, upperW, lowerW);
        // FR (+x, -z)
        dibujaPata(hipFrame(xFront, zRight), uModel, uColor, J.frHip, J.frKnee, upperLen, lowerLen, upperW, lowerW);
        // BL (-x, +z)
        dibujaPata(hipFrame(xBack, zLeft), uModel, uColor, J.blHip, J.blKnee, upperLen, lowerLen, upperW, lowerW);
        // BR (-x, -z)
        dibujaPata(hipFrame(xBack, zRight), uModel, uColor, J.brHip, J.brKnee, upperLen, lowerLen, upperW, lowerW);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}
