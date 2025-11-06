# 🐆 Práctica 10 – Animación por Keyframes 🎞️

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
Comprender y aplicar la técnica de **animación por cuadros clave (Keyframes)** para controlar el movimiento de un modelo 3D dentro de una escena programada con **OpenGL**, empleando **interpolación lineal** para generar una animación fluida a partir de posiciones y rotaciones registradas en archivos de texto.

### II. Objetivos específicos
- Implementar un **sistema de registro, lectura y reproducción** de animaciones mediante archivos `.txt` que almacenan los *keyframes* del modelo.  
- Comprender el proceso de **interpolación entre cuadros clave** y cómo este genera la ilusión de movimiento continuo.  
- Aplicar transformaciones **(traslación, rotación y escala)** al modelo animado a partir de los datos de cada cuadro.  
- Integrar el sistema de *keyframes* con un modelo 3D existente (en este caso, el **jaguar**) y controlar su desplazamiento mediante entrada de usuario.  
- Lograr la **reproducción visual de una animación personalizada**, como caminar y girar, usando los *keyframes* guardados.  

---

## 📝 Descripción de la práctica
En esta práctica se desarrolla un sistema de **animación por cuadros clave** dentro de una escena 3D programada en **OpenGL**, donde cada *keyframe* representa una posición y orientación del modelo en un instante de tiempo.  

El sistema permite **guardar, editar, borrar y reproducir** animaciones completas, almacenando los valores de traslación y rotación en un archivo externo (`keyframes_jaguar.txt`).  
La animación del **jaguar** se controla mediante entrada de teclado, pudiendo registrar su movimiento (caminar hacia adelante, retroceder, girar) y luego reproducirlo automáticamente por **interpolación lineal** entre cuadros consecutivos.  

El código implementa un esquema modular que incluye:
- Estructuras `FRAME` para almacenar los valores de movimiento y rotación.  
- Funciones de **lectura, escritura y actualización de archivos**.  
- Control de **modo edición y reproducción**.  
- Sistema de **mensajes en consola (LOG)** para depuración de eventos (`[SAVE]`, `[LOAD]`, `[PLAY]`, `[END]`).  


---
