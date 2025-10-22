# Práctica 8 – Iluminación 2 💡

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
- Comprender y utilizar **materiales** dentro de OpenGL/GLSL y continuar con la implementación de un **modelo de iluminación** (Phong) para escenas 3D.

### II. Objetivos específicos
- Crear materiales mediante parámetros de iluminación (ambient, diffuse, specular) que den **acabado realista** a objetos y texturas.  
- **Declarar y gestionar más de 8 luces** en OpenGL/GLSL (encender/apagar por banderas y teclas), considerando atenuación y costos.  
- Reconocer la **importancia del cálculo de normales** para el modelo de Phong y su efecto visual en las superficies.  

---

## 📝 Descripción de la práctica (lo implementado)
- **Escena** con:
  - **Vehículo jerárquico** (carrocería, cofre con bisagra, llantas con rodadura física).
  - **Lámpara + Foco** (modelo `Lampara.obj` y `Foco.obj`) con **luz puntual** cálida ligada al foco.
  - **Piso** texturizado y dos **bardas** (frontal y trasera) para visualizar claramente los haces de luz.
  - **Skybox**.
- **Luces**:
  - **Direccional** (ambient/difuso moderados).
  - **Spotlights del coche**:
    - **Frontal (azul)**, apuntando levemente hacia abajo y hacia adelante.
    - **Trasera (roja)**, apuntando hacia atrás.  
    - **Cofre (ámbar)**, acoplada al ángulo de apertura del cofre.
  - **Puntuales**:
    - **Foco de la lámpara** (cálida), **ON/OFF con “L”**.
    - **Carrocería del auto** (blanca), **ON/OFF con “K”** (a modo de luz de presencia).
- **Comportamiento**:
  - El **spotlight frontal** se sitúa un poco **más alto** que la defensa.
  - El **spotlight trasero** ilumina la barda posterior para pruebas al **retroceder**.
  - La **luz del cofre** rota jerárquicamente con la tapa (ámbar, no blanca/azul).
  - Las llantas rotan físicamente por **distancia recorrida** (`ángulo = distancia / radio`).
