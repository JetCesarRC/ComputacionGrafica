# Práctica 2 – Proyecciones y Transformaciones en OpenGL  

## 🎯 Objetivos de aprendizaje  

### I. Objetivo general  
El alumno empleará el **pipeline de renderizado** para definir proyecciones en **perspectiva** y **ortográfica**, delimitará su espacio de trabajo (**frustum**) y aplicará **transformaciones homogéneas** para modificar dinámicamente elementos geométricos en pantalla.  

### II. Objetivos específicos  
- Conocer las estructuras de datos de la biblioteca de matemáticas de **OpenGL (GLM)** para construir la **matriz de proyección** y la **matriz de modelo**.  
- Transformar elementos de la escena dinámicamente mediante **callbacks** de ratón y teclado.  
- Aplicar **traslaciones, rotaciones y escalados** a geometrías en un espacio tridimensional.  
- Emplear **mallas indexadas** (cubos y pirámides) y mallas con **color por vértice** (letras).  
- Implementar **shaders** para color fijo (rojo, verde, azul, café, verde oscuro) y shaders con color por vértice.  
- Utilizar la prueba de profundidad (**GL_DEPTH_TEST**) para un renderizado correcto en 3D.  

---

## 📝 Descripción de la práctica  
La práctica consistió en implementar un programa en **C++ con OpenGL**, utilizando las librerías **GLFW**, **GLEW** y **GLM** para la gestión del contexto gráfico y operaciones matemáticas.  

Se cumplieron los siguientes requerimientos:  
1. Dibujar las **iniciales del nombre del alumno**, cada una con un **color diferente**, empleando un shader con atributos de **color por vértice**.  
2. Generar una **casa 3D** formada únicamente a partir de **cubos y pirámides** (cuerpo, techo, puerta, ventanas y árboles).  
3. Implementar **fragment shaders de color fijo** para representar los diferentes elementos de la casa en colores rojo, verde, azul, café y verde oscuro.  
4. Representar la escena con **proyección en perspectiva** y aplicar **transformaciones homogéneas** para posicionar, escalar y rotar dinámicamente los objetos.  

