#include <cmath>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <magic_enum.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Core.hpp"

#include "Engine/dll/Loader.hpp"
#include "Engine/graphics/third_party.hpp"

#include "Engine/graphics/Shader.hpp"

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

struct VAO {
    enum class DisplayMode : std::uint32_t {
        POINTS = GL_POINTS,
        LINE_STRIP = GL_LINE_STRIP,
        LINE_LOOP = GL_LINE_LOOP,
        LINES = GL_LINES,
        LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,
        LINES_ADJACENCY = GL_LINES_ADJACENCY,
        TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
        TRIANGLE_FAN = GL_TRIANGLE_FAN,
        TRIANGLES = GL_TRIANGLES,
        TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
        TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
        PATCHES = GL_PATCHES,
    };


    static constexpr auto DISPLAY_MODES = std::to_array(
        {VAO::DisplayMode::POINTS,
         VAO::DisplayMode::LINE_STRIP,
         VAO::DisplayMode::LINE_LOOP,
         VAO::DisplayMode::LINES,
         VAO::DisplayMode::LINE_STRIP_ADJACENCY,
         VAO::DisplayMode::LINES_ADJACENCY,
         VAO::DisplayMode::TRIANGLE_STRIP,
         VAO::DisplayMode::TRIANGLE_FAN,
         VAO::DisplayMode::TRIANGLES,
         VAO::DisplayMode::TRIANGLE_STRIP_ADJACENCY,
         VAO::DisplayMode::TRIANGLES_ADJACENCY,
         VAO::DisplayMode::PATCHES});

    unsigned int object;
    DisplayMode mode;
    GLsizei count;

    static constexpr DisplayMode DEFAULT_MODE{DisplayMode::TRIANGLES};

    static auto emplace(entt::registry &world, const entt::entity &entity) -> VAO &
    {
        spdlog::trace("engine::core::VAO: emplace to {}", entity);
        VAO obj{0u, DEFAULT_MODE, 0};
        CALL_OPEN_GL(::glGenVertexArrays(1, &obj.object));
        return world.emplace<VAO>(entity, obj);
    }

    static auto on_destroy(entt::registry &world, entt::entity entity) -> void
    {
        spdlog::trace("engine::core::VAO: destroy of {}", entity);
        const auto &vao = world.get<VAO>(entity);
        CALL_OPEN_GL(::glDeleteVertexArrays(1, &vao.object));
    }

    enum class Attribute { POSITION, COLOR, NORMALS };
};

template<VAO::Attribute A>
struct VBO {
    unsigned int object;

    template<std::size_t S>
    static auto
        emplace(entt::registry &world, const entt::entity &entity, const std::array<float, S> &vertices, int stride_size)
            -> VBO<A> &
    {
        spdlog::trace("engine::core::VBO<{}>: emplace to {}", magic_enum::enum_name(A).data(), entity);

        const VAO *vao{nullptr};
        if (vao = world.try_get<VAO>(entity); !vao) { vao = &VAO::emplace(world, entity); }
        CALL_OPEN_GL(::glBindVertexArray(vao->object));

        VBO<A> obj{0u};
        CALL_OPEN_GL(::glGenBuffers(1, &obj.object));

        CALL_OPEN_GL(::glBindBuffer(GL_ARRAY_BUFFER, obj.object));
        CALL_OPEN_GL(::glBufferData(GL_ARRAY_BUFFER, S * sizeof(float), vertices.data(), GL_STATIC_DRAW));
        CALL_OPEN_GL(::glVertexAttribPointer(
            static_cast<GLuint>(A), stride_size, GL_FLOAT, GL_FALSE, stride_size * static_cast<int>(sizeof(float)), 0));
        CALL_OPEN_GL(::glEnableVertexAttribArray(static_cast<GLuint>(A)));

        world.patch<VAO>(entity, [](VAO &vao_obj) { vao_obj.count = S; });

        return world.emplace<VBO<A>>(entity, obj);
    }

    static auto on_destroy(entt::registry &world, entt::entity entity) -> void
    {
        spdlog::trace("engine::core::VBO<{}>: destroy of {}", magic_enum::enum_name(A).data(), entity);
        const auto &vbo = world.get<VBO<A>>(entity);
        CALL_OPEN_GL(::glDeleteBuffers(1, &vbo.object));
    }
};

template<std::size_t D, typename T>
struct Position {
    glm::vec<static_cast<int>(D), T> vec;
};

using Position3f = Position<3, float>;

template<std::size_t D, typename T>
struct Rotation {
    glm::vec<static_cast<int>(D), T> vec;
};

using Rotation3f = Rotation<3, float>;

template<std::size_t D, typename T>
struct Scale {
    glm::vec<static_cast<int>(D), T> vec;
};

using Scale3f = Scale<3, float>;

