/*
Practica: Auto con jerarquia, giro compartido de llantas y traslacion

Resumen funcional (para el reporte):
- Se renderiza un vehiculo compuesto por: carroceria, cofre y cuatro llantas (caucho + rin).
- Se agregan texturas adicionales para ojos, parrilla y cofre (estilo caricaturesco).
- El vehiculo se integra en un entorno con skybox y un piso plano.
- La camara se controla con teclado y mouse, y cuenta con elevacion vertical.
- Las llantas comparten un angulo de rodadura y giran sobre el eje X (rodadura real).
- El cofre abre y cierra sobre su bisagra con un tope de 45 grados.
- El vehiculo avanza y retrocede sobre el eje Z: negativo (adelante) y positivo (atras).

Controles:
- WASD: mover camara
- Mouse: mirar
- Z / X: subir / bajar camara
- E / R / T: rotacion global de la vista (acumula en la matriz de vista)
- Flecha Arriba: avanzar (movimiento -Z) y rodar llantas hacia adelante
- Flecha Abajo: retroceder (movimiento +Z) y rodar llantas hacia atras
- V / B: abrir / cerrar cofre (0 a 45 grados)
*/

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <vector>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "Texture.h"

// -------------------- Variables globales --------------------
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Model Carroceria_M, Cofre_M, Llanta1_M, Llanta2_M, Llanta3_M, Llanta4_M;
Model Rin1_M, Rin2_M, Rin3_M, Rin4_M;

// Modelos extra (ejercicio 3)
Model OjosCarro_M, Parrilla_M;

Texture cauchoTex;
Texture rinTex;
Texture ojosTex;
Texture parrillaTex;
Texture cofreTex;

Skybox skybox;

GLfloat deltaTime = 0.0f, lastTime = 0.0f;

static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";

// -------------------- Crear piso --------------------
void CreateObjects() {
    unsigned int floorIdx[] = { 0,2,1, 1,2,3 };
    GLfloat floorVtx[] = {
        //  x     y     z      u     v     nx  ny  nz
        -10.0f, 0.0f,-10.0f,  0.0f,  0.0f, 0, -1,  0,
         10.0f, 0.0f,-10.0f, 10.0f,  0.0f, 0, -1,  0,
        -10.0f, 0.0f, 10.0f,  0.0f, 10.0f, 0, -1,  0,
         10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0, -1,  0
    };

    Mesh* piso = new Mesh();
    piso->CreateMesh(floorVtx, floorIdx, 32, 6);
    meshList.push_back(piso);
}

// -------------------- Shaders --------------------
void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    shaderList[0].UseShader();
    glUniform1i(shaderList[0].getTextureLocation(), 0); // sampler2D theTexture → GL_TEXTURE0
}

