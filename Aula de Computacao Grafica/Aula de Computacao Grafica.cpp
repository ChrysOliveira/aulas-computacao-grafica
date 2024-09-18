#include <stdio.h>
#include <string.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <time.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

std::vector<Mesh*> listMesh;
std::vector<Shader*> listShader;
Window* window;

static const char* vertexLocation = "VertexShader.glsl";
static const char* fragmentLocation = "FragmentShader.glsl";

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

	Mesh* triangulo1 = new Mesh();
	triangulo1->CreateMesh(vertices, sizeof(vertices), indices, sizeof(indices));
	listMesh.push_back(triangulo1);

	Mesh* triangulo2 = new Mesh();
	triangulo2->CreateMesh(vertices, sizeof(vertices), indices, sizeof(indices));
	listMesh.push_back(triangulo2);
}


void CreateShader() {
	Shader* shader = new Shader();
	shader->CreateFromFile(vertexLocation, fragmentLocation);
	listShader.push_back(shader);
}

int main()
{
	window = new Window(1024, 768);
	window->Initialize();

	CreateTriagle();
	CreateShader();

	bool direction = true, sizeDirection = true, angleDirection = true;
	float triOffset = 0.0f, maxOffset = 0.7f, minOffset = -0.7f, incOffset = 0.01f;
	float size = 0.4f, maxSize = 0.7f, minSize = -0.7f, incSize = 0.01f;
	float angle = 0.0f, maxAngle = 360.0f, minAngle = -1.0f, incAngle = 0.5f;

	while (!window->ShouldClose()) {
		glfwPollEvents();

		glClearColor(0.6f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Shader* shader = listShader[0];
		shader->UseProgram();

		if (triOffset >= maxOffset || triOffset <= minOffset)
			direction = !direction;
		triOffset += direction ? incOffset : incOffset * -1;

		if (size >= maxSize || size <= minSize)
			sizeDirection = !sizeDirection;
		size += sizeDirection ? incSize : incSize * -1;

		if (angle >= maxAngle || angle <= minAngle)
			angleDirection = !angleDirection;
		angle += angleDirection ? incAngle : incAngle * -1;


		listMesh[0]->RenderMesh();

		glm::mat4 model(1.0f);

		model = glm::translate(model, glm::vec3(triOffset, triOffset, -1.5f));
		model = glm::scale(model, glm::vec3(0.4, 0.4, 0.4));
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(shader->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));

		listMesh[1]->RenderMesh();

		model = glm::translate(model, glm::vec3(-triOffset, -triOffset, -1.5f));
		model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(shader->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 projection = glm::perspective(1.0f, window->GetBufferWidth() / window->GetBufferHeight(), 0.1f, 100.0f);
		glUniformMatrix4fv(shader->GetUniformProjection(), 1, GL_FALSE, glm::value_ptr(projection));

		glUseProgram(0); 

		window->SwapBuffers();
	}

	window->~Window();
	return 0;
}