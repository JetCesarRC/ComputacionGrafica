#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <glew.h>
#include <glfw3.h>

// -----------------------------------------------------------------------------
// Parámetros de ventana
// -----------------------------------------------------------------------------
static const int WIDTH = 800;
static const int HEIGHT = 800;

// -----------------------------------------------------------------------------
// Recursos globales
// -----------------------------------------------------------------------------
static GLuint shader = 0;          // Programa de shaders (vertex + fragment)

// VAO/VBO por letra
static GLuint VAO_R = 0, VBO_R = 0;
static GLuint VAO_C = 0, VBO_C = 0;
static GLuint VAO_E = 0, VBO_E = 0;

// Número de vértices por letra (triángulos * 3)
// # triángulos × 3 vértices;
const GLsizei N_C = 36; // 12 triángulos
const GLsizei N_R = 36; // 12 triángulos
const GLsizei N_E = 48; // 16 triángulos

// -----------------------------------------------------------------------------
// Shaders (GLSL 330). Posición en location 0. Color fijo en el fragment.
// -----------------------------------------------------------------------------
static const char* vShader = R"(
#version 330
layout (location = 0) in vec3 pos;
void main() {
    gl_Position = vec4(pos, 1.0);
}
)";

static const char* fShader = R"(
#version 330
out vec4 color;
void main() {
    color = vec4(1.0, 1.0, 0.0, 1.0); // Amarillo
}
)";

