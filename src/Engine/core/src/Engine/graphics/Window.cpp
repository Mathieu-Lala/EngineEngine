#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

auto engine::core::Window::screenshot(const std::string_view filename) -> bool
{
    GLint viewport[4];

    CALL_OPEN_GL(::glGetIntegerv(GL_VIEWPORT, viewport));
    const auto &x = viewport[0];
    const auto &y = viewport[1];
    const auto &width = viewport[2];
    const auto &height = viewport[3];

    constexpr auto CHANNEL = 4ul;
    std::vector<char> pixels(static_cast<std::size_t>(width * height) * CHANNEL);

    CALL_OPEN_GL(::glPixelStorei(GL_PACK_ALIGNMENT, 1));
    CALL_OPEN_GL(::glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()));

    std::array<char, CHANNEL> pixel;
    for (auto j = 0; j < height / 2; ++j)
        for (auto i = 0; i < width; ++i) {
            const auto top = static_cast<std::size_t>(i + j * width) * pixel.size();
            const auto bottom = static_cast<std::size_t>(i + (height - j - 1) * width) * pixel.size();

            std::memcpy(pixel.data(), pixels.data() + top, pixel.size());
            std::memcpy(pixels.data() + top, pixels.data() + bottom, pixel.size());
            std::memcpy(pixels.data() + bottom, pixel.data(), pixel.size());
        }

    return !!::stbi_write_png(filename.data(), width, height, 4, pixels.data(), 0);
}

template<>
auto engine::core::Window::useEvent(const api::Pressed<api::MouseButton> &m) -> void
{
    ::ImGui_ImplGlfw_MouseButtonCallback(
        m_handle, magic_enum::enum_integer(m.source.button), GLFW_PRESS, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Released<api::MouseButton> &m) -> void
{
    ::ImGui_ImplGlfw_MouseButtonCallback(
        m_handle, magic_enum::enum_integer(m.source.button), GLFW_RELEASE, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Pressed<api::Key> &k) -> void
{
    ::ImGui_ImplGlfw_KeyCallback(
        m_handle, static_cast<int>(k.source.keycode), k.source.scancode, GLFW_PRESS, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Released<api::Key> &k) -> void
{
    ::ImGui_ImplGlfw_KeyCallback(
        m_handle, static_cast<int>(k.source.keycode), k.source.scancode, GLFW_RELEASE, 0 /* todo */);
}

template<>
auto engine::core::Window::useEvent(const api::Character &character) -> void
{
    ::ImGui_ImplGlfw_CharCallback(m_handle, character.codepoint);
}
