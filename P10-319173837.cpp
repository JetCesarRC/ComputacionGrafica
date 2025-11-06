/*
Animación por keyframes
La textura del skybox fue conseguida desde la página https ://opengameart.org/content/elyvisions-skyboxes?page=1
y edité en Gimp rotando 90 grados en sentido antihorario la imagen  sp2_up.png para poder ver continuidad.
Fuentes :
    https ://www.khronos.org/opengl/wiki/Keyframe_Animation
    http ://what-when-how.com/wp-content/uploads/2012/07/tmpcd0074_thumb.png
*/
// para cargar imagen

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//#include <assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

// Para archivo
#include <fstream>
#include <sstream>

// para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// === LOGGING / ARCHIVOS ===
static bool LOG = true;
static const char* JAG_FILE = "keyframes_jaguar.txt";

// variables simples
float angulovaria = 0.0f;
float alaAngulo = 0.0f;

// variables legacy no usadas (dejadas en 0 por compatibilidad)
float reproduciranimacion, habilitaranimacion, guardoFrame, reinicioFrame, ciclo, ciclo2, contador = 0;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture ArcoTexture;
Texture PuertaTexture;
Texture lumbreraTexture;
Texture aguilaTexture;
Texture habitat_ajoloteTexture;
Texture nidoTexture;
Texture troncoTexture;
Texture jaguarTexture;
Texture arbolTexture;
Texture piedrasTexture;
Texture ringTexture;

Model Arco_M;
Model Letrero_M;
Model Puerta_Derecha_M;
Model Puerta_Izquierda_M;
Model lumbrera;
Model aguila_M;
Model alaDer_M;
Model alaIzq_M;
Model habitat_ajolote_M;
Model nido_M;
Model tronco_M;
Model Jaguar_Cuerpo_M, Jaguar_TraDer_M, Jaguar_TraIzq_M, Jaguar_DelDer_M, Jaguar_DelIzq_M;
Model arbol_M;
Model piedras_M;
Model ring_M;

Skybox skybox;

// materiales
Material Material_brillante;
Material Material_opaco;

// timing
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// luz
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// teclado keyframes
void inputKeyframes(bool* keys);

// normales (Phong)
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
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

void CreateObjects()
{
    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,  0.0f, 0.0f,  0.0f,-1.0f,0.0f,
         10.0f, 0.0f, -10.0f, 10.0f, 0.0f,  0.0f,-1.0f,0.0f,
        -10.0f, 0.0f,  10.0f,  0.0f,10.0f,  0.0f,-1.0f,0.0f,
         10.0f, 0.0f,  10.0f, 10.0f,10.0f,  0.0f,-1.0f,0.0f
    };

    Mesh* obj3 = new Mesh();
    obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj3);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

/////////////////////////////// KEYFRAMES ///////////////////////

// Número de pasos entre cuadros (interpolación)
struct FRAME {
    float movX;
    float movY;
    float movXInc;
    float movYInc;
    float giro;
    float giroInc;
};

#define MAX_FRAMES 100

// Archivo activo (por defecto: jaguar)
std::string archivoActual = JAG_FILE;

// Águila
FRAME KeyFrameAguila[MAX_FRAMES];
int   FrameIndexAguila = 0;
float movAguila_x = 0, movAguila_y = 0, giroAguila = 0;
int   playIndexAguila = 0, i_curr_stepsAguila = 0;
bool  playAguila = false;

// Jaguar (traslación global del conjunto jaguar)
FRAME KeyFrameJaguar[MAX_FRAMES];
int   FrameIndexJaguar = 0;
float movJag_x = 0.0f, movJag_y = 0.0f, giroJag = 0.0f;
int   playIndexJaguar = 0, i_curr_stepsJaguar = 0;
bool  playJaguar = false;

// Control de teclas
int  reproducirAnimacion = 0;
bool modoEdicion = false;
bool modoRotacion = false;

int  i_max_steps = 100;

// --- UTILIDADES ---
static inline float normalizeDeg(float a) {
    while (a >= 360.0f) a -= 360.0f;
    while (a < 0.0f)  a += 360.0f;
    return a;
}

