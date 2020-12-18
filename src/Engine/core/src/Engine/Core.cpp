#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "Engine/Core.hpp"

#include "Engine/dll/Loader.hpp"

auto engine::core::Core::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int
{
    spdlog::info("{} - v{}", PROJECT_NAME, PROJECT_VERSION);

    std::string module_name{"default"};

    CLI::App app{PROJECT_NAME " description", argv[0]};
    [[maybe_unused]] const auto module_name_opt = app.add_option("-m,--module", module_name, "Module to load.");

    CLI11_PARSE(app, argc, argv);

    Core core{};

    if (const auto module_obj = core.loadModule(module_name)) {
        core.m_module = module_obj;
    } else {
        spdlog::error("Initializatio of module failed...");
        return 1;
    }

    spdlog::info("{}", core.m_module->name());

    core.loop();
    return 0;
}

auto engine::core::Core::loop() -> void
{
    while (m_is_running) {}
}

auto engine::core::Core::loadModule(const std::string_view name) -> const api::Module *
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
