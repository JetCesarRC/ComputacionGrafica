#version 330

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 4;

struct Light
{
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight 
{
    Light base;
    vec3 direction;
};

struct PointLight
{
    Light base;
    vec3 position;
    float constant;
    float linear;
    float exponent;
};

struct SpotLight
{
    PointLight base;
    vec3 direction;
    float edge;
};

struct Material
{
    float specularIntensity;
    float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;
uniform int useTexture;      // <-- NUEVO
uniform Material material;

uniform vec3 eyePosition;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(-direction); // direction apunta "desde la luz hacia el frag" en nuestro uso
    float diff = max(dot(N, L), 0.0);

    vec4 ambient  = vec4(light.color, 1.0) * light.ambientIntensity;
    vec4 diffuse  = vec4(light.color * light.diffuseIntensity * diff, 1.0);

    vec4 specular = vec4(0.0);
    if (diff > 0.0)
    {
        vec3 V = normalize(eyePosition - FragPos);
        vec3 R = reflect(-L, N);
        float spec = max(dot(V, R), 0.0);
        spec = pow(spec, material.shininess);
        specular = vec4(light.color * material.specularIntensity * spec, 1.0);
    }
    return ambient + diffuse + specular;
}

vec4 CalcDirectionalLight()
{
    // OJO: en este shader "directionalLight.direction" se toma como vector
    return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(PointLight pLight)
{
    vec3 dir = FragPos - pLight.position;  // del punto de luz al frag
    float distance = length(dir);
    dir = normalize(dir);

    vec4 c = CalcLightByDirection(pLight.base, dir);
    float att = pLight.exponent * distance * distance +
                pLight.linear * distance +
                pLight.constant;
    return c / max(att, 0.0001);
}

vec4 CalcSpotLight(SpotLight sLight)
{
    vec3 rayDir = normalize(FragPos - sLight.base.position);
    float spotFactor = dot(rayDir, normalize(sLight.direction)); // compara direcciones
    if (spotFactor > sLight.edge)
    {
        vec4 c = CalcPointLight(sLight.base);
        // Suavizado lineal en el borde
        float intensity = 1.0 - (1.0 - spotFactor) * (1.0 / max(1.0 - sLight.edge, 0.0001));
        return c * intensity;
    }
    return vec4(0.0);
}

vec4 CalcPointLights()
{
    vec4 total = vec4(0.0);
    for (int i = 0; i < pointLightCount; i++)
        total += CalcPointLight(pointLights[i]);
    return total;
}

vec4 CalcSpotLights()
{
    vec4 total = vec4(0.0);
    for (int i = 0; i < spotLightCount; i++)
        total += CalcSpotLight(spotLights[i]);
    return total;
}

void main()
{
    // Luz total
    vec4 finalcolor = CalcDirectionalLight() + CalcPointLights() + CalcSpotLights();

    // Base (textura o color)
    vec4 base = (useTexture == 1) ? (texture(theTexture, TexCoord) * vColor) : vColor;

    // Resultado
    color = base * finalcolor;

    // Si por alguna razon finalcolor fuera 0 (todas las luces OFF), evita negro absoluto:
    // color = max(color, 0.05 * base);
}
