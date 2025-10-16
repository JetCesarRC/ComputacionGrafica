#include "Window.h"

Window::Window()
{
    width = 800;
    height = 600;

    for (size_t i = 0; i < 1024; i++) keys[i] = false;

    muevex = 2.0f;

    lastX = 0.0f;
    lastY = 0.0f;
    xChange = 0.0f;
    yChange = 0.0f;
    mouseFirstMoved = true;

    mainWindow = nullptr;
    bufferWidth = bufferHeight = 0;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
    width = windowWidth;
    height = windowHeight;

    for (size_t i = 0; i < 1024; i++) keys[i] = false;

    muevex = 2.0f;

    lastX = 0.0f;
    lastY = 0.0f;
    xChange = 0.0f;
    yChange = 0.0f;
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

    mainWindow = glfwCreateWindow(width, height, "Practica 7 - Iluminacion 1", nullptr, nullptr);
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

// --- Teclado ---
void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!theWindow) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)   theWindow->keys[key] = true;
        if (action == GLFW_RELEASE) theWindow->keys[key] = false;
    }
}

// --- Mouse ---
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

void Window::applyP7Input(float dt,
    float camLiftSpeed, float heliSpeed,
    float& camLift, glm::vec3& heliPos)
{
    // Cámara: subir/bajar
    if (keys[GLFW_KEY_Z]) camLift += camLiftSpeed * dt;
    if (keys[GLFW_KEY_X]) camLift -= camLiftSpeed * dt;

    // Helicóptero: avanzar/retroceder en eje Z mundial
    if (keys[GLFW_KEY_UP]) heliPos.x -= heliSpeed * dt; // adelante (hacia -X)
    if (keys[GLFW_KEY_DOWN]) heliPos.x += heliSpeed * dt; // atrás (hacia +X)
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
