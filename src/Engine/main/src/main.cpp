#include <iostream>

#include "Engine/api.hpp"
#include "Engine/core.hpp"

int main()
{
    std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << "\n";
    std::cout << "ENGINE_API: " << ENGINE_API << " ENGINE_CORE: " << ENGINE_CORE << "\n";
}
