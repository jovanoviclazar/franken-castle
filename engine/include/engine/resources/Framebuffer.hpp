//
// Created by cvnpko on 12/1/25.
//

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <filesystem>
#include <string_view>
#include <utility>

namespace engine::resources {
class Framebuffer {

    friend class ResourcesController;

public:
    void destroy();

private:
    Framebuffer() = default;
    Framebuffer(std::string name, const unsigned int fbo, const unsigned int rbo, const unsigned int width, const unsigned int height, std::vector<unsigned int> color_buffer)
        : m_fbo(fbo)
        , m_rbo(rbo)
        , m_name(std::move(name))
        , m_width(width)
        , m_height(height)
        , m_color_buffers(std::move(color_buffer)) {}

    unsigned int m_fbo{};
    unsigned int m_rbo{};
    std::string m_name{};

    unsigned int m_width{};
    unsigned int m_height{};
    std::vector<unsigned int> m_color_buffers;
};
}// namespace engine::resources
#endif//FRAMEBUFFER_HPP