namespace widget {

auto debugSwitchDisplayMode(entt::registry &world, VAO::DisplayMode &display_mode)
{
    constexpr auto enum_name = magic_enum::enum_type_name<VAO::DisplayMode>();
    if (ImGui::BeginCombo(
            "##combo", fmt::format("{} = {}", enum_name.data(), magic_enum::enum_name(display_mode)).data())) {
        for (const auto &i : VAO::DISPLAY_MODES) {
            const auto is_selected = display_mode == i;
            if (ImGui::Selectable(magic_enum::enum_name(i).data(), is_selected)) {
                display_mode = i;
                for (const auto &entity : world.view<VAO>()) {
                    world.patch<VAO>(entity, [&display_mode](VAO &vao) { vao.mode = display_mode; });
                }
            }
            if (is_selected) { ImGui::SetItemDefaultFocus(); }
        }
        ImGui::EndCombo();
    }
}

} // namespace widget

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
    world.on_destroy<VAO>().connect<VAO::on_destroy>();
    world.on_destroy<VBO<VAO::Attribute::POSITION>>().connect<VBO<VAO::Attribute::POSITION>::on_destroy>();

    {
        constexpr auto positions = std::to_array({
            -0.5f,
            -0.5f,
            0.0f, // left
            0.5f,
            -0.5f,
            0.0f, // right
            0.0f,
            0.5f,
            0.0f // top
        });

        constexpr auto colors = std::to_array({
            1.0f,
            0.0f,
            0.0f,
            1.0f, // left
            0.0f,
            1.0f,
            0.0f,
            1.0f, // right
            0.0f,
            0.0f,
            1.0f,
            1.0f, // top
        });

        for (int i = 0; i != 10; i++) {
            const auto entity = world.create();
            VBO<VAO::Attribute::POSITION>::emplace(world, entity, positions, 3);
            VBO<VAO::Attribute::COLOR>::emplace(world, entity, colors, 4);
            world.emplace<Position3f>(entity, glm::vec3{i * 1.5, 0, 0});
            world.emplace<Rotation3f>(entity, glm::vec3{i * 10, i * 10, i * 10});
            world.emplace<Scale3f>(entity, glm::vec3{i, i, i});
        }
    }

    auto display_mode = VAO::DEFAULT_MODE;

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

        widget::debugSwitchDisplayMode(world, display_mode);

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

        const auto render =
            [&shader](const VAO &vao, const Position3f &pos, const Rotation3f &rot, const Scale3f &scale) {
                auto model = glm::mat4(1.0f);
                model = glm::translate(model, pos.vec);
                model = glm::rotate(model, glm::radians(rot.vec.x), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(rot.vec.y), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(rot.vec.z), glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, scale.vec);
                shader.setUniform("model", model);

                CALL_OPEN_GL(::glBindVertexArray(vao.object));
                CALL_OPEN_GL(::glDrawArrays(static_cast<GLenum>(vao.mode), 0, vao.count));
            };

        [[maybe_unused]] static constexpr auto NO_POSITION = glm::vec3{0.0f, 0.0f, 0.0f};
        [[maybe_unused]] static constexpr auto NO_ROTATION = glm::vec3{0.0f, 0.0f, 0.0f};
        [[maybe_unused]] static constexpr auto NO_SCALE = glm::vec3{1.0f, 1.0f, 1.0f};

        world.view<VAO>(entt::exclude<Position3f, Rotation3f, Scale3f>).each([&render](const auto &vao) {
            render(vao, {NO_POSITION}, {NO_ROTATION}, {NO_SCALE});
        });

        world.view<VAO, Position3f>(entt::exclude<Rotation3f, Scale3f>)
            .each([&render](const auto &vao, const auto &pos) { render(vao, pos, {NO_ROTATION}, {NO_SCALE}); });

        world.view<VAO, Rotation3f>(entt::exclude<Position3f, Scale3f>)
            .each([&render](const auto &vao, const auto &rot) { render(vao, {NO_POSITION}, rot, {NO_SCALE}); });

        world.view<VAO, Scale3f>(entt::exclude<Position3f, Rotation3f>)
            .each([&render](const auto &vao, const auto &scale) {
                render(vao, {NO_POSITION}, {NO_ROTATION}, scale);
            });

        world.view<VAO, Position3f, Scale3f>(entt::exclude<Rotation3f>)
            .each([&render](const auto &vao, const auto &pos, const auto &scale) {
                render(vao, pos, {NO_ROTATION}, scale);
            });

        world.view<VAO, Rotation3f, Scale3f>(entt::exclude<Position3f>)
            .each([&render](const auto &vao, const auto &rot, const auto &scale) {
                render(vao, {NO_POSITION}, rot, scale);
            });

        world.view<VAO, Position3f, Rotation3f>(entt::exclude<Scale3f>)
            .each([&render](const auto &vao, const auto &pos, const auto &rot) {
                render(vao, pos, rot, {NO_SCALE});
            });

        world.view<VAO, Position3f, Rotation3f, Scale3f>().each(
            [&render](const auto &vao, const auto &pos, const auto &rot, const auto &scale) {
                render(vao, pos, rot, scale);
            });

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
