#include <vector>
#include <cmath>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Material Material_brillante, Material_opaco;

Model Carroceria_M, Cofre_M, Llanta1_M, Llanta2_M, Llanta3_M, Llanta4_M;
Model Rin1_M, Rin2_M, Rin3_M, Rin4_M;
Model OjosCarro_M, Parrilla_M;
Model Lampara_M, Foco_M;

Texture cauchoTex, rinTex, ojosTex, parrillaTex, cofreTex;
Texture pisoTexture, bardaTexture;
Texture lamparaTex, focoTex;

Skybox skybox;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

static void CreateObjects() {
    unsigned int idx[] = { 0,2,1, 1,2,3 };
    GLfloat vtx[] = {
        -10.f, 0.f, -10.f,  0.f,  0.f,   0,1,0,
         10.f, 0.f, -10.f, 10.f,  0.f,   0,1,0,
        -10.f, 0.f,  10.f,  0.f, 10.f,   0,1,0,
         10.f, 0.f,  10.f, 10.f, 10.f,   0,1,0
    };
    Mesh* piso = new Mesh(); piso->CreateMesh(vtx, idx, 32, 6); meshList.push_back(piso); // 0

    // Barda frontal (normal +Z)
    unsigned int wIdx[] = { 0,1,2, 2,3,0 };
    GLfloat wVtx[] = {
        -0.5f,-0.5f,0.f, 0.f,0.f, 0,0,1,
         0.5f,-0.5f,0.f, 1.f,0.f, 0,0,1,
         0.5f, 0.5f,0.f, 1.f,1.f, 0,0,1,
        -0.5f, 0.5f,0.f, 0.f,1.f, 0,0,1
    };
    Mesh* wallFront = new Mesh(); wallFront->CreateMesh(wVtx, wIdx, 32, 6); meshList.push_back(wallFront); // 1

    // Barda trasera (normal -Z)
    unsigned int wbIdx[] = { 0,3,2, 2,1,0 };
    GLfloat wbVtx[] = {
        -0.5f,-0.5f,0.f, 0.f,0.f, 0,0,-1,
        -0.5f, 0.5f,0.f, 0.f,1.f, 0,0,-1,
         0.5f, 0.5f,0.f, 1.f,1.f, 0,0,-1,
         0.5f,-0.5f,0.f, 1.f,0.f, 0,0,-1
    };
    Mesh* wallBack = new Mesh(); wallBack->CreateMesh(wbVtx, wbIdx, 32, 6); meshList.push_back(wallBack); // 2
}

static void CreateShaders() {
    Shader* s = new Shader();
    s->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*s);
    shaderList[0].UseShader();
    glUniform1i(shaderList[0].getTextureLocation(), 0);
}

