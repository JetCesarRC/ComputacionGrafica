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

// ------------------------------- Config ---------------------------------
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*>       meshList;       // mallas con índices (cubo, pirámide)
std::vector<MeshColor*>  meshColorList;  // mallas con color por vértice (letras)
std::vector<Shader>      shaderList;     // shaders cargados

// Rutas de shaders existentes
static const char* vShader = "shaders/shader.vert";         // usa uniforms model+projection
static const char* fShader = "shaders/shader.frag";         // genérico (no se usa en casa)
static const char* vShaderColor = "shaders/shadercolor.vert";    // posición+color por vértice
static const char* fShaderColor = "shaders/shadercolor.frag";

// Nuevos fragment shaders de color fijo (para la casa)
static const char* fRed = "shaders/red.frag";
static const char* fGreen = "shaders/green.frag";
static const char* fBlue = "shaders/blue.frag";
static const char* fBrown = "shaders/brown.frag";      // café: 0.478, 0.255, 0.067
static const char* fDarkGreen = "shaders/darkgreen.frag";  // (0.0, 0.5, 0.0)

// Índices convenientes para shaderList (después de CreateShaders)
#define SH_INDEXED   shaderList[0] // vShader+fShader 
#define SH_COLORVAO  shaderList[1] // vShaderColor+fShaderColor (letras)

#define SH_RED       shaderList[2]
#define SH_GREEN     shaderList[3]
#define SH_BLUE      shaderList[4]
#define SH_BROWN     shaderList[5]
#define SH_DARKGREEN shaderList[6]

// Índices de malla
#define MESH_PYRAMID meshList[0]
#define MESH_CUBE    meshList[1]
#define MESH_PYRAMID_ROOF meshList[2]

// ------------------------------- Geometría ---------------------------------
// Pirámide triangular regular con índices
void CreaPiramide()
{
    // 4 vértices
    GLfloat vertices[] = {
        -0.5f, -0.5f,  0.0f,   // 0
         0.5f, -0.5f,  0.0f,   // 1
         0.0f,  0.5f, -0.25f,  // 2 (apex desplazado en -Z)
         0.0f, -0.5f, -0.5f    // 3
    };
    // 4 triángulos
    unsigned int indices[] = {
        0,1,2,
        1,3,2,
        3,0,2,
        1,0,3
    };
    Mesh* obj = new Mesh();
    obj->CreateMesh(vertices, indices, 12, 12);
    meshList.push_back(obj);
}

// Pirámide de base cuadrada centrada:
// base: y = -0.5, vértices (-0.5..0.5 en X/Z)  |  ápice: y = +0.5
void CreaPiramideBaseRect()
{
    GLfloat vertices[] = {
        // base (y = -0.5)
        -0.5f, -0.5f, -0.5f,   // 0
         0.5f, -0.5f, -0.5f,   // 1
         0.5f, -0.5f,  0.5f,   // 2
        -0.5f, -0.5f,  0.5f,   // 3
        // apex
         0.0f,  0.5f,  0.0f    // 4
    };

    unsigned int indices[] = {
        // base (dos triángulos)
        0, 1, 2,   2, 3, 0,
        // caras laterales
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };

    Mesh* pyr = new Mesh();
    pyr->CreateMesh(vertices, indices, /*numVerts*/ 5 * 3, /*numIndices*/ 18);
    meshList.push_back(pyr);
}

// Cubo con índices (8 vértices, 12 triángulos)
void CrearCubo()
{
    GLfloat cubo_vertices[] = {
        // front
        -0.5f, -0.5f,  0.5f,  // 0
         0.5f, -0.5f,  0.5f,  // 1
         0.5f,  0.5f,  0.5f,  // 2
        -0.5f,  0.5f,  0.5f,  // 3
        // back
        -0.5f, -0.5f, -0.5f,  // 4
         0.5f, -0.5f, -0.5f,  // 5
         0.5f,  0.5f, -0.5f,  // 6
        -0.5f,  0.5f, -0.5f   // 7
    };
    unsigned int cubo_indices[] = {
        // front
        0,1,2,  2,3,0,
        // right
        1,5,6,  6,2,1,
        // back
        7,6,5,  5,4,7,
        // left
        4,0,3,  3,7,4,
        // bottom
        4,5,1,  1,0,4,
        // top
        3,2,6,  6,7,3
    };
    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo);
}

