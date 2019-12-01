//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   TRABALHO FINAL
//
//                      José Braz
//                   Mario Figueiró

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include "utils.h"
#include "matrices.h"

// utils
#include "shader.h"

// model
#include "Spaceship.h"
#include "Asteroid.h"
#include "Player.h"
#include "obj_model.h"
#include "bullet.h"

/// Configurations
#define MAX_ASTEROIDS 35
#define ASTEROIDS_SPAWN_DISTANCE 20 // distance relative to spaceship
#define ASTEROIDS_DESTROY_DISTANCE 25 // distance relative to spaceship

#define SPACESHIP 0
#define ASTEROID  1
#define BULLET    2

#define PHI_MAX 1.570796f
#define PHI_MIN -1.570796f
#define THETA_MAX 3.1415f
#define THETA_MIN -3.1415f

unsigned int loadCubemap(std::vector<std::string> faces);
Asteroid generateNewAsteroid();
void gameOver();
bool testInterseption(Asteroid asteroid, Spaceship spaceship, glm::mat4 model);
bool testInterseption(Asteroid asteroid1, Asteroid asteroid2);
bool testInterseption(Asteroid asteroid, bullet b);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima


// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void TextRendering_ShowSpaceshipLife(GLFWwindow* window);
void TextRendering_ShowPlayerInfo(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
    std::vector<glm::vec4> vertices;
};

std::map<std::string, SceneObject> g_VirtualScene;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 3.14f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.5f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 6.0f; // Distância da câmera para a origem

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint vertex_shader_id;
GLuint fragment_shader_id;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;
GLint need_texture_uniform;
GLint bbox_min_uniform;
GLint bbox_max_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