void resetElements(FRAME* KeyFrame, float& movX, float& movY, float& giro)
{
    movX = KeyFrame[0].movX;
    movY = KeyFrame[0].movY;
    giro = KeyFrame[0].giro;
}

void interpolation(FRAME* KeyFrame, int playIndex, int i_max_steps)
{
    KeyFrame[playIndex].movXInc = (KeyFrame[playIndex + 1].movX - KeyFrame[playIndex].movX) / i_max_steps;
    KeyFrame[playIndex].movYInc = (KeyFrame[playIndex + 1].movY - KeyFrame[playIndex].movY) / i_max_steps;
    KeyFrame[playIndex].giroInc = (KeyFrame[playIndex + 1].giro - KeyFrame[playIndex].giro) / i_max_steps;
}

// --- ANIMAR (con logs por tramo) ---
void animate(FRAME* KeyFrame, int& playIndex, int& FrameIndex, int& i_curr_steps,
    int i_max_steps, bool& play,
    float& movX, float& movY, float& giro)
{
    if (!play) return;

    if (i_curr_steps >= i_max_steps) // fin de interpolación del tramo
    {
        playIndex++;
        if (playIndex > FrameIndex - 2) // fin de animación total
        {
            if (LOG) { printf("[END] Animación completada (%s)\n", archivoActual.c_str()); fflush(stdout); }
            playIndex = 0;
            play = false;
        }
        else
        {
            i_curr_steps = 0;
            interpolation(KeyFrame, playIndex, i_max_steps);
            if (LOG) {
                printf("[PLAY] idx=%d -> idx=%d  steps=%d  from:(%.3f,%.3f,%.3f)  d:(%.3f,%.3f,%.3f)\n",
                    playIndex, playIndex + 1, i_max_steps,
                    KeyFrame[playIndex].movX, KeyFrame[playIndex].movY, KeyFrame[playIndex].giro,
                    KeyFrame[playIndex].movXInc, KeyFrame[playIndex].movYInc, KeyFrame[playIndex].giroInc);
                fflush(stdout);
            }
        }
    }
    else
    {
        movX += KeyFrame[playIndex].movXInc;
        movY += KeyFrame[playIndex].movYInc;
        giro += KeyFrame[playIndex].giroInc;
        i_curr_steps++;
    }
}

/*
============================= Archivo =============================
*/

// --- GUARDAR / AÑADIR ---
void appendKeyframeToFile(const std::string& archivo, const FRAME& frame, int index)
{
    std::ofstream file(archivo, std::ios::app);
    if (!file.is_open()) {
        printf("Error al abrir %s para escribir.\n", archivo.c_str());
        return;
    }
    file << index << " " << frame.movX << " " << frame.movY << " " << frame.giro << "\n";
    file.close();
    if (LOG) {
        printf("[SAVE] %s: idx=%d  movX=%.3f movY=%.3f giro=%.3f\n",
            archivo.c_str(), index, frame.movX, frame.movY, frame.giro);
        fflush(stdout);
    }
}

void saveFrame(const std::string& archivo, FRAME* KeyFrame, int& FrameIndex,
    float movX, float movY, float giro)
{
    if (FrameIndex >= MAX_FRAMES) {
        printf("No hay espacio para más keyframes (MAX_FRAMES=%d)\n", MAX_FRAMES);
        return;
    }
    KeyFrame[FrameIndex].movX = movX;
    KeyFrame[FrameIndex].movY = movY;
    KeyFrame[FrameIndex].giro = normalizeDeg(giro);

    appendKeyframeToFile(archivo, KeyFrame[FrameIndex], FrameIndex);
    FrameIndex++;
    if (LOG) { printf("[SAVE] Nuevo tamaño del arreglo: %d\n", FrameIndex); fflush(stdout); }
}

