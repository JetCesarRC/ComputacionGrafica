#include "Window.h"
#include <cstdio>

Window::Window()
{
    width = 800;  height = 600;
    for (size_t i = 0; i < 1024; ++i) keys[i] = false;

    lastX = lastY = 0.0f;
    xChange = yChange = 0.0f;
    mouseFirstMoved = true;

    mainWindow = nullptr;
    bufferWidth = bufferHeight = 0;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
    width = windowWidth;  height = windowHeight;
    for (size_t i = 0; i < 1024; ++i) keys[i] = false;

    lastX = lastY = 0.0f;
    xChange = yChange = 0.0f;
    mouseFirstMoved = true;

    mainWindow = nullptr;
    bufferWidth = bufferHeight = 0;
}

int Window::Initialise()
{
    if (!glfwInit())
    {
        std::printf("Fallo inicializar GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    mainWindow = glfwCreateWindow(width, height, "Practica9", nullptr, nullptr);
    if (!mainWindow)
    {
        std::printf("Fallo al crear la ventana GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(mainWindow);

    createCallbacks();
    glfwSetWindowUserPointer(mainWindow, this);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::printf("Fallo inicializacion de GLEW\n");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, bufferWidth, bufferHeight);

    return 0;
}

void Window::createCallbacks()
{
    glfwSetKeyCallback(mainWindow, ManejaTeclado);
    glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
    GLfloat theChange = xChange;
    xChange = 0.0f;
    return theChange;
}

GLfloat Window::getYChange()
{
    GLfloat theChange = yChange;
    yChange = 0.0f;
    return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int /*code*/, int action, int /*mode*/)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!theWindow) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)   theWindow->keys[key] = true;
        if (action == GLFW_RELEASE) theWindow->keys[key] = false;
    }
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!theWindow) return;

    if (theWindow->mouseFirstMoved)
    {
        theWindow->lastX = static_cast<GLfloat>(xPos);
        theWindow->lastY = static_cast<GLfloat>(yPos);
        theWindow->mouseFirstMoved = false;
    }

    theWindow->xChange = static_cast<GLfloat>(xPos) - theWindow->lastX;
    theWindow->yChange = theWindow->lastY - static_cast<GLfloat>(yPos);

    theWindow->lastX = static_cast<GLfloat>(xPos);
    theWindow->lastY = static_cast<GLfloat>(yPos);
}

Window::~Window()
{
    if (mainWindow)
    {
        glfwDestroyWindow(mainWindow);
        mainWindow = nullptr;
    }
    glfwTerminate();
}
