#version 330 core

// Atributos de v�rtice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a fun��o BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;
// material
layout (location = 3) in vec3 material_diffuse_coefficients;
layout (location = 4) in vec3 material_speculate_coefficients;
layout (location = 5) in vec3 material_environment_coefficients;
layout (location = 6) in float material_specular_exponent_coefficients;

// Matrizes computadas no c�digo C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto est� sendo desenhado no momento
#define SPACESHIP 0
#define ASTEROID  1
#define BULLET    2
uniform int object_id;

// Par�metros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Vari�veis para acesso das imagens de textura
uniform sampler2D TextureImage0;

// Atributos de v�rtice que ser�o gerados como sa�da ("out") pelo Vertex Shader.
// ** Estes ser�o interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais ser�o recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec3 gouraud_color;
out vec2 texcoords;

out vec3 material_diffuse;
out vec3 material_speculate;
out vec3 material_environment;
out float material_specular_exponent;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// functions declarations
void spherical_mapping(in vec4 center, in vec4 position, out float U, out float V);

void main()
{
    // A vari�vel gl_Position define a posi��o final de cada v�rtice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estar� entre -1 e 1 ap�s divis�o por w.
    // Veja slides 144 e 150 do documento "Aula_09_Projecoes.pdf".
    //
    // O c�digo em "main.cpp" define os v�rtices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // opera��es de modelagem, defini��o da c�mera, e proje��o, para computar
    // as coordenadas finais em NDC (vari�vel gl_Position). Ap�s a execu��o
    // deste Vertex Shader, a placa de v�deo (GPU) far� a divis�o por W. Veja
    // slide 189 do documento "Aula_09_Projecoes.pdf".

    gl_Position = projection * view * model * model_coefficients;

    // Como as vari�veis acima  (tipo vec4) s�o vetores com 4 coeficientes,
    // tamb�m � poss�vel acessar e modificar cada coeficiente de maneira
    // independente. Esses s�o indexados pelos nomes x, y, z, e w (nessa
    // ordem, isto �, 'x' � o primeiro coeficiente, 'y' � o segundo, ...):
    //
    //     gl_Position.x = model_coefficients.x;
    //     gl_Position.y = model_coefficients.y;
    //     gl_Position.z = model_coefficients.z;
    //     gl_Position.w = model_coefficients.w;
    //

    // Agora definimos outros atributos dos v�rtices que ser�o interpolados pelo
    // rasterizador para gerar atributos �nicos para cada fragmento gerado.

    // Posi��o do v�rtice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posi��o do v�rtice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do v�rtice atual no sistema de coordenadas global (World).
    // Veja slide 107 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf".
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;

    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;

    material_diffuse           = material_diffuse_coefficients;
    material_speculate         = material_speculate_coefficients;
    material_environment       = material_environment_coefficients;
    material_specular_exponent = material_specular_exponent_coefficients;

    if (object_id == BULLET) {
        gouraud_color = vec3(1.0, 0.0, 0.0);
    } else if (object_id == ASTEROID) {
        vec4 light_position = normalize(vec4(1.0,1.0,0.0,1.0));
        vec4 l = normalize(light_position);
        vec4 n = normalize(normal);

        // Coordenadas de textura U e V
        float U = 0.0;
        float V = 0.0;

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        spherical_mapping(bbox_center, model_coefficients, U, V);
        vec3 Kd = texture(TextureImage0, vec2(U,V)).rgb;
        vec3 Ka = material_environment_coefficients;

        // Espectro da fonte de ilumina��o
        vec3 I = vec3(1.0, 1.0, 1.0);

        // Espectro da luz ambiente
        vec3 Ia = vec3(0.1, 0.1, 0.1);

        // Termo difuso utilizando a lei dos cossenos de Lambert
        vec3 lambert_diffuse_term = Kd * I * max(0, dot(n, l));

        // Termo ambiente
        vec3 ambient_term = Ka * Ia;

        gouraud_color = lambert_diffuse_term + ambient_term;
        gouraud_color = pow(gouraud_color, vec3(1.0,1.0,1.0)/2.2);
    } else {
        gouraud_color = vec3(0.0, 0.0, 0.0);
    }
}

void spherical_mapping(in vec4 center, in vec4 position, out float U, out float V) {
    vec4 p_line = center + normalize(position - center);
    vec4 p_vector = p_line - center;
    float phi = asin(p_vector.y);
    float theta = atan(p_vector.x, p_vector.z);

    U = (theta + M_PI) / (2*M_PI);
    V = (phi + M_PI_2) / M_PI;
}