int main() {
    mainWindow = Window(1366, 768);
    if (mainWindow.Initialise() != 0) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.f, 6.f, 55.f), glm::vec3(0.f, 1.f, 0.f), -90.f, -5.f, 10.f, 1.f);

    Carroceria_M.LoadModel("Models/Carroceria.obj");
    Cofre_M.LoadModel("Models/CofreTex.obj");
    Llanta1_M.LoadModel("Models/Llanta1.obj");
    Llanta2_M.LoadModel("Models/Llanta2.obj");
    Llanta3_M.LoadModel("Models/Llanta3.obj");
    Llanta4_M.LoadModel("Models/Llanta4.obj");
    Rin1_M.LoadModel("Models/Rin1.obj");
    Rin2_M.LoadModel("Models/Rin2.obj");
    Rin3_M.LoadModel("Models/Rin3.obj");
    Rin4_M.LoadModel("Models/Rin4.obj");
    OjosCarro_M.LoadModel("Models/OjosCarro.obj");
    Parrilla_M.LoadModel("Models/Parrilla.obj");

    Lampara_M.LoadModel("Models/Lampara.obj");
    Foco_M.LoadModel("Models/Foco.obj");

    pisoTexture = Texture("Textures/piso.tga");            pisoTexture.LoadTextureA();
    bardaTexture = Texture("Textures/barda.tga");           bardaTexture.LoadTexture();
    cauchoTex = Texture("Textures/TexturaLlanta.png");   cauchoTex.LoadTexture();
    rinTex = Texture("Textures/TexturaRin.png");      rinTex.LoadTexture();
    ojosTex = Texture("Textures/OjosCarro.tga");       ojosTex.LoadTexture();
    parrillaTex = Texture("Textures/Parrilla.tga");        parrillaTex.LoadTexture();
    cofreTex = Texture("Textures/Cofre.tga");           cofreTex.LoadTexture();

    lamparaTex = Texture("Textures/LamparaTex.tga");      lamparaTex.LoadTextureA();
    focoTex = Texture("Textures/Foco.tga");            focoTex.LoadTextureA();

    std::vector<std::string> faces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(faces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    glm::mat4 projection = glm::perspective(glm::radians(60.f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.f);

    float camLift = 0.f; const float LIFT_SPEED = 6.f;

    glm::vec3 carPos(5.f, 5.f, 0.f), prevCarPos = carPos;
    float carYawDeg = 0.f;
    const float CAR_MOVE_SPEED = 10.f, CAR_TURN_SPEED = 60.f;

    const glm::vec3 COFRE_PIV(-0.5f, 2.5f, -8.0f);
    const glm::vec3 HOOD_AXIS(1.f, 0.f, 0.f);
    float hoodDeg = 0.f; const float HOOD_SPEED = 60.f, HOOD_MAX = 45.f;

    const glm::vec3 L1_POS(-9.f, -3.5f, -11.f), L2_POS(-9.f, -3.5f, 11.f);
    const glm::vec3 L3_POS(9.f, -3.5f, -11.f), L4_POS(9.f, -3.5f, 11.f);
    const glm::vec3 WHEEL_AXIS(1.f, 0.f, 0.f);
    float wheelDeg = 0.f; const float WHEEL_RADIUS = 3.f;

    // Spotlights: delantero (azul), trasero (rojo), cofre (ámbar)
    const glm::vec3 HEADLIGHT_OFFSET(0.f, 1.30f, -8.f);
    const glm::vec3 HEADLIGHT_DIR_L = glm::normalize(glm::vec3(0.f, -0.18f, -1.f));
    const glm::vec3 HEADLIGHT_COLOR = glm::vec3(0.25f, 0.45f, 1.00f);

    const glm::vec3 TAILLIGHT_OFFSET(0.f, 1.30f, 8.f);
    const glm::vec3 TAILLIGHT_DIR_L = glm::normalize(glm::vec3(0.f, -0.10f, 1.f));
    const glm::vec3 TAILLIGHT_COLOR = glm::vec3(1.00f, 0.10f, 0.12f);

    const glm::vec3 HOODLIGHT_OFFSET_LOCAL(0.f, 0.25f, -1.2f);
    const glm::vec3 HOODLIGHT_DIR_L = glm::normalize(glm::vec3(0.f, 0.10f, -1.f));
    const glm::vec3 HOODLIGHT_COLOR = glm::vec3(1.00f, 0.78f, 0.22f);

    const float HEADLIGHT_EDGE_COS = (float)cos(glm::radians(9.0f));
    const float TAILLIGHT_EDGE_COS = (float)cos(glm::radians(9.0f));
    const float HOODLIGHT_EDGE_COS = (float)cos(glm::radians(12.0f));

    const float ATTEN_CONST = 1.f, ATTEN_LIN = 0.010f, ATTEN_QUAD = 0.0002f;

    // Direccional
    glm::vec3 dirColor(1.0f);
    glm::vec3 dirDirection = glm::normalize(glm::vec3(0.2f, 1.0f, 0.3f));
    float dirAmbient = 0.08f, dirDiffuse = 0.35f;

    // Lámpara
    const glm::vec3 lampPos(-15.0f, 0.0f, -12.0f);
    const float     LAMP_SCALE = 1.0f;
    const float     LAMP_ROT_Y = 0.0f;
    const glm::vec3 BULB_OFFSET_L(0.0f, 0.0f, 0.0f); // origen local del Foco.obj
    const float     BULB_SCALE = 1.0f;

    // Bardas
    const glm::vec2 WALL_SIZE(100.0f, 60.0f);
    const glm::vec3 WALL_FRONT_POS(5.0f, -1.0f + 0.5f * WALL_SIZE.y, -50.0f);
    const glm::vec3 WALL_BACK_POS(5.0f, -1.0f + 0.5f * WALL_SIZE.y, 50.0f);

    // PointLight buffers
    PointLight plAll[2];      // [0]=lamp, [1]=car
    PointLight plTmp[2];      // buffer a enviar
    unsigned   plCount = 0;

    GLfloat lastTime = 0.f;

    while (!mainWindow.getShouldClose()) {
        GLfloat now = glfwGetTime();
        GLfloat deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        mainWindow.applyGameplayInput(deltaTime, LIFT_SPEED, HOOD_SPEED, CAR_MOVE_SPEED, CAR_TURN_SPEED,
            camLift, hoodDeg, carYawDeg, carPos);
        hoodDeg = glm::clamp(hoodDeg, 0.f, HOOD_MAX);

        // Rodadura
        {
            glm::vec3 d = carPos - prevCarPos;
            float adv = glm::length(d);
            if (adv > 1e-6f) {
                float yawRad = glm::radians(carYawDeg);
                glm::vec3 fwd = glm::normalize(glm::vec3(sin(yawRad), 0.f, -cos(yawRad)));
                float sign = (glm::dot(d, fwd) >= 0.f) ? 1.f : -1.f;
                wheelDeg += sign * (adv / WHEEL_RADIUS) * (180.f / 3.14159265f);
            }
            prevCarPos = carPos;
        }

        glm::mat4 view = camera.calculateViewMatrix();
        view = glm::rotate(glm::mat4(1.f), glm::radians(mainWindow.getrotax()), glm::vec3(1, 0, 0)) * view;
        view = glm::rotate(glm::mat4(1.f), glm::radians(mainWindow.getrotay()), glm::vec3(0, 1, 0)) * view;
        view = glm::rotate(glm::mat4(1.f), glm::radians(mainWindow.getrotaz()), glm::vec3(0, 0, 1)) * view;
        view = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -camLift, 0.f)) * view;

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.DrawSkybox(view, projection);

        shaderList[0].UseShader();
        shaderList[0].SetMatrices(glm::mat4(1.f), view, projection);
        shaderList[0].SetEyePosition(camera.getCameraPosition());

        // Direccional
        glUniform3fv(shaderList[0].GetAmbientcolorLocation(), 1, glm::value_ptr(dirColor));
        glUniform1f(shaderList[0].GetAmbientIntensityLocation(), dirAmbient);
        glUniform1f(shaderList[0].GetDiffuseIntensityLocation(), dirDiffuse);
        glUniform3fv(shaderList[0].GetDirectionLocation(), 1, glm::value_ptr(dirDirection));

        // Transforms del coche
        glm::mat4 Mcar = glm::translate(glm::mat4(1.f), carPos);
        Mcar = glm::rotate(Mcar, glm::radians(carYawDeg), glm::vec3(0, 1, 0));

        // Spotlights del coche/ cofre
        glm::vec3 Fpos = glm::vec3(Mcar * glm::vec4(HEADLIGHT_OFFSET, 1.f));
        glm::vec3 Fdir = glm::normalize(glm::vec3(Mcar * glm::vec4(HEADLIGHT_DIR_L, 0.f)));
        glm::vec3 Rpos = glm::vec3(Mcar * glm::vec4(TAILLIGHT_OFFSET, 1.f));
        glm::vec3 Rdir = glm::normalize(glm::vec3(Mcar * glm::vec4(TAILLIGHT_DIR_L, 0.f)));
        glm::mat4 Mhood = glm::translate(Mcar, COFRE_PIV);
        Mhood = glm::rotate(Mhood, glm::radians(hoodDeg), HOOD_AXIS);
        glm::vec3 Hpos = glm::vec3(Mhood * glm::vec4(HOODLIGHT_OFFSET_LOCAL, 1.f));
        glm::vec3 Hdir = glm::normalize(glm::vec3(Mhood * glm::vec4(HOODLIGHT_DIR_L, 0.f)));

        bool* keys = mainWindow.getsKeys();
        bool movingFwd = keys[GLFW_KEY_UP];
        bool movingBack = keys[GLFW_KEY_DOWN];

        GLint program = 0; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glUniform1i(glGetUniformLocation(program, "spotLightCount"), 3);

        // Spotlight 0: faro azul (solo al avanzar)
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].base.base.color"), 1, glm::value_ptr(HEADLIGHT_COLOR));
        glUniform1f(glGetUniformLocation(program, "spotLights[0].base.base.ambientIntensity"), 0.18f);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].base.base.diffuseIntensity"), movingFwd ? 3.0f : 0.0f);
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].base.position"), 1, glm::value_ptr(Fpos));
        glUniform1f(glGetUniformLocation(program, "spotLights[0].base.constant"), ATTEN_CONST);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].base.linear"), ATTEN_LIN);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].base.exponent"), ATTEN_QUAD);
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].direction"), 1, glm::value_ptr(Fdir));
        glUniform1f(glGetUniformLocation(program, "spotLights[0].edge"), HEADLIGHT_EDGE_COS);

        // Spotlight 1: cofre ámbar
        glUniform3fv(glGetUniformLocation(program, "spotLights[1].base.base.color"), 1, glm::value_ptr(HOODLIGHT_COLOR));
        glUniform1f(glGetUniformLocation(program, "spotLights[1].base.base.ambientIntensity"), 0.12f);
        glUniform1f(glGetUniformLocation(program, "spotLights[1].base.base.diffuseIntensity"), 2.20f);
        glUniform3fv(glGetUniformLocation(program, "spotLights[1].base.position"), 1, glm::value_ptr(Hpos));
        glUniform1f(glGetUniformLocation(program, "spotLights[1].base.constant"), ATTEN_CONST);
        glUniform1f(glGetUniformLocation(program, "spotLights[1].base.linear"), ATTEN_LIN);
        glUniform1f(glGetUniformLocation(program, "spotLights[1].base.exponent"), ATTEN_QUAD);
        glUniform3fv(glGetUniformLocation(program, "spotLights[1].direction"), 1, glm::value_ptr(Hdir));
        glUniform1f(glGetUniformLocation(program, "spotLights[1].edge"), HOODLIGHT_EDGE_COS);

        // Spotlight 2: trasero rojo (solo al retroceder)
        glUniform3fv(glGetUniformLocation(program, "spotLights[2].base.base.color"), 1, glm::value_ptr(TAILLIGHT_COLOR));
        glUniform1f(glGetUniformLocation(program, "spotLights[2].base.base.ambientIntensity"), 0.12f);
        glUniform1f(glGetUniformLocation(program, "spotLights[2].base.base.diffuseIntensity"), movingBack ? 2.20f : 0.0f);
        glUniform3fv(glGetUniformLocation(program, "spotLights[2].base.position"), 1, glm::value_ptr(Rpos));
        glUniform1f(glGetUniformLocation(program, "spotLights[2].base.constant"), ATTEN_CONST);
        glUniform1f(glGetUniformLocation(program, "spotLights[2].base.linear"), ATTEN_LIN);
        glUniform1f(glGetUniformLocation(program, "spotLights[2].base.exponent"), ATTEN_QUAD);
        glUniform3fv(glGetUniformLocation(program, "spotLights[2].direction"), 1, glm::value_ptr(Rdir));
        glUniform1f(glGetUniformLocation(program, "spotLights[2].edge"), TAILLIGHT_EDGE_COS);

        // Piso
        {
            glm::mat4 model(1.f);
            model = glm::translate(model, glm::vec3(0.f, -1.0f, 0.f));
            model = glm::scale(model, glm::vec3(30.f, 1.f, 30.f));
            shaderList[0].SetMatrices(model, view, projection);
            shaderList[0].SetColor(glm::vec3(1.f));
            glActiveTexture(GL_TEXTURE0); pisoTexture.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetMaterial(0.3f, 4.0f);
            meshList[0]->RenderMesh();
            shaderList[0].SetUseTexture(false);
        }

        // Bardas
        {
            glm::mat4 M(1.f);
            M = glm::translate(M, WALL_FRONT_POS);
            M = glm::scale(M, glm::vec3(WALL_SIZE.x, WALL_SIZE.y, 1.f));
            shaderList[0].SetMatrices(M, view, projection);
            shaderList[0].SetColor(glm::vec3(1.f));
            glActiveTexture(GL_TEXTURE0); bardaTexture.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetMaterial(0.6f, 64.0f);
            meshList[1]->RenderMesh();
            shaderList[0].SetUseTexture(false);
        }
        {
            glm::mat4 M(1.f);
            M = glm::translate(M, WALL_BACK_POS);
            M = glm::scale(M, glm::vec3(WALL_SIZE.x, WALL_SIZE.y, 1.f));
            shaderList[0].SetMatrices(M, view, projection);
            shaderList[0].SetColor(glm::vec3(1.f));
            glActiveTexture(GL_TEXTURE0); bardaTexture.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetMaterial(0.6f, 64.0f);
            meshList[2]->RenderMesh();
            shaderList[0].SetUseTexture(false);
        }

        // Lámpara
        glm::mat4 Mlamp(1.0f);
        Mlamp = glm::translate(Mlamp, lampPos);
        Mlamp = glm::rotate(Mlamp, glm::radians(LAMP_ROT_Y), glm::vec3(0, 1, 0));
        Mlamp = glm::scale(Mlamp, glm::vec3(LAMP_SCALE));
        shaderList[0].SetMatrices(Mlamp, view, projection);
        glActiveTexture(GL_TEXTURE0); lamparaTex.UseTexture();
        shaderList[0].SetUseTexture(true);
        shaderList[0].SetColor(glm::vec3(1.f));
        shaderList[0].SetMaterial(0.3f, 4.0f);
        Lampara_M.RenderModel();
        shaderList[0].SetUseTexture(false);

        // Foco (su origen local = (0,0,0) -> posición mundial exacta del bulbo)
        glm::mat4 Mfoco = Mlamp;
        Mfoco = Mfoco * glm::translate(glm::mat4(1.0f), BULB_OFFSET_L);
        Mfoco = Mfoco * glm::scale(glm::mat4(1.0f), glm::vec3(BULB_SCALE));
        shaderList[0].SetMatrices(Mfoco, view, projection);
        glActiveTexture(GL_TEXTURE0); focoTex.UseTexture();
        shaderList[0].SetUseTexture(true);
        shaderList[0].SetColor(glm::vec3(1.f));
        shaderList[0].SetMaterial(4.0f, 256.0f);
        Foco_M.RenderModel();
        shaderList[0].SetUseTexture(false);

        // ========== POINT LIGHTS ==========
        plCount = 0;

        // Lámpara (tecla L)
        if (mainWindow.getLampPointOn()) {
            glm::vec3 bulbWorldPos = glm::vec3(Mfoco * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            plAll[0] = PointLight(
                1.0f, 0.95f, 0.85f,   // color cálido
                0.35f, 1.8f,          // intensidades
                bulbWorldPos.x, bulbWorldPos.y, bulbWorldPos.z,
                1.0f, 0.02f, 0.0005f  // atenuación
            );
            plTmp[plCount++] = plAll[0];
        }

        // Carrocería (tecla K)
        if (mainWindow.getFarolPointOn()) {
            glm::vec3 carPLpos = glm::vec3(Mcar * glm::vec4(0.f, 2.5f, 0.f, 1.0f));
            plAll[1] = PointLight(
                1.00f, 1.00f, 1.00f,
                0.12f, 1.4f,
                carPLpos.x, carPLpos.y, carPLpos.z,
                1.0f, 0.022f, 0.0008f
            );
            plTmp[plCount++] = plAll[1];
        }

        // Importante: NO escribir pointLightCount en otro lado; solo aquí:
        if (plCount > 0) shaderList[0].SetPointLights(plTmp, plCount);
        else             shaderList[0].SetPointLights(nullptr, 0);
        // ==================================

        // Carrocería
        shaderList[0].SetMatrices(Mcar, view, projection);
        shaderList[0].SetUseTexture(false);
        shaderList[0].SetColor(glm::vec3(1.f, 0.f, 0.f));
        shaderList[0].SetMaterial(1.1f, 64.0f);
        Carroceria_M.RenderModel();

        // Cofre
        {
            glm::mat4 Mh = Mhood;
            Mh = glm::rotate(Mh, glm::radians(180.f), glm::vec3(0, 1, 0));
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.f, -1.f);
            glActiveTexture(GL_TEXTURE0); cofreTex.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetColor(glm::vec3(1.f));
            shaderList[0].SetMatrices(Mh, view, projection);
            Cofre_M.RenderModel();
            glDisable(GL_POLYGON_OFFSET_FILL);
            shaderList[0].SetUseTexture(false);
        }

        // Alpha
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        {
            glm::mat4 M = Mcar;
            M = glm::translate(M, glm::vec3(0.f, 0.f, -0.02f));
            glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(-1.f, -1.f);
            glActiveTexture(GL_TEXTURE0); ojosTex.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetColor(glm::vec3(1.f));
            shaderList[0].SetMatrices(M, view, projection);
            OjosCarro_M.RenderModel();
            glDisable(GL_POLYGON_OFFSET_FILL);
            shaderList[0].SetUseTexture(false);
        }
        {
            glm::mat4 M = Mcar;
            M = glm::translate(M, glm::vec3(0.f, 0.f, -0.015f));
            glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(-1.f, -1.f);
            glActiveTexture(GL_TEXTURE0); parrillaTex.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetColor(glm::vec3(1.f));
            shaderList[0].SetMatrices(M, view, projection);
            Parrilla_M.RenderModel();
            glDisable(GL_POLYGON_OFFSET_FILL);
            shaderList[0].SetUseTexture(false);
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        auto drawWheel = [&](const glm::vec3& pos, Model& w, Model& r) {
            glm::mat4 M = Mcar;
            M = glm::translate(M, pos);
            M = glm::rotate(M, glm::radians(wheelDeg), WHEEL_AXIS);

            glActiveTexture(GL_TEXTURE0); cauchoTex.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetColor(glm::vec3(1.f));
            shaderList[0].SetMatrices(M, view, projection);
            w.RenderModel();

            glm::mat4 R = glm::scale(M, glm::vec3(1.05f));
            glActiveTexture(GL_TEXTURE0); rinTex.UseTexture();
            shaderList[0].SetUseTexture(true);
            shaderList[0].SetColor(glm::vec3(1.f));
            shaderList[0].SetMatrices(R, view, projection);
            r.RenderModel();
            shaderList[0].SetUseTexture(false);
        };
        drawWheel(L1_POS, Llanta1_M, Rin1_M);
        drawWheel(L2_POS, Llanta2_M, Rin2_M);
        drawWheel(L3_POS, Llanta3_M, Rin3_M);
        drawWheel(L4_POS, Llanta4_M, Rin4_M);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}