# Práctica 3 - Modelado Geométrico

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
El alumno aplicará **modelado geométrico** para construir objetos 3D a partir de **primitivas**, configurará una **cámara sintética** y utilizará el **pipeline de renderizado** para visualizar y transformar la escena.

### II. Objetivos específicos
- **Descomponer** modelos complejos en **elementos primitivos** (prisma rectangular, cubo, cilindro, cono y esfera).
- **Instanciar** y **parametrizar** sub-primitivas para conformar un modelo compuesto.
- **Configurar la cámara virtual** (posición, dirección de vista y vector up) para observar la escena.
- Aplicar **transformaciones homogéneas** (traslación, rotación y escala) a los elementos.
- Asignar **color por uniform** en los shaders y mantener **GL_DEPTH_TEST** habilitado para un render 3D correcto.

---

## 📝 Descripción de la práctica
La práctica consiste en implementar, en **C++ con OpenGL** (usando **GLFW**, **GLEW** y **GLM**), la construcción de **primitivas 3D** y su **instanciación** para formar un **modelo complejo** con al menos **seis** elementos. Se deberán aplicar **transformaciones** para posicionar, orientar y escalar cada parte; configurar una **cámara sintética** para la visualización; y utilizar una **variable uniform** para colorear las instancias. Como evidencia, se documentan los fragmentos de código clave y capturas de ejecución; y como anexo se incluye la **pirámide de Rubik (Pyraminx)** construida a partir de primitivas.
