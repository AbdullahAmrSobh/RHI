#include <iostream>

#include <RHI-Vulkan/Loader.hpp>
#include <RHI/RHI.hpp>

int main(int argc, const char* argv[])
{
    auto context = RHI::Vulkan::CreateNew();

    std::cout << "Number: " << context->GetNum() << std::endl;
    std::cout << "Name:   " << context->GetName() << std::endl;

    delete context;
}