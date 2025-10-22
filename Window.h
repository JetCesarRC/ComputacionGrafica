#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>

// Clase Window: manejo de la ventana, entradas de teclado/mouse y callbacks GLFW
class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight);

    int Initialise();

    // --- Getters básicos ---
    GLfloat getBufferWidth() { return static_cast<GLfloat>(bufferWidth); }
    GLfloat getBufferHeight() { return static_cast<GLfloat>(bufferHeight); }

    GLfloat getXChange();
    GLfloat getYChange();

    GLfloat getmuevex() { return muevex; }

    // --- Rotaciones globales ---
    GLfloat getrotax() const { return rotax; }
    GLfloat getrotay() const { return rotay; }
    GLfloat getrotaz() const { return rotaz; }

    bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }

    void swapBuffers() { glfwSwapBuffers(mainWindow); }

    void applyGameplayInput(float dt,
        float liftSpeed, float hoodSpeed, float carMove, float carTurn,
        float& camLift, float& hoodDeg, float& carYawDeg, glm::vec3& carPos);

    // --- NUEVO: toggles para point lights ---
    bool getLampPointOn()  const { return lampPointOn; }   // Lámpara (tecla L)
    bool getFarolPointOn() const { return farolPointOn; }  // Carrocería (tecla K)

    ~Window();

private:
    GLFWwindow* mainWindow;
    GLint width, height;
    bool keys[1024];

    GLint bufferWidth, bufferHeight;

    void createCallbacks();

    GLfloat lastX;
    GLfloat lastY;
    GLfloat xChange;
    GLfloat yChange;

    GLfloat muevex;

    // Rotaciones globales
    GLfloat rotax;
    GLfloat rotay;
    GLfloat rotaz;

    bool mouseFirstMoved;

    // --- NUEVO: estados de encendido para point lights ---
    bool lampPointOn;   // toggle con tecla L
    bool farolPointOn;  // toggle con tecla K

    // Callbacks estáticos de GLFW
    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
