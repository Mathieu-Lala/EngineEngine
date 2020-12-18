#pragma once

#include "Engine/graphics/third_party.hpp"

namespace engine {
namespace core {

class Window {
public:
    Window(int width, int height, const std::string_view name)
    {
        m_handle = ::glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
        if (m_handle == nullptr) { throw std::logic_error("Engine::Window initialization failed"); }
        ::glfwMakeContextCurrent(m_handle);
    }

    auto create_ui_context() -> bool
    {
        IMGUI_CHECKVERSION();

        m_ui_context = ImGui::CreateContext();
        if (!m_ui_context) { return false; }

        if (!ImGui_ImplGlfw_InitForOpenGL(m_handle, true)) { return false; }

        if (!ImGui_ImplOpenGL3_Init()) { return false; }

        return true;
    }

    ~Window()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext(m_ui_context);

        ::glfwDestroyWindow(m_handle);
    }

    [[nodiscard]] auto isOpen() const noexcept -> bool { return ::glfwWindowShouldClose(m_handle) == GLFW_FALSE; }

    auto render() -> void { ::glfwSwapBuffers(m_handle); }

private:
    ::GLFWwindow *m_handle{nullptr};
    ::ImGuiContext *m_ui_context{nullptr};
};

} // namespace core
} // namespace engine
