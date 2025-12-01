//
// Created by cvnpko on 12/1/25.
//

// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Framebuffer.hpp>

namespace engine::resources {
void Framebuffer::destroy() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    if (!m_color_buffers.empty()) {
        CHECKED_GL_CALL(glDeleteTextures, m_color_buffers.size(), m_color_buffers.data());
        m_color_buffers.clear();
    }
    if (m_rbo != 0) {
        CHECKED_GL_CALL(glDeleteRenderbuffers, 1, &m_rbo);
        m_rbo = 0;
    }
    if (m_fbo != 0) {
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
        m_fbo = 0;
    }
}
}// namespace engine::resources
