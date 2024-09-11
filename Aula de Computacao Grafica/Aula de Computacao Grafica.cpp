#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <chrono>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include "Mesh.h"

const GLint WIDTH = 800, HEIGHT = 600;
GLuint shaderProgram;

std::vector<Mesh*> listMesh;

//Vertex Shader
static const char* vShader = "                  \n\
#version 330                                    \n\
                                                \n\
layout(location=0) in vec3 pos;                 \n\
uniform mat4 model;								\n\
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
in vec4 vColor;                                 \n\
out vec4 color;                                 \n\
                                                \n\
void main(){                                    \n\
  color = vColor;                               \n\
}";

void CreateTriagle() {
	//1. Definir os pontos dos vértices
	GLfloat vertices[] = {
		-1.0f, 1.0f, 1.0f,
		-1.0, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
	};

	GLuint indices[] = {
		0, 1, 5,
		0, 4, 5,

		1, 3, 5,
		3, 5, 7,

		4, 5, 7,
		4, 6, 7,

		0, 2, 6,
		0, 4, 6,

		2, 3, 6,
		3, 6, 7,

		0, 1, 2,
		1, 2, 3
	};

	//VAO
	Mesh* triangulo1 = new Mesh();
	triangulo1->CreateMesh(vertices, sizeof(vertices), indices, sizeof(indices));

	Mesh* triangulo2 = new Mesh();
	triangulo2->CreateMesh(vertices, sizeof(vertices), indices, sizeof(indices));

	listMesh.push_back(triangulo1);
	listMesh.push_back(triangulo2);
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
	//GLEW = GERENCIADOR DE PACOTES DO OPENGL
	//GLFW = GERENCIADOR DE JANELAS
	auto t_start = std::chrono::high_resolution_clock::now();

	if (!glfwInit()) {
		printf("Não funcionou o GLFW");
		return 1;
	}

	//definindo as versoes min e max do glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//defini para so usar o basico do opengl
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//compatibilidade entre funcoes antigas
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "computacao-grafica", NULL, NULL);

	if (!window) {
		printf("Janela não foi aberta");
		glfwTerminate();
		return 1;
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

	//define que eh esta janela que estamos trabalhando
	glfwMakeContextCurrent(window);

	//usa as funcoes dos drives da sua placa de video
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		printf("Nao foi iniciado o GLEW");
		glfwTerminate();
		glfwDestroyWindow(window);
		return 1;
	}
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, bufferWidth, bufferHeight);

	//Criar um triangulo
	CreateTriagle();
	CompileShaders();

	bool direction = true, sizeDirection = true, angleDirection = true;//true = direita / false = esquerda
	float triOffset = 0.0f, maxOffset = 0.7f, minOffset = -0.7f, incOffset = 0.01f;
	float size = 0.0f, maxSize = 0.7f, minSize = -0.7f, incSize = 0.01f;
	float angle = 0.0f, maxAngle = 360.0f, minAngle = -1.0f, incAngle = 0.5f;

	//Loop until the window close
	while (!glfwWindowShouldClose(window)) {
		//Get + Handle user input events
		glfwPollEvents();

		//Clear Window (100% = 255)
		glClearColor(0.52f, 0.36f, 0.87f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Desenhar o triangulol
		glUseProgram(shaderProgram); //Busca o programa que está com o shader (triangulo)

		listMesh[0]->RenderMesh();

		GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor"); //procura a entrada chamada triangleColor
		
		float r = (float)rand()/RAND_MAX;
		float g = (float)rand()/RAND_MAX;
		float b = (float)rand()/RAND_MAX;
		glUniform3f(uniColor, r, g, b); //atualiza o valor da entrada com a cor vermelha

		/*auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);*/

		//Mover nosso triangulo
			
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
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));

		GLint uniModel = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		listMesh[1]->RenderMesh();

		//criar matriz
		glm::mat4 model2(1.0f);

		//Movimentações do triangulo
		model2 = glm::translate(model2, glm::vec3(-triOffset, -triOffset, 0.0f));

		//Tamanho do triangulo
		model2 = glm::scale(model2, glm::vec3(0.4, 0.4, 0.4));

		//Rotação
		model2 = glm::rotate(model2, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));

		GLint uniModel2 = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel2, 1, GL_FALSE, glm::value_ptr(model2));

		//glUniform3f(uniColor, 1.0f, 0.0f, 0.0f); //atualiza o valor da entrada com a cor vermelha
		//glDrawArrays(GL_TRIANGLES, 0, 3); //Desenha um triangulo | 0: Array index | 2: Número de pontos (vértices)
		glBindVertexArray(0);
		glUseProgram(0); //remove o programa da memória

		glfwSwapBuffers(window);
	}

	return 0;
}
