#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    void CommandList::Reset() { vkResetCommandBuffer(m_handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); };

    void CommandList::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;
        beginInfo.pNext                    = nullptr;
        beginInfo.pInheritanceInfo         = nullptr;

        vkBeginCommandBuffer(m_handle, &beginInfo);
    };

    void CommandList::End() { vkEndCommandBuffer(m_handle); };

    void CommandList::SetViewports(const Viewport* viewports, uint32_t count)
    {
        std::vector<VkViewport> vkViewports(count, VkViewport());

        for (uint32_t i = 0; i < count; i++)
        {
            vkViewports[i].x        = viewports[i].minX;
            vkViewports[i].y        = viewports[i].minY;
            vkViewports[i].width    = viewports[i].maxX - viewports[i].minX;
            vkViewports[i].height   = viewports[i].maxY - viewports[i].minY;
            vkViewports[i].minDepth = viewports[i].minZ;
            vkViewports[i].maxDepth = viewports[i].maxZ;
        }

        vkCmdSetViewport(m_handle, 0, count, vkViewports.data());
    }

    void CommandList::SetScissors(const Rect* scissors, uint32_t count)
    {
        std::vector<VkRect2D> vkScissors(count, VkRect2D());

        for (uint32_t i = 0; i < count; i++)
        {
            vkScissors[i].offset.x      = scissors[i].x;
            vkScissors[i].offset.y      = scissors[i].y;
            vkScissors[i].extent.width  = scissors[i].x + scissors[i].sizeX;
            vkScissors[i].extent.height = scissors[i].y + scissors[i].sizeY;
        }

        vkCmdSetScissor(m_handle, 0, count, vkScissors.data());
    }

    void CommandList::Submit(const CopyCommand& command) {}

    void CommandList::Submit(const DrawCommand& command) {}

    void CommandList::Submit(const DispatchCommand& command) {}

} // namespace Vulkan
} // namespace RHI
