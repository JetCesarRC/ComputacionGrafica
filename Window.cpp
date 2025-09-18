#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;

	rueda1 = rueda2 = rueda3 = rueda4 = 0.0f;
	rotax = rotay = rotaz = 0.0f;
	articulacion1 = articulacion2 = articulacion3 = 0.0f;
	articulacion4 = articulacion5 = articulacion6 = 0.0f;

	bufferWidth = bufferHeight = 0;
	lastX = lastY = 0.0f;
	xChange = yChange = 0.0f;
	mouseFirstMoved = true;

	for (size_t i = 0; i < 1024; i++) keys[i] = 0;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	rueda1 = rueda2 = rueda3 = rueda4 = 0.0f;
	rotax = rotay = rotaz = 0.0f;
	articulacion1 = articulacion2 = articulacion3 = 0.0f;
	articulacion4 = articulacion5 = articulacion6 = 0.0f;

	bufferWidth = bufferHeight = 0;
	lastX = lastY = 0.0f;
	xChange = yChange = 0.0f;
	mouseFirstMoved = true;

	for (size_t i = 0; i < 1024; i++) keys[i] = 0;
}

int Window::Initialise()
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	mainWindow = glfwCreateWindow(width, height, "Practica XX: Nombre de la práctica", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamaño de Buffer
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);
	glfwSwapInterval(1);

	//MANEJAR TECLADO y MOUSE
	createCallbacks();


	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); //HABILITAR BUFFER DE PROFUNDIDAD
							 // Asignar valores de la ventana y coordenadas
							 
							 //Asignar Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);
	//Callback para detectar que se está usando la ventana
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

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Rotaciones globales: solo incrementan en GLFW_PRESS
	if (key == GLFW_KEY_E && action == GLFW_PRESS) theWindow->rotax += 10.0f;
	if (key == GLFW_KEY_R && action == GLFW_PRESS) theWindow->rotay += 10.0f;
	if (key == GLFW_KEY_T && action == GLFW_PRESS) theWindow->rotaz += 10.0f;

	if (key == GLFW_KEY_F) { theWindow->articulacion1 += 10.0f; }
	if (key == GLFW_KEY_G) { theWindow->articulacion2 += 10.0f; }
	if (key == GLFW_KEY_H) { theWindow->articulacion3 += 10.0f; }

	// J/K con tope de yaw ±70°
	if (key == GLFW_KEY_J) {
		float yaw = theWindow->articulacion5 - theWindow->articulacion4;
		if (yaw > -70.0f) {
			theWindow->articulacion4 += 10.0f; // izquierda
		}
	}
	if (key == GLFW_KEY_K) {
		float yaw = theWindow->articulacion5 - theWindow->articulacion4;
		if (yaw < 70.0f) {
			theWindow->articulacion5 += 10.0f; // derecha
		}
	}

	if (key == GLFW_KEY_L) { theWindow->articulacion6 += 10.0f; }

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		const char* key_name = glfwGetKeyName(GLFW_KEY_D, 0);
		//printf("se presiono la tecla: %s\n",key_name);
	}

	// Ruedas: +10° (U/I/O/P) y -10° (Z/X/C/V)
	if (key == GLFW_KEY_U && action == GLFW_PRESS) theWindow->rueda1 += 10.0f;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) theWindow->rueda1 -= 10.0f;

	if (key == GLFW_KEY_I && action == GLFW_PRESS) theWindow->rueda2 += 10.0f;
	if (key == GLFW_KEY_X && action == GLFW_PRESS) theWindow->rueda2 -= 10.0f;

	if (key == GLFW_KEY_O && action == GLFW_PRESS) theWindow->rueda3 += 10.0f;
	if (key == GLFW_KEY_C && action == GLFW_PRESS) theWindow->rueda3 -= 10.0f;

	if (key == GLFW_KEY_P && action == GLFW_PRESS) theWindow->rueda4 += 10.0f;
	if (key == GLFW_KEY_V && action == GLFW_PRESS) theWindow->rueda4 -= 10.0f;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
			//printf("se presiono la tecla %d'\n", key);
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
			//printf("se solto la tecla %d'\n", key);
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();

}
