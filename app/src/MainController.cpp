//
// Created by cvnpko on 11/20/25.
//
#include <app/MainController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <random>

namespace engine::main::app {
void MainPlatformEventObserver::on_key(engine::platform::Key key) {
    // spdlog::info("Keyboard event: key={}, state={}", key.name(), key.state_str());
}

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    // spdlog::info("MousePosition: {} {}", position.x, position.y);
}

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->camera()->Position = {-45, 1, 0};
    graphics->camera()->MovementSpeed *= 4.0f;

    auto observer = std::make_unique<MainPlatformEventObserver>();
    engine::core::Controller::get<engine::platform::PlatformController>()->register_platform_event_observer(
            std::move(observer));

    m_trees.push_back(get_model_matrix({{-35.0f, 0.0f, 40.0f}, {2.6f, 2.6f, 2.6f}}));
    m_trees.push_back(get_model_matrix({{0.0f, 0.0f, 34.0f}, {3.0f, 3.0f, 3.0f}}));
    m_trees.push_back(get_model_matrix({{-40.0f, 0.0f, -45.0f}, {2.2f, 2.2f, 2.2f}}));
    m_trees.push_back(get_model_matrix({{-5.0f, 0.0f, -35.0f}, {2.8f, 2.8f, 2.8f}}));
    m_trees.push_back(get_model_matrix({{15.0f, 0.0f, -40.0f}, {3.2f, 3.2f, 3.2f}}));
    m_trees.push_back(get_model_matrix({{38.0f, 0.0f, -35.0f}, {3.20f, 3.20f, 3.20f}}));
    m_trees.push_back(get_model_matrix({{38.0f, 0.0f, -10.0f}, {2.6f, 2.6f, 2.6f}}));
    m_trees.push_back(get_model_matrix({{38.0f, 0.0f, 30.0f}, {3.8f, 3.8f, 3.8f}}));
    m_trees.push_back(get_model_matrix({{36.0f, 0.0f, 8.0f}, {2.6f, 2.6f, 2.6f}}));
    m_trees.push_back(get_model_matrix({{20.0f, 0.0f, 25.0f}, {2.8f, 2.8f, 2.8f}}));
    m_trees.push_back(get_model_matrix({{15.0f, 0.0f, 40.0f}, {2.6f, 2.6f, 2.6f}}));

    auto tree = engine::core::Controller::get<engine::resources::ResourcesController>()->model("pine_tree");
    engine::graphics::OpenGL::load_instancing(m_trees, tree->meshes());

    std::random_device rd;
    std::mt19937 gen(rd());
    {
        std::uniform_real_distribution<float> distx(-49.9f, -18.1f);
        std::uniform_real_distribution<float> disty(-0.01f, 0.01f);
        std::uniform_real_distribution<float> distz(-49.9f, 49.9f);
        std::uniform_real_distribution<float> dists(0.8f, 1.2f);
        std::uniform_real_distribution<float> dista(0.0f, 360.0f);
        glm::vec3 vec_n = {0.0f, 1.0f, 0.0f};
        for (int i = 0; i < 400000; i++) {
            ModelParams tmp{{distx(gen), disty(gen), distz(gen)},
                            {dists(gen), dists(gen), dists(gen)}};
            tmp.Rotate.emplace_back(vec_n, dista(gen));
            m_grass.push_back(get_model_matrix(tmp));
        }
    }
    {
        std::uniform_real_distribution<float> distx(-11.9f, 49.9f);
        std::uniform_real_distribution<float> disty(-0.01f, 0.01f);
        std::uniform_real_distribution<float> distz(-49.9f, 49.9f);
        std::uniform_real_distribution<float> dists(0.8f, 1.2f);
        std::uniform_real_distribution<float> dista(0.0f, 360.0f);
        glm::vec3 vec_n = {0.0f, 1.0f, 0.0f};
        for (int i = 0; i < 600000; i++) {
            ModelParams tmp{{distx(gen), disty(gen), distz(gen)},
                            {dists(gen), dists(gen), dists(gen)}};
            tmp.Rotate.emplace_back(vec_n, dista(gen));
            m_grass.push_back(get_model_matrix(tmp));
        }
    }
    auto grass = engine::core::Controller::get<engine::resources::ResourcesController>()->model("grass");
    engine::graphics::OpenGL::load_instancing(m_grass, grass->meshes());

    m_plank.Rotate.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);

    m_mystery_machine.Rotate.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE)
                .state() == engine::platform::Key::State::JustPressed) {
        return false;
    }
    return true;
}

void MainController::poll_events() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1)
                .state() == engine::platform::Key::State::JustPressed) {
        m_cursor_enabled = !m_cursor_enabled;
        platform->set_enable_cursor(m_cursor_enabled);
    }
    if (platform->key(engine::platform::KEY_B)
                .state() == engine::platform::Key::State::JustPressed) {
        m_bridge_opening++;
    }
    if (platform->key(engine::platform::KEY_F)
                .state() == engine::platform::Key::State::JustPressed) {
        m_spotlight_enabled = !m_spotlight_enabled;
    }
}

