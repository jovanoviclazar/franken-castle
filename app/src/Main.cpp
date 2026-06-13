#include <app/Main.hpp>
#include <app/MainController.hpp>

namespace engine::main::app {
void MainApp::app_setup() {
    auto main_controller = register_controller<MainController>();
    main_controller->after(core::Controller::get<core::EngineControllersEnd>());
    main_controller->after(core::Controller::get<engine::graphics::GraphicsController>());
}
}// namespace engine::main::app
int main(int argc, char **argv) {
    return std::make_unique<engine::main::app::MainApp>()->run(argc, argv);
}
