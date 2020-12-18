#pragma once

#include <iostream>

#include <Engine/Module.hpp>

namespace example {

class Module : public engine::api::Module {
public:

    Module()
    {
        std::cout << "Example ctor\n";
    }

    virtual ~Module()
    {
        std::cout << "Example dtor\n";
    }

    auto name() const noexcept -> const char *
    {
        return "Example";
    }

    auto instance() const noexcept -> void *
    {
        return nullptr;
    }

};

} // namespace example
