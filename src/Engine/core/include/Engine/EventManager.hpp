#pragma once

#include <vector>

#include "Engine/third_party.hpp"
#include "Engine/Event.hpp"

#include "Engine/graphics/Window.hpp"
#include "Engine/helpers/overloaded.hpp"

namespace engine {
namespace core {

class EventManager {
public:
    EventManager() { s_instance = this; }

    auto registerWindow(const Window &window)
    {
        ::glfwSetWindowCloseCallback(window.get(), callback_eventClose);
        ::glfwSetWindowSizeCallback(window.get(), callback_eventResized);
        ::glfwSetWindowPosCallback(window.get(), callback_eventMoved);
        ::glfwSetKeyCallback(window.get(), callback_eventKeyBoard);
        ::glfwSetMouseButtonCallback(window.get(), callback_eventMousePressed);
        ::glfwSetCursorPosCallback(window.get(), callback_eventMouseMoved);
        ::glfwSetCharCallback(window.get(), callback_char);

        /// todo : ImGui_ImplGlfw_ScrollCallback
        /// glfwSetScrollCallback(window, scroll_callback);
        /// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)

        m_buffer_events.emplace_back(api::OpenWindow{});
        m_events_processed.emplace_back(api::TimeElapsed{});
    }

    auto getNextEvent()
    {
        const auto event = fetchEvent();

        std::visit(
            overloaded{
                [](api::TimeElapsed &prev, const api::TimeElapsed &next) { prev.elapsed += next.elapsed; },
                [&](const auto &, const std::monostate &) {},
                [&](const auto &, const auto &next) { m_events_processed.push_back(next); }},
            m_events_processed.back(),
            event);

        return event;
    }

    auto setCurrentTimepoint(const std::chrono::steady_clock::time_point &t) { m_lastTimePoint = t; }

private:
    static EventManager *s_instance;

    std::chrono::steady_clock::time_point m_lastTimePoint;

    std::vector<api::Event> m_buffer_events; // input buffer

    std::vector<api::Event> m_events_processed; // previous event

    auto fetchEvent() -> api::Event
    {
        ::glfwPollEvents();

        if (m_buffer_events.empty()) {
            return api::TimeElapsed{getElapsedTime()};
        } else {
            const auto event = m_buffer_events.front();
            m_buffer_events.erase(m_buffer_events.begin());
            return event;
        }
    }

    auto getElapsedTime() noexcept -> std::chrono::nanoseconds
    {
        const auto newTimePoint = std::chrono::steady_clock::now();
        const auto timeElapsed = newTimePoint - m_lastTimePoint;
        m_lastTimePoint = newTimePoint;
        return timeElapsed;
    }

    static auto callback_eventClose(::GLFWwindow *window) -> void
    {
        ::glfwSetWindowShouldClose(window, false);
        s_instance->m_buffer_events.emplace_back(api::CloseWindow{});
    }

    static auto callback_eventResized(GLFWwindow *, int w, int h) -> void
    {
        s_instance->m_buffer_events.emplace_back(api::ResizeWindow{w, h});
    }

    static auto callback_eventMoved(GLFWwindow *, int x, int y) -> void
    {
        s_instance->m_buffer_events.emplace_back(api::MoveWindow{x, y});
    }

    static auto callback_eventKeyBoard(GLFWwindow *, int key, int scancode, int action, int mods) -> void
    {
        // clang-format off
        api::Key k{
            .alt        = !!(mods & GLFW_MOD_ALT),
            .control    = !!(mods & GLFW_MOD_CONTROL),
            .system     = !!(mods & GLFW_MOD_SUPER),
            .shift      = !!(mods & GLFW_MOD_SHIFT),
            .scancode   = scancode,
            .key        = key
        };
        // clang-format on
        switch (action) {
        case GLFW_PRESS: s_instance->m_buffer_events.emplace_back(api::Pressed<api::Key>{k}); break;
        case GLFW_RELEASE:
            s_instance->m_buffer_events.emplace_back(api::Released<api::Key>{k});
            break;
            // case GLFW_REPEAT: s_instance->m_buffer_events.emplace_back(???{ key }); break; // todo
            // default: std::abort(); break;
        };
    }

    static auto
        callback_eventMousePressed(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods) -> void
    {
        double x = 0;
        double y = 0;
        ::glfwGetCursorPos(window, &x, &y);
        switch (action) {
        case GLFW_PRESS:
            s_instance->m_buffer_events.emplace_back(api::Pressed<api::MouseButton>{button, {x, y}});
            break;
        case GLFW_RELEASE:
            s_instance->m_buffer_events.emplace_back(api::Released<api::MouseButton>{button, {x, y}});
            break;
            // default: std::abort(); break;
        };
    }

    static auto callback_eventMouseMoved(GLFWwindow *, double x, double y) -> void
    {
        s_instance->m_buffer_events.emplace_back(api::Moved<api::Mouse>{x, y});
    }

    static auto callback_char(GLFWwindow *, unsigned int codepoint) -> void
    {
        s_instance->m_buffer_events.emplace_back(api::Character{codepoint});
    }
};

} // namespace core
} // namespace engine
