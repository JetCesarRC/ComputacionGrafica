#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();

	GLfloat getBufferWidth() { return (GLfloat)bufferWidth; }
	GLfloat getBufferHeight() { return (GLfloat)bufferHeight; }

	GLfloat getXChange();
	GLfloat getYChange();

	GLfloat getmuevex() { return muevex; }
	GLfloat getangulocola() { return angulocola; }

	//Getters para rotaciones globales
	GLfloat getrotax() { return rotax; }
	GLfloat getrotay() { return rotay; }
	GLfloat getrotaz() { return rotaz; }

	bool getShouldClose() {
		return glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	~Window();

private:
	GLFWwindow* mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;

	void createCallbacks();

	GLfloat lastX, lastY;
	GLfloat xChange, yChange;
	GLfloat muevex;
	GLfloat angulocola = 0.0f;

	//Variables para rotaciones acumuladas
	GLfloat rotax = 0.0f;
	GLfloat rotay = 0.0f;
	GLfloat rotaz = 0.0f;

	bool mouseFirstMoved = true;

	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