void MainController::update() {
    update_camera();
    if (m_bridge_opening > 0) {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        float dt = 20 * platform->dt();
        if (m_bridge_opened) {
            m_bridge_radius -= dt;
            if (m_bridge_radius < 0.0f) {
                m_bridge_radius = 0.0f;
                m_bridge_opening--;
                m_bridge_opened = false;
            }
        } else {
            m_bridge_radius += dt;
            if (m_bridge_radius > 90.0f) {
                m_bridge_radius = 90.0f;
                m_bridge_opening--;
                m_bridge_opened = true;
            }
        }
        // spdlog::info("Bridge radius: {}", m_bridge_radius);
        // spdlog::info("Bridge opening: {}", m_bridge_opening);
    }
}

void MainController::update_camera() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_W)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KEY_S)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KEY_A)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KEY_D)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
    }
    auto mouse = platform->mouse();
    camera->rotate_camera(mouse.dx, mouse.dy);
    camera->zoom(mouse.scroll);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    engine::core::Controller::get<engine::graphics::GraphicsController>()->bind_g_frame_buffer();
    engine::graphics::OpenGL::clear_buffers();
    // draw_skybox();
    draw_floor();
    draw_water();
    draw_alligator();
    draw_grass();
    draw_tree();
    draw_castle();
    draw_plank();
    draw_bridge();
    draw_mystery_machine();
    auto shader1 = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("ssao");
    engine::core::Controller::get<engine::graphics::GraphicsController>()->draw_ssao(shader1);
    auto shader2 = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("ssao_blur");
    engine::core::Controller::get<engine::graphics::GraphicsController>()->draw_ssao_blur(shader2);
    auto shader3 = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("ssao_light");
    engine::core::Controller::get<engine::graphics::GraphicsController>()->draw_ssao_light(shader3, m_spotlight_enabled);
}


void MainController::end_draw() {
    engine::core::Controller::get<engine::platform::PlatformController>()->swap_buffers();
}

void MainController::draw_skybox() {
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("skybox");
    auto skybox_cube = engine::core::Controller::get<engine::resources::ResourcesController>()->skybox("skybox");
    engine::core::Controller::get<engine::graphics::GraphicsController>()->draw_skybox(shader, skybox_cube);
}

void MainController::draw_floor() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto floor = engine::core::Controller::get<engine::resources::ResourcesController>()->model("floor");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_floor));
    // set_light(shader);
    floor->draw(shader);
}

void MainController::draw_water() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto water = engine::core::Controller::get<engine::resources::ResourcesController>()->model("water");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_water));
    // set_light(shader);
    water->draw(shader);
}

void MainController::draw_alligator() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto alligator = engine::core::Controller::get<engine::resources::ResourcesController>()->model("alligator");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_alligator));
    // set_light(shader);
    alligator->draw(shader);
}

void MainController::draw_grass() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("instancing");
    auto grass = engine::core::Controller::get<engine::resources::ResourcesController>()->model("grass");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    // set_light(shader);
    grass->draw_instancing(shader, m_grass.size());
}

void MainController::draw_tree() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("instancing");
    auto pine_tree = engine::core::Controller::get<engine::resources::ResourcesController>()->model("pine_tree");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    // set_light(shader);
    pine_tree->draw_instancing(shader, m_trees.size());
}

void MainController::draw_castle() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto castle = engine::core::Controller::get<engine::resources::ResourcesController>()->model("castle");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_castle));
    // set_light(shader);
    castle->draw(shader);
}

void MainController::draw_plank() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto plank = engine::core::Controller::get<engine::resources::ResourcesController>()->model("plank");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_plank));
    // set_light(shader);
    plank->draw(shader);
}

void MainController::draw_bridge() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto bridge = engine::core::Controller::get<engine::resources::ResourcesController>()->model("bridge");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", rotate(get_model_matrix(m_bridge), glm::radians(m_bridge_radius), m_bridge_vec));
    // set_light(shader);
    bridge->draw(shader);
}

void MainController::draw_mystery_machine() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto mystery_machine = engine::core::Controller::get<engine::resources::ResourcesController>()->model("mystery_machine");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", get_model_matrix(m_mystery_machine));
    // set_light(shader);
    mystery_machine->draw(shader);
}

void MainController::set_light(const resources::Shader *shader) const {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    shader->set_vec3("dirLight.direction", glm::vec3(1.0f, -1.0f, 1.0f));
    shader->set_vec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    shader->set_vec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
    shader->set_vec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    shader->set_vec3("spotLight.position", graphics->camera()->Position);
    shader->set_vec3("spotLight.direction", graphics->camera()->Front);
    shader->set_vec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.diffuse", m_spotlight_enabled ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.specular", m_spotlight_enabled ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_float("spotLight.constant", 1.0f);
    shader->set_float("spotLight.linear", 0.09f);
    shader->set_float("spotLight.quadratic", 0.032f);
    shader->set_float("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->set_float("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

glm::mat4 MainController::get_model_matrix(ModelParams par) {
    glm::mat4 ret = translate(glm::mat4(1.0f), par.Position);
    for (auto &r: par.Rotate) {
        ret = rotate(ret, glm::radians(r.second), r.first);
    }
    ret = scale(ret, par.Scale);
    return ret;
}
}// namespace engine::main::app
