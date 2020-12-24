#include "Engine/graphics/Window.hpp"

engine::core::Window::Window(int width, int height, const std::string_view name)
{
    m_handle = ::glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    if (m_handle == nullptr) { throw std::logic_error("Engine::Window initialization failed"); }
    ::glfwMakeContextCurrent(m_handle);
}

auto engine::core::Window::create_ui_context() -> bool
{
    IMGUI_CHECKVERSION();

    m_ui_context = ImGui::CreateContext();
    if (!m_ui_context) { return false; }

    if (!::ImGui_ImplGlfw_InitForOpenGL(m_handle, false)) { return false; }

    if (!::ImGui_ImplOpenGL3_Init()) { return false; }

    return true;
}

engine::core::Window::~Window()
{
    ::ImGui_ImplOpenGL3_Shutdown();
    ::ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext(m_ui_context);

    ::glfwDestroyWindow(m_handle);
}

auto engine::core::Window::isOpen() const noexcept -> bool
{
    return ::glfwWindowShouldClose(m_handle) == GLFW_FALSE;
}

auto engine::core::Window::render() -> void { ::glfwSwapBuffers(m_handle); }

template<>
auto engine::core::Window::useEvent(const api::Pressed<api::MouseButton> &m) -> void
{
    ::ImGui_ImplGlfw_MouseButtonCallback(m_handle, m.source.button, GLFW_PRESS, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Released<api::MouseButton> &m) -> void
{
    ::ImGui_ImplGlfw_MouseButtonCallback(m_handle, m.source.button, GLFW_RELEASE, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Pressed<api::Key> &k) -> void
{
    ::ImGui_ImplGlfw_KeyCallback(m_handle, k.source.key, k.source.scancode, GLFW_PRESS, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Released<api::Key> &k) -> void
{
    ::ImGui_ImplGlfw_KeyCallback(m_handle, k.source.key, k.source.scancode, GLFW_RELEASE, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Character &character) -> void
{
    ::ImGui_ImplGlfw_CharCallback(m_handle, character.codepoint);
}
