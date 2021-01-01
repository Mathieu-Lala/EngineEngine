#pragma once

#include <iostream>

#include <Engine/Module.hpp>
#include <Engine/component/all.hpp>

namespace example {

namespace data { // tmp

// clang-format off
constexpr auto triangle_positions = std::to_array({
    -0.5f, -0.5f, 0.0f, // left
    0.5f, -0.5f, 0.0f, // right
    0.0f, 0.5f, 0.0f // top
});

constexpr auto triangle_colors = std::to_array({
    1.0f, 0.0f, 0.0f, 0.5f, // left
    0.0f, 1.0f, 0.0f, 0.5f, // right
    0.0f, 0.0f, 1.0f, 0.5f, // top
});

constexpr auto square_positions = std::to_array({
    0.5f, 0.5f, 0.0f, // top right
    0.5f, -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f, 0.5f, 0.0f // top left
});

constexpr auto square_colors = std::to_array({
    1.0f, 0.0f, 1.0f, 1.0f,// top right
    1.0f, 1.0f, 0.0f, 1.0f, // bottom right
    0.0f, 1.0f, 1.0f, 1.0f, // bottom left
    1.0f, 0.0f, 1.0f, 1.0f// top left
});

constexpr auto square_indices = std::to_array({
    0u, 1u, 3u, // first triangle
    1u, 2u, 3u // second triangle
});

constexpr auto cube_positions = std::to_array({
    -0.5f, 0.5f,  -0.5f, // Point A 0
    -0.5f, 0.5f,  0.5f,  // Point B 1
    0.5f,  0.5f,  -0.5f, // Point C 2
    0.5f,  0.5f,  0.5f,  // Point D 3
    -0.5f, -0.5f, -0.5f, // Point E 4
    -0.5f, -0.5f, 0.5f,  // Point F 5
    0.5f,  -0.5f, -0.5f, // Point G 6
    0.5f,  -0.5f, 0.5f,  // Point H 7
});

constexpr auto cube_colors = std::to_array({
    0.0f, 0.0f, 0.0f, 1.0f, // Point A 0
    0.0f, 0.0f, 1.0f, 1.0f, // Point B 1
    0.0f, 1.0f, 0.0f, 1.0f, // Point C 2
    0.0f, 1.0f, 1.0f, 1.0f, // Point D 3
    1.0f, 0.0f, 0.0f, 1.0f, // Point E 4
    1.0f, 0.0f, 1.0f, 1.0f, // Point F 5
    1.0f, 1.0f, 0.0f, 1.0f, // Point G 6
    1.0f, 1.0f, 1.0f, 1.0f // Point H 7
});

constexpr auto cube_indices = std::to_array({
    /*Above ABC,BCD*/
    0u, 1u, 2u, 1u, 2u, 3u,
    /*Following EFG,FGH*/
    4u, 5u, 6u, 5u, 6u, 7u,
    /*Left ABF,AEF*/
    0u, 1u, 5u, 0u, 4u, 5u,
    /*Right side CDH,CGH*/
    2u, 3u, 7u, 2u, 6u, 7u,
    /*ACG,AEG*/
    0u, 2u, 6u, 0u, 4u, 6u,
    /*Behind BFH,BDH*/
    1u, 5u, 7u, 1u, 3u, 7u
});
// clang-format on

} // namespace data

class Scene : public engine::api::Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    entt::registry *m_world;

    auto onCreate(entt::registry &world) noexcept -> void
    {
        m_world = &world;
        for (int i = 1; i != 1000; i++) {
            const auto triangle = world.create();
            engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, triangle, data::triangle_positions, 3);
            engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world, triangle, data::triangle_colors, 4);
            world.emplace<engine::api::Position3f>(triangle, glm::vec3{i * 1.5, 0, 0});
            world.emplace<engine::api::Rotation3f>(triangle, glm::vec3{i * 10, i * 10, i * 10});
            world.emplace<engine::api::Scale3f>(triangle, glm::vec3{i, i, i});
        }

        {
            const auto square = world.create();
            engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, square, data::square_positions, 3);
            engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world, square, data::square_colors, 4);
            engine::api::EBO::emplace(world, square, data::square_indices);
        }

        {
            const auto cube = world.create();
            engine::api::VBO<engine::api::VAO::Attribute::POSITION>::emplace(world, cube, data::cube_positions, 3);
            engine::api::VBO<engine::api::VAO::Attribute::COLOR>::emplace(world, cube, data::cube_colors, 4);
            engine::api::EBO::emplace(world, cube, data::cube_indices);
            world.emplace<engine::api::Position3f>(cube, glm::vec3{0, 0, 2.0f});
        }
    }

    auto onUpdate() noexcept -> void {
    }

    auto onDestroy() noexcept -> void { std::cout << "Scene destroyed\n"; }
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
