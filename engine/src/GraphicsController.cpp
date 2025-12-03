
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Skybox.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <random>
#include <spdlog/spdlog.h>

namespace engine::graphics {

void GraphicsController::initialize() {
    const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto resources = engine::core::Controller::get<resources::ResourcesController>();
    auto handle = platform->window()->handle_();
    m_perspective_params.FOV = glm::radians(m_camera.Zoom);
    m_perspective_params.Width = static_cast<float>(platform->window()->width());
    m_perspective_params.Height = static_cast<float>(platform->window()->height());
    m_perspective_params.Near = 0.1f;
    m_perspective_params.Far = 100.f;
    m_ortho_params.Bottom = 0.0f;
    m_ortho_params.Top = static_cast<float>(platform->window()->height());
    m_ortho_params.Left = 0.0f;
    m_ortho_params.Right = static_cast<float>(platform->window()->width());
    m_ortho_params.Near = 0.1f;
    m_ortho_params.Far = 100.0f;

    platform->register_platform_event_observer(std::make_unique<GraphicsPlatformEventObserver>(this));
    CHECKED_GL_CALL(glViewport, 0, 0, platform->window()->width(), platform->window()->height());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
    RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");

    float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_quad_vbo);
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    auto g_buffer = resources->framebuffer("g_buffer");
    register_resizable_framebuffer(g_buffer);
    g_buffer->generate_texture("g_position", GL_COLOR_ATTACHMENT0, platform->window()->width(), platform->window()->height(), GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    g_buffer->generate_texture("g_normal", GL_COLOR_ATTACHMENT1, platform->window()->width(), platform->window()->height(), GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, 0, 0);
    g_buffer->generate_texture("g_albedo", GL_COLOR_ATTACHMENT2, platform->window()->width(), platform->window()->height(), GL_RGBA, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, 0, 0);
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    g_buffer->draw_buffers(attachments, 3);
    g_buffer->generate_renderbuffer(platform->window()->width(), platform->window()->height());
    g_buffer->check_status();

    auto ssao_fbo = resources->framebuffer("ssao_fbo");
    register_resizable_framebuffer(ssao_fbo);
    auto ssao_blur_fbo = resources->framebuffer("ssao_blur_fbo");
    register_resizable_framebuffer(ssao_blur_fbo);

    ssao_fbo->generate_texture("color_buffer", GL_COLOR_ATTACHMENT0, platform->window()->width(), platform->window()->height(), GL_RED, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST, 0, 0);
    ssao_fbo->check_status();

    ssao_blur_fbo->generate_texture("color_buffer", GL_COLOR_ATTACHMENT0, platform->window()->width(), platform->window()->height(), GL_RED, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST, 0, 0);
    ssao_blur_fbo->check_status();

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;
        scale = 0.1f + (1.0f - 0.1f) * scale * scale;
        sample *= scale;
        m_ssao_kernel.push_back(sample);
    }

    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        m_ssao_noise.push_back(noise);
    }
    CHECKED_GL_CALL(glGenTextures, 1, &m_noise_texture);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_noise_texture);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &m_ssao_noise[0]);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void GraphicsController::register_resizable_framebuffer(resources::Framebuffer *fb) {
    m_resize_framebuffer.push_back(fb);
}

std::vector<resources::Framebuffer *> GraphicsController::get_resize_framebuffers() {
    return m_resize_framebuffer;
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params().Width = static_cast<float>(width);
    m_graphics->perspective_params().Height = static_cast<float>(height);
    m_graphics->orthographic_params().Right = static_cast<float>(width);
    m_graphics->orthographic_params().Top = static_cast<float>(height);
    CHECKED_GL_CALL(glViewport, 0, 0, width, height);
    auto resources = engine::core::Controller::get<resources::ResourcesController>();
    for (auto &fb: m_graphics->get_resize_framebuffers()) {
        fb->resize(width, height);
    }
}

std::string_view GraphicsController::name() const {
    return "GraphicsController";
}

void GraphicsController::begin_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicsController::end_gui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsController::draw_ssao(const resources::Shader *ssao_shader, const resources::Shader *blur_shader, const resources::Shader *light_shader) {
    auto resources = engine::core::Controller::get<resources::ResourcesController>();
    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto g_buffer = resources->framebuffer("g_buffer");
    auto ssao_fbo = resources->framebuffer("ssao_fbo");
    ssao_fbo->bind();
    engine::graphics::OpenGL::clear_buffers();
    ssao_shader->use();
    ssao_shader->set_int("gPosition", 0);
    ssao_shader->set_int("gNormal", 1);
    ssao_shader->set_int("texNoise", 2);
    ssao_shader->set_float("width", platform->window()->width());
    ssao_shader->set_float("height", platform->window()->height());
    for (unsigned int i = 0; i < 64; ++i) {
        ssao_shader->set_vec3("samples[" + std::to_string(i) + "]", m_ssao_kernel[i]);
    }
    ssao_shader->set_mat4("projection", projection_matrix());
    g_buffer->bind_texture("g_position", GL_TEXTURE0);
    g_buffer->bind_texture("g_normal", GL_TEXTURE1);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE2);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_noise_texture);
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    ssao_fbo->unbind();

    auto ssao_blur_fbo = resources->framebuffer("ssao_blur_fbo");
    ssao_blur_fbo->bind();
    engine::graphics::OpenGL::clear_buffers();
    blur_shader->use();
    blur_shader->set_int("ssaoInput", 0);
    ssao_fbo->bind_texture("color_buffer", GL_TEXTURE0);
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    ssao_blur_fbo->unbind();

    engine::graphics::OpenGL::clear_buffers();
    light_shader->use();
    light_shader->set_int("gPosition", 0);
    light_shader->set_int("gNormal", 1);
    light_shader->set_int("gAlbedo", 2);
    light_shader->set_int("ssao", 3);
    g_buffer->bind_texture("g_position", GL_TEXTURE0);
    g_buffer->bind_texture("g_normal", GL_TEXTURE1);
    g_buffer->bind_texture("g_albedo", GL_TEXTURE2);
    ssao_blur_fbo->bind_texture("color_buffer", GL_TEXTURE3);
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
    CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    CHECKED_GL_CALL(glDepthFunc, GL_LESS);// set depth function back to default
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}
}// namespace engine::graphics
