#pragma once

#include <memory>

#include <entt/entt.hpp>

#include <Engine/Module.hpp>

#include "Engine/dll/Handle.hpp"

namespace engine {
namespace core {

class Core {
public:
    static auto main(int argc, char **argv) -> int;

    Core() = default;
    ~Core() = default;

private:
    auto loadModule(const std::string_view) -> const api::Module *;

    auto loop() -> void;

private:
    entt::resource_cache<dll::Handle> m_cache_module_handle;
    entt::resource_cache<api::Module> m_cache_module;

    const api::Module *m_module{nullptr};

    bool m_is_running{false};
};

} // namespace core
} // namespace engine
