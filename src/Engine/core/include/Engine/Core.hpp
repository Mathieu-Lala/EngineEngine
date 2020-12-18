#pragma once

#include <memory>

#include <entt/entt.hpp>

#include <Engine/Module.hpp>

#include "Engine/dll/Handle.hpp"
#include "Engine/graphics/Window.hpp"

namespace engine {
namespace core {

class Core {
public:
    static auto main(int argc, char **argv) -> int;

    Core() = default;
    ~Core() = default;

private:
    auto load_module(const std::string_view) -> const api::Module *;
    auto initialize_graphics(int glfw_context_major, int glfw_context_minor) -> bool;

    auto loop() -> void;

private:
    entt::resource_cache<dll::Handle> m_cache_module_handle;
    entt::resource_cache<api::Module> m_cache_module;

    const api::Module *m_module{nullptr};

    bool m_is_running{false};

    std::unique_ptr<Window> m_window{};

};

} // namespace core
} // namespace engine
