#pragma once

#include <entt/entt.hpp>

namespace engine {
namespace core {

namespace widget {

struct ComponentTree {
    auto draw(entt::registry &world) const -> void;

private:
    template<typename T>
    auto drawComponentTweaker(T &) const -> void
    {
        ImGui::Text("<not implemented>");
    }
};

} // namespace widget

} // namespace core
} // namespace engine
