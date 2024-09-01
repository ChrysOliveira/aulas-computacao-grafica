#include <stdio.h>
#include <string.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//Window dimensions
const GLint WIDTH = 800, HEIGHT = 600;

GLuint VAO, VBO, EBO, shaderProgram;

//Vertex Shader
static const char* vShader = "                  \n\
#version 330                                    \n\
                                                \n\
layout(location=0) in vec3 pos;                 \n\
                                                \n\
uniform mat4 model;                             \n\
                                                \n\
out vec4 vColor;                                \n\
                                                \n\
void main(){                                    \n\
  gl_Position = model * vec4(pos, 1.0);         \n\
  vColor = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);  \n\
}";

//Fragment Shader
static const char* fShader = "                  \n\
#version 330                                    \n\
                                                \n\
uniform vec3 triangleColor;                     \n\
                                                \n\
in vec4 vColor;                                 \n\
out vec4 color;                                 \n\
                                                \n\
void main(){                                    \n\
  color = vColor;                               \n\
}";

void CreateTriagle() {
    //1. Definir os pontos dos vértices
    GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f, //Vertice 1 (x, y, z)
		0.0f, 1.0f, 0.0f,   //Vertice 2 (x, y, z)
		1.0f, -1.0f, 0.0f,	//Vertice 3 (x, y, z)
		0.0f, 0.0f, 1.0f    //Vertice 3 (x, y, z)
    };

    GLuint indices[] = {
		0, 1, 2,
		0, 1, 3,
		0, 2, 3,
		1, 2, 3
	};

    //VAO
    glGenVertexArrays(1, &VAO); //Gera um VAO ID
    glBindVertexArray(VAO); //Atribuindo o ID ao VAO

    // EBO
    glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Carregar os dados de vértice para a placa de vídeo
    //Vertex Buffer Object: VBO
    glGenBuffers(1, &VBO); //Gera um VBO ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //Transforma o VBO em um Array Buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copia os dados ao VBO

    //GL_STATIC_DRAW: Os dados do vértice serão carregados uma vez e desenhados várias vezes (por exemplo, o mundo).
    //GL_DYNAMIC_DRAW: Os dados do vértice serão criados uma vez, alterados de tempos em tempos, mas desenhados muitas vezes mais do que isso.
    //GL_STREAM_DRAW : Os dados do vértice serão carregados uma vez e desenhados uma vez.

    //Vertex Attribute Pointer - Atributos dos dados na memória
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //0: shader location | 2: numero de valores de vertice (x,y) | GL_FLOAT: tipo dos valores
    //GL_FALSE: normalizado | 0: pular elemento (cor) | 0: offset
//Vertex Attribute Pointer Enable
    glEnableVertexAttribArray(0); //0: shader location

    //Limpar o Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Limpar o Vertex Array
    glBindVertexArray(0);
}

void AddShader(GLuint program, const char* shaderCode, GLenum shaderType) {
    //Começamos a compilar cada shader
    //1. Criar um shader
    GLuint _shader = glCreateShader(shaderType);

    //2. Atribui o código do GLSL para o shader
        //2.1 Guarda a variavel localmente (converte char para GLchar*)
    const GLchar* code[1];
    code[0] = shaderCode;

    //2.2 Anexando o código a shader
    glShaderSource(_shader, 1, code, NULL); //1: numero de códigos | NULL: final da string onde encontrar NULL

    //3. Compila o shader
    glCompileShader(_shader); //compilar o shader

    //4. Tratamento de erros de compilação
    GLint result = 0;
    GLchar eLog[1024] = { 0 };
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &result); //Verifica o resultado
    if (!result) {
        glGetProgramInfoLog(_shader, sizeof(eLog), NULL, eLog);
        printf("Erro ao compilar o %d shader: '%s'\n", shaderType, eLog);
        return;
    }

    //5. Adiciona o shader ao programa (recebido como parâmetro)
    glAttachShader(program, _shader); //adiciona o shader ao programa
}

