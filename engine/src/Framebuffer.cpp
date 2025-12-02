//
// Created by cvnpko on 12/1/25.
//

// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Framebuffer.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>

namespace engine::resources {
Framebuffer::Framebuffer() {
    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_fbo);
}

void Framebuffer::destroy() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    for (auto &buff: m_color_buffers) {
        CHECKED_GL_CALL(glDeleteTextures, 1, &buff.second);
    }
    m_color_buffers.clear();
    if (m_rbo != 0) {
        CHECKED_GL_CALL(glDeleteRenderbuffers, 1, &m_rbo);
        m_rbo = 0;
    }
    if (m_fbo != 0) {
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
        m_fbo = 0;
    }
}

void Framebuffer::bind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::generate_texture(const std::string &name, unsigned int attachment, int width, int height,
                                   int internalFormat, unsigned int format, unsigned int type,
                                   unsigned int minFilter, unsigned int magFilter,
                                   unsigned int wrapS, unsigned int wrapT) {
    auto &result = m_color_buffers[name];
    if (!result) {
        spdlog::info("load_framebuffer(name={})", name);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
        unsigned int color_buffer = 0;
        CHECKED_GL_CALL(glGenTextures, 1, &color_buffer);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, color_buffer);
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, (void *) 0);
        if (minFilter != 0) CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        if (magFilter != 0) CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        if (wrapS != 0) CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        if (wrapT != 0) CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, color_buffer, 0);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        m_color_buffers[name] = color_buffer;
    }
}

void Framebuffer::draw_buffers(unsigned int attachments[], int num) {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
    CHECKED_GL_CALL(glDrawBuffers, num, attachments);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::generate_renderbuffer(int width, int height) {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
    CHECKED_GL_CALL(glGenRenderbuffers, 1, &m_rbo);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, m_rbo);
    CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::check_status() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
    RG_GUARANTEE(CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete!");
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind_texture(const std::string &name, unsigned int tex) {
    if (!m_color_buffers[name]) {
        spdlog::error("error_framebuffer(name={})", name);
    }
    CHECKED_GL_CALL(glActiveTexture, tex);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[name]);
}

}// namespace engine::resources
