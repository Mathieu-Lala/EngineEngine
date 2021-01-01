#include <cmath>

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <CLI/CLI.hpp>
#include <magic_enum.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/component/all.hpp>

#include "Engine/Core.hpp"

#include "Engine/dll/Loader.hpp"
#include "Engine/third_party.hpp"

#include "Engine/Camera.hpp"
#include "Engine/graphics/Shader.hpp"
#include "Engine/json/Event.hpp"

#include "Engine/widget/DisplayOption.hpp"
#include "Engine/widget/ComponentTree.hpp"
#include "Engine/widget/CameraWidget.hpp"

#include "Engine/helpers/overloaded.hpp"

auto engine::core::Core::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::info("{} - v{}", PROJECT_NAME, PROJECT_VERSION);

    std::string module_name{"default"};
    int glfw_major = 3;
    int glfw_minor = 3;
    int window_width = 300;
    int window_height = 300;

    CLI::App app{PROJECT_NAME " description", argv[0]};
    app.set_config("--config", "engine-config.ini");
    app.add_option("-m,--module", module_name, "Module to load.");
    app.add_option("--glfw-major", glfw_major, "Major version of GLFW.");
    app.add_option("--glfw-minor", glfw_minor, "Minor version of GLFW.");
    app.add_option("--window-width", window_width, "Initial width of the rendering window.");
    app.add_option("--window-height", window_height, "Initial height of the rendering window.");
    app.add_flag(
        "--version",
        [](auto v) -> void {
            if (v == 1) {
                std::cout << PROJECT_VERSION << "\n";
                std::exit(0);
            }
        },
        "Print the version number and exit.");

    CLI11_PARSE(app, argc, argv);

    Core core{};

    if (const auto module_obj = core.load_module(module_name)) {
        core.m_module = module_obj;
    } else {
        spdlog::error("Initialization of module failed...");
        return 1;
    }

    if (!core.initialize_graphics(glfw_major, glfw_minor)) {
        spdlog::error("Initialization of graphical context failed...");
        return 1;
    }

    core.m_window = std::make_unique<Window>(window_width, window_height, PROJECT_NAME " - Rendering window");
    core.m_event_manager.registerWindow(*core.m_window);

    if (const auto err = ::glewInit(); err != GLEW_OK) {
        spdlog::error("Engine::Core GLEW An error occured '{}' 'code={}'", ::glewGetErrorString(err), err);
        return 1;
    }

    if (!core.m_window->create_ui_context()) {
        spdlog::error("Engine::Core ImGui failed to create context");
        return 1;
    }

    spdlog::info("{}", core.m_module->name());

    ::glfwSwapInterval(0);

    core.loop();
    return 0;
}

engine::core::Core::~Core() { ::glfwTerminate(); }