// Letras C-R-E con color por vértice (XYZRGB). Creamos 3 MeshColor (una por letra).
// Cada letra tendrá color sólido asignando el mismo RGB a todos sus vértices.
void CrearLetrasyFiguras_CRE()
{
    // --- C (36 vértices = 12 triángulos) color ROJO ---
    const float Rc = 1.0f, Gc = 0.0f, Bc = 0.0f;
    GLfloat Cverts[] = {
        // x, y, z, R, G, B
        -0.95f,-0.00f,0.0f, Rc,Gc,Bc,  -0.80f,-0.45f,0.0f, Rc,Gc,Bc,  -0.80f, 0.00f,0.0f, Rc,Gc,Bc,
        -0.95f,-0.45f,0.0f, Rc,Gc,Bc,  -0.80f,-0.45f,0.0f, Rc,Gc,Bc,  -0.95f, 0.00f,0.0f, Rc,Gc,Bc,
        -0.95f, 0.45f,0.0f, Rc,Gc,Bc,  -0.80f, 0.45f,0.0f, Rc,Gc,Bc,  -0.80f, 0.00f,0.0f, Rc,Gc,Bc,
        -0.95f, 0.45f,0.0f, Rc,Gc,Bc,  -0.80f, 0.00f,0.0f, Rc,Gc,Bc,  -0.95f, 0.00f,0.0f, Rc,Gc,Bc,

        -0.95f, 0.45f,0.0f, Rc,Gc,Bc,  -0.55f, 0.60f,0.0f, Rc,Gc,Bc,  -0.55f, 0.45f,0.0f, Rc,Gc,Bc,
        -0.95f, 0.45f,0.0f, Rc,Gc,Bc,  -0.55f, 0.60f,0.0f, Rc,Gc,Bc,  -0.95f, 0.60f,0.0f, Rc,Gc,Bc,
        -0.55f, 0.45f,0.0f, Rc,Gc,Bc,  -0.55f, 0.60f,0.0f, Rc,Gc,Bc,  -0.35f, 0.45f,0.0f, Rc,Gc,Bc,
        -0.35f, 0.60f,0.0f, Rc,Gc,Bc,  -0.55f, 0.60f,0.0f, Rc,Gc,Bc,  -0.35f, 0.45f,0.0f, Rc,Gc,Bc,

        -0.95f,-0.60f,0.0f, Rc,Gc,Bc,  -0.55f,-0.45f,0.0f, Rc,Gc,Bc,  -0.55f,-0.60f,0.0f, Rc,Gc,Bc,
        -0.95f,-0.60f,0.0f, Rc,Gc,Bc,  -0.55f,-0.45f,0.0f, Rc,Gc,Bc,  -0.95f,-0.45f,0.0f, Rc,Gc,Bc,
        -0.55f,-0.60f,0.0f, Rc,Gc,Bc,  -0.55f,-0.45f,0.0f, Rc,Gc,Bc,  -0.35f,-0.60f,0.0f, Rc,Gc,Bc,
        -0.55f,-0.45f,0.0f, Rc,Gc,Bc,  -0.35f,-0.60f,0.0f, Rc,Gc,Bc,  -0.35f,-0.45f,0.0f, Rc,Gc,Bc
    };
    MeshColor* C = new MeshColor();
    C->CreateMeshColor(Cverts, 36 * 6);
    meshColorList.push_back(C);

    // --- R (36 vértices) color VERDE ---
    const float Rr = 0.0f, Gr = 1.0f, Br = 0.0f;
    GLfloat Rverts[] = {
        -0.20f,-0.60f,0.0f, Rr,Gr,Br,  -0.05f, 0.05f,0.0f, Rr,Gr,Br,  -0.05f,-0.60f,0.0f, Rr,Gr,Br,
        -0.20f,-0.60f,0.0f, Rr,Gr,Br,  -0.05f, 0.05f,0.0f, Rr,Gr,Br,  -0.20f, 0.05f,0.0f, Rr,Gr,Br,
        -0.20f,-0.05f,0.0f, Rr,Gr,Br,  -0.05f, 0.05f,0.0f, Rr,Gr,Br,  -0.20f, 0.60f,0.0f, Rr,Gr,Br,
        -0.20f, 0.60f,0.0f, Rr,Gr,Br,  -0.05f, 0.60f,0.0f, Rr,Gr,Br,  -0.05f, 0.05f,0.0f, Rr,Gr,Br,

        -0.05f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.60f,0.0f, Rr,Gr,Br,
        -0.05f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.60f,0.0f, Rr,Gr,Br,  -0.05f, 0.60f,0.0f, Rr,Gr,Br,

        -0.05f, 0.05f,0.0f, Rr,Gr,Br,   0.25f, 0.05f,0.0f, Rr,Gr,Br,   0.25f, 0.20f,0.0f, Rr,Gr,Br,
        -0.05f, 0.05f,0.0f, Rr,Gr,Br,   0.25f, 0.20f,0.0f, Rr,Gr,Br,  -0.05f, 0.20f,0.0f, Rr,Gr,Br,

         0.10f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.20f,0.0f, Rr,Gr,Br,
         0.10f, 0.45f,0.0f, Rr,Gr,Br,   0.25f, 0.20f,0.0f, Rr,Gr,Br,   0.10f, 0.20f,0.0f, Rr,Gr,Br,

        -0.05f, 0.05f,0.0f, Rr,Gr,Br,   0.05f, 0.05f,0.0f, Rr,Gr,Br,   0.25f,-0.60f,0.0f, Rr,Gr,Br,
        -0.05f, 0.05f,0.0f, Rr,Gr,Br,   0.25f,-0.60f,0.0f, Rr,Gr,Br,   0.15f,-0.60f,0.0f, Rr,Gr,Br
    };
    MeshColor* R = new MeshColor();
    R->CreateMeshColor(Rverts, 36 * 6);
    meshColorList.push_back(R);

    // --- E (48 vértices) color AZUL ---
    const float Re = 0.0f, Ge = 0.0f, Be = 1.0f;
    GLfloat Everts[] = {
         0.40f, 0.60f,0.0f, Re,Ge,Be,  0.55f,-0.05f,0.0f, Re,Ge,Be,  0.55f, 0.60f,0.0f, Re,Ge,Be,
         0.40f,-0.05f,0.0f, Re,Ge,Be,  0.55f,-0.05f,0.0f, Re,Ge,Be,  0.40f, 0.60f,0.0f, Re,Ge,Be,
         0.40f,-0.05f,0.0f, Re,Ge,Be,  0.40f,-0.60f,0.0f, Re,Ge,Be,  0.55f,-0.05f,0.0f, Re,Ge,Be,
         0.40f,-0.60f,0.0f, Re,Ge,Be,  0.55f,-0.60f,0.0f, Re,Ge,Be,  0.55f,-0.05f,0.0f, Re,Ge,Be,

         0.55f, 0.45f,0.0f, Re,Ge,Be,  0.80f, 0.45f,0.0f, Re,Ge,Be,  0.80f, 0.60f,0.0f, Re,Ge,Be,
         0.55f, 0.45f,0.0f, Re,Ge,Be,  0.80f, 0.60f,0.0f, Re,Ge,Be,  0.55f, 0.60f,0.0f, Re,Ge,Be,
         0.80f, 0.45f,0.0f, Re,Ge,Be,  0.95f, 0.45f,0.0f, Re,Ge,Be,  0.95f, 0.60f,0.0f, Re,Ge,Be,
         0.80f, 0.45f,0.0f, Re,Ge,Be,  0.80f, 0.60f,0.0f, Re,Ge,Be,  0.95f, 0.60f,0.0f, Re,Ge,Be,

         0.75f, 0.10f,0.0f, Re,Ge,Be,  0.75f,-0.05f,0.0f, Re,Ge,Be,  0.95f, 0.10f,0.0f, Re,Ge,Be,
         0.95f,-0.05f,0.0f, Re,Ge,Be,  0.95f, 0.10f,0.0f, Re,Ge,Be,  0.75f,-0.05f,0.0f, Re,Ge,Be,
         0.75f,-0.05f,0.0f, Re,Ge,Be,  0.55f,-0.05f,0.0f, Re,Ge,Be,  0.55f, 0.10f,0.0f, Re,Ge,Be,
         0.75f,-0.05f,0.0f, Re,Ge,Be,  0.75f, 0.10f,0.0f, Re,Ge,Be,  0.55f, 0.10f,0.0f, Re,Ge,Be,

         0.55f,-0.60f,0.0f, Re,Ge,Be,  0.80f,-0.60f,0.0f, Re,Ge,Be,  0.80f,-0.45f,0.0f, Re,Ge,Be,
         0.55f,-0.60f,0.0f, Re,Ge,Be,  0.80f,-0.45f,0.0f, Re,Ge,Be,  0.55f,-0.45f,0.0f, Re,Ge,Be,
         0.95f,-0.45f,0.0f, Re,Ge,Be,  0.95f,-0.60f,0.0f, Re,Ge,Be,  0.80f,-0.45f,0.0f, Re,Ge,Be,
         0.80f,-0.60f,0.0f, Re,Ge,Be,  0.80f,-0.45f,0.0f, Re,Ge,Be,  0.95f,-0.60f,0.0f, Re,Ge,Be
    };
    MeshColor* E = new MeshColor();
    E->CreateMeshColor(Everts, 48 * 6);
    meshColorList.push_back(E);
}

