#pragma once

#include <cstdio>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Clase Window: ventana, entrada de teclado/mouse y callbacks GLFW
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

    bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }

    void swapBuffers() { glfwSwapBuffers(mainWindow); }

    void applyP7Input(float dt,
        float camLiftSpeed, float heliSpeed,
        float& camLift, glm::vec3& heliPos);

    ~Window();

private:
    void createCallbacks();

    // Callbacks estáticos de GLFW
    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

private:
    GLFWwindow* mainWindow;
    GLint width, height;
    bool keys[1024];

    GLint bufferWidth, bufferHeight;

    GLfloat lastX;
    GLfloat lastY;
    GLfloat xChange;
    GLfloat yChange;

    // Valor fijo para no romper el código existente que lo lee
    GLfloat muevex;

    bool mouseFirstMoved;
};