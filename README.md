# Práctica 5 – Adaptación y Carga de Modelos

## 🎯 Objetivos de aprendizaje
### I. Objetivo general

Comprender cómo está estructurado un modelo 3D creado en software de modelado para cargarlo correctamente en OpenGL.

Emplear la biblioteca Assimp para incorporar modelos complejos dentro de escenas de OpenGL. 


### II. Objetivos específicos

Configurar Assimp para importar modelos en una escena con OpenGL.

Adaptar el código de la aplicación y de los shaders para soportar geometrías simples y complejas cargadas con Assimp.

Separar, agrupar y adecuar un modelo 3D en el software de modelado para su correcta importación y despliegue en OpenGL. 

---

## 📝 Descripción de la práctica

En esta práctica el alumno integra un modelo propio de coche (carrocería, cofre y cuatro llantas) en un entorno 3D con OpenGL (GLFW, GLEW, GLM) y Assimp, cuidando escala, pivotes y jerarquía. Se implementa la carga de modelos .obj, la composición padre–hijo (carrocería como nodo base; cofre y llantas como hijos), y controles de teclado que sincronizan la rotación compartida de las llantas con la traslación global del vehículo (avance/retroceso). Además, se añade un cofre animado con límite angular, una cámara interactiva (WASD + mouse + elevación) y un skybox con un piso gris para referencia espacial. Al finalizar, el alumno demuestra el flujo completo de la práctica: desde la adaptación del modelo en el DCC (separación y pivotes), hasta la carga con Assimp, la propagación de transformaciones en la jerarquía y la interacción en tiempo real con animaciones dependientes de deltaTime, documentando el resultado con fragmentos de código y capturas de ejecución.