///////////////////////////////////
// Lógica do jogo
Player player = Player();
Spaceship spaceship = Spaceship();
std::vector<Asteroid> asteroids;
std::vector<bullet> bullets;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// events
bool leftKeyPressed = false;
bool rightKeyPressed = false;
bool topKeyPressed = false;
bool downKeyPressed = false;
bool spacePressed = false;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // start rand numbers
    srand(time(NULL));

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "ASTEROIDS 3D", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 217-219 do documento "Aula_03_Rendering_Pipeline_Grafico.pdf".
    //

    Shader objectsShader("../../src/shaders/shader_vertex.glsl", "../../src/shaders/shader_fragment.glsl");
    model_uniform           = glGetUniformLocation(objectsShader.ID, "model"); // Variável da matriz "model"
    view_uniform            = glGetUniformLocation(objectsShader.ID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform      = glGetUniformLocation(objectsShader.ID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    object_id_uniform       = glGetUniformLocation(objectsShader.ID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    need_texture_uniform    = glGetUniformLocation(objectsShader.ID, "need_texture");
    bbox_min_uniform        = glGetUniformLocation(objectsShader.ID, "bbox_min");
    bbox_max_uniform        = glGetUniformLocation(objectsShader.ID, "bbox_max");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    objectsShader.use();
    objectsShader.setInt("TextureImage0", 0);
    objectsShader.setInt("TextureImage1", 1);
    objectsShader.setInt("TextureImage2", 2);
    glUseProgram(0);

//    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/texture/basalt.jpg");      // TextureImage0
    LoadTextureImage("../../data/texture/steel.jpg");       // TextureImage1

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/SpaceShip.obj", "../../data/");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);
    ObjModel bulletModel("../../data/bullet.obj", "../../data/");
    ComputeNormals(&bulletModel);
    BuildTrianglesAndAddToVirtualScene(&bulletModel);
    ObjModel asteroidModel("../../data/asteroid.obj", "../../data/");
    ComputeNormals(&asteroidModel);
    BuildTrianglesAndAddToVirtualScene(&asteroidModel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slide 108 do documento "Aula_09_Projecoes.pdf".
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 22-34 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Variáveis auxiliares utilizadas para chamada à função
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    float nearplane = -0.1f;  // Posição do "near plane"
    float farplane  = -40.0f; // Posição do "far plane"

    //// SKY BOX INIT
    Shader skyboxShader("../../src/shaders/skybox.vs", "../../src/shaders/skybox.fs");
    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // Carrega as imagens do Cube map
    std::vector<std::string> faces = {
        "../../data/cubesmaps/simple/right.jpg",
        "../../data/cubesmaps/simple/left.jpg",
        "../../data/cubesmaps/simple/top.jpg",
        "../../data/cubesmaps/simple/bottom.jpg",
        "../../data/cubesmaps/simple/front.jpg",
        "../../data/cubesmaps/simple/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    asteroids.push_back(generateNewAsteroid());

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        objectsShader.use();

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi) + spaceship.position.y - 1.0;
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta) + spaceship.position.z;
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta) + spaceship.position.x;



        if (!g_UsePerspectiveProjection)
        {
            g_CameraTheta = 3.14f; // Ângulo no plano ZX em relação ao eixo Z
            g_CameraPhi = 0.5f;   // Ângulo em relação ao eixo Y
            g_CameraDistance = 6.0f; // Distância da câmera para a origem
            r = g_CameraDistance;
            y = r*sin(g_CameraPhi);
            z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
            x = r*cos(g_CameraPhi)*sin(g_CameraTheta);
        }


        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 172-182 do documento "Aula_08_Sistemas_de_Coordenadas.pdf".
        glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_lookat_l    = spaceship.position; // Ponto "l", para onde a câmera (look-at) estará sempre olhando
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slide 186 do documento "Aula_08_Sistemas_de_Coordenadas.pdf".
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        // Actions
        if (leftKeyPressed)
            spaceship.bendLeft(deltaTime);
        if (rightKeyPressed)
            spaceship.bendRight(deltaTime);
        if (topKeyPressed)
            spaceship.speedUp(deltaTime);
        if (downKeyPressed)
            spaceship.brake(deltaTime);
        if (spacePressed){
            std::cout << "Shoot!" << std::endl;
            bullets.push_back(spaceship.shoot());
            spacePressed = false;
        }

        glm::mat4 model = Matrix_Identity();
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // asteroids logic
        float random_float = static_cast <float> (rand()) / static_cast <float> (RAND_MAX * deltaTime);
        if (asteroids.size() < MAX_ASTEROIDS) {
            asteroids.push_back(generateNewAsteroid());
        }

        // remove asteroid very far
        auto it = asteroids.begin();
        while (it != asteroids.end()) {
            Asteroid asteroid = *it;
            glm::vec4 vecRelative = asteroid.position - spaceship.position;
            if (norm(vecRelative) >= ASTEROIDS_DESTROY_DISTANCE) {
                it = asteroids.erase(it);
            } else {
                it++;
            }
        }
        // remove bullet very far
        auto it2 = bullets.begin();
        while (it2 != bullets.end()) {
            bullet _bullet = *it2;
            glm::vec4 vecRelative = _bullet.current_position - spaceship.position;
            if (norm(vecRelative) >= ASTEROIDS_DESTROY_DISTANCE) {
                it2 = bullets.erase(it2);
            } else {
                it2++;
            }
        }

        /////////////////////////////
        // New objects new positions
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].computeNewPosition(deltaTime);
            model = Matrix_Translate(bullets[i].current_position)
                  * Matrix_Scale(0.1f, 0.1f, 0.1f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, BULLET);
            DrawVirtualObject("bullet");
        }

        for (int i = 0; i < asteroids.size(); i++) {
            asteroids[i].computeNewPosition(deltaTime);
            model = Matrix_Translate(asteroids[i].position)
                  * Matrix_Rotate_Z((float)glfwGetTime() * asteroids[i].rotation.z)
                  * Matrix_Rotate_X((float)glfwGetTime() * asteroids[i].rotation.x)
                  * Matrix_Rotate_Y((float)glfwGetTime() * asteroids[i].rotation.y)
                  * Matrix_Scale(asteroids[i].scale, asteroids[i].scale, asteroids[i].scale);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, ASTEROID);
            DrawVirtualObject("asteroid1");
        }

        // Desenhamos o modelo da nave
        glm::vec4 new_position = spaceship.computeNewPosition(deltaTime);
        model = Matrix_Translate(new_position.x, new_position.y, new_position.z)
//              * Matrix_Rotate_Z(0.0f)
              * Matrix_Rotate_X(spaceship.phi)
              * Matrix_Rotate_Y(spaceship.theta)
              * Matrix_Scale(spaceship.scale, spaceship.scale, spaceship.scale);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, SPACESHIP);
        glUniform1i(need_texture_uniform, 1);
        DrawVirtualObject("Cube_Cube_Base");
        glUniform1i(need_texture_uniform, 0);
        DrawVirtualObject("Cube_Cube_Black");

        ////////////////////////
        // Test interception
        for (int i = 0; i < asteroids.size(); i++) {
            if (testInterseption(asteroids[i], spaceship, model)) {
                asteroids.erase(asteroids.begin() + i);
                spaceship.life--;
                if (spaceship.life <= 0) {
                    gameOver();
                }
            }

            for (int j = i+1; j < asteroids.size(); j++) {
                if (testInterseption(asteroids[i], asteroids[j])) {
                    asteroids.erase(asteroids.begin() + i);
                }
            }

            for (int j = 0; j < bullets.size(); j++) {
                if (testInterseption(asteroids[i], bullets[j])) {
                    player.score += 100;
                    asteroids.erase(asteroids.begin() + i);
                }
            }
        }

        // Print game information
        TextRendering_ShowFramesPerSecond(window);
        TextRendering_ShowSpaceshipLife(window);
        TextRendering_ShowPlayerInfo(window);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    glfwTerminate();

    return 0;
}


// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slide 100 do documento "Aula_20_e_21_Mapeamento_de_Texturas.pdf"
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a,c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// teste esfera-triangulo
bool testInterseption(Asteroid asteroid, Spaceship spaceship, glm::mat4 model) {
    // 1) teste esfera-esfera (barato)
    float sphere_radius = (1/asteroid.scale) * 0.04;
    float spaceship_radius = (1/spaceship.scale) * 0.35;
    float distance = norm(asteroid.position - spaceship.position);
    if ((sphere_radius + spaceship_radius) > distance) {
        // 2) caso passar, testar se algum vertice do modelo da
        //    nave está dentro da esfera. esfera-ponto (custoso)
        std::vector<glm::vec4> vertices = g_VirtualScene["Cube_Cube_Base"].vertices;
        for (int i = 0; i < g_VirtualScene["Cube_Cube_Black"].vertices.size(); i++) {
            vertices.push_back(g_VirtualScene["Cube_Cube_Black"].vertices[i]);
        }
        for (int i = 0; i < vertices.size(); i++) {
            // scale, rotate, ...
            glm::vec4 vertice = matrixVectorProduct(model, vertices[i]);
            float distance = norm(vertice - asteroid.position);
            if (distance < sphere_radius) {
                return true;
            }
        }
    }
    return false;
}

// teste esfera-esfera
bool testInterseption(Asteroid asteroid1, Asteroid asteroid2) {
    float C = 0.04;
    float r1 = (1/asteroid1.scale) * C;
    float r2 = (1/asteroid2.scale) * C;
    float distance = norm(asteroid1.position - asteroid2.position);
    return (r1 + r2) > distance;
}

// teste raio-esfera
bool testInterseption(Asteroid asteroid, bullet b) {
    float r = (1/asteroid.scale) * 0.04;
    glm::vec4 c = b.start_position;
    glm::vec4 s = asteroid.position;
    glm::vec4 d = b.direction;
    float A = std::pow(norm(d), 2);
    float B = dotproduct(scalarproduct(d, 2.0f), (c - s));
    float C = std::pow(norm(c - s), 2) - std::pow(r, 2);

    float delta = std::pow(B, 2) - 4 * A * C;
    if (delta >= 0) {
        float t1 = (-B + std::sqrt(delta)) / (2*A);
        float t2 = (-B - std::sqrt(delta)) / (2*A);
        return b.t >= t1;
    } else {
        return false;
    }
}


// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    std::vector<float>  material_diffuse_coefficients;
    std::vector<float>  material_speculate_coefficients;
    std::vector<float>  material_environment_coefficients;
    std::vector<float>  material_specular_exponent_coefficients;

    std::vector<glm::vec4> vertices;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);


            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W
                vertices.push_back(glm::vec4(vx, vy, vz, 1.0f));

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
                if (shape < model->materials.size()) {
                    // diffuse
                    material_diffuse_coefficients.push_back(model->materials[shape].diffuse[0]);
                    material_diffuse_coefficients.push_back(model->materials[shape].diffuse[1]);
                    material_diffuse_coefficients.push_back(model->materials[shape].diffuse[2]);
                    // specular
                    material_speculate_coefficients.push_back(model->materials[shape].specular[0]);
                    material_speculate_coefficients.push_back(model->materials[shape].specular[1]);
                    material_speculate_coefficients.push_back(model->materials[shape].specular[2]);
                    // ambient
                    material_environment_coefficients.push_back(model->materials[shape].ambient[0]);
                    material_environment_coefficients.push_back(model->materials[shape].ambient[1]);
                    material_environment_coefficients.push_back(model->materials[shape].ambient[2]);
                    // specular
                    material_specular_exponent_coefficients.push_back(model->materials[shape].shininess);
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name                   = model->shapes[shape].name;
        theobject.first_index            = first_index; // Primeiro índice
        theobject.num_indices            = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode         = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;
        theobject.bbox_min               = bbox_min;
        theobject.bbox_max               = bbox_max;
        theobject.vertices               = vertices;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 2)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (!material_diffuse_coefficients.empty())
    {
        GLuint VBO_material_coefficients_id;
        glGenBuffers(1, &VBO_material_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_material_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, material_diffuse_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, material_diffuse_coefficients.size() * sizeof(float), material_diffuse_coefficients.data());
        location = 3; // "(location = 3)" em "shader_vertex.glsl"
        number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if (!material_speculate_coefficients.empty())
    {
        GLuint VBO_material_coefficients_id;
        glGenBuffers(1, &VBO_material_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_material_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, material_speculate_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, material_speculate_coefficients.size() * sizeof(float), material_speculate_coefficients.data());
        location = 4; // "(location = 4)" em "shader_vertex.glsl"
        number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if (!material_environment_coefficients.empty())
    {
        GLuint VBO_material_coefficients_id;
        glGenBuffers(1, &VBO_material_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_material_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, material_environment_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, material_environment_coefficients.size() * sizeof(float), material_environment_coefficients.data());
        location = 5; // "(location = 5)" em "shader_vertex.glsl"
        number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if (!material_speculate_coefficients.empty())
    {
        GLuint VBO_material_coefficients_id;
        glGenBuffers(1, &VBO_material_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_material_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, material_speculate_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, material_speculate_coefficients.size() * sizeof(float), material_speculate_coefficients.data());
        location = 6; // "(location = 6)" em "shader_vertex.glsl"
        number_of_dimensions = 1; // float em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega as texturas do cube
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// gerar os asteroides em um raio maximo com relação a nave
// e garantir que eles venha em direção a nave
Asteroid generateNewAsteroid() {
    // converter para coordenadas esfericas
    glm::vec4 c = spaceship.position;
    glm::vec4 displacement = c - glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    float rho = ASTEROIDS_SPAWN_DISTANCE;
    float phi = PHI_MIN + static_cast <float> (rand()) /(static_cast<float>(RAND_MAX/(PHI_MAX-PHI_MIN)));
    float theta = THETA_MIN + static_cast <float> (rand()) /(static_cast<float>(RAND_MAX/(THETA_MAX-THETA_MIN)));
    float anti_theta = theta > 0 ? theta - PI : theta + PI;
    float anti_phi = -phi;

    glm::vec4 start_position = toCartesianFromSpherical(rho, theta, phi) + displacement;
    glm::vec4 end_position = toCartesianFromSpherical(rho, anti_theta, anti_phi) + displacement;

    float phi_gap = PI/4;
    float rho2 = 3 * ASTEROIDS_SPAWN_DISTANCE / 2;
    float theta2 = THETA_MIN + static_cast <float> (rand()) /(static_cast<float>(RAND_MAX/(THETA_MAX-THETA_MIN)));
    float phi2 = phi > 0 ? phi - phi_gap : phi + phi_gap;
    float anti_theta2 = theta2 > 0 ? theta2 - PI : theta2 + PI;
    float anti_phi2 = -phi2;

    glm::vec4 start_middle_position = toCartesianFromSpherical(rho2, theta2, phi2) + displacement;
    glm::vec4 end_middle_position = toCartesianFromSpherical(rho2, anti_theta2, anti_phi2) + displacement;

    std::vector<glm::vec4> controlPoints = {start_position,
                                            start_middle_position,
                                            end_middle_position,
                                            end_position};
    Asteroid newAsteroid(start_position, controlPoints);

    return newAsteroid;
}

void gameOver() {
    std::cout << "Game Over" << std::endl;
    std::exit(0);

}
///////////////////////////////////////////////

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_CameraDistance -= 0.1f*yoffset;

    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    static float old_seconds = (float)glfwGetTime();
    static float lastShootTime;

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = !g_UsePerspectiveProjection;
    }
    
    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    /////////////////////
    // Controles da nave
    leftKeyPressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    rightKeyPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    topKeyPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    downKeyPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        spacePressed = true;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

void TextRendering_ShowSpaceshipLife(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    std::stringstream ss;
    ss << spaceship.life;
    std::string life;
    ss >> life;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, life, 0.0f- life.size()*charwidth, 0.3f-lineheight, 1.0f);
}

void TextRendering_ShowPlayerInfo(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    std::stringstream ss;
    ss << player.score;
    std::string score;
    ss >> score;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, "Score: " + score, -1.0f, -0.99f, 1.0f);
}



// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
