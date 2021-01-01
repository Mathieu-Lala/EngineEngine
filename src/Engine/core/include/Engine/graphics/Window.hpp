#pragma once

#include <glm/vec2.hpp>

#include "Engine/third_party.hpp"
#include "Engine/Event.hpp"

namespace engine {
namespace core {

class Core;

class Window {
public:
    Window(int width, int height, const std::string_view name);

    ~Window();

    auto get() const noexcept -> ::GLFWwindow * { return m_handle; }

    auto create_ui_context() -> bool;

    [[nodiscard]] auto isOpen() const noexcept -> bool;

    auto render() -> void;

    auto screenshot(const std::string_view filename) -> bool;

    template<typename T = double>
    [[nodiscard]] auto getAspectRatio() const noexcept -> T
    {
        int width{};
        int height{};
        ::glfwGetWindowSize(m_handle, &width, &height);
        return static_cast<T>(width) / static_cast<T>(height);
    }

    template<typename T = double>
    [[nodiscard]] auto getSize() const noexcept -> glm::vec<2, T>
    {
        int width{};
        int height{};
        ::glfwGetWindowSize(m_handle, &width, &height);
        return {static_cast<T>(width), static_cast<T>(height)};
    }

    template<typename EventType>
    auto useEvent([[maybe_unused]] const EventType &) -> void
    {
        spdlog::warn("Window#useEvent<T>() : T={} not implemented", EventType::name);
    }

private:
    ::GLFWwindow *m_handle{nullptr};
    ::ImGuiContext *m_ui_context{nullptr};
};

template<>
auto Window::useEvent(const api::Pressed<api::MouseButton> &) -> void;

template<>
auto Window::useEvent(const api::Released<api::MouseButton> &) -> void;

template<>
auto Window::useEvent(const api::Pressed<api::Key> &) -> void;

template<>
auto Window::useEvent(const api::Released<api::Key> &) -> void;

template<>
auto Window::useEvent(const api::Character &) -> void;

} // namespace core
} // namespace engine
