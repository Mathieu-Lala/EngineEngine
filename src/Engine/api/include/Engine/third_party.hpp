#pragma once

// just an helper file to have the header in the right order

#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#    define GLFW_EXPOSE_NATIVE_WIN32
#    include <windows.h>
#endif

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef NDEBUG

#include <stdexcept>
#include <spdlog/spdlog.h>

namespace engine::core {

inline constexpr auto GetGLErrorStr(GLenum err)
{
    switch (err) {
    case GL_NO_ERROR: return "OPEN_GL: No error";
    case GL_INVALID_ENUM: return "OPEN_GL: Invalid enum";
    case GL_INVALID_VALUE: return "OPEN_GL: Invalid value";
    case GL_INVALID_OPERATION: return "OPEN_GL: Invalid operation";
    case GL_STACK_OVERFLOW: return "OPEN_GL: Stack overflow";
    case GL_STACK_UNDERFLOW: return "OPEN_GL: Stack underflow";
    case GL_OUT_OF_MEMORY: return "OPEN_GL: Out of memory";
    default: return "OPEN_GL: Unknown error";
    }
}

} // namespace engine::core

#    define CALL_OPEN_GL(call)                                                           \
        do {                                                                             \
            call;                                                                        \
            if (const auto err = ::glGetError(); GL_NO_ERROR != err) {                   \
                if constexpr (!noexcept(__func__)) {                                     \
                    throw std::runtime_error(engine::core::GetGLErrorStr(err));          \
                } else {                                                                 \
                    spdlog::error("CALL_OPEN_GL: {}", engine::core::GetGLErrorStr(err)); \
                }                                                                        \
            }                                                                            \
        } while (0)

#else

#    define CALL_OPEN_GL(call) \
        do {                   \
            call;              \
        } while (0)

#endif
