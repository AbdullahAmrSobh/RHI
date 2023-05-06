#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/Command.hpp"

namespace Vulkan
{

class Context;
class PipelineState;

class BarrierTracker 
{
public:
    RHI::ImagePassAttachment* GetImagePassAttachment(Image& image);

    vk::ImageLayout GetImageLayout(RHI::ImagePassAttachment& attachment); 
};

class RHI_EXPORT CommandList final
    : public RHI::CommandList
    , public DeviceObject<vk::CommandBuffer>
{
public:
    CommandList(vk::CommandBuffer& commandBuffer);
    ~CommandList();

    void Begin();
    void End();

    void BeginRendering();
    void EndRendering();

    void CopyImage(const RHI::ImageSize& imageSize, const RHI::ImageCopyInfo& srcImage, RHI::ImageCopyInfo& dstImage) override;
    void CopyBuffer(const RHI::BufferCopyInfo& srcBuffer, RHI::ImageCopyInfo& dstBuffer) override;
    void CopyBufferToImage(const RHI::BufferCopyInfo& srcBuffer, RHI::ImageCopyInfo& dstImage) override;
    void CopyImageToBuffer(const RHI::ImageCopyInfo& srcImage, RHI::BufferCopyInfo& dstBuffer) override;
    void BeginPredication(const RHI::Buffer& buffer, uint64_t offset, RHI::PredicationOp operation) override;
    void EndPredication() override;
    void SetPipelineState(const RHI::PipelineState& pso) override;
    void BindShaderResourceGroup(const RHI::ShaderResourceGroup& group) override;
    void SetRenderArea(const RHI::DrawArea& drawArea) override;
    void DrawIndexed(const RHI::DrawIndexedData& indexedData) override;
    void DrawUnindexed(const RHI::DrawUnindexedData& unindexedData) override;
    void DispatchRay(const RHI::DispatchRaysData& raysData) override;
    void Dispatch(uint32_t offset[3], uint32_t groupCount[3]) override;

private:
    BarrierTracker*                           m_barrierTracker; 
    PassState*                                m_passState;
    PipelineState*                            m_currentPipelineState;
    std::shared_ptr<vk::UniquePipelineLayout> m_layout;
};

}  // namespace Vulkan