// ------------------------------- Shaders ---------------------------------
void CreateShaders()
{
    // 0: shader índice genérico (no colorea; útil si quisieras debug)
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    // 1: shader con atributo de color (letras C-R-E)
    Shader* shader2 = new Shader();
    shader2->CreateFromFiles(vShaderColor, fShaderColor);
    shaderList.push_back(*shader2);

    // 2..6: shaders de color fijo para la casa
    Shader* shRed = new Shader();      shRed->CreateFromFiles(vShader, fRed);           shaderList.push_back(*shRed);
    Shader* shGreen = new Shader();    shGreen->CreateFromFiles(vShader, fGreen);       shaderList.push_back(*shGreen);
    Shader* shBlue = new Shader();     shBlue->CreateFromFiles(vShader, fBlue);         shaderList.push_back(*shBlue);
    Shader* shBrown = new Shader();    shBrown->CreateFromFiles(vShader, fBrown);       shaderList.push_back(*shBrown);
    Shader* shDarkG = new Shader();    shDarkG->CreateFromFiles(vShader, fDarkGreen);   shaderList.push_back(*shDarkG);
}

// ----------------------------- Helpers de dibujo --------------------------
void DrawMesh(Shader& shader, Mesh* mesh, const glm::mat4& model, const glm::mat4& projection)
{
    shader.useShader();
    GLuint uModel = shader.getModelLocation();
    GLuint uProj = shader.getProjectLocation();
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
    mesh->RenderMesh();
    glUseProgram(0);
}

