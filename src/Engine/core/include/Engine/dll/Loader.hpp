#pragma once

#include <entt/entt.hpp>

#include <Engine/Module.hpp>

#include "Engine/dll/Handle.hpp"

namespace engine {
namespace core {
namespace dll {

struct HandleLoader : entt::resource_loader<HandleLoader, Handle> {
    auto load(const std::string_view module_name) const -> std::shared_ptr<Handle>
    {
        return std::make_shared<Handle>(Handle::set_extension(module_name));
    }
};

struct ModuleLoader : entt::resource_loader<ModuleLoader, api::Module> {
    auto load(const Handle &module_handle) const -> std::shared_ptr<api::Module>
    {
        const auto ctor = module_handle.load<api::Module::ctor>("module_ctor");
        const auto dtor = module_handle.load<api::Module::dtor>("module_dtor");

        return std::shared_ptr<api::Module>(ctor(), dtor);
    }
};

} // namespce dll
} // namespace core
} // namespace engine
