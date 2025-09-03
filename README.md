#Práctica 2 – Proyecciones y puertos de vista. Transformaciones geométricas

##🎯 Objetivos de aprendizaje
###I. Objetivo general
El alumno empleará el pipeline de renderizado para definir proyecciones en perspectiva y ortográfica, delimitará su frustum de visualización y aplicará transformaciones homogéneas para modificar dinámicamente elementos geométricos mediante entradas de teclado y ratón.

###II. Objetivos específicos

Construir la matriz de proyección y la matriz de modelo utilizando GLM.

Aplicar traslación, rotación y escala a geometrías simples en 3D.

Gestionar mallas indexadas (cubos, pirámides) y mallas con color por vértice (letras).

Cargar y utilizar shaders para color fijo (rojo, verde, azul, café, verde oscuro) y para color por vértice.

Habilitar y aprovechar GL_DEPTH_TEST para el manejo correcto de la profundidad.

##📝 Descripción de la práctica
La práctica consiste en implementar una escena en C++/OpenGL (GLFW, GLEW, GLM) que muestre:

Las iniciales del alumno, cada una con un color diferente, renderizadas con un shader que recibe atributos de color por vértice.

Una casa 3D construida exclusivamente a partir de cubos y pirámides (cuerpo, techo, puerta, ventanas y árboles), coloreada mediante fragment shaders de color fijo (rojo, verde, azul, café y verde oscuro).
La escena utiliza proyección en perspectiva, prueba de profundidad y transformaciones homogéneas para posicionar, escalar y, opcionalmente, animar los objetos (por ejemplo, una rotación suave para apreciar el volumen). Se sugiere incorporar callbacks de teclado/ratón para ajustar parámetros en tiempo real.
