#pragma once

#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <Engine/Module.hpp>
#include <Engine/component/all.hpp>

#include "Example/CheckeredFloor.hpp"

namespace example {

class Scene : public engine::api::Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    entt::registry *m_world;

    CheckeredFloor floor;

    auto onCreate(entt::registry &world) noexcept -> void final
    {
        m_world = &world;
        /*        for (int i = 1; i != 1000; i++) {
                    const auto triangle = world.create();
                    engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, triangle,
           data::triangle_positions, 3); engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world,
           triangle, data::triangle_colors, 4); world.emplace<engine::api::Position3f>(triangle, glm::vec3{i
           * 1.5, 0, 0}); world.emplace<engine::api::Rotation3f>(triangle, glm::vec3{i * 10, i * 10, i * 10});
                    world.emplace<engine::api::Scale3f>(triangle, glm::vec3{i, i, i});
                }

                {
                    const auto square = world.create();
                    engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, square,
           data::square_positions, 3); engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world,
           square, data::square_colors, 4); engine::api::EBO::emplace(world, square, data::square_indices);
                }
        */

        {
            const auto cube = world.create();
            engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, cube, data::cube_positions, 3);
            engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world, cube, data::cube_colors, 4);
            engine::api::EBO::emplace(world, cube, data::cube_indices);
            world.emplace<engine::api::Position3f>(cube, glm::vec3{0, 0, 0.0f});
        }

        floor.create(floor.m_size_of_square, floor.m_number_of_square, world);
    }

    auto onDrawUI() noexcept -> void final
    {
        ImGui::Begin("CheckeredFloor");

        auto floor_updated = false;
        floor_updated |= ImGui::DragFloat("Floor Tile Size", &floor.m_size_of_square, 0.1f, 0.01f, 100.0f);
        floor_updated |= ImGui::DragInt("Floor Number of Square", &floor.m_number_of_square, 0.1f, 1, 20);

        floor_updated |= ImGui::ColorEdit3("Dark Color", glm::value_ptr(floor.m_dark_color));
        floor_updated |= ImGui::ColorEdit3("Light Color", glm::value_ptr(floor.m_light_color));
        if (floor_updated) floor.create(floor.m_size_of_square, floor.m_number_of_square, *m_world);

        ImGui::End();
    }

    auto onUpdate() noexcept -> void final {}

    auto onDestroy() noexcept -> void final { std::cout << "Scene destroyed\n"; }
};

class Module : public engine::api::Module {
public:
    Module() { std::cout << "Example ctor\n"; }

    virtual ~Module() { std::cout << "Example dtor\n"; }

    auto name() const noexcept -> const char * { return "Example"; }

    auto instance() const noexcept -> void * { return reinterpret_cast<void *>(new Scene{}); }

    constexpr auto getCategory() const noexcept -> Category final
    {
        return engine::api::Module::Category::SCENE;
    }
};

} // namespace example