void DrawMeshColor(Shader& shader, MeshColor* mesh, const glm::mat4& model, const glm::mat4& projection)
{
    shader.useShader();
    GLuint uModel = shader.getModelLocation();
    GLuint uProj = shader.getProjectLocation();
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
    mesh->RenderMeshColor();
    glUseProgram(0);
}

// ------------------------------- Casa 3D -----------------------------------
void DrawHouse(const glm::mat4& projection, float timeSec)
{
    // Base de escena: aleja en -Z y añade una rotación suave para apreciación del volumen
    glm::mat4 base(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, 0.0f, -6.0f));
    base = glm::rotate(base, 0.25f * timeSec, glm::vec3(0.0f, 1.0f, 0.0f)); // giro leve

    // 1) Cuerpo de la casa (cubo ROJO)
    {
        glm::mat4 model = base;
        model = glm::scale(model, glm::vec3(2.0f, 1.2f, 1.2f));
        DrawMesh(SH_RED, MESH_CUBE, model, projection);
    }

    // 2) Techo (pirámide AZUL) con base EXACTA a la del cuerpo
    {
        glm::mat4 model = base;

        const float houseTopY = 1.2f * 0.5f;  // = 0.6
        const float eps = 0.01f;

        const float sX = 2.0f;
        const float sZ = 1.2f;
        const float sY = 0.9f;

        const float transY = houseTopY + eps + 0.5f * sY;

        model = glm::translate(model, glm::vec3(0.0f, transY, 0.0f));
        model = glm::scale(model, glm::vec3(sX, sY, sZ));

        DrawMesh(SH_BLUE, MESH_PYRAMID_ROOF, model, projection);
    }

    // 3) Puerta (cubo CAFÉ)
    {
        glm::mat4 model = base;
        model = glm::translate(model, glm::vec3(0.0f, -0.30f, 0.61f));
        model = glm::scale(model, glm::vec3(0.35f, 0.6f, 0.08f));
        DrawMesh(SH_BROWN, MESH_CUBE, model, projection);
    }

    // 4) Ventanas (cubos VERDES)
    {
        glm::mat4 L = base;
        L = glm::translate(L, glm::vec3(-0.6f, 0.15f, 0.61f));
        L = glm::scale(L, glm::vec3(0.4f, 0.35f, 0.08f));
        DrawMesh(SH_GREEN, MESH_CUBE, L, projection);

        glm::mat4 R = base;
        R = glm::translate(R, glm::vec3(0.6f, 0.15f, 0.61f));
        R = glm::scale(R, glm::vec3(0.4f, 0.35f, 0.08f));
        DrawMesh(SH_GREEN, MESH_CUBE, R, projection);
    }

    // 5) Árbol izquierdo (tronco CAFÉ + follaje VERDE OSCURO)
    {
        // ----- Tronco (elige aquí la posición y escala) -----
        const glm::vec3 trunkPos(-1.7f, -0.10f, -0.60f);
        const glm::vec3 trunkScale(0.25f, 0.70f, 0.25f);

        glm::mat4 trunk = base;
        trunk = glm::translate(trunk, trunkPos);
        trunk = glm::scale(trunk, trunkScale);
        DrawMesh(SH_BROWN, MESH_CUBE, trunk, projection);

        // Altura del tope del tronco en mundo:
        const float trunkTopY = trunkPos.y + 0.5f * trunkScale.y;

        // ----- Follaje -----
        const float eps = 0.01f;
        const float leafSX = 0.90f, leafSY = 1.00f, leafSZ = 0.90f;
        const float zCenterFix = 0.1875f;  // corrige el desfase hacia -Z de la pirámide

        // -0.5*leafSY + transY = trunkTopY + eps  ->  transY = trunkTopY + eps + 0.5*leafSY
        const float leavesY = trunkTopY + eps + 0.5f * leafSY;

        glm::mat4 leaves = base;
        leaves = glm::translate(leaves, glm::vec3(trunkPos.x, leavesY, trunkPos.z));
        leaves = glm::translate(leaves, glm::vec3(0.0f, 0.0f, zCenterFix));
        leaves = glm::scale(leaves, glm::vec3(leafSX, leafSY, leafSZ));
        DrawMesh(SH_DARKGREEN, MESH_PYRAMID, leaves, projection);
    }

    // 6) Árbol derecho (tronco CAFÉ + follaje VERDE OSCURO)
    {
        // ----- Tronco -----
        const glm::vec3 trunkPos(1.7f, -0.10f, -0.60f);
        const glm::vec3 trunkScale(0.25f, 0.70f, 0.25f);

        glm::mat4 trunk = base;
        trunk = glm::translate(trunk, trunkPos);
        trunk = glm::scale(trunk, trunkScale);
        DrawMesh(SH_BROWN, MESH_CUBE, trunk, projection);

        const float trunkTopY = trunkPos.y + 0.5f * trunkScale.y;

        // ----- Follaje -----
        const float eps = 0.01f;
        const float leafSX = 0.90f, leafSY = 1.00f, leafSZ = 0.90f;
        const float zCenterFix = 0.1875f;

        const float leavesY = trunkTopY + eps + 0.5f * leafSY;

        glm::mat4 leaves = base;
        leaves = glm::translate(leaves, glm::vec3(trunkPos.x, leavesY, trunkPos.z));
        leaves = glm::translate(leaves, glm::vec3(0.0f, 0.0f, zCenterFix));
        leaves = glm::scale(leaves, glm::vec3(leafSX, leafSY, leafSZ));
        DrawMesh(SH_DARKGREEN, MESH_PYRAMID, leaves, projection);
    }
}

