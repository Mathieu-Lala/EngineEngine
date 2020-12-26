#pragma once

#include <entt/entt.hpp>

#include "Engine/api.hpp"

namespace engine {
namespace api {

class Module {
public:
    virtual ~Module() = default;

    virtual auto name() const noexcept -> const char * = 0;

    virtual auto instance() const noexcept -> void * = 0;

    // allow the possibilty to have different type of plug-in ...
    enum class Category {
        SCENE,
    };

    constexpr virtual auto getCategory() const noexcept -> Category = 0;

    using ctor = Module *(*) ();
    using dtor = void (*)(Module *);
};

class Scene {
public:
    virtual ~Scene() = default;

    virtual auto onCreate(entt::registry &world) noexcept -> void = 0;

    virtual auto onUpdate() noexcept -> void = 0;

    virtual auto onDestroy() noexcept -> void = 0;
};

} // namespace api
} // namespace engine
