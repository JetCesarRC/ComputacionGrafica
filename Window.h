#pragma once
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>

// Clase Window: administración de ventana + input de teclado/mouse
class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight);

    // Crea contexto, GLEW y registra callbacks. Devuelve 0 si OK.
    int Initialise();

    // --- Getters básicos ---
    GLfloat getBufferWidth()  const { return static_cast<GLfloat>(bufferWidth); }
    GLfloat getBufferHeight() const { return static_cast<GLfloat>(bufferHeight); }

    // Delta de mouse desde el último frame (se reinicia a 0 al leer)
    GLfloat getXChange();
    GLfloat getYChange();

    // Estado de ventana / entradas
    bool   getShouldClose() const { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }

    // Swap de buffers
    void swapBuffers() { glfwSwapBuffers(mainWindow); }

    // Acceso al puntero de GLFW (útil para setear input mode, etc.)
    GLFWwindow* getGLFWwindow() const { return mainWindow; }

    ~Window();

private:
    GLFWwindow* mainWindow;
    GLint width, height;

    bool  keys[1024];
    GLint bufferWidth, bufferHeight;

    // Mouse
    GLfloat lastX, lastY;
    GLfloat xChange, yChange;
    bool    mouseFirstMoved;

    void createCallbacks();

    // Callbacks estáticos de GLFW
    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};