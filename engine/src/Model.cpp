
// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

void Model::draw(const Shader *shader) {
    shader->use();
    for (auto &mesh: m_meshes) {
        mesh.draw(shader);
    }
}

void Model::draw_instancing(const Shader *shader, uint32_t amount) {
    shader->use();
    for (auto &mesh: m_meshes) {
        mesh.draw_instancing(shader, amount);
    }
}

void Model::destroy() {
    for (auto &mesh: m_meshes) {
        mesh.destroy();
    }
}


void Model::load_instancing(const std::vector<glm::mat4> &data, const std::vector<resources::Mesh> &meshes) {
    uint32_t buffer;
    CHECKED_GL_CALL(glGenBuffers, 1, &buffer);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, buffer);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, data.size() * sizeof(glm::mat4), &data[0], GL_STATIC_DRAW);
    for (const auto &mesh: meshes) {
        uint32_t vao = mesh.get_vao();
        CHECKED_GL_CALL(glBindVertexArray, vao);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 3);
        CHECKED_GL_CALL(glVertexAttribPointer, 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) 0);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 4);
        CHECKED_GL_CALL(glVertexAttribPointer, 4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
        CHECKED_GL_CALL(glEnableVertexAttribArray, 5);
        CHECKED_GL_CALL(glVertexAttribPointer, 5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
        CHECKED_GL_CALL(glEnableVertexAttribArray, 6);
        CHECKED_GL_CALL(glVertexAttribPointer, 6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

        CHECKED_GL_CALL(glVertexAttribDivisor, 3, 1);
        CHECKED_GL_CALL(glVertexAttribDivisor, 4, 1);
        CHECKED_GL_CALL(glVertexAttribDivisor, 5, 1);
        CHECKED_GL_CALL(glVertexAttribDivisor, 6, 1);

        CHECKED_GL_CALL(glBindVertexArray, 0);
    }
}


}// namespace engine::resources
