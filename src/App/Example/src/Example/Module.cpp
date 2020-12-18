#include "Example/Module.hpp"

EXTERN_C auto module_ctor() -> engine::api::Module *
{
    return new example::Module{};
}

EXTERN_C auto module_dtor(engine::api::Module *obj) -> void
{
    if (const auto obj_example = dynamic_cast<example::Module *>(obj); obj_example != nullptr)
        delete obj_example;
}
