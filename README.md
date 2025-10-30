# Práctica 9 – Modelado jerárquico + Iluminación 2 💡

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
Aplicar **modelado jerárquico y control interactivo** en una escena 3D con OpenGL/GLSL, integrando **iluminación (Phong)** y **texturizado** para lograr animaciones coherentes (puertas y letrero) y un **dado de 8 caras** con física básica y resultado aleatorio reproducible por entrada de usuario.

### II. Objetivos específicos
- Separar y manipular de forma independiente los subcomponentes del arco: **panel del letrero**, **puerta izquierda** y **puerta derecha**.  
- Implementar un **letrero tipo LED/LCD** que desplaza cíclicamente el texto **“PROYECTO CGEIHC”** de derecha a izquierda mediante **offset UV**.  
- Programar dos mecanismos de apertura: **rotación** para la puerta izquierda y **deslizamiento** para la puerta derecha, con **interpolación suave** y **tecla de activación**.  
- Construir un **dado octaédrico texturizado**, con **normales por cara** y **física simple** (caída, rebote, amortiguación), ajustando su **orientación final** para que el **número aleatorio** quede hacia arriba; **repetir tirada** con tecla.  
- Integrar **iluminación direccional** y **spotlight ligado a cámara**, verificando la **respuesta especular/difusa** en el piso y en el dado texturizado.  

---

## 📝 Descripción de la práctica
En esta práctica se desarrolla una escena con **un arco Torii** dividido en submallas funcionales (**panel del letrero** y **dos puertas**) y un **dado de 8 caras** que **cae, rebota y se estabiliza** mostrando un valor aleatorio hacia arriba. La **cámara libre** controla un **spotlight (linterna)** que ilumina el piso y el dado, complementado por una **luz direccional**. El letrero superior desplaza el mensaje **“PROYECTO CGEIHC”** de manera **cíclica** usando un **atlas** y **offset UV** controlado por tiempo. Las puertas se animan bajo un **pequeño autómata de estados**: la **izquierda** abre/cierra con **rotación** sobre su bisagra; la **derecha** lo hace con **traslación** sobre un vector de deslizamiento. El dado octaédrico usa **normales planas por cara** para resaltar correctamente el **Phong** bajo la linterna; su **textura** se ajusta para evitar inversión (espejo) y su **orientación final** se interpola con **slerp** para que la cara objetivo quede hacia +Y; la tirada se **reinicia con la tecla** indicada.

### Actividades principales:
1. **Estructura de escena y recursos**  
   - Cargar modelos (Torii y subpartes), **skybox**, piso y texturas.  
   - Configurar **materiales** (opaco/brillante) y **shaders** de iluminación (Phong).  

2. **Letrero tipo LED/LCD**  
   - Construir un **quad por carácter** con atlas 5×3 y **mapeo UV** por celda.  
   - Animar el **scroll horizontal** del texto mediante **offset** y módulo de longitud.  

3. **Puertas con dos mecanismos**  
   - **Izquierda (rotación):** transformar respecto a la **bisagra** y aplicar **ease** para abrir/cerrar.  
   - **Derecha (deslizamiento):** trasladar a lo largo del **vector de guía**, sincronizada con la misma tecla.  

4. **Dado octaédrico con física e iluminación**  
   - Definir malla con **normales por cara**; corregir **U** para evitar espejo en numeración.  
   - Simular **caída y rebote** (gravedad, amortiguación) y **asentamiento** hacia la **cara objetivo** usando **slerp**.  
   - Iluminar con **luz direccional** y **spotlight de cámara**; validar brillo difuso/especular en piso y dado.  

5. **Interacción**  
   - **Espacio**: repetir tirada del dado. **K**: alternar apertura/cierre de las puertas. **WASD + mouse**: cámara libre.
