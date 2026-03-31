#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	muevex = 2.0f;
	rotax = rotay = rotaz = 0.0f;

	for (size_t i = 0; i < 1024; i++)
		keys[i] = 0;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	muevex = 2.0f;
	rotax = rotay = rotaz = 0.0f;

	for (size_t i = 0; i < 1024; i++)
		keys[i] = 0;
}

int Window::Initialise()
{
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	mainWindow = glfwCreateWindow(width, height, "Practica XX: Nombre de la practica", NULL, NULL);
	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}

	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);

	createCallbacks();
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this);

	return 0; // IMPORTANTE
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

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Movimiento ejemplo
	if (key == GLFW_KEY_Y) theWindow->muevex += 1.0f;
	if (key == GLFW_KEY_U) theWindow->muevex -= 1.0f;

	// Rotaciones globales (E/R/T)
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		theWindow->rotax += 5.0f;
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		theWindow->rotay += 5.0f;
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		theWindow->rotaz += 5.0f;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS) theWindow->keys[key] = true;
		else if (action == GLFW_RELEASE) theWindow->keys[key] = false;
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = (GLfloat)xPos;
		theWindow->lastY = (GLfloat)yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = (GLfloat)(xPos - theWindow->lastX);
	theWindow->yChange = (GLfloat)(theWindow->lastY - yPos);

	theWindow->lastX = (GLfloat)xPos;
	theWindow->lastY = (GLfloat)yPos;
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