// -----------------------------------------------------------------------------
// Crea la geometría de las letras y configura los VAO/VBO correspondientes.
// Todas las figuras están en coordenadas NDC [-1, 1].
// -----------------------------------------------------------------------------
static void CrearLetras() {
    // ---------------- C (izquierda) -------------------
    const GLfloat Cverts[] = {
        // Rectángulo vertical izquierdo (cuatro triángulos)
        -0.95f, -0.0f, 0.0f,   -0.80f, -0.45f, 0.0f,   -0.80f, 0.0f, 0.0f,
        -0.95f, -0.45f, 0.0f,   -0.80f, -0.45f, 0.0f,   -0.95f, 0.0f, 0.0f,
        -0.95f,  0.45f, 0.0f,   -0.80f,  0.45f, 0.0f,   -0.80f, 0.0f, 0.0f,
        -0.95f,  0.45f, 0.0f,   -0.80f,  0.0f, 0.0f,   -0.95f, 0.0f, 0.0f,


        // Barra horizontal superior (cuatro triángulos)
        -0.95f,  0.45f, 0.0f,   -0.55f,  0.60f, 0.0f,   -0.55f, 0.45f, 0.0f,
        -0.95f,  0.45f, 0.0f,   -0.55f,  0.60f, 0.0f,   -0.95f, 0.60f, 0.0f,
        -0.55f,  0.45f, 0.0f,   -0.55f,  0.60f, 0.0f,   -0.35f, 0.45f, 0.0f,
        -0.35f,  0.60f, 0.0f,   -0.55f,  0.60f, 0.0f,   -0.35f, 0.45f, 0.0f,

        // Barra horizontal inferior (dos triángulos)
        -0.95f, -0.60f, 0.0f,   -0.55f, -0.45f, 0.0f,   -0.55f,-0.60f, 0.0f,
        -0.95f, -0.60f, 0.0f,   -0.55f, -0.45f, 0.0f,   -0.95f,-0.45f, 0.0f,
        -0.55f, -0.60f, 0.0f,   -0.55f, -0.45f, 0.0f,   -0.35f,-0.60f, 0.0f,
        -0.55f, -0.45f, 0.0f,   -0.35f, -0.60f, 0.0f,   -0.35f,-0.45f, 0.0f,
    };

    glGenVertexArrays(1, &VAO_C);
    glBindVertexArray(VAO_C);
    glGenBuffers(1, &VBO_C);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cverts), Cverts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ---------------- R (centro) -------------------
    const GLfloat Rverts[] = {
        // barra vertical izquierda
        -0.20f, -0.60f, 0.0f,  -0.05f,  0.05f, 0.0f,  -0.05f, -0.60f, 0.0f,
        -0.20f, -0.60f, 0.0f,  -0.05f,  0.05f, 0.0f,  -0.20f,  0.05f, 0.0f,
        -0.20f, -0.05f, 0.0f,  -0.05f,  0.05f, 0.0f,  -0.20f,  0.60f, 0.0f,
        -0.20f,  0.60f, 0.0f,  -0.05f,  0.60f, 0.0f,  -0.05f,  0.05f, 0.0f,

        // horizontal superior
        -0.05f,  0.45f, 0.0f,   0.25f,  0.45f, 0.0f,   0.25f, 0.60f, 0.0f,
        -0.05f,  0.45f, 0.0f,   0.25f,  0.60f, 0.0f,  -0.05f, 0.60f, 0.0f,

        // horizontal media
        -0.05f, 0.05f, 0.0f,    0.25f, 0.05f, 0.0f,    0.25f, 0.20f, 0.0f,
        -0.05f, 0.05f, 0.0f,    0.25f, 0.20f, 0.0f,   -0.05f, 0.20f, 0.0f,

        // vertical superior derecha
         0.10f, 0.45f, 0.0f,    0.25f, 0.45f, 0.0f,    0.25f, 0.20f, 0.0f,
         0.10f, 0.45f, 0.0f,    0.25f, 0.20f, 0.0f,    0.10f, 0.20f, 0.0f,

        // diagonal inferior (trapecio)
        -0.05f, 0.05f, 0.0f,    0.05f, 0.05f, 0.0f,    0.25f, -0.60f, 0.0f,
        -0.05f, 0.05f, 0.0f,    0.25f,-0.60f, 0.0f,    0.15f, -0.60f, 0.0f,
    };
    glGenVertexArrays(1, &VAO_R);
    glBindVertexArray(VAO_R);
    glGenBuffers(1, &VBO_R);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_R);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Rverts), Rverts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ---------------- E (derecha) ------------------
    const GLfloat Everts[] = {
        // barra vertical
         0.40f,  0.60f, 0.0f,   0.55f, -0.05f, 0.0f,   0.55f,  0.60f, 0.0f,
         0.40f, -0.05f, 0.0f,   0.55f, -0.05f, 0.0f,   0.40f,  0.60f, 0.0f,
         0.40f, -0.05f, 0.0f,   0.40f, -0.60f, 0.0f,   0.55f, -0.05f, 0.0f,
         0.40f, -0.60f, 0.0f,   0.55f, -0.60f, 0.0f,   0.55f, -0.05f, 0.0f,

        // horizontal superior
         0.55f,  0.45f, 0.0f,   0.80f,  0.45f, 0.0f,   0.80f,  0.60f, 0.0f,
         0.55f,  0.45f, 0.0f,   0.80f,  0.60f, 0.0f,   0.55f,  0.60f, 0.0f,
         0.80f,  0.45f, 0.0f,   0.95f,  0.45f, 0.0f,   0.95f,  0.60f, 0.0f,
         0.80f,  0.45f, 0.0f,   0.80f,  0.60f, 0.0f,   0.95f,  0.60f, 0.0f,

        // horizontal media (rectángulo principal)
         0.75f,  0.10f, 0.0f,   0.75f, -0.05f, 0.0f,   0.95f,  0.10f, 0.0f,
         0.95f, -0.05f, 0.0f,   0.95f,  0.10f, 0.0f,   0.75f, -0.05f, 0.0f,
         0.75f, -0.05f, 0.0f,   0.55f, -0.05f, 0.0f,   0.55f,  0.10f, 0.0f,
         0.75f, -0.05f, 0.0f,   0.75f,  0.10f, 0.0f,   0.55f,  0.10f, 0.0f,

        // horizontal inferior
         0.55f, -0.60f, 0.0f,   0.80f, -0.60f,0.0f,   0.80f, -0.45f, 0.0f,
         0.55f, -0.60f, 0.0f,   0.80f, -0.45f,0.0f,   0.55f, -0.45f, 0.0f,
         0.95f, -0.45f, 0.0f,   0.95f, -0.60f,0.0f,   0.80f, -0.45f, 0.0f,
         0.80f, -0.60f, 0.0f,   0.80f, -0.45f,0.0f,   0.95f, -0.60f, 0.0f,

    };
    glGenVertexArrays(1, &VAO_E);
    glBindVertexArray(VAO_E);
    glGenBuffers(1, &VBO_E);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_E);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Everts), Everts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// -----------------------------------------------------------------------------
