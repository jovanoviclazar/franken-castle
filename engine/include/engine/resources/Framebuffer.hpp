//
// Created by cvnpko on 12/1/25.
//

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace engine::resources {
class Framebuffer {

    friend class ResourcesController;

public:
    void resize(int width, int height);

    void destroy();

    void bind();

    void unbind();

    void generate_texture(const std::string &name, unsigned int attachment, int width, int height,
                          int internalFormat, unsigned int format, unsigned int type,
                          unsigned int minFilter, unsigned int magFilter,
                          unsigned int wrapS, unsigned int wrapT);

    void draw_buffers(unsigned int attachments[], int num);

    void generate_renderbuffer(int width, int height);

    void check_status();

    void bind_texture(const std::string &name, unsigned int tex);

private:
    Framebuffer();

    unsigned int m_fbo{};
    unsigned int m_rbo{};
    std::string m_name{};

    unsigned int m_width{};
    unsigned int m_height{};
    std::unordered_map<std::string, unsigned int> m_color_buffers;
};
}// namespace engine::resources
#endif//FRAMEBUFFER_HPP
