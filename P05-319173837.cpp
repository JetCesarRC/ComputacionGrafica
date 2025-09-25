/*
Practica: Auto con jerarquia, giro compartido de llantas y traslacion

Resumen funcional (para el reporte):
- Se renderiza un vehiculo compuesto por: carroceria, cofre y cuatro llantas.
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

// -------------------- Variables globales de la aplicacion --------------------
// Ventana y gestion de entrada
Window mainWindow;

// Listas de mallas auxiliares (piso) y del shader basico
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

// Camara con desplazamiento y rotacion controlados por teclado y mouse
Camera camera;

// Modelos del auto
Model Carroceria_M, Cofre_M, Llanta1_M, Llanta2_M, Llanta3_M, Llanta4_M;

// Entorno de cubo para cielo
Skybox skybox;

// Control de tiempo para animaciones dependientes de deltaTime
GLfloat deltaTime = 0.0f, lastTime = 0.0f;

// Rutas de archivos de shader
static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";

// -------------------- Construccion de geometria simple (piso) --------------------
// Crea un plano como piso, con indices y atributos minimos (pos, uv, normal)
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
    meshList.push_back(piso);   // meshList[0] = piso
}

// -------------------- Carga de shaders --------------------
void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main() {
    // -------------------- Inicializacion de ventana y contexto OpenGL --------------------
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    // -------------------- Configuracion inicial de la camara --------------------
    // Se ubica la camara alejada y ligeramente elevada para evitar iniciar dentro del auto.
    // Parametros: posicion, vector up, yaw, pitch, moveSpeed, turnSpeed
    camera = Camera(
        glm::vec3(0.0f, 6.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, -5.0f,
        10.0f, 1.0f
    );

    // -------------------- Carga de modelos del auto --------------------
    // Se asume que los OBJ ya estan escalados en Blender (escala global 9x segun indicacion previa).
    Carroceria_M.LoadModel("Models/Carroceria.obj");
    Cofre_M.LoadModel("Models/Cofre.obj");
    Llanta1_M.LoadModel("Models/Llanta1.obj");
    Llanta2_M.LoadModel("Models/Llanta2.obj");
    Llanta3_M.LoadModel("Models/Llanta3.obj");
    Llanta4_M.LoadModel("Models/Llanta4.obj");

    // -------------------- Skybox del entorno --------------------
    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    // IDs de uniformes de shader
    GLuint uProj = 0, uModel = 0, uView = 0, uColor = 0;

    // -------------------- Matriz de proyeccion --------------------
    // FOV de 60 grados para abarcar mas escena con el auto escalado.
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f
    );

    // -------------------- Estado de camara vertical (elevacion) --------------------
    float camLift = 0.0f;
    const float LIFT_SPEED = 6.0f;

    // -------------------- Parametros de posicionamiento y animacion del auto --------------------
    // El coche avanza hacia -Z y retrocede hacia +Z.
    glm::vec3 carPos = glm::vec3(5.0f, 5.0f, 0.0f);
    const float CAR_MOVE_SPEED = 10.0f; // unidades por segundo

    // Bisagra y rotacion del cofre
    const glm::vec3 COFRE_PIV = glm::vec3(-0.5f, 2.5f, -8.0f);
    const glm::vec3 HOOD_AXIS = glm::vec3(1.0f, 0.0f, 0.0f); // eje de rotacion del cofre
    float hoodDeg = 0.0f;                 // rango 0..45
    const float HOOD_SPEED = 60.0f;       // grados por segundo
    const float HOOD_MAX = 45.0f;       // tope de apertura

    // Pivotes de las llantas respecto a la carroceria
    const glm::vec3 L1_POS = glm::vec3(-9.0f, -3.5f, -11.0f); // delantera derecha
    const glm::vec3 L2_POS = glm::vec3(-9.0f, -3.5f, 11.0f); // delantera izquierda
    const glm::vec3 L3_POS = glm::vec3(9.0f, -3.5f, -11.0f); // trasera  derecha
    const glm::vec3 L4_POS = glm::vec3(9.0f, -3.5f, 11.0f); // trasera  izquierda

    // Eje de rodadura de las llantas: X (giran hacia adelante y atras al moverse en Z)
    const glm::vec3 WHEEL_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);

    // Angulo de rodadura compartido por las cuatro llantas
    float wheelDeg = 0.0f;
    const float WHEEL_RATE = 200.0f; // grados por segundo

    // -------------------- Bucle principal de renderizado --------------------
    while (!mainWindow.getShouldClose()) {
        // Calculo de deltaTime para animaciones fluidas
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime; lastTime = now;

        // Procesamiento de entrada de usuario
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        bool* keys = mainWindow.getsKeys();

        // Elevacion vertical de la camara con Z y X
        if (keys[GLFW_KEY_Z]) camLift += LIFT_SPEED * deltaTime;
        if (keys[GLFW_KEY_X]) camLift -= LIFT_SPEED * deltaTime;

        // Traslacion del auto y rodadura sincronizada de llantas
        // Flecha arriba: mover hacia -Z (adelante) y aumentar rodadura
        if (keys[GLFW_KEY_UP]) {
            carPos.z -= CAR_MOVE_SPEED * deltaTime;
            wheelDeg += WHEEL_RATE * deltaTime;   // sentido de giro hacia avance
        }
        // Flecha abajo: mover hacia +Z (atras) y disminuir rodadura
        if (keys[GLFW_KEY_DOWN]) {
            carPos.z += CAR_MOVE_SPEED * deltaTime;
            wheelDeg -= WHEEL_RATE * deltaTime;   // sentido de giro hacia retroceso
        }

        // Control del cofre con V y B, limitado a 45 grados
        {
            float s = HOOD_SPEED * deltaTime;
            if (keys[GLFW_KEY_V]) hoodDeg += s;
            if (keys[GLFW_KEY_B]) hoodDeg -= s;
            hoodDeg = glm::clamp(hoodDeg, 0.0f, HOOD_MAX);
        }

        // Construccion de la matriz de vista:
        // 1) vista base de camara,
        // 2) rotaciones globales acumuladas (E/R/T),
        // 3) traslacion vertical por camLift.
        glm::mat4 view = camera.calculateViewMatrix();
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotax()), glm::vec3(1, 0, 0)) * view;
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotay()), glm::vec3(0, 1, 0)) * view;
        view = glm::rotate(glm::mat4(1.0f), glm::radians(mainWindow.getrotaz()), glm::vec3(0, 0, 1)) * view;
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -camLift, 0.0f)) * view;

        // Limpieza de buffers antes de dibujar
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Dibujo del skybox con la vista y proyeccion actuales
        skybox.DrawSkybox(view, projection);

        // Activacion del shader principal y envio de matrices comunes
        shaderList[0].UseShader();
        uModel = shaderList[0].GetModelLocation();
        uProj = shaderList[0].GetProjectionLocation();
        uView = shaderList[0].GetViewLocation();
        uColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        // -------------------- Render del piso (gris) --------------------
        glm::mat4 model(1.0f);
        glm::vec3 color = glm::vec3(0.5f); // gris para contraste con el auto
        model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // -------------------- Jerarquia del auto --------------------
        // Nodo base: posicion global del vehiculo
        glm::mat4 carBase(1.0f);
        carBase = glm::translate(carBase, carPos);

        // 1) Carroceria (rojo)
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(carBase));
        Carroceria_M.RenderModel();

        // 2) Cofre (hijo de carroceria, pivote y rotacion)
        {
            glm::mat4 M = carBase;
            M = glm::translate(M, COFRE_PIV);
            M = glm::rotate(M, glm::radians(hoodDeg), HOOD_AXIS);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            Cofre_M.RenderModel();
        }

        // 3) Llantas (negras) con rodadura compartida sobre el eje X
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(0.0f)));

        auto drawWheel = [&](const glm::vec3& pos, Model& wheelModel) {
            glm::mat4 M = carBase;
            M = glm::translate(M, pos);                               // llevar al pivote local de la llanta
            M = glm::rotate(M, glm::radians(wheelDeg), WHEEL_AXIS);   // rodadura sobre X
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            wheelModel.RenderModel();
            };

        drawWheel(L1_POS, Llanta1_M); // delantera derecha
        drawWheel(L2_POS, Llanta2_M); // delantera izquierda
        drawWheel(L3_POS, Llanta3_M); // trasera derecha
        drawWheel(L4_POS, Llanta4_M); // trasera izquierda

        // Final de dibujo
        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
