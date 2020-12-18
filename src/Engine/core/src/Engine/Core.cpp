#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "Engine/Core.hpp"

#include "Engine/dll/Loader.hpp"
#include "Engine/graphics/third_party.hpp"

#include "Engine/graphics/Shader.hpp"

auto engine::core::Core::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int
{
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

auto engine::core::Core::loop() -> void
{
    constexpr auto VERT_SH = R"(#version 450
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

    constexpr auto FRAG_SH = R"(#version 450
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

    Shader shader(VERT_SH, FRAG_SH);
    shader.use();

    float vertices[] = {
        -0.5f,
        -0.5f,
        0.0f, // left
        0.5f,
        -0.5f,
        0.0f, // right
        0.0f,
        0.5f,
        0.0f // top
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    m_is_running = true;
    while (m_is_running && m_window->isOpen()) {
        ::glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowMetricsWindow();
        ImGui::Render();

        ::glClearColor(0.0f, 1.0f, 0.2f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->render();
    }
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
