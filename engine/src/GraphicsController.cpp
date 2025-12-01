
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
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

    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_g_buffer);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_g_buffer);
    CHECKED_GL_CALL(glGenTextures, 1, &m_g_position);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_g_position);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, platform->window()->width(), platform->window()->height(), 0, GL_RGBA, GL_FLOAT, (void *) 0);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_g_position, 0);
    CHECKED_GL_CALL(glGenTextures, 1, &m_g_normal);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_g_normal);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, platform->window()->width(), platform->window()->height(), 0, GL_RGBA, GL_FLOAT, (void *) 0);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_g_normal, 0);
    CHECKED_GL_CALL(glGenTextures, 1, &m_g_albedo);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_g_albedo);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA, platform->window()->width(), platform->window()->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *) 0);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_g_albedo, 0);
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    CHECKED_GL_CALL(glDrawBuffers, 3, attachments);
    CHECKED_GL_CALL(glGenRenderbuffers, 1, &m_rbo_depth);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, m_rbo_depth);
    CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, platform->window()->width(), platform->window()->height());
    CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_depth);
    RG_GUARANTEE(CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete!");
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_ssao_fbo);
    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_ssao_blur_fbo);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_ssao_fbo);

    CHECKED_GL_CALL(glGenTextures, 1, &m_ssao_color_buffer);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_ssao_color_buffer);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RED, platform->window()->width(), platform->window()->height(), 0, GL_RED, GL_FLOAT, (void *) 0);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssao_color_buffer, 0);
    RG_GUARANTEE(CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "SSAO Framebuffer not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, m_ssao_blur_fbo);
    glGenTextures(1, &m_ssao_color_buffer_blur);
    glBindTexture(GL_TEXTURE_2D, m_ssao_color_buffer_blur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, platform->window()->width(), platform->window()->height(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssao_color_buffer_blur, 0);
    RG_GUARANTEE(CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "SSAO Blur Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

void GraphicsController::bind_g_frame_buffer() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_g_buffer);
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params().Width = static_cast<float>(width);
    m_graphics->perspective_params().Height = static_cast<float>(height);
    m_graphics->orthographic_params().Right = static_cast<float>(width);
    m_graphics->orthographic_params().Top = static_cast<float>(height);
    CHECKED_GL_CALL(glViewport, 0, 0, width, height);
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

void GraphicsController::draw_ssao(const resources::Shader *shader) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssao_fbo);
    engine::graphics::OpenGL::clear_buffers();
    shader->use();
    shader->set_int("gPosition", 0);
    shader->set_int("gNormal", 1);
    shader->set_int("texNoise", 2);
    for (unsigned int i = 0; i < 64; ++i) {
        shader->set_vec3("samples[" + std::to_string(i) + "]", m_ssao_kernel[i]);
    }
    shader->set_mat4("projection", projection_matrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_noise_texture);
    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsController::draw_ssao_blur(const resources::Shader *shader) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssao_blur_fbo);
    engine::graphics::OpenGL::clear_buffers();
    shader->use();
    shader->set_int("ssaoInput", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ssao_color_buffer);
    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsController::draw_ssao_light(const resources::Shader *shader, bool spotlight) {
    engine::graphics::OpenGL::clear_buffers();
    shader->use();
    shader->set_int("gPosition", 0);
    shader->set_int("gNormal", 1);
    shader->set_int("gAlbedo", 2);
    shader->set_int("ssao", 3);
    shader->set_vec3("dirLight.direction", glm::vec3(1.0f, -1.0f, 1.0f));
    shader->set_vec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    shader->set_vec3("dirLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    shader->set_vec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    shader->set_vec3("spotLight.position", camera()->Position);
    shader->set_vec3("spotLight.direction", camera()->Front);
    shader->set_vec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.diffuse", spotlight ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.specular", spotlight ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_float("spotLight.constant", 1.0f);
    shader->set_float("spotLight.linear", 0.09f);
    shader->set_float("spotLight.quadratic", 0.032f);
    shader->set_float("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->set_float("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_g_albedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_ssao_color_buffer_blur);
    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