// Compilación y linkeo de shaders. Si falla, imprime el log en consola.
// -----------------------------------------------------------------------------
static void AddShader(GLuint program, const char* codeSrc, GLenum type) {
    GLuint sh = glCreateShader(type);
    const GLchar* src[1] = { codeSrc };
    const GLint   len[1] = { (GLint)std::strlen(codeSrc) };
    glShaderSource(sh, 1, src, len);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLchar log[1024] = { 0 };
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Error al compilar shader %d:\n%s\n", type, log);
        glDeleteShader(sh);
        return;
    }

    glAttachShader(program, sh);
    glDeleteShader(sh); 
}

static void CompileShaders() {
    shader = glCreateProgram();
    if (!shader) {
        std::fprintf(stderr, "No se pudo crear el programa de shaders.\n");
        return;
    }

    AddShader(shader, vShader, GL_VERTEX_SHADER);
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);

    GLint ok = GL_FALSE;
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLchar log[1024] = { 0 };
        glGetProgramInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Error al linkear:\n%s\n", log);
        return;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &ok);
    if (!ok) {
        GLchar log[1024] = { 0 };
        glGetProgramInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Error al validar programa:\n%s\n", log);
        return;
    }
}

// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------
int main() {
    // Inicializa GLFW
    if (!glfwInit()) {
        std::fprintf(stderr, "Falló inicializar GLFW.\n");
        return 1;
    }

    // Contexto OpenGL 3.3 Core (compatibilidad amplia)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Crea la ventana
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Primer ventana", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Fallo al crear la ventana con GLFW.\n");
        glfwTerminate();
        return 1;
    }

    // Contexto actual
    glfwMakeContextCurrent(window);

    // Tamaño del framebuffer y viewport inicial
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    // Inicializa GLEW (carga funciones modernas)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::fprintf(stderr, "Falló inicialización de GLEW.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Carga de recursos
    CrearLetras();
    CompileShaders();

    // Semilla para el color de fondo aleatorio
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Fondo aleatorio cada 2 segundos
        static double lastChange = 0.0;
        static float  r = 0.0f, g = 0.0f, b = 0.0f;

        double now = glfwGetTime();
        if (now - lastChange >= 2.0) {
            r = static_cast<float>(std::rand()) / RAND_MAX;
            g = static_cast<float>(std::rand()) / RAND_MAX;
            b = static_cast<float>(std::rand()) / RAND_MAX;
            lastChange = now;
        }

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dibujo
        glUseProgram(shader);

        glBindVertexArray(VAO_C);
        glDrawArrays(GL_TRIANGLES, 0, N_C);
        glBindVertexArray(0);

        glBindVertexArray(VAO_R);
        glDrawArrays(GL_TRIANGLES, 0, N_R);
        glBindVertexArray(0);

        glBindVertexArray(VAO_E);
        glDrawArrays(GL_TRIANGLES, 0, N_E);
        glBindVertexArray(0);

        glUseProgram(0);

        // Presenta en pantalla
        glfwSwapBuffers(window);
    }

    // Limpieza de recursos
    if (VAO_C) glDeleteVertexArrays(1, &VAO_C);
    if (VBO_C) glDeleteBuffers(1, &VBO_C);
    if (VAO_R) glDeleteVertexArrays(1, &VAO_R);
    if (VBO_R) glDeleteBuffers(1, &VBO_R);
    if (VAO_E) glDeleteVertexArrays(1, &VAO_E);
    if (VBO_E) glDeleteBuffers(1, &VBO_E);
    if (shader) glDeleteProgram(shader);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}