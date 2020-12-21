#include <cmath>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <magic_enum.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/component/all.hpp>

#include "Engine/Core.hpp"

#include "Engine/dll/Loader.hpp"
#include "Engine/third_party.hpp"

#include "Engine/graphics/Shader.hpp"

#include "Engine/widget/DisplayOption.hpp"

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
    app.add_option("-m,--module", module_name, "Module to load.");
    app.add_option("--glfw-major", glfw_major, "Major version of GLFW.");
    app.add_option("--glfw-minor", glfw_minor, "Minor version of GLFW.");
    app.add_option("--window-width", window_width, "Initial width of the rendering window.");
    app.add_option("--window-height", window_height, "Initial height of the rendering window.");
    // app.set_version_flag("-v,--version", "VERSION v" PROJECT_VERSION);

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
    ::glfwSwapInterval(0);

    if (const auto err = ::glewInit(); err != GLEW_OK) {
        spdlog::error("Engine::Core GLEW An error occured '{}' 'code={}'", ::glewGetErrorString(err), err);
        return 1;
    }

    if (!core.m_window->create_ui_context()) {
        spdlog::error("Engine::Core ImGui failed to create context");
        return 1;
    }

    spdlog::info("{}", core.m_module->name());

    core.loop();
    return 0;
}

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

    Shader shader(VERT_SH, FRAG_SH);
    shader.use();

    entt::registry world;
    world.on_destroy<api::VAO>().connect<api::VAO::on_destroy>();
    world.on_destroy<api::VBO<api::VAO::Attribute::POSITION>>()
        .connect<api::VBO<api::VAO::Attribute::POSITION>::on_destroy>();
    world.on_destroy<api::VBO<api::VAO::Attribute::COLOR>>().connect<api::VBO<api::VAO::Attribute::COLOR>::on_destroy>();
    world.on_destroy<api::EBO>().connect<api::EBO::on_destroy>();

    {
        // clang-format off
        constexpr auto triangle_positions = std::to_array({
            -0.5f, -0.5f, 0.0f, // left
            0.5f, -0.5f, 0.0f, // right
            0.0f, 0.5f, 0.0f // top
        });

        constexpr auto triangle_colors = std::to_array({
            1.0f, 0.0f, 0.0f, 1.0f, // left
            0.0f, 1.0f, 0.0f, 1.0f, // right
            0.0f, 0.0f, 1.0f, 1.0f, // top
        });
        // clang-format on

        for (int i = 1; i != 12; i++) {
            const auto entity = world.create();
            api::VBO<api::VAO::Attribute::POSITION>::emplace(world, entity, triangle_positions, 3);
            api::VBO<api::VAO::Attribute::COLOR>::emplace(world, entity, triangle_colors, 4);
            world.emplace<api::Position3f>(entity, glm::vec3{i * 1.5, 0, 0});
            world.emplace<api::Rotation3f>(entity, glm::vec3{i * 10, i * 10, i * 10});
            world.emplace<api::Scale3f>(entity, glm::vec3{i, i, i});
        }

        // clang-format off
        constexpr auto square_positions = std::to_array({
            0.5f, 0.5f, 0.0f, // top right
            0.5f, -0.5f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f // top left
        });
        constexpr auto square_colors = std::to_array({
            1.0f, 0.0f, 1.0f, 1.0f,// top right
            1.0f, 1.0f, 0.0f, 1.0f, // bottom right
            0.0f, 1.0f, 1.0f, 1.0f, // bottom left
            1.0f, 0.0f, 1.0f, 1.0f// top left
        });
        constexpr auto square_indices = std::to_array({
            0u, 1u, 3u, // first triangle
            1u, 2u, 3u // second triangle
        });
        // clang-format on

        const auto entity = world.create();
        api::VBO<api::VAO::Attribute::POSITION>::emplace(world, entity, square_positions, 3);
        api::VBO<api::VAO::Attribute::COLOR>::emplace(world, entity, square_colors, 4);
        api::EBO::emplace(world, entity, square_indices);
    }

    auto display_mode = api::VAO::DEFAULT_MODE;

    const auto projection =
        glm::perspective(glm::radians(45.0f), m_window->getAspectRatio<float>(), 0.1f, 100.0f);
    shader.setUniform("projection", projection);

    m_is_running = true;
    while (m_is_running && m_window->isOpen()) {
        ::glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowMetricsWindow();

        ImGui::Begin("Display Options", nullptr, ImGuiWindowFlags_NoBackground);

        widget::DisplayMode{}.draw(world, display_mode);

        ImGui::End(); // Display Options

        ImGui::Render();

        const auto radius = 10.0f;
        const auto camX = static_cast<float>(std::sin(::glfwGetTime())) * radius;
        const auto camY = static_cast<float>(std::sin(::glfwGetTime())) * radius;
        const auto camZ = static_cast<float>(std::cos(::glfwGetTime())) * radius;
        const auto view =
            glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setUniform("view", view);

        constexpr auto CLEAR_COLOR = glm::vec4{0.0f, 1.0f, 0.2f, 1.0f};

        CALL_OPEN_GL(::glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a));
        CALL_OPEN_GL(::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        system_rendering(shader, world);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->render();
    }

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
