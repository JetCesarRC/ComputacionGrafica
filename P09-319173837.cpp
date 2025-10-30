#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include <cstdio>
#include <vector>
#include <cmath>
#include <map>
#include <cstring>
#include <random>
#include <ctime>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "Material.h"

static const float PI = 3.14159265358979323846f;

//=========================== PUERTAS (FSM) =====================================
static glm::vec3 bisagraPuertaIzqLocal(-9.0f, 10.0f, 0.0f);
static glm::vec3 colocacionPuertaIzqLocal(0.0f);
static glm::vec3 colocacionPuertaDerLocal(0.0f);
static glm::vec3 dirDeslizamientoPuerta(10.0f, 0.0f, 0.0f);

static float distanciaPuertaDerAbierta = 1.20f;
static float anguloPuertaIzqAbierta = 90.0f;
static float velocidadPuerta = 1.0f;

static int  teclaTogglePuerta = GLFW_KEY_K;
static bool teclaToggleAnterior = false;

enum class EstadoPuerta { Cerrado, Abriendo, Abierto, Cerrando };
static EstadoPuerta estadoPuerta = EstadoPuerta::Cerrado;
static float tPuerta = 0.0f;

static float easeSmooth(float x) {
    x = glm::clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f);
}
static void ActualizarPuertas(float dt, bool teclaKActual) {
    if (teclaKActual && !teclaToggleAnterior) {
        if (estadoPuerta == EstadoPuerta::Cerrado || estadoPuerta == EstadoPuerta::Cerrando)
            estadoPuerta = EstadoPuerta::Abriendo;
        else
            estadoPuerta = EstadoPuerta::Cerrando;
    }
    teclaToggleAnterior = teclaKActual;

    if (estadoPuerta == EstadoPuerta::Abriendo) {
        tPuerta += velocidadPuerta * dt;
        if (tPuerta >= 1.0f) { tPuerta = 1.0f; estadoPuerta = EstadoPuerta::Abierto; }
    }
    else if (estadoPuerta == EstadoPuerta::Cerrando) {
        tPuerta -= velocidadPuerta * dt;
        if (tPuerta <= 0.0f) { tPuerta = 0.0f; estadoPuerta = EstadoPuerta::Cerrado; }
    }
}

//=========================== LETRERO (marquesina) ==============================
// Se usa un atlas 5x3 y un offset por carácter enviado al FS para seleccionar celda.
static const char* textoLetrero = " PROYECTO CGEIHC ";
static float velocidadLetrero = 2.0f;
static float desplazamientoLetrero = 0.0f;

static int   letreroSlotsVisibles = 10;
static const float letrasCols = 5.0f;
static const float letrasRows = 3.0f;

Mesh* quadLetrero = nullptr;
Texture LetrasTexture;

static glm::vec3 posPanelLetreroEnTorii(0.0f, 4.5f, 3.0f);
static glm::vec3 escalaPanelLetrero(2.5f, 1.0f, 0.2f);
static glm::vec3 rotPanelLetreroEulerDeg(0.20f, 0.0f, 0.0f);

// Letras mirando hacia -Z (coincide con orientación del panel)
static glm::vec3 posTextoLetreroLocal(-8.0f, 34.0f, 3.5f);
static float escalaLetreroChar = 3.2f;
static float espacioEntreChars = 2.0f;

struct LetrasUVCell { float col; float row; };
static std::map<char, LetrasUVCell> mapaLetras = {
    { 'P',{0,0} },{ 'R',{1,0} },{ 'O',{2,0} },{ 'Y',{3,0} },{ 'E',{4,0} },
    { 'C',{0,1} },{ 'T',{1,1} },                { 'G',{3,1} },
    { 'I',{0,2} },{ 'H',{1,2} },                { ' ',{4,2} },
};

//=========================== OBJETOS / RENDER ==================================
Window mainWindow;
std::vector<Mesh*>  meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture pisoTexture;
Texture dado8Texture;