// ------------------------------- MAIN -------------------------------------
int main()
{
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    // Importante para 3D:
    glEnable(GL_DEPTH_TEST);

    // Crear mallas
    CreaPiramide();           // 0 = pirámide triangular (para árboles)
    CrearCubo();              // 1 = cubo
    CreaPiramideBaseRect();   // 2 = pirámide base cuadrada (para el TECHO)
    CrearLetrasyFiguras_CRE(); // meshColorList[0]=C, [1]=R, [2]=E

    // Cargar shaders
    CreateShaders();

    // Proyección: usa PERSPECTIVA (mejor profundidad que ortho)
    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(),
        0.1f, 100.0f);

    // Bucle
    double t0 = glfwGetTime();
    while (!mainWindow.getShouldClose())
    {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double t = glfwGetTime() - t0;

        // (A) LETRAS
        {
            // velocidad (radianes/seg)
            float ang = static_cast<float>(t) * 1.2f;

            // transform raíz del conjunto: posición en Z y rotación sobre Y
            glm::mat4 root = glm::mat4(1.0f);
            root = glm::translate(root, glm::vec3(0.0f, 0.0f, -4.0f));
            root = glm::rotate(root, ang, glm::vec3(0.0f, 1.0f, 0.0f));

            // C 
            DrawMeshColor(SH_COLORVAO, meshColorList[0], root, projection);

            // R 
            glm::mat4 modelR = glm::translate(root, glm::vec3(0.20f, 0.0f, 0.0f));
            DrawMeshColor(SH_COLORVAO, meshColorList[1], modelR, projection);

            // E 
            glm::mat4 modelE = glm::translate(root, glm::vec3(0.45f, 0.0f, 0.0f));
            DrawMeshColor(SH_COLORVAO, meshColorList[2], modelE, projection);
        }

        // (B) CASA 3D (VISIBLE)
        //DrawHouse(projection, static_cast<float>(t));

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}