auto engine::core::Core::system_rendering(Shader &shader, entt::registry &world) const noexcept
{
    const auto render =
        [&shader]<bool has_ebo>(
            const api::VAO &vao, const api::Position3f &pos, const api::Rotation3f &rot, const api::Scale3f &scale) {
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, pos.vec);
            model = glm::rotate(model, glm::radians(rot.vec.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rot.vec.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rot.vec.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale.vec);
            shader.setUniform("model", model);

            CALL_OPEN_GL(::glBindVertexArray(vao.object));
            if constexpr (has_ebo) {
                CALL_OPEN_GL(::glDrawElements(static_cast<GLenum>(vao.mode), vao.count, GL_UNSIGNED_INT, 0));
            } else {
                CALL_OPEN_GL(::glDrawArrays(static_cast<GLenum>(vao.mode), 0, vao.count));
            }
        };

    [[maybe_unused]] static constexpr auto NO_POSITION = glm::vec3{0.0f, 0.0f, 0.0f};
    [[maybe_unused]] static constexpr auto NO_ROTATION = glm::vec3{0.0f, 0.0f, 0.0f};
    [[maybe_unused]] static constexpr auto NO_SCALE = glm::vec3{1.0f, 1.0f, 1.0f};

    // note : it should be a better way..

    // without ebo

    world.view<api::VAO>(entt::exclude<api::EBO, api::Position3f, api::Rotation3f, api::Scale3f>)
        .each([&render](const auto &vao) {
            render.operator()<false>(vao, {NO_POSITION}, {NO_ROTATION}, {NO_SCALE});
        });

    world.view<api::VAO, api::Position3f>(entt::exclude<api::EBO, api::Rotation3f, api::Scale3f>)
        .each([&render](const auto &vao, const auto &pos) {
            render.operator()<false>(vao, pos, {NO_ROTATION}, {NO_SCALE});
        });

    world.view<api::VAO, api::Rotation3f>(entt::exclude<api::EBO, api::Position3f, api::Scale3f>)
        .each([&render](const auto &vao, const auto &rot) {
            render.operator()<false>(vao, {NO_POSITION}, rot, {NO_SCALE});
        });

    world.view<api::VAO, api::Scale3f>(entt::exclude<api::EBO, api::Position3f, api::Rotation3f>)
        .each([&render](const auto &vao, const auto &scale) {
            render.operator()<false>(vao, {NO_POSITION}, {NO_ROTATION}, scale);
        });

    world.view<api::VAO, api::Position3f, api::Scale3f>(entt::exclude<api::EBO, api::Rotation3f>)
        .each([&render](const auto &vao, const auto &pos, const auto &scale) {
            render.operator()<false>(vao, pos, {NO_ROTATION}, scale);
        });

    world.view<api::VAO, api::Rotation3f, api::Scale3f>(entt::exclude<api::EBO, api::Position3f>)
        .each([&render](const auto &vao, const auto &rot, const auto &scale) {
            render.operator()<false>(vao, {NO_POSITION}, rot, scale);
        });

    world.view<api::VAO, api::Position3f, api::Rotation3f>(entt::exclude<api::EBO, api::Scale3f>)
        .each([&render](const auto &vao, const auto &pos, const auto &rot) {
            render.operator()<false>(vao, pos, rot, {NO_SCALE});
        });

    world.view<api::VAO, api::Position3f, api::Rotation3f, api::Scale3f>(entt::exclude<api::EBO>)
        .each([&render](const auto &vao, const auto &pos, const auto &rot, const auto &scale) {
            render.operator()<false>(vao, pos, rot, scale);
        });

    // with ebo

    world.view<api::EBO, api::VAO>(entt::exclude<api::Position3f, api::Rotation3f, api::Scale3f>)
        .each([&render](const auto &, const auto &vao) {
            render.operator()<true>(vao, {NO_POSITION}, {NO_ROTATION}, {NO_SCALE});
        });

    world.view<api::EBO, api::VAO, api::Position3f>(entt::exclude<api::Rotation3f, api::Scale3f>)
        .each([&render](const auto &, const auto &vao, const auto &pos) {
            render.operator()<true>(vao, pos, {NO_ROTATION}, {NO_SCALE});
        });

    world.view<api::EBO, api::VAO, api::Rotation3f>(entt::exclude<api::Position3f, api::Scale3f>)
        .each([&render](const auto &, const auto &vao, const auto &rot) {
            render.operator()<true>(vao, {NO_POSITION}, rot, {NO_SCALE});
        });

    world.view<api::EBO, api::VAO, api::Scale3f>(entt::exclude<api::Position3f, api::Rotation3f>)
        .each([&render](const auto &, const auto &vao, const auto &scale) {
            render.operator()<true>(vao, {NO_POSITION}, {NO_ROTATION}, scale);
        });

    world.view<api::EBO, api::VAO, api::Position3f, api::Scale3f>(entt::exclude<api::Rotation3f>)
        .each([&render](const auto &, const auto &vao, const auto &pos, const auto &scale) {
            render.operator()<true>(vao, pos, {NO_ROTATION}, scale);
        });

    world.view<api::EBO, api::VAO, api::Rotation3f, api::Scale3f>(entt::exclude<api::Position3f>)
        .each([&render](const auto &, const auto &vao, const auto &rot, const auto &scale) {
            render.operator()<true>(vao, {NO_POSITION}, rot, scale);
        });

    world.view<api::EBO, api::VAO, api::Position3f, api::Rotation3f>(entt::exclude<api::Scale3f>)
        .each([&render](const auto &, const auto &vao, const auto &pos, const auto &rot) {
            render.operator()<true>(vao, pos, rot, {NO_SCALE});
        });

    world.view<api::EBO, api::VAO, api::Position3f, api::Rotation3f, api::Scale3f>().each(
        [&render](const auto &, const auto &vao, const auto &pos, const auto &rot, const auto &scale) {
            render.operator()<true>(vao, pos, rot, scale);
        });
}

auto engine::core::Core::loop() -> void
{
    constexpr auto VERT_SH = R"(#version 450
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColors;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fragColors;

void main()
{
    gl_Position = projection * view * model * vec4(inPos, 1.0f);

    fragColors = inColors;
}
)";

    constexpr auto FRAG_SH = R"(#version 450
in vec4 fragColors;

out vec4 FragColor;