Model ToriiArco_M;
Model PuertaIzquierda_M;
Model PuertaDerecha_M;
Model LetreroPanel_M;

Skybox  skybox;

Material materialBrillante;
Material materialOpaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

DirectionalLight mainLight;
SpotLight        spotLights[MAX_SPOT_LIGHTS];
unsigned int     spotLightCount = 0;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Uniform cache para evitar búsquedas por nombre cada frame
static GLuint U_MODEL = 0, U_PROJ = 0, U_VIEW = 0, U_EYE = 0, U_COLOR = 0, U_OFF = 0, U_SPEC = 0, U_SHINE = 0;

static const glm::vec3 COLOR_BLANCO(1.0f);
static const glm::vec2 OFFSET_CERO(0.0f);

// Piso con normales hacia -Y (consistente con FS y luz direccional)
static void CreateFloor() {
    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        // x      y      z        u     v       nx     ny    nz
        -10.0f, 0.0f, -10.0f,   0.0f,  0.0f,    0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f,  0.0f,    0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,    0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,    0.0f, -1.0f, 0.0f
    };
    Mesh* piso = new Mesh();
    piso->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(piso); // 0
}

// Quad de letra (UV ya orientadas; el FS recibe un offset por carácter)
static void CreateQuadLetrero() {
    float uSize = 1.0f / letrasCols;
    float vSize = 1.0f / letrasRows;

    GLfloat verts[] = {
        // x      y      z        u       v        nx    ny    nz
        -0.5f,  0.5f, 0.0f,    uSize,  vSize,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,    uSize,  0.0f,    0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,    0.0f,   0.0f,    0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,    0.0f,   vSize,   0.0f, 0.0f, 1.0f
    };
    unsigned int inds[] = { 0,1,2, 0,2,3 };

    quadLetrero = new Mesh();
    quadLetrero->CreateMesh(verts, inds, sizeof(verts) / sizeof(GLfloat), sizeof(inds) / sizeof(unsigned int));
}

static void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

//=========================== OCTAEDRO (DADO) ===================================
// Normales planas por cara y U invertida para evitar espejo del atlas.
static Mesh* meshOcta = nullptr;
static void CrearOctaedro() {
    const float N = 0.57735027f;

    GLfloat V[] = {
        // x,y,z,            u,     v,          nx,   ny,   nz
         0.0f,  0.7f,  0.0f,   0.00f, 0.96f,     N,    N,    N,
         0.7f,  0.0f,  0.0f,   0.25f, 0.96f,     N,    N,    N,
         0.0f,  0.0f,  0.7f,   0.125f,0.46f,     N,    N,    N,

         0.0f,  0.7f,  0.0f,   0.24f, 0.97f,    -N,    N,    N,
         0.0f,  0.0f,  0.7f,   0.49f, 0.97f,    -N,    N,    N,
        -0.7f,  0.0f,  0.0f,   0.365f,0.47f,    -N,    N,    N,

         0.0f,  0.7f,  0.0f,   0.50f, 0.98f,    -N,    N,   -N,
        -0.7f,  0.0f,  0.0f,   0.75f, 0.98f,    -N,    N,   -N,
         0.0f,  0.0f, -0.7f,   0.625f,0.48f,    -N,    N,   -N,

         0.0f,  0.7f,  0.0f,   0.74f, 0.98f,     N,    N,   -N,
         0.0f,  0.0f, -0.7f,   0.98f, 0.98f,     N,    N,   -N,
         0.7f,  0.0f,  0.0f,   0.865f,0.48f,     N,    N,   -N,

          0.0f, -0.7f,  0.0f,   0.00f, 0.48f,     N,   -N,    N,
          0.0f,  0.0f,  0.7f,   0.25f, 0.48f,     N,   -N,    N,
          0.7f,  0.0f,  0.0f,   0.125f,-0.02f,    N,   -N,    N,

          0.0f, -0.7f,  0.0f,   0.25f, 0.48f,    -N,   -N,    N,
         -0.7f,  0.0f,  0.0f,   0.50f, 0.48f,    -N,   -N,    N,
          0.0f,  0.0f,  0.7f,   0.375f,-0.02f,   -N,   -N,    N,

          0.0f, -0.7f,  0.0f,   0.50f, 0.50f,    -N,   -N,   -N,
          0.0f,  0.0f, -0.7f,   0.75f, 0.50f,    -N,   -N,   -N,
         -0.7f,  0.0f,  0.0f,   0.625f,0.00f,    -N,   -N,   -N,

          0.0f, -0.7f,  0.0f,   0.75f, 0.48f,     N,   -N,   -N,
          0.7f,  0.0f,  0.0f,   1.00f, 0.48f,     N,   -N,   -N,
          0.0f,  0.0f, -0.7f,   0.875f,-0.02f,    N,   -N,   -N,
    };
    unsigned int I[24]; for (unsigned i = 0; i < 24; ++i) I[i] = i;

    for (int i = 0; i < 24; ++i) V[i * 8 + 3] = 1.0f - V[i * 8 + 3]; // U' = 1 - U

    meshOcta = new Mesh();
    meshOcta->CreateMesh(V, I, 24 * 8, 24);
}

