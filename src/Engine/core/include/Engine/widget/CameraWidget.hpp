#pragma once

#include <Engine/graphics/Shader.hpp>
#include <Engine/Camera.hpp>

namespace engine {
namespace core {

namespace widget {

struct CameraWidget {
    engine::core::Shader &shader;
    engine::core::Camera &camera;

    auto draw(bool &camera_auto_move) const -> void
    {
        ImGui::Text("Projection Mode ");
        bool projection_changed{false};
        int new_projection = magic_enum::enum_integer(camera.getProjectionType());
        for (const auto &i : magic_enum::enum_values<Camera::ProjectionType>()) {
            ImGui::SameLine();
            projection_changed |= ImGui::RadioButton(
                magic_enum::enum_name(i).data(), &new_projection, magic_enum::enum_integer(i));
        }
        if (projection_changed) {
            camera.setProjectionType(magic_enum::enum_cast<Camera::ProjectionType>(new_projection).value());
            shader.setUniform("projection", camera.getProjection());
        }
        ImGui::InputFloat3("Position", &camera.getPosition().x, 3);
        ImGui::SameLine();
        ImGui::Checkbox("Automove", &camera_auto_move);
        auto new_fov = camera.getFOV();
        if (ImGui::SliderFloat("FOV", &new_fov, 0.001f, 179.999f, "%.3f")) { camera.setFOV(new_fov); }
        auto neaw_near = camera.getNear();
        if (ImGui::SliderFloat("Near", &neaw_near, 0.001f, 1000.0f, "%.3f")) { camera.setNear(neaw_near); }
        auto near_far = camera.getFar();
        if (ImGui::SliderFloat("Far", &near_far, 0.001f, 1000.0f, "%.3f")) { camera.setFar(near_far); }
    }

private:
};

} // namespace widget

} // namespace core
} // namespace engine
