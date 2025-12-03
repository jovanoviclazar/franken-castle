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
    void resize(int32_t width, int32_t height);

    void destroy();

    void bind();

    void unbind();

    void generate_texture(const std::string &name, uint32_t attachment, int32_t width, int32_t height,
                          int32_t internalFormat, uint32_t format, uint32_t type,
                          uint32_t minFilter, uint32_t magFilter,
                          uint32_t wrapS, uint32_t wrapT);

    void draw_buffers(uint32_t attachments[], int32_t num);

    void generate_renderbuffer(int32_t width, int32_t height);

    void check_status();

    void bind_texture(const std::string &name, uint32_t tex);

private:
    Framebuffer();

    uint32_t m_fbo{};
    uint32_t m_rbo{};
    std::string m_name{};

    uint32_t m_width{};
    uint32_t m_height{};
    std::unordered_map<std::string, uint32_t> m_color_buffers;
};
}// namespace engine::resources
#endif//FRAMEBUFFER_HPP