//=========================== DADO (FÍSICA + ORIENTACIÓN) =======================
// Posición inicial editable para ubicar la tirada en el escenario
static const glm::vec3 DADO_POS_INICIAL(0.0f, 5.5f, 40.0f);

enum class RollState { Idle, Falling, Settling };

static glm::vec3 dicePos = DADO_POS_INICIAL;
static glm::quat diceRot(1, 0, 0, 0);
static glm::vec3 diceAngVel(3.0f, 5.0f, 2.5f);
static float     diceScale = 6.0f;

static const float g = -25.0f;
static const float bounce = 0.32f;
static const float floorY = -2.0f;
static const float diceRadius = 0.7f;

static float vy = 0.0f;
static RollState rollState = RollState::Idle;
static int  targetFace = 0;
static glm::quat targetRot = glm::quat(1, 0, 0, 0);
static float settleTime = 0.0f;

static std::mt19937 rng((uint32_t)time(nullptr));
static std::uniform_int_distribution<int> dist8(0, 7);
static glm::quat faceUpRot[8];

static void BuildFaceUpTable() {
    std::vector<glm::vec3> tri; tri.reserve(24);
    auto pushV = [&](float x, float y, float z) { tri.emplace_back(x, y, z); };

    // 8 caras del octaedro (triángulos) para derivar normales por cara
    pushV(0.0f, 0.7f, 0.0f); pushV(0.7f, 0.0f, 0.0f); pushV(0.0f, 0.0f, 0.7f);
    pushV(0.0f, 0.7f, 0.0f); pushV(0.0f, 0.0f, 0.7f); pushV(-0.7f, 0.0f, 0.0f);
    pushV(0.0f, 0.7f, 0.0f); pushV(-0.7f, 0.0f, 0.0f); pushV(0.0f, 0.0f, -0.7f);
    pushV(0.0f, 0.7f, 0.0f); pushV(0.0f, 0.0f, -0.7f); pushV(0.7f, 0.0f, 0.0f);

    pushV(0.0f, -0.7f, 0.0f); pushV(0.0f, 0.0f, 0.7f); pushV(0.7f, 0.0f, 0.0f);
    pushV(0.0f, -0.7f, 0.0f); pushV(-0.7f, 0.0f, 0.0f); pushV(0.0f, 0.0f, 0.7f);
    pushV(0.0f, -0.7f, 0.0f); pushV(0.0f, 0.0f, -0.7f); pushV(-0.7f, 0.0f, 0.0f);
    pushV(0.0f, -0.7f, 0.0f); pushV(0.7f, 0.0f, 0.0f);  pushV(0.0f, 0.0f, -0.7f);

    auto quatFromTo = [](const glm::vec3& a, const glm::vec3& b)->glm::quat {
        glm::vec3 va = glm::normalize(a), vb = glm::normalize(b);
        float d = glm::clamp(glm::dot(va, vb), -1.0f, 1.0f);
        if (d > 0.9999f) return glm::quat(1, 0, 0, 0);
        if (d < -0.9999f) {
            glm::vec3 axis = glm::normalize(glm::abs(va.x) < 0.9f ? glm::cross(va, glm::vec3(1, 0, 0))
                : glm::cross(va, glm::vec3(0, 1, 0)));
            return glm::angleAxis(PI, axis);
        }
        glm::vec3 axis = glm::normalize(glm::cross(va, vb));
        float angle = std::acos(d);
        return glm::angleAxis(angle, axis);
        };

    for (int f = 0; f < 8; ++f) {
        glm::vec3 a = tri[f * 3 + 0], b = tri[f * 3 + 1], c = tri[f * 3 + 2];
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        faceUpRot[f] = quatFromTo(n, glm::vec3(0, 1, 0)); // rotación para dejar esa cara hacia +Y
    }
}

