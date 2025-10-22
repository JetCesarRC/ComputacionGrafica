#include "Window.h"

Window::Window()
{
    width = 800;
    height = 600;

    for (size_t i = 0; i < 1024; i++) keys[i] = false;

    muevex = 0.0f;

    lastX = 0.0f;
    lastY = 0.0f;
    xChange = 0.0f;
    yChange = 0.0f;
    mouseFirstMoved = true;

    rotax = rotay = rotaz = 0.0f;

    // Estados iniciales de point lights (encendidas por defecto)
    lampPointOn = true;   // L
    farolPointOn = true;   // K

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

    rotax = rotay = rotaz = 0.0f;

    // Estados iniciales de point lights (encendidas por defecto)
    lampPointOn = true;   // L
    farolPointOn = true;   // K

    mainWindow = nullptr;
    bufferWidth = bufferHeight = 0;
}

int Window::Initialise()
{
    if (!glfwInit())
    {
        printf("Fallo inicializar GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    mainWindow = glfwCreateWindow(width, height, "PracticaXX:Nombre de la practica", NULL, NULL);
    if (!mainWindow)
    {
        printf("Fallo en crearse la ventana con GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(mainWindow);

    createCallbacks();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Fallo inicializacion de GLEW\n");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, bufferWidth, bufferHeight);
    glfwSetWindowUserPointer(mainWindow, this);

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

void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!theWindow) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Ejemplo simple de variable auxiliar (no afecta gameplay)
    if ((key == GLFW_KEY_Y) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        theWindow->muevex += 1.0f;
    if ((key == GLFW_KEY_U) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        theWindow->muevex -= 1.0f;

    // --- NUEVO: toggles de point lights con flanco de tecla ---
    if (key == GLFW_KEY_L && action == GLFW_PRESS)  // Lámpara
        theWindow->lampPointOn = !theWindow->lampPointOn;

    if (key == GLFW_KEY_K && action == GLFW_PRESS)  // Carrocería
        theWindow->farolPointOn = !theWindow->farolPointOn;

    // Registrar estado de teclas
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

void Window::applyGameplayInput(float dt,
    float liftSpeed, float hoodSpeed, float carMove, float carTurn,
    float& camLift, float& hoodDeg, float& carYawDeg, glm::vec3& carPos)
{
    // Elevación de cámara
    if (keys[GLFW_KEY_Z]) camLift += liftSpeed * dt;
    if (keys[GLFW_KEY_X]) camLift -= liftSpeed * dt;

    // Giro del vehículo (yaw sobre Y)
    if (keys[GLFW_KEY_LEFT])  carYawDeg += carTurn * dt;
    if (keys[GLFW_KEY_RIGHT]) carYawDeg -= carTurn * dt;

    // Dirección adelante/atrás
    float yawRad = glm::radians(carYawDeg);
    glm::vec3 fwd = glm::normalize(glm::vec3(sin(yawRad), 0.0f, -cos(yawRad)));

    if (keys[GLFW_KEY_UP])   carPos += fwd * (carMove * dt);
    if (keys[GLFW_KEY_DOWN]) carPos -= fwd * (carMove * dt);

    // Cofre (abrir/cerrar)
    float s = hoodSpeed * dt;
    if (keys[GLFW_KEY_V]) hoodDeg += s;
    if (keys[GLFW_KEY_B]) hoodDeg -= s;
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