# Práctica 4 - Modelado Jerárquico

## 🎯 Objetivos de aprendizaje
### I. Objetivo general

El alumno aplicará modelado jerárquico para construir y animar modelos 3D a partir de primitivas, configurará una cámara sintética y utilizará el pipeline de renderizado para propagar transformaciones en una estructura padre–hijo con articulaciones controladas por teclado.

### II. Objetivos específicos

Descomponer modelos complejos en submódulos y definir su árbol jerárquico (padre–hijo y pivotes).

Instanciar y parametrizar primitivas (cubo, cilindro, cono, pirámide y esfera) para formar partes reutilizables.

Encadenar transformaciones homogéneas (traslación, rotación, escala) usando marcos intermedios para heredar/recuperar referencia.

Controlar grados de libertad con teclas y respetar límites angulares por articulación.

Configurar la cámara virtual y mantener GL_DEPTH_TEST habilitado para un render 3D correcto.

Documentar con fragmentos de código y capturas de ejecución.

--- 

## 📝 Descripción de la práctica

La práctica consiste en implementar, en C++ con OpenGL (usando GLFW, GLEW y GLM), dos modelos jerárquicos: una grúa articulada con base piramidal y ruedas independientes, y un animal robot con cuatro patas (2 DOF cada una) y dos orejas articuladas. Se deberán instanciar primitivas, definir pivotes, y encadenar transformaciones para que los movimientos se propaguen correctamente a lo largo de la jerarquía. Las articulaciones se controlan por teclado con límites de seguridad (p. ej., caderas ±35°, rodillas 0–85°, orejas ±30°). Como evidencia, se incluyen fragmentos de código clave y capturas de la ejecución mostrando distintas poses y configuraciones.
