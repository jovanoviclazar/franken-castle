//
// Created by cvnpko on 12/3/25.
//

#ifndef SCREENQUAD_HPP
#define SCREENQUAD_HPP


namespace engine::resources {

class ScreenQuad {
public:
    ScreenQuad();

    void init();

    void bind();

    void unbind();

    void draw();

private:
    void destroy();

    uint32_t m_vao{};
    uint32_t m_vbo{};
    std::vector<float> m_vertices{};
};
}// namespace engine::resources

#endif//SCREENQUAD_HPP
