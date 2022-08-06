#include <iostream>
#include "RHI/RHI.hpp"
#include <vulkan/vulkan.h>

namespace RHI
{
    void RHI::say_hello() { 
        std::cout << "Hello, World!" << std::endl;
    }

} // namespace RHI