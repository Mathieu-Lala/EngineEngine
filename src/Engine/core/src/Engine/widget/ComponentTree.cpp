#include <Engine/component/all.hpp>

#include "Engine/widget/ComponentTree.hpp"

// todo : should use world.patch

template<>
auto engine::core::widget::ComponentTree::drawComponentTweaker(api::Position3f &position) const -> void
{
    ImGui::InputFloat3("position", &position.vec.x, 3);
}

template<>
auto engine::core::widget::ComponentTree::drawComponentTweaker(api::Rotation3f &rotation) const -> void
{
    ImGui::InputFloat3("rotation", &rotation.vec.x, 3);
}

template<>
auto engine::core::widget::ComponentTree::drawComponentTweaker(api::Scale3f &scale) const -> void
{
    ImGui::InputFloat3("scale", &scale.vec.x, 3);
}

template<>
auto engine::core::widget::ComponentTree::drawComponentTweaker(api::Name &name) const -> void
{
    char buffer[255] = {0};
    std::strcpy(buffer, name.str.data());
    if (ImGui::InputText("name", buffer, sizeof(buffer))) { name.str = buffer; }
}

auto engine::core::widget::ComponentTree::draw(entt::registry &world) const -> void
{
    static std::optional<entt::entity> selected = {};
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);
        world.each([this, &world](const auto &entity) {
            const auto &name = world.get_or_emplace<api::Name>(entity, fmt::format("<anonymous#{}>", entity));
            if (ImGui::Selectable(name.str.data(), selected.has_value() && selected.value() == entity))
                selected = entity;
        });

        ImGui::EndChild();
    }
    ImGui::SameLine();

    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text(
            "Entity: %s",
            selected.has_value() ? world.get<api::Name>(selected.value()).str.data() : "No entity selected");
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
            const auto try_variant = [&world, this]<typename Variant>(entt::entity e) {
                try {
                    if (world.has<Variant>(e)) {
                        if (ImGui::BeginTabItem(Variant::name.data())) {
                            ImGui::TextWrapped(
                                R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. )");
                            this->drawComponentTweaker(world.get<Variant>(e));
                            ImGui::SameLine();
                            if (ImGui::Button(fmt::format("Delete###{}", Variant::name).data())) {
                                spdlog::warn("Deleting component {} of {}", Variant::name, e);
                                world.remove<Variant>(e);
                            }
                            ImGui::EndTabItem();
                        }
                    }
                } catch (const std::exception &) {
                }
            };

            const auto draw_component =
                [&try_variant]<typename... T>(entt::entity entity, const std::variant<std::monostate, T...> &)
            {
                (try_variant.template operator()<T>(entity), ...);
            };

            if (selected.has_value()) { draw_component(selected.value(), api::Component{}); }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        if (ImGui::Button("Delete")) {
            if (selected.has_value()) {
                spdlog::warn("Deleting entity {}", selected.value());
                world.destroy(selected.value());
                selected = {};
            }
        }
        ImGui::EndGroup();
    }
}
