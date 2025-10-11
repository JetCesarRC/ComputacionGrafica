# Práctica 6 – Texturizado

## 🎯 Objetivos de aprendizaje

### I. Objetivo general
Comprender el proceso completo de creación, preparación e implementación de texturas en OpenGL, desde la edición de imágenes hasta su mapeo sobre geometrías y modelos tridimensionales.

### II. Objetivos específicos
- Utilizar programas de edición de imágenes (como **GIMP**) para escalar, modificar canales de color y exportar imágenes adecuadas para texturizado.  
- Emplear una **librería de carga de imágenes** (por ejemplo, *stb_image*) para importar y asociar texturas en OpenGL.  
- Integrar coordenadas de textura (UV) con la información geométrica de objetos y modelos 3D.  
- Comprender la relación entre los parámetros de mapeo, materiales y su visualización en un entorno tridimensional.  

---

## 📝 Descripción de la práctica
En esta práctica el alumno desarrolla un entorno 3D en OpenGL (GLFW, GLEW, GLM, stb_image) para implementar **texturizado sobre objetos y modelos tridimensionales**.  
Se parte de la base de la práctica anterior (carga de modelos con Assimp) y se incorporan **texturas creadas y editadas en software externo**, aplicadas tanto a geometrías generadas por código como a modelos importados.

### Actividades principales:
1. **Edición de imágenes:**  
   - Modificar archivos de imagen para crear texturas (ajuste de tamaño, color y transparencia).  
   - Exportar las imágenes a formatos compatibles (TGA, PNG, JPG).  

2. **Texturizado en 3ds Max / Blender:**  
   - Asignar materiales y realizar el *unwrap UVW* en un modelo (ejemplo: cubo o coche).  
   - Mapear correctamente las texturas sobre la superficie y exportar con su archivo `.mtl`.  

3. **Implementación en OpenGL:**  
   - Cargar texturas con *stb_image* y asignarlas mediante coordenadas UV.  
   - Aplicar texturas a geometrías básicas (piso, cubo, octaedro).  
   - Texturizar modelos importados (carrocería, cofre, parrilla y ojos del vehículo).  
   - Añadir un skybox como fondo de la escena.

4. **Ejercicio integrador:**  
   - Crear un **dado de 8 caras (octaedro)** correctamente texturizado.  
   - Texturizar el **modelo de coche** con detalles personalizados en el cofre, parrilla y ojos.  
   - Implementar efectos visuales que resalten las texturas (por ejemplo, desplazamiento leve para destacar los detalles).  
