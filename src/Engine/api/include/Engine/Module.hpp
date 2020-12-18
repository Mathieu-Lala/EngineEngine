#pragma once

#include "Engine/api.hpp"

namespace engine {
namespace api {

class Module {
public:

    virtual ~Module() = default;

    virtual auto name() const noexcept -> const char * = 0;

    virtual auto instance() const noexcept -> void * = 0;

    using ctor = Module *(*)();
    using dtor = void (*)(Module *);

};

} // namespace api
} // namespace engine
