#pragma once

#include <array>
#include <chrono>
#include <string_view>
#include <variant>

#include <magic_enum.hpp>

namespace engine {
namespace api {

// Event Helper

template<typename Source>
struct Pressed {
    constexpr static std::string_view name{"Pressed"};
    constexpr static std::array elements{std::string_view{"source"}};
    Source source;
};

template<typename Source>
struct Released {
    constexpr static std::string_view name{"Released"};
    constexpr static std::array elements{std::string_view{"source"}};
    Source source;
};

template<typename Source>
struct Moved {
    constexpr static std::string_view name{"Moved"};
    constexpr static std::array elements{std::string_view{"source"}};
    Source source;
};

template<typename Source>
struct Connected {
    constexpr static std::string_view name{"Connected"};
    constexpr static std::array elements{std::string_view{"source"}};
    Source source;
};

template<typename Source>
struct Disconnected {
    constexpr static std::string_view name{"Disconnected"};
    constexpr static std::array elements{std::string_view{"source"}};
    Source source;
};

// Event Type

/// Window Related

struct CloseWindow { // note : should be Disonnected<Window>
    constexpr static std::string_view name{"CloseWindow"};
    constexpr static std::array<std::string_view, 0> elements{};
};

struct OpenWindow { // note : should be Connected<Window>
    constexpr static std::string_view name{"OpenWindow"};
    constexpr static std::array<std::string_view, 0> elements{};
};

struct ResizeWindow {
    constexpr static std::string_view name{"ResizeWindow"};
    constexpr static auto elements = std::to_array<std::string_view>({"width", "height"});
    int width;
    int height;
};

struct MoveWindow { // note : could use Moved<Window> ?
    constexpr static std::string_view name{"MoveWindow"};
    constexpr static auto elements = std::to_array<std::string_view>({"x", "y"});
    int x;
    int y;
};

struct TimeElapsed {
    constexpr static std::string_view name{"TimeElapsed"};
    constexpr static std::array elements{std::string_view{"elapsed"}};
    std::chrono::steady_clock::duration elapsed;
};

/// Device Related

struct Key {
    constexpr static std::string_view name{"Key"};
    constexpr static auto elements =
        std::to_array<std::string_view>({"alt", "control", "system", "shift", "scancode", "key"});
    bool alt;
    bool control;
    bool system;
    bool shift;

    // todo : normalize this
    int scancode;
    int key;
};

struct Mouse {
    constexpr static std::string_view name{"Mouse"};
    constexpr static auto elements = std::to_array<std::string_view>({"x", "y"});
    double x;
    double y;
};

struct MouseButton {
    constexpr static std::string_view name{"MouseButton"};
    constexpr static auto elements = std::to_array<std::string_view>({"button", "mouse"});

    enum class Button {
        BUTTON_1 = GLFW_MOUSE_BUTTON_1,
        BUTTON_2 = GLFW_MOUSE_BUTTON_2,
        BUTTON_3 = GLFW_MOUSE_BUTTON_3,
        BUTTON_4 = GLFW_MOUSE_BUTTON_4,
        BUTTON_5 = GLFW_MOUSE_BUTTON_5,
        BUTTON_6 = GLFW_MOUSE_BUTTON_6,
        BUTTON_7 = GLFW_MOUSE_BUTTON_7,
        BUTTON_8 = GLFW_MOUSE_BUTTON_8,
        BUTTON_LAST = GLFW_MOUSE_BUTTON_LAST,
        BUTTON_LEFT = GLFW_MOUSE_BUTTON_LEFT,
        BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
        BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
    };

    constexpr static auto toButton(int value) noexcept
    {
        return magic_enum::enum_cast<Button>(value).value_or(Button::BUTTON_LAST);
    }

    Button button;
    Mouse mouse;
};

struct Character {
    constexpr static std::string_view name{"Character"};
    constexpr static auto elements = std::to_array<std::string_view>({"codepoint"});
    std::uint32_t codepoint;
};

struct Joystick {
    constexpr static std::string_view name{"Joystick"};
    constexpr static auto elements = std::to_array<std::string_view>({"id", "axes", "buttons"});

    enum Axis {
        LSX, // left stick X
        LSY, // left stick Y
#ifndef WIN32
        LST, // left shoulder trigger
#endif
        RSX, // right stick X
        RSY, // right stick Y

#ifdef WIN32
        LST, // left shoulder trigger
#endif
        RST, // right shoulder trigger

        AXES_MAX = 6,
    };

    enum Buttons {

        ACTION_BOTTOM, // A
        ACTION_RIGHT,  // B
        ACTION_LEFT,   // X
        ACTION_TOP,    // Y

        LS, // left shoulder button
        RS, // right shoulder button

        CENTER1, // Back
        CENTER2, // Start

#ifndef WIN32    // Windows override the buttons so we can't use it on Windows
        CENTER3, // Center (xbox home)
#endif
        LSB, // left stick button
        RSB, // right stick button

        UP,    // d-pad top
        RIGHT, // d-pad right
        DOWN,  // d-pad down
        LEFT,  // d-pad left

        NOT_MAPPED, // not used

        BUTTONS_MAX,
    };

    int id;
    std::array<float, AXES_MAX> axes{};
    std::array<bool, BUTTONS_MAX> buttons{};
};

struct JoystickAxis {
    constexpr static std::string_view name{"JoystickAxis"};
    constexpr static auto elements = std::to_array<std::string_view>({"id", "axis", "value"});

    int id;
    Joystick::Axis axis;
    float value;
};

struct JoystickButton {
    constexpr static std::string_view name{"JoystickButton"};
    constexpr static auto elements = std::to_array<std::string_view>({"id", "button"});

    int id;
    Joystick::Buttons button;
};

// EventType

using Event = std::variant<
    std::monostate,

    OpenWindow,
    CloseWindow,
    ResizeWindow,
    MoveWindow,

    TimeElapsed,

    Pressed<Key>,
    Released<Key>,
    Character,

    Moved<Mouse>,
    Pressed<MouseButton>,
    Released<MouseButton>,

    Connected<Joystick>,
    Disconnected<Joystick>,

    Pressed<JoystickButton>,
    Released<JoystickButton>,
    Moved<JoystickAxis>

    >;

} // namespace api
} // namespace engine