static void RollDice() {
    targetFace = dist8(rng);
    float yaw = ((float)rand() / RAND_MAX) * glm::two_pi<float>();
    glm::quat yawQ = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
    targetRot = yawQ * faceUpRot[targetFace];

    dicePos = DADO_POS_INICIAL;
    vy = 0.0f;
    diceAngVel = glm::vec3(
        3.0f + 2.0f * ((float)rand() / RAND_MAX),
        5.0f + 2.5f * ((float)rand() / RAND_MAX),
        2.0f + 1.5f * ((float)rand() / RAND_MAX)
    );
    diceRot = glm::quat(1, 0, 0, 0);
    rollState = RollState::Falling;
    settleTime = 0.0f;
}

//=========================== MAIN =============================================
int main() {
    mainWindow = Window(1366, 768);
    if (mainWindow.Initialise() != 0) { printf("Error al inicializar la ventana\n"); return 1; }

    glfwSetInputMode(mainWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    CreateFloor();
    CreateShaders();
    CreateQuadLetrero();
    CrearOctaedro();
    BuildFaceUpTable();

    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f,
        20.0f, 1.0f
    );

    pisoTexture = Texture("Textures/piso.tga");            pisoTexture.LoadTextureA();
    LetrasTexture = Texture("Textures/Letras.png");          LetrasTexture.LoadTextureA();
    dado8Texture = Texture("Textures/dado8_numeros.png");   dado8Texture.LoadTextureA();

    ToriiArco_M = Model(); ToriiArco_M.LoadModel("Models/ArcoTorii.obj");
    PuertaIzquierda_M = Model(); PuertaIzquierda_M.LoadModel("Models/PuertaIzquierda.obj");
    PuertaDerecha_M = Model(); PuertaDerecha_M.LoadModel("Models/PuertaDerecha.obj");
    LetreroPanel_M = Model(); LetreroPanel_M.LoadModel("Models/Letrero.obj");

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    materialBrillante = Material(4.0f, 256);
    materialOpaco = Material(0.3f, 4);

    // Luz direccional fija y spotlight ligado a cámara
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.0f, -1.0f, -0.3f);
    spotLightCount = 1;
    spotLights[0] = SpotLight(
        1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        20.0f
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f
    );

    // Cacheo de uniformes y subida de constantes estáticas
    Shader& shader = shaderList[0];
    shader.UseShader();
    U_MODEL = shader.GetModelLocation();
    U_PROJ = shader.GetProjectionLocation();
    U_VIEW = shader.GetViewLocation();
    U_EYE = shader.GetEyePositionLocation();
    U_COLOR = shader.getColorLocation();
    U_OFF = shader.getOffsetLocation();
    U_SPEC = shader.GetSpecularIntensityLocation();
    U_SHINE = shader.GetShininessLocation();

    glUniformMatrix4fv(U_PROJ, 1, GL_FALSE, glm::value_ptr(projection));
    shader.SetDirectionalLight(&mainLight);
    glActiveTexture(GL_TEXTURE0); // todas las texturas en la unidad 0

    // Model matrices constantes
    const glm::mat4 MODEL_PISO = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f)), glm::vec3(30.0f, 1.0f, 30.0f));
    const glm::mat4 MODEL_TORII = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -12.0f));
    const glm::mat4 MODEL_PANEL =
        glm::scale(
            glm::rotate(
                glm::rotate(
                    glm::rotate(
                        glm::translate(MODEL_TORII, posPanelLetreroEnTorii),
                        glm::radians(rotPanelLetreroEulerDeg.x), glm::vec3(1, 0, 0)),
                    glm::radians(rotPanelLetreroEulerDeg.y), glm::vec3(0, 1, 0)),
                glm::radians(rotPanelLetreroEulerDeg.z), glm::vec3(0, 0, 1)),
            escalaPanelLetrero);

    const float REST_Y = floorY + diceRadius * diceScale * 0.90f;

    lastTime = (GLfloat)glfwGetTime();
    bool prevSpace = false;
    RollDice();

    while (!mainWindow.getShouldClose()) {
        GLfloat now = (GLfloat)glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // FSM puertas
        {
            bool kAct = mainWindow.getsKeys()[teclaTogglePuerta];
            ActualizarPuertas(deltaTime, kAct);
        }

        // Scroll continuo del letrero (con wrap)
        desplazamientoLetrero += velocidadLetrero * deltaTime;
        {
            int L = (int)std::strlen(textoLetrero);
            while (desplazamientoLetrero >= (float)L) desplazamientoLetrero -= (float)L;
            while (desplazamientoLetrero < 0.0f)     desplazamientoLetrero += (float)L;
        }

        // Re-roll con espacio sólo en reposo
        bool spaceNow = mainWindow.getsKeys()[GLFW_KEY_SPACE];
        if (spaceNow && !prevSpace && rollState == RollState::Idle) RollDice();
        prevSpace = spaceNow;

        // Física del dado
        if (rollState == RollState::Falling) {
            vy += g * deltaTime;
            dicePos.y += vy * deltaTime;

            diceAngVel *= (1.0f - 0.40f * deltaTime);
            float speed = glm::length(diceAngVel);
            if (speed > 1e-4f) {
                glm::vec3 axis = diceAngVel / speed;
                diceRot = glm::normalize(glm::angleAxis(speed * deltaTime, axis) * diceRot);
            }

            if (dicePos.y <= REST_Y) {
                dicePos.y = REST_Y;
                if (std::fabs(vy) > 2.5f) { vy = -vy * bounce; diceAngVel *= 0.6f; }
                else { vy = 0.0f; rollState = RollState::Settling; settleTime = 0.0f; }
            }
        }
        else if (rollState == RollState::Settling) {
            settleTime += deltaTime;
            float k = 1.0f - std::exp(-6.0f * deltaTime);
            diceRot = glm::normalize(glm::slerp(diceRot, targetRot, k));
            diceAngVel *= (1.0f - 2.5f * deltaTime);
            if (settleTime > 0.6f && glm::length(diceAngVel) < 0.15f) {
                diceRot = targetRot;
                rollState = RollState::Idle;
            }
        }

        //============================ Render ===================================
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMat = camera.calculateViewMatrix();
        skybox.DrawSkybox(viewMat, projection);

        shader.UseShader();

        glUniformMatrix4fv(U_VIEW, 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniform3f(U_EYE,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Spotlight pegado a cámara
        spotLights[0].SetFlash(camera.getCameraPosition(), camera.getCameraDirection());
        shader.SetSpotLights(spotLights, spotLightCount);

        glUniform3fv(U_COLOR, 1, glm::value_ptr(COLOR_BLANCO));
        glUniform2fv(U_OFF, 1, glm::value_ptr(OFFSET_CERO));

        // Opacos
        materialOpaco.UseMaterial(U_SPEC, U_SHINE);

        // Piso
        glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(MODEL_PISO));
        pisoTexture.UseTexture();
        meshList[0]->RenderMesh();

        // Torii
        glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(MODEL_TORII));
        ToriiArco_M.RenderModel();

        // Panel + Letras (panel sin culling; letras sin depth y con alpha)
        GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(MODEL_PANEL));
        LetreroPanel_M.RenderModel();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        LetrasTexture.UseTexture();
        materialBrillante.UseMaterial(U_SPEC, U_SHINE);

        {
            int L = (int)std::strlen(textoLetrero);
            float scroll = desplazamientoLetrero;
            int   base = (int)std::floor(scroll);
            float frac = scroll - (float)base;

            for (int slot = 0; slot < letreroSlotsVisibles; ++slot) {
                int idx = (base + slot) % L;
                if (idx < 0) idx += L;

                char ch = textoLetrero[idx];
                auto it = mapaLetras.find(ch);
                if (it == mapaLetras.end()) it = mapaLetras.find(' ');
                if (it == mapaLetras.end()) continue;

                float uOffsetX = (it->second.col) / letrasCols;
                float vOffsetY = 1.0f - (it->second.row + 1.0f) / letrasRows;
                glm::vec2 uvOff(uOffsetX, vOffsetY);
                glUniform2fv(U_OFF, 1, glm::value_ptr(uvOff));

                float xLocal = (-(float)slot + frac) * espacioEntreChars;

                glm::mat4 modelLetra = MODEL_PANEL;
                modelLetra = glm::translate(modelLetra, posTextoLetreroLocal);
                modelLetra = glm::rotate(modelLetra, glm::radians(180.0f), glm::vec3(0, 1, 0));
                modelLetra = glm::translate(modelLetra, glm::vec3(xLocal, 0.0f, 0.0f));
                modelLetra = glm::scale(modelLetra, glm::vec3(escalaLetreroChar * 0.5f, escalaLetreroChar * 1.5f, 1.0f));

                glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(modelLetra));
                quadLetrero->RenderMesh();
            }
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        if (cullWasEnabled) glEnable(GL_CULL_FACE);
        glUniform2fv(U_OFF, 1, glm::value_ptr(OFFSET_CERO));

        // Puerta izquierda (bisagra)
        {
            float tSuave = easeSmooth(tPuerta);
            float anguloActualIzq = glm::mix(0.0f, anguloPuertaIzqAbierta, tSuave);

            glm::mat4 modeloPuertaIzq = MODEL_TORII
                * glm::translate(glm::mat4(1.0f), colocacionPuertaIzqLocal)
                * glm::translate(glm::mat4(1.0f), bisagraPuertaIzqLocal)
                * glm::rotate(glm::mat4(1.0f), glm::radians(anguloActualIzq), glm::vec3(0, 1, 0))
                * glm::translate(glm::mat4(1.0f), -bisagraPuertaIzqLocal);

            glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(modeloPuertaIzq));
            materialOpaco.UseMaterial(U_SPEC, U_SHINE);
            PuertaIzquierda_M.RenderModel();
        }

        // Puerta derecha (deslizante)
        {
            float tSuave = easeSmooth(tPuerta);
            float desplazamientoDer = glm::mix(0.0f, distanciaPuertaDerAbierta, tSuave);

            glm::mat4 modeloPuertaDer = MODEL_TORII
                * glm::translate(glm::mat4(1.0f), colocacionPuertaDerLocal)
                * glm::translate(glm::mat4(1.0f), dirDeslizamientoPuerta * desplazamientoDer);

            glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(modeloPuertaDer));
            materialOpaco.UseMaterial(U_SPEC, U_SHINE);
            PuertaDerecha_M.RenderModel();
        }

        // Dado
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, dicePos);
            model *= glm::toMat4(diceRot);
            model = glm::scale(model, glm::vec3(diceScale));

            glUniformMatrix4fv(U_MODEL, 1, GL_FALSE, glm::value_ptr(model));
            glUniform2fv(U_OFF, 1, glm::value_ptr(OFFSET_CERO));
            materialBrillante.UseMaterial(U_SPEC, U_SHINE);
            dado8Texture.UseTexture();
            meshOcta->RenderMesh();
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
