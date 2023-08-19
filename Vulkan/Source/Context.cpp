#include "Context.hpp"

namespace RHI
{
namespace Vulkan
{

Context* CreateNew()
{
    return new ::Vulkan::Context();
}

}  // namespace Vulkan
}  // namespace RHI

namespace Vulkan
{
std::string Context::GetName() const
{
    return "Hello, Vulkan";
}
}  // namespace Vulkan