// -------------------- Main --------------------
int main() {
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(
        glm::vec3(0.0f, 6.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, -5.0f,
        10.0f, 1.0f
    );

    // -------------------- Modelos --------------------
    Carroceria_M.LoadModel("Models/Carroceria.obj");
    Cofre_M.LoadModel("Models/CofreTex.obj");   // sustituye al Cofre original
    Llanta1_M.LoadModel("Models/Llanta1.obj");
    Llanta2_M.LoadModel("Models/Llanta2.obj");
    Llanta3_M.LoadModel("Models/Llanta3.obj");
    Llanta4_M.LoadModel("Models/Llanta4.obj");
    Rin1_M.LoadModel("Models/Rin1.obj");
    Rin2_M.LoadModel("Models/Rin2.obj");
    Rin3_M.LoadModel("Models/Rin3.obj");
    Rin4_M.LoadModel("Models/Rin4.obj");

    // Modelos extra
    OjosCarro_M.LoadModel("Models/OjosCarro.obj");
    Parrilla_M.LoadModel("Models/Parrilla.obj");

    // -------------------- Texturas --------------------
    cauchoTex = Texture("Textures/TexturaLlanta.png");
    cauchoTex.LoadTexture();
    rinTex = Texture("Textures/TexturaRin.png");
    rinTex.LoadTexture();

    ojosTex = Texture("Textures/OjosCarro.tga");
    ojosTex.LoadTexture();
    parrillaTex = Texture("Textures/Parrilla.tga");
    parrillaTex.LoadTexture();
    cofreTex = Texture("Textures/Cofre.tga");
    cofreTex.LoadTexture();

    // -------------------- Skybox --------------------
    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    GLuint uProj = 0, uModel = 0, uView = 0, uColor = 0, uUseTex = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f
    );

    float camLift = 0.0f;
    const float LIFT_SPEED = 6.0f;

    glm::vec3 carPos = glm::vec3(5.0f, 5.0f, 0.0f);
    const float CAR_MOVE_SPEED = 10.0f;

    const glm::vec3 COFRE_PIV = glm::vec3(-0.5f, 2.5f, -8.0f);
    const glm::vec3 HOOD_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
    float hoodDeg = 0.0f;
    const float HOOD_SPEED = 60.0f;
    const float HOOD_MAX = 45.0f;

    const glm::vec3 L1_POS = glm::vec3(-9.0f, -3.5f, -11.0f);
    const glm::vec3 L2_POS = glm::vec3(-9.0f, -3.5f, 11.0f);
    const glm::vec3 L3_POS = glm::vec3(9.0f, -3.5f, -11.0f);
    const glm::vec3 L4_POS = glm::vec3(9.0f, -3.5f, 11.0f);

    const glm::vec3 WHEEL_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
    float wheelDeg = 0.0f;
    const float WHEEL_RATE = 200.0f;

    // -------------------- Loop --------------------
    while (!mainWindow.getShouldClose()) {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime; lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        bool* keys = mainWindow.getsKeys();

        if (keys[GLFW_KEY_Z]) camLift += LIFT_SPEED * deltaTime;
        if (keys[GLFW_KEY_X]) camLift -= LIFT_SPEED * deltaTime;

        if (keys[GLFW_KEY_UP]) {
            carPos.z -= CAR_MOVE_SPEED * deltaTime;
            wheelDeg += WHEEL_RATE * deltaTime;
        }
        if (keys[GLFW_KEY_DOWN]) {
            carPos.z += CAR_MOVE_SPEED * deltaTime;
            wheelDeg -= WHEEL_RATE * deltaTime;
        }

        float s = HOOD_SPEED * deltaTime;
        if (keys[GLFW_KEY_V]) hoodDeg += s;
        if (keys[GLFW_KEY_B]) hoodDeg -= s;
        hoodDeg = glm::clamp(hoodDeg, 0.0f, HOOD_MAX);

        glm::mat4 view = camera.calculateViewMatrix();
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotax()), glm::vec3(1, 0, 0)) * view;
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotay()), glm::vec3(0, 1, 0)) * view;
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotaz()), glm::vec3(0, 0, 1)) * view;
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -camLift, 0.0f)) * view;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(view, projection);

        shaderList[0].UseShader();
        uModel = shaderList[0].GetModelLocation();
        uProj = shaderList[0].GetProjectionLocation();
        uView = shaderList[0].GetViewLocation();
        uColor = shaderList[0].getColorLocation();
        uUseTex = shaderList[0].getUseTextureLocation();

        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        // Piso
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(uUseTex, 0);
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(0.5f)));
        meshList[0]->RenderMesh();

        // Carroceria
        glUniform1i(uUseTex, 0);
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        glm::mat4 Mcar = glm::translate(glm::mat4(1.0f), carPos);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(Mcar));
        Carroceria_M.RenderModel();

        // Cofre con textura
        {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), carPos);
            M = glm::translate(M, COFRE_PIV);
            M = glm::rotate(M, glm::radians(hoodDeg), HOOD_AXIS);

            // Rotación extra de 180 grados para que mire igual que el cofre original
            M = glm::rotate(M, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);

            cofreTex.UseTexture();
            glUniform1i(uUseTex, 1);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            Cofre_M.RenderModel();

            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // Ojos (sobresalir un poco)
        {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), carPos);

            // Si tu OjosCarro.obj ya está colocado en el parabrisas correctamente,
            // sólo empújalo un poco hacia el frente del auto (−Z):
            const float EYES_POP = -0.02f; // más negativo = más hacia fuera del frente
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, EYES_POP));

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);

            ojosTex.UseTexture();
            glUniform1i(uUseTex, 1);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            OjosCarro_M.RenderModel();

            glDisable(GL_POLYGON_OFFSET_FILL);
        }
        // Parrilla (sobresalir un poco)
        {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), carPos);

            const float GRILL_POP = -0.015f;
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, GRILL_POP));

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);

            parrillaTex.UseTexture();
            glUniform1i(uUseTex, 1);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            Parrilla_M.RenderModel();

            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // Llantas + Rin
        auto drawWheel = [&](const glm::vec3& pos, Model& wheelModel, Model& rinModel) {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), carPos);
            M = glm::translate(M, pos);
            M = glm::rotate(M, glm::radians(wheelDeg), WHEEL_AXIS);

            cauchoTex.UseTexture();
            glUniform1i(uUseTex, 1);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            wheelModel.RenderModel();

            rinTex.UseTexture();
            glUniform1i(uUseTex, 1);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glm::mat4 R = glm::scale(M, glm::vec3(1.05f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(R));
            rinModel.RenderModel();
            };

        drawWheel(L1_POS, Llanta1_M, Rin1_M);
        drawWheel(L2_POS, Llanta2_M, Rin2_M);
        drawWheel(L3_POS, Llanta3_M, Rin3_M);
        drawWheel(L4_POS, Llanta4_M, Rin4_M);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
