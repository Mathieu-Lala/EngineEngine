#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

#if defined(_WIN32)
#    define WIN32_MEAN_AND_LEAN
#    include <windows.h>
#else
#    include <dlfcn.h>
#endif

namespace engine {
namespace core {
namespace dll {

class Handle {
public:
    Handle() noexcept;
    Handle(Handle &&) noexcept;
    Handle &operator=(Handle &&) noexcept;
    explicit Handle(const std::string_view libpath);

    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    ~Handle();

    bool is_valid() const noexcept;

    void open(const std::string_view libpath);
    void close();

    template<typename T>
    T load(const std::string_view symbol_name) const
    {
#if defined(_WIN32)
        auto symbol = ::GetProcAddress(this->m_handle, symbol_name.data());
#else
        auto symbol = ::dlsym(this->m_handle, symbol_name.data());
#endif
        if (!symbol) throw error{};

        return reinterpret_cast<T>(symbol);
    }

    class error : public std::runtime_error {
    public:
        explicit error(const std::string &msg = getLastError());
        virtual ~error() = default;

        static std::string getLastError();
    };

    static auto set_extension(const std::string_view) noexcept -> std::string;

private:
#if defined(_WIN32)
    using raw_t = HINSTANCE;
    static constexpr raw_t EMPTY = nullptr;
#else
    using raw_t = void *;
    static constexpr raw_t EMPTY = nullptr;
#endif

    raw_t m_handle;
};

} // namespace dll
} // namespace core
} // namespace engine