void main()
{
    FragColor = fragColors;
}
)";

    CALL_OPEN_GL(::glEnable(GL_DEPTH_TEST));
    CALL_OPEN_GL(::glEnable(GL_BLEND));
    CALL_OPEN_GL(::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


    Shader shader(VERT_SH, FRAG_SH);
    shader.use();

    entt::registry world;
#define SET_DESTRUCTOR(Type) world.on_destroy<Type>().connect<Type::on_destroy>()
    SET_DESTRUCTOR(api::VAO);
    SET_DESTRUCTOR(api::VBO<api::VAO::Attribute::POSITION>);
    SET_DESTRUCTOR(api::VBO<api::VAO::Attribute::COLOR>);
    SET_DESTRUCTOR(api::EBO);
#undef SET_DESTRUCTOR

    std::unique_ptr<api::Scene> scene{nullptr};

    if (m_module->getCategory() == api::Module::Category::SCENE) {
        scene = std::unique_ptr<api::Scene>(reinterpret_cast<api::Scene *>(m_module->instance()));
    }

    if (scene == nullptr) {
        spdlog::error("engine::core::Core : A scene is required !");
        return;
    }

    scene->onCreate(world);

    auto display_mode = api::VAO::DEFAULT_MODE;
    bool camera_auto_move{true};

    Camera camera{*m_window, glm::vec3{5, 5, 5}};

    struct widgetDebugHandle {
        const std::string_view name;
        bool is_displayed;
        std::function<void(bool &)> callable;
    };

    auto debugWidget = std::to_array<widgetDebugHandle>(
        {{"Metrics", false, [](bool &is_displayed) { ImGui::ShowMetricsWindow(&is_displayed); }},
         {"Demo", false, [](bool &is_displayed) { ImGui::ShowDemoWindow(&is_displayed); }},
         {"Display Options",
          false,
          [widget = widget::DisplayMode{display_mode}, &world](bool &is_displayed) {
              ImGui::Begin("Display Options", &is_displayed);
              widget.draw(world);
              ImGui::End();
          }},
         {"Components Tree",
          false,
          [widget = widget::ComponentTree{}, &world](bool &is_displayed) {
              ImGui::Begin("Components", &is_displayed);
              widget.draw(world);
              ImGui::End();
          }},
         {"Camera",
          true,
          [widget = widget::CameraWidget{shader, camera}, &camera_auto_move](bool &is_displayed) {
              ImGui::Begin("Camera", &is_displayed);
              widget.draw(camera_auto_move);
              ImGui::End();
          }},
         {"Events", true, [&](bool &is_displayed) {
              ImGui::Begin("Events", &is_displayed);
              ImGui::Text("Number of Event processed: %ld", m_event_manager.getEventsProcessed().size());
              auto v = static_cast<float>(m_event_manager.getTimeScaler());
              if (ImGui::SliderFloat("Time scaler", &v, 0.0f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
                  m_event_manager.setTimeScaler(static_cast<double>(v));
              }

              const auto event = m_event_manager.getLastEvent();

              nlohmann::json as_json;
              to_json(as_json, event);
              ImGui::Text("Last event :\n%s", as_json.dump(4).data());

              ImGui::End();
          }}});

    glm::vec2 mouse_pos;
    glm::vec2 mouse_pos_when_pressed;
    std::int64_t timeElapsedSinceBegining{0l};

    std::unordered_map<api::Key::Code, bool> keyboard_state;
    for (const auto &i : magic_enum::enum_values<api::Key::Code>()) { keyboard_state[i] = false; }

    std::array<bool, magic_enum::enum_integer(api::MouseButton::Button::BUTTON_LAST)> state_mouse_button;
    std::fill(std::begin(state_mouse_button), std::end(state_mouse_button), false);

    constexpr auto time_to_string = [](std::time_t now) -> std::string {
        const auto tp = std::localtime(&now);
        char buffer[32];
        return std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tp) ? buffer : "1970-01-01_00:00:00";
    };

    m_is_running = true;
    while (m_is_running && m_window->isOpen()) {
        const auto event = m_event_manager.getNextEvent();

        bool timeElapsed{false};

        std::visit(
            overloaded{
                [&](const api::OpenWindow &) {
                    m_event_manager.setCurrentTimepoint(std::chrono::steady_clock::now());
                },
                [&](const api::CloseWindow &) { m_is_running = false; },
                [&timeElapsed](const api::TimeElapsed &) { timeElapsed = true; },
                [&mouse_pos](const api::Moved<api::Mouse> &mouse) {
                    mouse_pos = {mouse.source.x, mouse.source.y};
                },
                [&](const api::Pressed<api::MouseButton> &e) {
                    m_window->useEvent(e);

                    state_mouse_button[static_cast<std::size_t>(magic_enum::enum_integer(e.source.button))] = true;
                    mouse_pos_when_pressed = {e.source.mouse.x, e.source.mouse.y};
                },
                [&](const api::Released<api::MouseButton> &e) {
                    m_window->useEvent(e);

                    state_mouse_button[static_cast<std::size_t>(magic_enum::enum_integer(e.source.button))] =
                        false;
                },
                [&](const api::Pressed<api::Key> &e) {
                    m_window->useEvent(e);
                    keyboard_state[e.source.keycode] = true;
                },
                [&](const api::Released<api::Key> &e) {
                    m_window->useEvent(e);
                    keyboard_state[e.source.keycode] = false;
                },
                [&](const api::Character &e) { m_window->useEvent(e); },
                [](const auto &) {}},
            event);

        scene->onUpdate();

        if (timeElapsed) {
            const auto dt_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::get<api::TimeElapsed>(event).elapsed);
            timeElapsedSinceBegining += dt_ms.count();

            for (auto i = 0ul; i != state_mouse_button.size(); i++) {
                if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) && state_mouse_button[i]) {
                    camera.handleMouseInput(
                        magic_enum::enum_cast<api::MouseButton::Button>(static_cast<int>(i)).value(),
                        mouse_pos,
                        mouse_pos_when_pressed,
                        dt_ms);
                }
            }

            if (keyboard_state[api::Key::Code::KEY_F12]) {
                spdlog::info("take screenshot");
                std::filesystem::create_directories("screenshot/");
                const auto file = fmt::format("screenshot/{}.png", time_to_string(std::time(nullptr)));
                if (!m_window->screenshot(file)) { spdlog::warn("failed to take a screenshot: {}", file); }
            }

            if (camera_auto_move) {
                constexpr auto radius = 10.0f;
                camera.setPosition(
                    {std::sin(static_cast<float>(timeElapsedSinceBegining) / 1000.0f) * radius,
                     std::sin(static_cast<float>(timeElapsedSinceBegining) / 1000.0f) * radius / 3.0f,
                     std::cos(static_cast<float>(timeElapsedSinceBegining) / 1000.0f) * radius * 3.0f});
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            scene->onDrawUI();

            ImGui::Begin("Debug Panel", nullptr);
            for (auto &[name, is_displayed, _] : debugWidget) { ImGui::Checkbox(name.data(), &is_displayed); }
            ImGui::End();

            for (auto &[_, is_displayed, func] : debugWidget) {
                if (is_displayed) { func(is_displayed); }
            }

            ImGui::Render();

            if (camera.hasChanged<Camera::Matrix::VIEW>()) {
                const auto view = glm::lookAt(camera.getPosition(), camera.getTargetCenter(), camera.getUp());
                shader.setUniform("view", view);
                camera.setChangedFlag<Camera::Matrix::VIEW>(false);
            }

            if (camera.hasChanged<Camera::Matrix::PROJECTION>()) {
                const auto projection = camera.getProjection();
                shader.setUniform("projection", projection);
                camera.setChangedFlag<Camera::Matrix::PROJECTION>(false);
            }

            constexpr auto CLEAR_COLOR = glm::vec4{0.0f, 1.0f, 0.2f, 1.0f};

            CALL_OPEN_GL(::glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a));
            CALL_OPEN_GL(::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            system_rendering(shader, world);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            m_window->render();
        }
    }

    scene->onDestroy();

    world.clear();
}

auto engine::core::Core::load_module(const std::string_view name) -> const api::Module *
try {
    const auto uid_handle = entt::hashed_string{fmt::format("dll::handle/{}", name).data()};
    const auto uid_module = entt::hashed_string{fmt::format("api::module/{}", name).data()};
    if (const auto handle = m_cache_module_handle.load<dll::HandleLoader>(uid_handle, name); handle) {
        if (const auto module_obj = m_cache_module.load<dll::ModuleLoader>(uid_module, handle.get()); module_obj) {
            return module_obj.operator->();
        }
    }
    return nullptr;
} catch (const dll::Handle::error &e) {
    spdlog::error("Could not load module... : {}", e.what());
    return nullptr;
}

auto engine::core::Core::initialize_graphics(int glfw_context_major, int glfw_context_minor) -> bool
{
    ::glfwSetErrorCallback([](int code, const char *message) {
        spdlog::error("engine::core::Core [GLFW] An error occured '{}' 'code={}'\n", message, code);
    });

    if (::glfwInit() == GLFW_FALSE) { return false; }

    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfw_context_major);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfw_context_minor);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    spdlog::trace("engine::core::Core [GLFW] Version: '{}'\n", ::glfwGetVersionString());

    return true;
}
