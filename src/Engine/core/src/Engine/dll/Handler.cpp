#include <utility>
#include "Engine/dll/Handler.hpp"

using namespace engine::core::dll;

Handler::Handler() noexcept : m_handler(EMPTY) {}

Handler::Handler(Handler &&o) noexcept : m_handler(std::exchange(o.m_handler, EMPTY)) {}

auto Handler::operator=(Handler &&o) noexcept -> Handler &
{
    this->m_handler = std::exchange(o.m_handler, EMPTY);
    return *this;
}

Handler::Handler(const std::string_view libpath) : m_handler(EMPTY) { this->open(libpath); }

Handler::~Handler() { this->close(); }

auto Handler::is_valid() const noexcept -> bool { return this->m_handler != EMPTY; }

auto Handler::open(const std::string_view libpath) -> void
{
    this->close();

#if defined(_WIN32)
    this->m_handler = ::LoadLibrary(libpath.data());
#else
    this->m_handler = ::dlopen(libpath.data(), RTLD_LAZY);
#endif

    if (!this->is_valid()) throw error{};
}

auto Handler::close() -> void
{
    if (!this->is_valid()) return;

#if defined(_WIN32)
    auto ok = ::FreeLibrary(this->m_handler);
#else
    auto ok = !::dlclose(this->m_handler);
#endif

    this->m_handler = EMPTY;

    if (!ok) throw error{};
}

Handler::error::error(const std::string &msg) : std::runtime_error(msg) {}

auto Handler::error::getLastError() -> std::string
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
    std::string message(buffer, size);
    ::LocalFree(buffer);
    return message;
#else
    return ::dlerror();
#endif
}
