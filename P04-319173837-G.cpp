// Práctica 4 – Modelado jerárquico (grúa con base, ruedas y canasta).
// Idea central: encadenar transformaciones con matrices (padre→hijo) y reutilizar marcos (model, modelaux, modelCabina).
// Controles: E/R/T (rotaciones de la cabina), F/G/H (articulaciones 1–3), J/K (yaw de la canasta), ruedas (teclas mapeadas en Window.cpp).

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

const float toRadians = 3.14159265f / 180.0f;
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
Sphere sp = Sphere(1.0, 20, 20); // esfera unitaria (radio=1) para las articulaciones

// --- Geometrías base ---

// Cubo unitario centrado en el origen; se escala/rota/traslada con 'model'
void CrearCubo()
{
    unsigned int cubo_indices[] = {
        // 6 caras × 2 triángulos
        0, 1, 2,  2, 3, 0,   // front
        1, 5, 6,  6, 2, 1,   // right
        7, 6, 5,  5, 4, 7,   // back
        4, 0, 3,  3, 7, 4,   // left
        4, 5, 1,  1, 0, 4,   // bottom
        3, 2, 6,  6, 7, 3    // top
    };

    GLfloat cubo_vertices[] = {
        // 8 vértices de un cubo unitario
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };

    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo);
}

// Pirámide triangular regular (4 vértices, 4 caras)
void CrearPiramideTriangular()
{
    unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2,
        3, 0, 2,
        1, 0, 3
    };

    GLfloat vertices[] = {
        -0.5f, -0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
         0.0f,  0.5f, -0.25f,
         0.0f, -0.5f, -0.5f
    };

    Mesh* obj = new Mesh();
    obj->CreateMesh(vertices, indices, 12, 12);
    meshList.push_back(obj);
}

// Cilindro unitario a lo largo del eje Y, con tapas; res=“rebanadas”, R=radio
void CrearCilindro(int res, float R)
{
    int n, i;
    GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

    vector<GLfloat> vertices;
    vector<unsigned int> indices;

    // Paredes: para cada ángulo generamos 2 vértices (y=-0.5 y y=+0.5)
    for (n = 0; n <= res; n++) {
        if (n != res) { x = R * cos(n * dt); z = R * sin(n * dt); }
        else { x = R * cos(0);      z = R * sin(0); }

        // (x, -0.5, z) y (x, +0.5, z)
        vertices.push_back(x); vertices.push_back(y);     vertices.push_back(z);
        vertices.push_back(x); vertices.push_back(0.5f);  vertices.push_back(z);
    }

    // Circunferencia inferior (y=-0.5)
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
    }

    // Circunferencia superior (y=+0.5)
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(0.5f); vertices.push_back(z);
    }

    // Usamos drawArrays con un índice secuencial (0..N-1)
    for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, (unsigned int)vertices.size(), (unsigned int)indices.size());
    meshList.push_back(cilindro);
}

// Cono con vértice superior en y=+0.5 y base en y=-0.5 (res=“rebanadas”, R=radio)
void CrearCono(int res, float R)
{
    int n, i;
    GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

    vector<GLfloat> vertices;
    vector<unsigned int> indices;

    // Vértice del cono
    vertices.push_back(0.0f); vertices.push_back(0.5f); vertices.push_back(0.0f);

    // Circunferencia de la base
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt);
        z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
    }
    // Cerrar círculo (punto adicional)
    vertices.push_back(R * cos(0)); vertices.push_back(-0.5f); vertices.push_back(R * sin(0));

    for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, (unsigned int)vertices.size(), (unsigned int)indices.size());
    meshList.push_back(cono);
}

// Pirámide cuadrangular centrada; base en y=-0.5 y ápice en y=+0.5
void CrearPiramideCuadrangular()
{
    // 6 triángulos: 4 laterales + 2 para la base
    vector<unsigned int> indices = {
        0, 3, 4,  3, 2, 4,  2, 1, 4,  1, 0, 4,  0, 1, 2,  0, 2, 4
    };

    vector<GLfloat> vertices = {
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.0f,  0.5f,  0.0f
    };

    Mesh* piramide = new Mesh();
    piramide->CreateMeshGeometry(vertices, indices, 15, 18);
    meshList.push_back(piramide);
}