// --- LEER DESDE ARCHIVO ---
void leerArchivo(const std::string& archivo, FRAME* KeyFrame, int& FrameIndex)
{
    std::ifstream file(archivo);
    if (!file.is_open()) {
        printf("No se encontró %s, se creará al guardar.\n", archivo.c_str());
        FrameIndex = 0;
        return;
    }

    FrameIndex = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        int idx;
        FRAME temp{};
        bool ok = false;

        // Formato 1: idx x y giro
        if (iss >> idx >> temp.movX >> temp.movY >> temp.giro) {
            ok = true;
        }
        else {
            // Formato 2: x y giro (sin idx)
            iss.clear(); iss.str(line);
            if (iss >> temp.movX >> temp.movY >> temp.giro) ok = true;
        }

        if (!ok) continue;
        temp.giro = normalizeDeg(temp.giro);

        if (FrameIndex < MAX_FRAMES) {
            KeyFrame[FrameIndex++] = temp;
            if (LOG) {
                printf("[LOAD] %s: idx=%d  movX=%.3f movY=%.3f giro=%.3f\n",
                    archivo.c_str(), FrameIndex - 1, temp.movX, temp.movY, temp.giro);
            }
        }
        else {
            printf("AVISO: Se alcanzó MAX_FRAMES=%d leyendo %s\n", MAX_FRAMES, archivo.c_str());
            break;
        }
    }
    file.close();
    printf("Se cargaron %d frames desde %s\n", FrameIndex, archivo.c_str());
    fflush(stdout);
}

// --- MODIFICAR ---
void actualizarArchivo(const std::string& archivo, int index, const FRAME& newFrame)
{
    std::ifstream fileIn(archivo);
    if (!fileIn.is_open()) {
        printf("No se puede abrir %s para modificar.\n", archivo.c_str());
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fileIn, line)) lines.push_back(line);
    fileIn.close();

    std::ofstream fileOut(archivo);
    if (!fileOut.is_open()) {
        printf("No se puede escribir %s\n", archivo.c_str());
        return;
    }

    bool found = false;
    for (auto& l : lines) {
        std::istringstream iss(l);
        int idx; float x, y, g;
        if (iss >> idx >> x >> y >> g) {
            if (idx == index) {
                fileOut << index << " " << newFrame.movX << " " << newFrame.movY << " " << newFrame.giro << "\n";
                found = true;
            }
            else fileOut << l << "\n";
        }
        else {
            fileOut << l << "\n";
        }
    }
    fileOut.close();
    if (found) {
        printf("Frame %d modificado correctamente en %s.\n", index, archivo.c_str());
        if (LOG) {
            printf("[EDIT] %s: idx=%d => movX=%.3f movY=%.3f giro=%.3f\n",
                archivo.c_str(), index, newFrame.movX, newFrame.movY, newFrame.giro);
        }
    }
    else {
        printf("Frame %d no encontrado en %s.\n", index, archivo.c_str());
    }
    fflush(stdout);
}

// --- BORRAR ---
void borrarFrame(const std::string& archivo, int index)
{
    std::ifstream fileIn(archivo);
    if (!fileIn.is_open()) {
        printf("No se puede abrir %s para borrar.\n", archivo.c_str());
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fileIn, line)) lines.push_back(line);
    fileIn.close();

    std::ofstream fileOut(archivo);
    if (!fileOut.is_open()) {
        printf("No se puede escribir %s\n", archivo.c_str());
        return;
    }

    bool found = false;
    for (auto& l : lines) {
        std::istringstream iss(l);
        int idx;
        if (iss >> idx) {
            if (idx == index) { found = true; continue; }
            fileOut << l << "\n";
        }
        else {
            fileOut << l << "\n";
        }
    }
    fileOut.close();
    if (found) {
        printf("Frame %d eliminado correctamente de %s.\n", index, archivo.c_str());
        if (LOG) { printf("[DEL] %s: idx=%d borrado\n", archivo.c_str(), index); }
    }
    else {
        printf("Frame %d no encontrado en %s.\n", index, archivo.c_str());
    }
    fflush(stdout);
}

// --- DUMP A CONSOLA ---
void dumpKeyframes(const FRAME* K, int count, const char* tag) {
    printf("\n===== DUMP %s (%d frames) =====\n", tag, count);
    for (int i = 0; i < count; ++i) {
        printf(" %d  %.6f  %.6f  %.6f\n", i, K[i].movX, K[i].movY, K[i].giro);
    }
    printf("================================\n\n");
    fflush(stdout);
}

