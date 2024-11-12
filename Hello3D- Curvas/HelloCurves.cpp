#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STL
#include <vector>

#include <random>
#include <algorithm>

// Classes utilitárias
#include "Shader.h"

struct Curve {
    std::vector<glm::vec3> controlPoints; // Pontos de controle da curva
    std::vector<glm::vec3> curvePoints;   // Pontos da curva
    glm::mat4x4 M;          // Matriz dos coeficientes da curva
};


// Outras funções
void initializeBernsteinMatrix(glm::mat4x4 &matrix);
void generateBezierCurvePoints(Curve &curve, int numPoints);
void initializeCatmulRomMatrix(glm::mat4x4 &matrix);
void generateCatmulRomCurvePoints(Curve &curve, int numPoints);
void displayCurve(const Curve &curve);
GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 600, HEIGHT = 600;

int main()
{

    // Inicialização da GLFW
    glfwInit();
    // Criação da janela GLFW
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Curvas Parametricas!", nullptr, nullptr);
    glfwMakeContextCurrent(window);


    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Compilando e buildando o programa de shader
    Shader shader = Shader("./hello-curves.vs", "./hello-curves.fs");

    // Estrutura para armazenar a curva de Bézier e pontos de controle
    Curve curvaBezier;
    Curve curvaCatmulRom;
    curvaBezier.controlPoints = {
    glm::vec3(-0.6, -0.4, 0.0),
    glm::vec3(-0.6, -0.4, 0.0),
	glm::vec3(-0.4, -0.6, 0.0),
	glm::vec3(-0.2, -0.2, 0.0),
	glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(0.2, 0.2, 0.0),
	glm::vec3(0.4, 0.6, 0.0),
	glm::vec3(0.6, 0.4, 0.0),
	glm::vec3(0.6, 0.4, 0.0)
    };
    curvaCatmulRom.controlPoints = curvaBezier.controlPoints;
    //curvaBezier.controlPoints = { glm::vec3(-0.8f, -0.4f, 0.0f), glm::vec3(-0.4f, 0.4f, 0.0f),
    //                              glm::vec3(0.4f, 0.4f, 0.0f), glm::vec3(0.8f, -0.4f, 0.0f) };

    // Gerar pontos da curva de Bézier
    int numCurvePoints = 100; // Quantidade de pontos por segmento na curva
    generateBezierCurvePoints(curvaBezier, numCurvePoints);
    generateCatmulRomCurvePoints(curvaCatmulRom, numCurvePoints);


    //Cria os buffers de geometria dos pontos da curva
    GLuint VAOControl = generateControlPointsBuffer(curvaBezier.controlPoints);
    GLuint VAOBezierCurve = generateControlPointsBuffer(curvaBezier.curvePoints);
    GLuint VAOCatmulRomCurve = generateControlPointsBuffer(curvaCatmulRom.curvePoints);

    cout << curvaBezier.controlPoints.size() << endl;
    cout << curvaBezier.curvePoints.size() << endl;
    cout << curvaCatmulRom.curvePoints.size() << endl;


    shader.Use();

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Desenhar pontos de controle maiores e com cor diferenciada
        glBindVertexArray(VAOControl);
        shader.setVec4("finalColor", 1.0f, 0.0f, 0.0f,1.0f); // Vermelho para pontos de controle
        glPointSize(8.0f);
        glDrawArrays(GL_POINTS, 0, curvaBezier.controlPoints.size());
        
        // Desenhar pontos da curva de Bezier e conectar com linhas
        glBindVertexArray(VAOBezierCurve);
        shader.setVec4("finalColor", 0.0f, 0.0f, 1.0f,1.0f); // Azul para a curva
        glLineWidth(10.0);
        glDrawArrays(GL_LINE_STRIP, 0, curvaBezier.curvePoints.size()); // Desenha a curva como uma linha contínua
      
       // Desenhar pontos da curva de Bezier e conectar com linhas
        glBindVertexArray(VAOCatmulRomCurve);
        shader.setVec4("finalColor", 0.0f, 1.0f, 0.0f,1.0f); // Azul para a curva
        glDrawArrays(GL_LINE_STRIP, 0, curvaCatmulRom.curvePoints.size()); // Desenha a curva como uma linha contínua

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &VAOControl);
    glDeleteVertexArrays(1, &VAOBezierCurve);
    glDeleteVertexArrays(1, &VAOCatmulRomCurve);
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();

    return 0;
}

void initializeBernsteinMatrix(glm::mat4 &matrix)
{
    matrix[0] = glm::vec4(1.0f, -3.0f, 3.0f, -1.0f);
    matrix[1] = glm::vec4(0.0f, 3.0f, -6.0f, 3.0f);
    matrix[2] = glm::vec4(0.0f, 0.0f, 3.0f, -3.0f);
    matrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}


void initializeCatmulRomMatrix(glm::mat4 &matrix)
{
    matrix[0] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
    matrix[1] = glm::vec4(2.0f, -5.0f, 4.0f, -1.0f);
    matrix[2] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
    matrix[3] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
}

void generateBezierCurvePoints(Curve &curve, int numPoints) {
    curve.curvePoints.clear(); // Limpa quaisquer pontos antigos da curva

    initializeBernsteinMatrix(curve.M);
    // Calcular os pontos ao longo da curva com base em Bernstein
     // Loop sobre os pontos de controle em grupos de 4

    float piece = 1.0/ (float) numPoints;
    float t;
    for (int i = 0; i < curve.controlPoints.size() - 3; i += 3) {
        
        // Gera pontos para o segmento atual
        for (int j = 0; j < numPoints;j++) {
            t = j * piece;
            
            // Vetor t para o polinômio de Bernstein
            glm::vec4 T(t * t * t, t * t, t, 1);
            
            glm::mat4x3 G = glm::mat4x3(
                curve.controlPoints[i],
                curve.controlPoints[i + 1],
                curve.controlPoints[i + 2],
                curve.controlPoints[i + 3]
            );

            // Calcula o ponto da curva multiplicando tVector, a matriz de Bernstein e os pontos de controle
            glm::vec3 point = G * curve.M * T;
            
            curve.curvePoints.push_back(point);
        }
    }
}

void generateCatmulRomCurvePoints(Curve &curve, int numPoints) {
    curve.curvePoints.clear(); // Limpa quaisquer pontos antigos da curva

    initializeCatmulRomMatrix(curve.M);
    // Calcular os pontos ao longo da curva com base em Bernstein
     // Loop sobre os pontos de controle em grupos de 4

    float piece = 1.0 / (float) numPoints;
    float t;
    for (int i = 0; i < curve.controlPoints.size()-3; i += 3) {
        
        // Gera pontos para o segmento atual
        for (int j = 0; j < numPoints;j++) {
            t = j * piece;
            
            // Vetor t para o polinômio de Bernstein
            glm::vec4 T(t * t * t, t * t, t, 1);
            
            glm::vec3 P0 = curve.controlPoints[i];
			glm::vec3 P1 = curve.controlPoints[i + 1];
			glm::vec3 P2 = curve.controlPoints[i + 2];
			glm::vec3 P3 = curve.controlPoints[i + 3];

			glm::mat4x3 G(P0, P1, P2, P3);


            // Calcula o ponto da curva multiplicando tVector, a matriz de Bernstein e os pontos de controle
            glm::vec3 point = G * curve.M * T;
            point = point * glm::vec3(0.5, 0.5, 0.5);
            curve.curvePoints.push_back(point);
        }
    }
}


GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints)
{
	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat)* 3, controlPoints.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

