#pragma once

#include <Engine/component/all.hpp>

#include "Example/data.hpp"

namespace example {

using namespace engine::api;

class CheckeredFloor {
public:
    CheckeredFloor() = default;

    float m_size_of_square{10.f};
    std::int32_t m_number_of_square{10};

    glm::vec4 m_dark_color{0.3, 0.3, 0.3, 1.0};
    glm::vec4 m_light_color{0.7, 0.7, 0.7, 1.0};

    auto create(float size_of_square, std::int32_t number_of_square, entt::registry &world)
    {
        world.destroy(m_previous.begin(), m_previous.end());
        m_previous.clear();

        const auto dark_color = data::to_vertices<4ul>(m_dark_color);
        const auto light_color = data::to_vertices<4ul>(m_light_color);

        m_size_of_square = size_of_square;
        m_number_of_square = number_of_square;
        const auto offset = static_cast<float>(number_of_square) / 2.0f;
        for (auto y = -offset; y < offset; y++) {
            for (auto x = -offset; x < offset; x++) {
                const auto square = world.create();

                const auto x_int = static_cast<int>(x) + ((number_of_square % 2) && (x < 0.0f));
                const auto y_int = static_cast<int>(y) + ((number_of_square % 2) && (y < 0.0f));
                const auto is_dark_tile = ((x_int % 2) || (y_int % 2)) && !((x_int % 2) && (y_int % 2));

                VBO<VAO::Attribute::POSITION>::emplace(world, square, data::square_positions, 3);
                VBO<VAO::Attribute::COLOR>::emplace(world, square, is_dark_tile ? dark_color : light_color, 4);
                EBO::emplace(world, square, data::square_indices);

                world.emplace<Position3f>(
                    square, glm::vec3{(x + 0.5f) * size_of_square, 0, (y + 0.5f) * size_of_square});
                world.emplace<Rotation3f>(square, glm::vec3{90, 0, 0});
                world.emplace<Scale3f>(square, glm::vec3{size_of_square, size_of_square, size_of_square});

                world.emplace<Name>(square, fmt::format("cf_{}_{}", int{x}, int{y}));

                m_previous.emplace_back(square);
            }
        }
    }

private:
    std::vector<entt::entity> m_previous;
};

} // namespace example