//////////////////////////// FIN KEYFRAMES ////////////////////////

inline glm::mat4 rotateAroundPivotScaled(
    const glm::mat4& jag,          // T * R * S del jaguar
    const glm::vec3& pivotLocal,   // pivote pre-escala
    float degrees,
    const glm::vec3& axis,
    float jagScale
) {
    const glm::vec3 P = pivotLocal * jagScale;
    glm::mat4 m = jag;
    m = glm::translate(m, P);
    m = glm::rotate(m, glm::radians(degrees), axis);
    m = glm::translate(m, -P);
    return m;
}

inline glm::mat4 rotarConPivote(const glm::mat4& base, glm::vec3 pivotLocal, float grados, glm::vec3 eje)
{
    glm::mat4 m = base;
    m = glm::translate(m, pivotLocal);
    m = glm::rotate(m, glm::radians(grados), eje);
    m = glm::translate(m, -pivotLocal);
    return m;
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 10.0f, 1.0f);

    plainTexture = Texture("Textures/plain.png"); plainTexture.LoadTextureA();
    pisoTexture = Texture("Textures/Nordic_Moss_se4rwei_1K_BaseColor.jpg"); pisoTexture.LoadTextureA();
    ArcoTexture = Texture("Textures/Mossy_Stone_Floor_veilfjxn_1K_BaseColor.jpg"); ArcoTexture.LoadTextureA();
    PuertaTexture = Texture("Textures/Rusty_Metal_Sheet_tj4kedvcw_1K_BaseColor.jpg"); PuertaTexture.LoadTextureA();
    lumbreraTexture = Texture("Textures/lumbrera.png"); lumbreraTexture.LoadTextureA();
    aguilaTexture = Texture("Textures/Fogel_Mat_Diffuse_Color.png"); aguilaTexture.LoadTextureA();
    habitat_ajoloteTexture = Texture("Textures/rock_wall_13_diff_4k.jpg"); habitat_ajoloteTexture.LoadTextureA();
    nidoTexture = Texture("Textures/wood_table_diff_4k.jpg"); nidoTexture.LoadTextureA();
    troncoTexture = Texture("Textures/pine_bark_diff_4k.png"); troncoTexture.LoadTextureA();
    jaguarTexture = Texture("Textures/Jaguar_BaseColor.png"); jaguarTexture.LoadTextureA();
    arbolTexture = Texture("Textures/bark_willow_diff_4k.png"); arbolTexture.LoadTextureA();
    piedrasTexture = Texture("Textures/CavePlatform4_Base_Specular.png"); piedrasTexture.LoadTextureA();
    ringTexture = Texture("Textures/Lona.png"); ringTexture.LoadTextureA();

    Arco_M.LoadModel("Models/Arco.fbx");
    Letrero_M.LoadModel("Models/Letrero.fbx");
    Puerta_Derecha_M.LoadModel("Models/Puerta_Der.fbx");
    Puerta_Izquierda_M.LoadModel("Models/Puerta_Izq.fbx");
    lumbrera.LoadModel("Models/Lumbrera.fbx");
    aguila_M.LoadModel("Models/aguila.fbx");
    alaDer_M.LoadModel("Models/alaDerecha.fbx");
    alaIzq_M.LoadModel("Models/alaIzquierda.fbx");
    habitat_ajolote_M.LoadModel("Models/habitat_ajolote.fbx");
    nido_M.LoadModel("Models/nido.fbx");
    tronco_M.LoadModel("Models/tronco.fbx");
    Jaguar_Cuerpo_M.LoadModel("Models/CuerpoJaguar.fbx");
    Jaguar_TraDer_M.LoadModel("Models/TraDerJaguar.fbx");
    Jaguar_TraIzq_M.LoadModel("Models/TraIzqJaguar.fbx");
    Jaguar_DelDer_M.LoadModel("Models/DelDerJaguar.fbx");
    Jaguar_DelIzq_M.LoadModel("Models/DelIzqJaguar.fbx");
    arbol_M.LoadModel("Models/arbol.fbx");
    piedras_M.LoadModel("Models/piedras.fbx");
    ring_M.LoadModel("Models/Ring.fbx");

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/nx.png",
        "Textures/Skybox/px.png",
        "Textures/Skybox/ny.png",
        "Textures/Skybox/py.png",
        "Textures/Skybox/nz.png",
        "Textures/Skybox/pz.png"
    };
    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    mainLight = DirectionalLight(
        1.0f, 0.98f, 0.92f,
        0.8f, 0.6f,
        0.0f, -1.0f, -0.3f
    );

    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 2.5f, 1.5f,
        0.3f, 0.2f, 0.1f);
    pointLightCount++;

    unsigned int spotLightCount = 0;
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f);
    spotLightCount++;

    spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
        1.0f, 2.0f,
        5.0f, 10.0f, 0.0f,
        0.0f, -5.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
    GLuint uniformColor = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

    printf("\n=====================================\n");
    printf("      CONTROLES DE KEYFRAMES\n");
    printf("=====================================\n");
    printf(" [F2]         Utilizar el ÁGUILA   (keyframes_aguila.txt)\n");
    printf(" [F3]         Utilizar el JAGUAR   (%s)\n", JAG_FILE);
    printf(" [ESPACIO]    Reproducir animación del archivo activo\n");
    printf(" [0]          Reiniciar animación para volver a reproducir\n");
    printf(" [1]          Activar/Desactivar modo edición\n");
    printf("    ├─ H / K  Mover en eje X  (H = +X / K = -X)\n");
    printf("    ├─ U / J  Mover en eje Y  (U = +Y / J = -Y)  (Y = tu eje 'adelante-atrás')\n");
    printf("    └─ R      Activar/Desactivar modo rotación\n");
    printf(" [Flecha Izq/Der]  Rotar el modelo en eje Y\n");
    printf(" [L]          Guardar frame actual en el archivo activo\n");
    printf(" [M]          Modificar último frame guardado\n");
    printf(" [B]          Borrar último frame del archivo\n");
    printf(" [P]          Imprimir (dump) keyframes del archivo activo\n");
    printf(" LOG = %s\n", LOG ? "ON" : "OFF");
    printf("=====================================\n\n");

    glm::mat4 model(1.0);
    glm::mat4 modelaux(1.0);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec2 toffset = glm::vec2(0.0f, 0.0f);
    glm::vec3 lowerLight = glm::vec3(0.0f, 0.0f, 0.0f);

    // Loop principal
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        angulovaria += 0.5f * deltaTime;
        alaAngulo = 15.0f * sin(angulovaria / 1.6);

        // Entrada
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Keyframes (teclado)
        inputKeyframes(mainWindow.getsKeys());

        // Animar águila y jaguar desde archivos activos
        animate(KeyFrameAguila, playIndexAguila, FrameIndexAguila, i_curr_stepsAguila,
            i_max_steps, playAguila, movAguila_x, movAguila_y, giroAguila);

        animate(KeyFrameJaguar, playIndexJaguar, FrameIndexJaguar, i_curr_stepsJaguar,
            i_max_steps, playJaguar, movJag_x, movJag_y, giroJag);

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformTextureOffset = shaderList[0].getOffsetLocation();

        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, 1);
        shaderList[0].SetSpotLights(spotLights, 2);

        model = glm::mat4(1.0);
        modelaux = glm::mat4(1.0);
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        toffset = glm::vec2(0.0f, 0.0f);

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh(); // piso

        // Entrada, puertas y letrero
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(20.0f, -2.0f, 10.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        modelaux = model;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        ArcoTexture.UseTexture();
        Arco_M.RenderModel();

        // Puerta derecha
        model = modelaux;
        model = glm::translate(model, glm::vec3(-2.1f, 0.0f, 3.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        PuertaTexture.UseTexture();
        Puerta_Derecha_M.RenderModel();

        // Puerta izquierda
        model = modelaux;
        model = glm::translate(model, glm::vec3(3.1f, 0.0f, 3.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        PuertaTexture.UseTexture();
        Puerta_Izquierda_M.RenderModel();

        // Letrero
        model = modelaux;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Letrero_M.RenderModel();

        // Luminarias
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(26.0, -2.0f, 7.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        modelaux = model;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        lumbreraTexture.UseTexture();
        lumbrera.RenderModel();
        model = modelaux;
        model = glm::translate(model, glm::vec3(11.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        lumbreraTexture.UseTexture();
        lumbrera.RenderModel();

        // Águila con aleteo + keyframes 2D
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-60.0f, 14.0f, 155.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(movAguila_x, movAguila_y, 0.0f));
        model = glm::rotate(model, glm::radians(giroAguila), glm::vec3(0, 0, 1));
        modelaux = model;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        aguilaTexture.UseTexture();
        aguila_M.RenderModel();

        model = modelaux;
        model = glm::translate(model, glm::vec3(0.3f, -1.0f, 1.9f));
        model = glm::rotate(model, glm::radians(alaAngulo), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        alaDer_M.RenderModel();

        model = modelaux;
        model = glm::translate(model, glm::vec3(-0.3f, -1.0f, 2.0f));
        model = glm::rotate(model, -glm::radians(alaAngulo), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        alaIzq_M.RenderModel();

        // Nido
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-60.0f, -2.0f, 155.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        nidoTexture.UseTexture();
        nido_M.RenderModel();

        // Habitat ajolote
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(15.0f, -2.0f, 180.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        habitat_ajoloteTexture.UseTexture();
        habitat_ajolote_M.RenderModel();

        // ================== JAGUAR ANIMADO ==================
        static float pasoT = 0.0f;
        pasoT += deltaTime * 2.2f;   // velocidad del paso (ciclo de patas)

        // Oscilación de patas (grados)
        const float ampHombro = 25.0f;   // delanteras
        const float ampCadera = 30.0f;   // traseras
        float angDelDer = ampHombro * sinf(pasoT);
        float angDelIzq = -ampHombro * sinf(pasoT);
        float angTraDer = -ampCadera * sinf(pasoT);
        float angTraIzq = ampCadera * sinf(pasoT);

        // POSICIÓN Y ORIENTACIÓN global del jaguar (TR, sin escala)
        const glm::vec3 JAG_POS = glm::vec3(100.0f, 3.0f, 50.0f);
        const float     JAG_SCALE = 0.02f;
        const float     JAG_FIX_PITCH = -90.0f; // FBX miraba a +Y
        const float     JAG_YAW_BASE = 0.0f;

        glm::mat4 jagTR = glm::mat4(1.0f);
        // movJag_x : desplazamiento en X; movJag_y : desplazamiento en Z (con tu convención)
        jagTR = glm::translate(jagTR, JAG_POS + glm::vec3(movJag_x, 0.0f, movJag_y));
        jagTR = glm::rotate(jagTR, glm::radians(JAG_FIX_PITCH), glm::vec3(1, 0, 0));
        jagTR = glm::rotate(jagTR, glm::radians(JAG_YAW_BASE + giroJag), glm::vec3(0, 0, 1));

        // offsets base locales (pre-escala)
        const glm::vec3 baseDelDer = glm::vec3(1.0f, 2.5f, -1.2f);
        const glm::vec3 baseDelIzq = glm::vec3(-1.0f, 2.5f, -1.2f);
        const glm::vec3 baseTraDer = glm::vec3(1.0f, -3.0f, -0.6f);
        const glm::vec3 baseTraIzq = glm::vec3(-1.0f, -3.0f, -0.6f);

        // pivotes locales (pre-escala)
        const glm::vec3 pivDelDer = glm::vec3(0.0f, 0.0f, 0.0f);
        const glm::vec3 pivDelIzq = glm::vec3(0.0f, 0.0f, 0.0f);
        const glm::vec3 pivTraDer = glm::vec3(0.0f, 0.0f, 0.0f);
        const glm::vec3 pivTraIzq = glm::vec3(0.0f, 0.0f, 0.0f);

        auto colocarRotarEscalar = [](const glm::mat4& TR,
            glm::vec3 base, glm::vec3 pivotLocal,
            float grados, glm::vec3 eje, float scale)
            {
                glm::mat4 m = TR;
                m = glm::translate(m, base);
                m = glm::translate(m, pivotLocal);
                m = glm::rotate(m, glm::radians(grados), eje);
                m = glm::translate(m, -pivotLocal);
                m = glm::scale(m, glm::vec3(scale));
                return m;
            };

        // Cuerpo
        {
            glm::mat4 m = jagTR;
            m = glm::scale(m, glm::vec3(JAG_SCALE));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            jaguarTexture.UseTexture();
            Jaguar_Cuerpo_M.RenderModel();
        }

        const glm::vec3 ejeHombro = glm::vec3(1, 0, 0);
        const glm::vec3 ejeCadera = glm::vec3(1, 0, 0);

        // Delantera derecha
        {
            glm::mat4 m = colocarRotarEscalar(jagTR, baseDelDer, pivDelDer, angDelDer, ejeHombro, JAG_SCALE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            jaguarTexture.UseTexture();
            Jaguar_DelDer_M.RenderModel();
        }
        // Delantera izquierda
        {
            glm::mat4 m = colocarRotarEscalar(jagTR, baseDelIzq, pivDelIzq, angDelIzq, ejeHombro, JAG_SCALE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            jaguarTexture.UseTexture();
            Jaguar_DelIzq_M.RenderModel();
        }
        // Trasera derecha
        {
            glm::mat4 m = colocarRotarEscalar(jagTR, baseTraDer, pivTraDer, angTraDer, ejeCadera, JAG_SCALE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            jaguarTexture.UseTexture();
            Jaguar_TraDer_M.RenderModel();
        }
        // Trasera izquierda
        {
            glm::mat4 m = colocarRotarEscalar(jagTR, baseTraIzq, pivTraIzq, angTraIzq, ejeCadera, JAG_SCALE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            jaguarTexture.UseTexture();
            Jaguar_TraIzq_M.RenderModel();
        }

        // Árbol
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(95.0f, -2.0f, 110.0f));
        model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        arbolTexture.UseTexture();
        arbol_M.RenderModel();

        // Piedras
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(108.0f, 10.0f, 120.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        piedrasTexture.UseTexture();
        piedras_M.RenderModel();

        // Ring
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(20.0f, 0.0f, 80.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        ringTexture.UseTexture();
        ring_M.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}

void inputKeyframes(bool* keys)
{
    // CAMBIAR MODELO ACTIVO
    if (keys[GLFW_KEY_F2]) {
        archivoActual = "keyframes_aguila.txt";
    }
    if (keys[GLFW_KEY_F3]) {
        archivoActual = JAG_FILE;
    }

    FRAME* KeyFrameActivo;
    int* FrameIndexActivo;
    float* movX;
    float* movY;
    float* giro;
    int* playIndex;
    int* i_curr_steps;
    bool* play;

    if (archivoActual == std::string(JAG_FILE)) {
        KeyFrameActivo = KeyFrameJaguar;
        FrameIndexActivo = &FrameIndexJaguar;
        movX = &movJag_x;  movY = &movJag_y;  giro = &giroJag;
        playIndex = &playIndexJaguar;
        i_curr_steps = &i_curr_stepsJaguar;
        play = &playJaguar;
    }
    else {
        KeyFrameActivo = KeyFrameAguila;
        FrameIndexActivo = &FrameIndexAguila;
        movX = &movAguila_x; movY = &movAguila_y; giro = &giroAguila;
        playIndex = &playIndexAguila;
        i_curr_steps = &i_curr_stepsAguila;
        play = &playAguila;
    }

    static bool pressed1 = false;
    if (keys[GLFW_KEY_1]) {
        if (!pressed1) {
            modoEdicion = !modoEdicion;
            modoRotacion = false;
            printf(modoEdicion ? "Modo edición ACTIVADO.\n" : "Modo edición DESACTIVADO.\n");
            pressed1 = true;
        }
    }
    else pressed1 = false;

    static bool pressedR = false;
    if (modoEdicion && keys[GLFW_KEY_R]) {
        if (!pressedR) {
            modoRotacion = !modoRotacion;
            printf(modoRotacion ? "Modo rotación ACTIVADO.\n" : "Modo traslación ACTIVADO.\n");
            pressedR = true;
        }
    }
    else pressedR = false;

    if (modoEdicion) {
        float vel = 0.05f, velRot = 1.5f;

        if (!modoRotacion) {
            float yaw = glm::radians(*giro);

            glm::vec2 forward = glm::vec2(sinf(yaw), cosf(yaw));
            glm::vec2 right = glm::vec2(forward.y, -forward.x); // perpendicular a forward

            // H/K = strafe derecha/izquierda (local right/left)
            if (keys[GLFW_KEY_H]) { *movX += right.x * vel; *movY += right.y * vel; }
            if (keys[GLFW_KEY_K]) { *movX -= right.x * vel; *movY -= right.y * vel; }

            if (keys[GLFW_KEY_U]) { *movX -= forward.x * vel; *movY -= forward.y * vel; } // ahora U = avanzar
            if (keys[GLFW_KEY_J]) { *movX += forward.x * vel; *movY += forward.y * vel; } // ahora J = retroceder
        }
        else {
            // --- Rotación sobre el eje Y del jaguar (yaw) ---
            if (keys[GLFW_KEY_LEFT])  *giro += velRot;
            if (keys[GLFW_KEY_RIGHT]) *giro -= velRot;
        }
    }

    // GUARDAR
    static bool pressedL = false;
    if (keys[GLFW_KEY_L]) {
        if (!pressedL) {
            saveFrame(archivoActual, KeyFrameActivo, *FrameIndexActivo, *movX, *movY, *giro);
            pressedL = true;
        }
    }
    else pressedL = false;

    // MODIFICAR
    static bool pressedM = false;
    if (keys[GLFW_KEY_M]) {
        if (!pressedM && *FrameIndexActivo > 0) {
            int modIndex = *FrameIndexActivo - 1;
            KeyFrameActivo[modIndex].movX = *movX;
            KeyFrameActivo[modIndex].movY = *movY;
            KeyFrameActivo[modIndex].giro = normalizeDeg(*giro);
            actualizarArchivo(archivoActual, modIndex, KeyFrameActivo[modIndex]);
            pressedM = true;
        }
    }
    else pressedM = false;

    // BORRAR
    static bool pressedB = false;
    if (keys[GLFW_KEY_B]) {
        if (!pressedB && *FrameIndexActivo > 0) {
            borrarFrame(archivoActual, *FrameIndexActivo - 1);
            (*FrameIndexActivo)--;
            pressedB = true;
        }
    }
    else pressedB = false;

    // DUMP
    static bool pressedP = false;
    if (keys[GLFW_KEY_P]) {
        if (!pressedP) {
            dumpKeyframes(KeyFrameActivo, *FrameIndexActivo, archivoActual.c_str());
            pressedP = true;
        }
    }
    else pressedP = false;

    // REPRODUCIR
    if (keys[GLFW_KEY_SPACE]) {
        if (reproducirAnimacion < 1) {
            if (!*play) {
                printf("Reproduciendo animación desde: %s\n", archivoActual.c_str());
                leerArchivo(archivoActual, KeyFrameActivo, *FrameIndexActivo);
                if (*FrameIndexActivo > 1) {
                    resetElements(KeyFrameActivo, *movX, *movY, *giro);
                    *playIndex = 0;
                    *i_curr_steps = 0;
                    interpolation(KeyFrameActivo, *playIndex, i_max_steps);
                    if (LOG) {
                        printf("[PLAY] idx=%d -> idx=%d  steps=%d  from:(%.3f,%.3f,%.3f)  d:(%.3f,%.3f,%.3f)\n",
                            *playIndex, *playIndex + 1, i_max_steps,
                            KeyFrameActivo[*playIndex].movX, KeyFrameActivo[*playIndex].movY, KeyFrameActivo[*playIndex].giro,
                            KeyFrameActivo[*playIndex].movXInc, KeyFrameActivo[*playIndex].movYInc, KeyFrameActivo[*playIndex].giroInc);
                        fflush(stdout);
                    }
                    *play = true;
                    reproducirAnimacion++;
                }
                else {
                    printf("No hay suficientes frames en %s para reproducir.\n", archivoActual.c_str());
                }
            }
        }
    }
    else reproducirAnimacion = 0;
}
