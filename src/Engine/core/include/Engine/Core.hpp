#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include <Engine/Module.hpp>

#include "Engine/dll/Handler.hpp"

namespace engine {

namespace core {

class Core {
public:
    static auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
    {
        spdlog::info("{} - v{}", PROJECT_NAME, PROJECT_VERSION);

        CLI::App app{PROJECT_NAME " description", argv[0]};
        CLI11_PARSE(app, argc, argv);

        try {
            dll::Handler module_handle{"libexample-d.so"};

            const auto ctor = module_handle.load<api::Module::ctor>("module_ctor");
            const auto dtor = module_handle.load<api::Module::dtor>("module_dtor");

            const auto ptr = std::unique_ptr<api::Module, api::Module::dtor>(ctor(), dtor);

            spdlog::info(ptr->name());
            return 0;

        } catch (const dll::Handler::error &e) {
            spdlog::error("{}", e.what());
            return 1;
        }
    }
};

} // namespace core

} // namespace engine
