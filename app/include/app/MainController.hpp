//
// Created by cvnpko on 11/20/25.
//

#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <memory>
#include <spdlog/spdlog.h>
namespace engine::main::app {

class MainPlatformEventObserver final : public engine::platform::PlatformEventObserver {
public:
    void on_key(engine::platform::Key key) override;

    void on_mouse_move(engine::platform::MousePosition position) override;
};

class MainController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "main::app::MainController";
    }

private:
    struct ModelParams {
        glm::vec3 Position;
        glm::vec3 Scale;
        std::vector<std::pair<glm::vec3, float>> Rotate;
    };

    void initialize() override;

    bool loop() override;

    void poll_events() override;

    void update() override;

    static void update_camera();

    void begin_draw() override;

    void draw() override;

    void end_draw() override;

    void draw_skybox();

    void draw_floor();

    void draw_water();

    void draw_alligator();

    void draw_grass();

    void draw_tree();

    void draw_castle();

    void draw_plank();

    void draw_bridge();

    void draw_mystery_machine();

    glm::mat4 get_model_matrix(ModelParams par);

    void set_light(const resources::Shader *shader) const;

    ModelParams m_floor{{0.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.1f}};
    ModelParams m_castle{{1.5f, -0.6f, 0.0f}, {0.12f, 0.12f, 0.12f}};
    ModelParams m_bridge{{-11.2f, 0.25f, -4.8f}, {5.0f, 4.8f, 8.0f}};
    ModelParams m_plank{{-11.2f, 0.0f, -4.8f}, {5.0f, 4.8f, 4.0f}};
    ModelParams m_water{{-15.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.1f}};
    ModelParams m_alligator{{-15.0f, -0.49f, 15.0f}, {4.0f, 4.0f, 4.0f}};
    ModelParams m_mystery_machine{{-35.0f, 0.0f, 5.0f}, {1.5f, 1.5f, 1.5f}};

    std::vector<glm::mat4> m_trees;
    std::vector<glm::mat4> m_grass;
    bool m_cursor_enabled{true};
    int m_bridge_opening{0};
    bool m_bridge_opened{false};
    glm::vec3 m_bridge_vec{0.0f, 0.0f, 1.0f};
    float m_bridge_radius{0.0f};
    bool m_spotlight_enabled{false};
};
}// namespace engine::main::app
#endif//MAINCONTROLLER_HPP
