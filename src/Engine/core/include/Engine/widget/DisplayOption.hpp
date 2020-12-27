#pragma once

#include <entt/entt.hpp>
#include <magic_enum.hpp>

#include <Engine/component/all.hpp>

namespace engine {
namespace core {

namespace widget {

struct DisplayMode {
    api::VAO::DisplayMode &display_mode;

    auto draw(entt::registry &world) const -> void
    {
        constexpr auto enum_name = magic_enum::enum_type_name<api::VAO::DisplayMode>()
            .substr(std::strlen("engine::api::"));
        if (ImGui::BeginCombo(
                "##combo", fmt::format("{} = {}", enum_name.data(), magic_enum::enum_name(display_mode)).data())) {
            for (const auto &i : api::VAO::DISPLAY_MODES) {
                const auto is_selected = display_mode == i;
                if (ImGui::Selectable(magic_enum::enum_name(i).data(), is_selected)) {
                    display_mode = i;
                    for (const auto &entity : world.view<api::VAO>()) {
                        world.patch<api::VAO>(
                            entity, [this](api::VAO &vao) { vao.mode = display_mode; });
                    }
                }
                if (is_selected) { ImGui::SetItemDefaultFocus(); }
            }
            ImGui::EndCombo();
        }
    }
};

} // namespace widget

} // namespace core
} // namespace engine
