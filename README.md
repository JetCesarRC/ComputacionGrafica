# Práctica 7 – Iluminación 1 💡

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
Comprender e implementar diferentes **fuentes de iluminación en OpenGL y GLSL**, aplicando el **modelo de iluminación de Phong** para generar efectos realistas sobre materiales y modelos tridimensionales.

### II. Objetivos específicos
- Crear e instanciar múltiples fuentes de iluminación (**direccional**, **puntual** y **spotlight**) dentro de OpenGL y GLSL.  
- Definir y ajustar parámetros de iluminación (*ambient*, *diffuse*, *specular*, *atenuación*) para representar materiales con realismo visual.  
- Controlar dinámicamente la posición, dirección y color de las luces en una escena 3D.  
- Aplicar iluminación combinada sobre modelos texturizados importados con la librería **Assimp**.  

---

## 🧠 Fundamento teórico
Esta práctica se basa en el **modelo de iluminación de Phong**, el cual descompone la interacción entre luz y superficie en tres componentes principales:  
1. **Ambiental:** luz indirecta presente en toda la escena.  
2. **Difusa:** luz que depende del ángulo de incidencia entre la fuente y la superficie.  
3. **Especular:** brillo reflejado en dirección del observador.  

Se estudian tres tipos de fuentes de luz implementadas en OpenGL:  
- **Direccional:** simula la luz del sol, con rayos paralelos.  
- **Puntual:** emite luz desde una posición hacia todas las direcciones, con atenuación según la distancia.  
- **Spotlight:** proyecta un haz cónico en una dirección específica, controlando su ángulo y suavizado.  

Las principales bibliotecas utilizadas son:  
**OpenGL 3.3+, GLFW, GLEW, GLM, stb_image y Assimp.**

---

## 📝 Descripción de la práctica
En esta práctica el alumno desarrolla una escena tridimensional en OpenGL con **múltiples fuentes de luz dinámicas**.  
El entorno incluye modelos importados, geometrías básicas y texturas personalizadas creadas en **GIMP** y **Blender**.  
Se aplican técnicas de iluminación realista mediante **shaders (GLSL)** y materiales configurables.

### Actividades principales:
1. **Configuración del entorno y modelos:**  
   - Importar los modelos `uh60.obj` (helicóptero), `Lampara.obj` y `Foco.obj`.  
   - Aplicar texturas personalizadas (`LamparaTex.tga`, `Foco.tga`, `Agave.tga`).  

2. **Implementación de fuentes de luz:**  
   - **Luz direccional:** simula la iluminación ambiental general.  
   - **Luz puntual:** asignada a la lámpara, con color cálido y atenuación realista.  
   - **Spotlight del helicóptero:** de color amarillo, sigue la posición del modelo y apunta al suelo.  
   - **Spotlight de cámara:** simula una linterna ligada a la vista del observador.  

3. **Control de movimiento:**  
   - Movimiento del helicóptero con las teclas **↑ (adelante)** y **↓ (atrás)**.  
   - Control de elevación de la cámara con **Z/X**.  

4. **Integración visual:**  
   - Renderizado del escenario con **piso texturizado**, **vegetación con transparencia (Agave)** y **skybox**.  
   - Aplicación del modelo de **Phong** a todos los objetos.  
   - Ajuste de parámetros de **atenuación y brillo especular**.
