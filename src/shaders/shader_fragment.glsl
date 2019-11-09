#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Correspondentes do material (se tiver)
in vec3 material_diffuse;
in vec3 material_speculate;
in vec3 material_environment;
in float material_specular_exponent;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec3 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// functions declarations
void spherical_mapping(in vec4 center, in vec4 position, out float U, out float V);
vec4 normalize(in vec4 vector, in vec4 min_lenght, in vec4 max_leght);

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2*n * dot(n, l); // o vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    switch (object_id) {
    case SPHERE:
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        spherical_mapping(bbox_center, position_model, U, V);
        Kd = material_diffuse;
        Ks = material_speculate;
        Ka = material_environment;
        q  = material_specular_exponent;
        break;
    }

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0, 1.0, 1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2);

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = Kd * I * max(0, dot(n, l));

    // Termo ambiente
    vec3 ambient_term = Ka * Ia;

    // Termo especular utilizando o modelo de iluminação de Phong
    vec3 phong_specular_term = Ks * I * max(0, pow(dot(r, v), q));

    // Cor final do fragmento calculada com uma combinação dos termos difuso,
    color = lambert_diffuse_term + ambient_term + phong_specular_term;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

void spherical_mapping(in vec4 center, in vec4 position, out float U, out float V) {
    vec4 p_line = center + normalize(position - center);
    vec4 p_vector = p_line - center;
    float phi = asin(p_vector.y);
    float theta = atan(p_vector.x, p_vector.z);

    U = (theta + M_PI) / (2*M_PI);
    V = (phi + M_PI_2) / M_PI;
}

vec4 normalize(in vec4 vector, in vec4 min_lenght, in vec4 max_leght) {
    float x = (vector.x - min_lenght.x) / (max_leght.x - min_lenght.x);
    float y = (vector.y - min_lenght.y) / (max_leght.y - min_lenght.y);
    float z = (vector.z - min_lenght.z) / (max_leght.z - min_lenght.z);
    return vec4(x, y, z, 1.0);
}