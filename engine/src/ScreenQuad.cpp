//
// Created by cvnpko on 12/3/25.
//

// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ScreenQuad.hpp>

namespace engine::resources {
ScreenQuad::ScreenQuad() {
    m_vertices = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f};
}

void ScreenQuad::init() {

    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_vbo);

    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);

    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void ScreenQuad::bind() {
    glBindVertexArray(m_vao);
}

void ScreenQuad::unbind() {
    glBindVertexArray(0);
}

void ScreenQuad::draw() {
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void ScreenQuad::destroy() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}
}// namespace engine::resources
