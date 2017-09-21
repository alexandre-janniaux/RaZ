#include <array>
#include <iostream>

#include <GL/glew.h>

#include "RaZ/RaZ.hpp"

const unsigned int WIDTH = 800, HEIGHT = 600;

int main() {
  Raz::Window window(WIDTH, HEIGHT, "Test");

  Raz::VertexShader vertShader("../shaders/vert.glsl");
  Raz::FragmentShader fragShader("../shaders/frag.glsl");

  Raz::ShaderProgram program({ vertShader, fragShader });

  Raz::Triangle triangle({ -0.9f, -0.5f, 0.0f,
                            0.0f, -0.5f, 0.0f,
                           -0.45f, 0.5f, 0.0f });

  Raz::Triangle triangle2({ 1.0f,  -0.5f, 0.0f,
                            0.45f,  0.5f, 0.0f,
                            0.55f, -0.5f, 0.0f });

  std::array<GLuint, 2> VAOs, VBOs;
  glGenVertexArrays(2, VAOs.data());
  glGenBuffers(2, VBOs.data());

  glBindVertexArray(VAOs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle.getVertices()), triangle.getVertices().data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  glBindVertexArray(VAOs[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle2.getVertices()), triangle2.getVertices().data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  // Uncommenting this call will display scene in wireframe
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  while (!window.shouldClose()) {
    window.pollEvents();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program.getIndex());
    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(program.getIndex());
    glBindVertexArray(VAOs[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);

    window.swapBuffers();
  }

  glDeleteVertexArrays(2, VAOs.data());
  glDeleteBuffers(2, VBOs.data());

  return 0;
}