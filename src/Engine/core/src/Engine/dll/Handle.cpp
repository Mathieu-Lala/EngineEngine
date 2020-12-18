#include <utility>
#include "Engine/dll/Handle.hpp"

using namespace engine::core::dll;

Handle::Handle() noexcept : m_handle(EMPTY) {}

Handle::Handle(Handle &&o) noexcept : m_handle(std::exchange(o.m_handle, EMPTY)) {}

auto Handle::operator=(Handle &&o) noexcept -> Handle &
{
    this->m_handle = std::exchange(o.m_handle, EMPTY);
    return *this;
}

Handle::Handle(const std::string_view libpath) : m_handle(EMPTY) { this->open(libpath); }

Handle::~Handle() { this->close(); }

auto Handle::is_valid() const noexcept -> bool { return this->m_handle != EMPTY; }

auto Handle::open(const std::string_view libpath) -> void
{
    this->close();

#if defined(_WIN32)
    this->m_handle = ::LoadLibrary(libpath.data());
#else
    this->m_handle = ::dlopen(libpath.data(), RTLD_LAZY);
#endif

    if (!this->is_valid()) throw error{};
}

auto Handle::close() -> void
{
    if (!this->is_valid()) return;

#if defined(_WIN32)
    auto ok = ::FreeLibrary(this->m_handle);
#else
    auto ok = !::dlclose(this->m_handle);
#endif

    this->m_handle = EMPTY;

    if (!ok) throw error{};
}

Handle::error::error(const std::string &msg) : std::runtime_error(msg) {}

auto Handle::error::getLastError() -> std::string
{
#if defined(_WIN32)
    const auto id = ::GetLastError();
    if (!id) return "";
    LPSTR buffer = nullptr;
    const auto size = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        id,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer),
        0,
        NULL);
    const auto message = std::string(buffer, size);
    ::LocalFree(buffer);
    return message;
#else
    return ::dlerror();
#endif
}

auto Handle::set_extension(const std::string_view in) noexcept -> std::string
{
#if defined(_WIN32)
#    ifdef NDEBUG
    return std::string{in} + ".dll";
#    else
    return std::string{in} + "-d.dll";
#    endif
#else
#    ifdef NDEBUG
    return std::string{"lib"} + in.data() + ".so";
#    else
    return std::string{"lib"} + in.data() + "-d.so";
#    endif
#endif
}
