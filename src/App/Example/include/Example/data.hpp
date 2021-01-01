#pragma once

#include <array>

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

template<std::size_t N>
constexpr auto to_vertices(const glm::vec4 &in) -> std::array<float, N * 4>
{
    std::array<float, N * 4> out;
    for (auto i = 0ul; i != N * 4ul; i += 4ul) {
        out[i + 0] = in.x;
        out[i + 1] = in.y;
        out[i + 2] = in.z;
        out[i + 3] = in.w;
    }
    return out;
}

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

} // namespace example