void CompileShaders() {
    //1.0 Cria o programa
    shaderProgram = glCreateProgram(); //Inicia o programa
    if (!shaderProgram) {
        printf("Erro ao criar o Shader Program");
        return;
    }

    //2.0 Compila o Vertex Shader
    AddShader(shaderProgram, vShader, GL_VERTEX_SHADER); //Adiciona o Vertex Shader ao programa
    //3.0 Compila o Fragment Shader
    AddShader(shaderProgram, fShader, GL_FRAGMENT_SHADER); //Adiciona o Vertex Shader ao programa

    //4.0 Cria o link do programa
    glLinkProgram(shaderProgram); //Criar o link do programa na GPU

    //5.0 Tratamento de erros
    GLint result = 0;
    GLchar eLog[1024] = { 0 };
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result); //Verifica o resultado
    if (!result) {
        glGetProgramInfoLog(shaderProgram, sizeof(eLog), NULL, eLog);
        printf("Erro ao linkar o programa: '%s'\n", eLog);
        return;
    }

    //6.0 Validação se o programa está rodando
    glValidateProgram(shaderProgram); //Validação do programa
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &result); //Verifica o resultado
    if (!result) {
        glGetProgramInfoLog(shaderProgram, sizeof(eLog), NULL, eLog);
        printf("Erro ao validar o programa: '%s'\n", eLog);
        return;
    }
}

int main()
{
    auto t_start = std::chrono::high_resolution_clock::now();

    //initialize GLFW
    if (!glfwInit()) {
        printf("GLFW Inicialization fail");
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    //Setting GLFW window propeties
    //OpenGL Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //Core profile = No Backwards compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Allow foward compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Teste", NULL, NULL);
    if (!mainWindow) {
        printf("GLFW window create fail!");
        glfwTerminate();
        return 1;
    }

    //Get buffer size information
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    //Set the context for GLEW to use
    glfwMakeContextCurrent(mainWindow);

    //Allow modern extentions features
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        printf("Glew Inicialization fail!");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    //Setup viewports sizes
    glViewport(0, 0, bufferWidth, bufferHeight);

    //Criar o triangulo
    CreateTriagle();
    CompileShaders();

	bool direction = true, sizeDirection = true, angleDirection = true;//true = direita / false = esquerda
	float triOffset = 0.0f, maxOffset = 0.7f, minOffset = -0.7f, incOffset = 0.01f;
	float size = 0.0f, maxSize = 0.7f, minSize = -0.7f, incSize = 0.01f;
	float angle = 0.0f, maxAngle = 360.0f, minAngle = -1.0f, incAngle = 0.5f;

    //Loop until the window close
    while (!glfwWindowShouldClose(mainWindow)) {
        //Get + Handle user input events
        glfwPollEvents();

        //Clear Window (100% = 255)
		glClearColor(0.52f, 0.36f, 0.87f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Desenhar o triangulo
        glUseProgram(shaderProgram); //Busca o programa que está com o shader (triangulo)
        glBindVertexArray(VAO); //Bind o VAO

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor"); //procura a entrada chamada triangleColor
        float r = (float)rand()/RAND_MAX;
        float g = (float)rand()/RAND_MAX;
        float b = (float)rand()/RAND_MAX;
        glUniform3f(uniColor, r, g, b); //atualiza o valor da entrada com cor aleatoria

        //auto t_now = std::chrono::high_resolution_clock::now();
        //float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        //glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

        // Move triangulo
        
		if (triOffset >= maxOffset || triOffset <= minOffset)
			direction = !direction;
		triOffset += direction ? incOffset : incOffset * -1;

		if (size >= maxSize || size <= minSize)
			sizeDirection = !sizeDirection;
		size += sizeDirection ? incSize : incSize * -1;

		if (angle >= maxAngle || angle <= minAngle)
			angleDirection = !angleDirection;
		angle += angleDirection ? incAngle : incAngle * -1;

		//criar matriz
		glm::mat4 model(1.0f);

		//Movimentações do triangulo
		model = glm::translate(model, glm::vec3(triOffset, triOffset, 0.0f));

		//Tamanho do triangulo
		model = glm::scale(model, glm::vec3(0.4, 0.4, 0.4));

		//Rotação
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

		GLint uniModel = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));


        //glUniform3f(uniColor, 1.0f, 0.0f, 0.0f); //atualiza o valor da entrada com a cor vermelha
        glDrawArrays(GL_TRIANGLES, 0, 3); //Desenha um triangulo | 0: Array index | 2: Número de pontos (vértices)
        glBindVertexArray(0);
        glUseProgram(0); //remove o programa da memória

        glfwSwapBuffers(mainWindow);
    }
    return 0;
}