// Carga y compila los shaders (único programa en esta práctica)
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    // Ventana + contexto OpenGL
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    // Cargar geometrías al meshList (índices documentados)
    CrearCubo();                 // 0
    CrearPiramideTriangular();   // 1
    CrearCilindro(15, 1.0f);     // 2 (ruedas usan este)
    CrearCono(25, 2.0f);         // 3
    CrearPiramideCuadrangular(); // 4
    CreateShaders();

    // Cámara (posición, up, yaw, pitch, velocidades)
    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 0.3f
    );

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;

    // Proyección en perspectiva (FOV=60°, near=0.1, far=100)
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        static_cast<float>(mainWindow.getBufferWidth()) / static_cast<float>(mainWindow.getBufferHeight()),
        0.1f, 100.0f
    );

    // Esfera (malla de articulaciones)
    sp.init();
    sp.load();

    // Matrices auxiliares para “guardar y restaurar” marcos de referencia
    glm::mat4 model(1.0f), modelaux(1.0f), modelCabina(1.0f);
    glm::vec3 color = glm::vec3(0.0f);

    // Bucle principal
    while (!mainWindow.getShouldClose())
    {
        // Delta de tiempo para controles suaves
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        // Entrada: teclado y mouse (WASD + movimiento de cámara)
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Limpieza de buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activar shader y ubicar las direcciones de los uniforms
        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        // Subir matrices de proyección y vista (una vez por frame)
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // ======== CABINA (cubo) ========
        // Marco base de la grúa; orden de transformaciones: T → Rx → Ry → Rz → S
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, -4.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f)); // tecla E
        model = glm::rotate(model, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f)); // tecla R
        model = glm::rotate(model, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f)); // tecla T

        // Guardamos dos referencias: modelCabina (para “volver” al final) y modelaux (antes de escalar)
        modelCabina = model;
        modelaux = model;

        // Dibujar la cabina (cubo escalado)
        model = glm::scale(model, glm::vec3(4.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();
        model = modelaux; // restauramos el marco sin la escala

        // ======== BASE: Pirámide cuadrangular “incrustada” bajo la cabina ========
        // Tomamos el marco de cabina sin escala y bajamos 1 unidad para centrar la pirámide
        glm::mat4 baseCabina = model;
        glm::mat4 framePiramide = glm::translate(baseCabina, glm::vec3(0.0f, -1.0f, 0.0f));

        // Escala de la pirámide (ancho, alto, fondo)
        const float sx = 3.5f, sy = 1.2f, sz = 1.8f;

        // Dibujar pirámide (framePiramide + scale)
        model = framePiramide;
        model = glm::scale(model, glm::vec3(sx, sy, sz));
        color = glm::vec3(0.7f, 0.5f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[4]->RenderMeshGeometry();

        // ======== RUEDAS PEGADAS A LA BASE DE LA PIRÁMIDE ========
        // Calculamos semiejes de la base y el plano Y de la base ya escalada (yBase)
        const float hx = 0.5f * sx;   // semieje X
        const float hz = 0.5f * sz;   // semieje Z
        const float yBase = -0.5f * sy; // base en el marco local de la pirámide

        // Parámetros visuales de la rueda (cilindro unitario escalado)
        const float r = 0.5f;     // radio de la llanta
        const float t = 0.3f;     // grosor (a lo largo del eje de giro)
        const float margin = 0.05f; // pequeño margen para que “sobresalga” de la cara

        // Dibuja una rueda en el cuadrante (sxSign, szSign) con ángulo propio 'angDeg'
        auto dibujaRueda = [&](float sxSign, float szSign, float angDeg) {
            glm::mat4 m = framePiramide;
            // Posicionamos el centro: pegado a ±X, cerca de ±Z y tocando la base (y = yBase - r)
            m = glm::translate(m, glm::vec3(
                sxSign * (hx + t * 0.5f + margin),
                yBase - r + 0.08f,                 // pequeño ajuste visual para contacto
                szSign * (hz - 0.15f)
            ));
            // El cilindro está orientado a lo largo de Y; rotamos 90° para que su eje sea X (eje de giro de la rueda)
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            // Giro propio de la rueda (manejado desde Window con teclas)
            m = glm::rotate(m, glm::radians(angDeg), glm::vec3(1.0f, 0.0f, 0.0f));
            // Escala final (grosor, radio, radio)
            m = glm::scale(m, glm::vec3(t, r, r));

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.2f, 0.2f, 0.2f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[2]->RenderMeshGeometry(); // cilindro
         };

        // Orden: (+x,+z) delantera derecha; (-x,+z) delantera izquierda; (+x,-z) trasera derecha; (-x,-z) trasera izquierda
        dibujaRueda(+1.f, +1.f, mainWindow.getrueda1());
        dibujaRueda(-1.f, +1.f, mainWindow.getrueda2());
        dibujaRueda(+1.f, -1.f, mainWindow.getrueda3());
        dibujaRueda(-1.f, -1.f, mainWindow.getrueda4());

        // Volvemos al marco de la cabina (sin escala) para seguir con el brazo
        model = baseCabina;

        // ======== ARTICULACIÓN 1 (esfera) ========
        // Rotación alrededor de Z local (tecla F)
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        sp.render();

        // ======== BRAZO 1 (cubo alargado) ========
        // Se rota 135° para orientar el brazo y se traslada medio largo antes de escalar (centra el cubo en el segmento)
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
        modelaux = model; // guardamos para continuar el encadenamiento
        model = glm::scale(model, glm::vec3(5.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();
        model = modelaux;

        // ======== ARTICULACIÓN 2 (esfera) ========
        // Nos movemos al extremo del brazo 1 y aplicamos la rotación de la articulación 2 (tecla G)
        model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // ======== BRAZO 2 ========
        // Bajamos medio largo en -Y local y escalamos el cubo para formar la barra vertical
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(1.0f, 5.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // ======== ARTICULACIÓN 3 (esfera) ========
        // Volvemos al centro del cubo (modelaux), nos movemos a su extremo y rotamos (tecla H)
        model = modelaux;
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // ======== BRAZO 3 (barra), ART. 4 (yaw) y CANASTA ========
        // Extiende otro segmento hacia -Y (longitud 4 → traslado 2 para centrar el cubo)
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        glm::mat4 baseBrazo3 = model;              // punto de anclaje para la siguiente articulación
        model = glm::scale(model, glm::vec3(1.0f, 4.0f, 1.0f));
        color = glm::vec3(0.0f, 0.8f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[0]->RenderMesh();

        // Articulación 4: yaw izquierda/derecha con J/K. Se limita a ±70° sumando izquierda/derecha (K−J).
        model = baseBrazo3;
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        float yawCanastaDeg = (mainWindow.getarticulacion5() - mainWindow.getarticulacion4());
        yawCanastaDeg = glm::clamp(yawCanastaDeg, -70.0f, 70.0f);
        model = glm::rotate(model, glm::radians(yawCanastaDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render(); // esfera en el pivote de la canasta

        // Canasta: prisma rectangular articulado por una cara lateral (trasladamos +ancho/2 en X antes de escalar)
        const float ancho = 1.6f, alto = 2.8f, fondo = 1.2f;
        model = glm::translate(model, glm::vec3(ancho * 0.5f, 0.0f, 0.0f));
        glm::mat4 baseCanasta = model;
        model = glm::scale(model, glm::vec3(ancho, alto, fondo));
        color = glm::vec3(1.0f, 0.8f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[0]->RenderMesh();

        // Volver al marco de la cabina 
        model = modelCabina;

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}