#pragma once

#include <entt/entt.hpp>

#include <Engine/component/all.hpp>

namespace engine {
namespace core {

namespace widget {

struct ComponentTree {
    auto draw(entt::registry &world) const -> void
    {
        if (ImGui::TreeNode("Scene")) {
            world.each([this, &world](const auto &entity) {
                const auto &name =
                    world.get_or_emplace<api::Name>(entity, fmt::format("<anonymous#{}>", entity));
                if (ImGui::TreeNodeEx(
                        fmt::format("##{}", entity).data(),
                        ImGuiTreeNodeFlags_None,
                        "#%d : %s",
                        static_cast<int>(entity),
                        name.str.data())) {
                    const auto draw_component = [&]<typename... T>(const std::variant<std::monostate, T...> &)
                    {
                        const auto try_variant = [&]<typename Variant>() {
                            try {
                                if (world.has<Variant>(entity)) {
                                    ImGui::Text("%s: ", Variant::name.data());
                                    ImGui::SameLine();
                                    this->drawComponentTweaker(world.get<Variant>(entity));
                                    ImGui::SameLine();
                                    if (ImGui::Button(fmt::format("Delete###{}", Variant::name).data())) {
                                        spdlog::warn("Deleting component {} of {}", Variant::name, entity);
                                        world.remove<Variant>(entity);
                                    }
                                }
                            } catch (const std::exception &) {
                            }
                        };

                        (try_variant.template operator()<T>(), ...);
                    };

                    draw_component(api::Component{});
                    ImGui::TreePop();
                }
            });
            ImGui::TreePop();
        }
    }

private:
    template<typename T>
    auto drawComponentTweaker(T &) const -> void
    {
        ImGui::Text("<not implemented>");
    }
};

// todo : should use world.patch

template<>
auto ComponentTree::drawComponentTweaker(api::Position3f &position) const -> void
{
    ImGui::InputFloat3("position", &position.vec.x, 3);
}

template<>
auto ComponentTree::drawComponentTweaker(api::Rotation3f &rotation) const -> void
{
    ImGui::InputFloat3("rotation", &rotation.vec.x, 3);
}

template<>
auto ComponentTree::drawComponentTweaker(api::Scale3f &scale) const -> void
{
    ImGui::InputFloat3("scale", &scale.vec.x, 3);
}

template<>
auto ComponentTree::drawComponentTweaker(api::Name &name) const -> void
{
    char buffer[255] = {0};
    std::strcpy(buffer, name.str.data());
    if (ImGui::InputText("name", buffer, sizeof(buffer))) { name.str = buffer; }
}

} // namespace widget

} // namespace core
} // namespace engine
