#include "RHI/Resources.hpp"

namespace RHI
{

    //////////////////////////////////////////////////////////////////////////////////////////
    /// BindGroupData
    //////////////////////////////////////////////////////////////////////////////////////////

    void BindGroupData::BindImages(uint32_t index, TL::Span<Handle<ImageView>> handles, uint32_t arrayOffset)
    {
        BindGroupData::ResourceImageBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views = { handles.begin(), handles.end() };
        m_bindings[index] = binding;
    }

    void BindGroupData::BindBuffers(uint32_t index, TL::Span<Handle<Buffer>> handles, uint32_t arrayOffset)
    {
        BindGroupData::ResourceBufferBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views = { handles.begin(), handles.end() };
        m_bindings[index] = binding;
    }

    void BindGroupData::BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset)
    {
        BindGroupData::ResourceSamplerBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.samplers = { samplers.begin(), samplers.end() };
        m_bindings[index] = binding;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Swapchain
    //////////////////////////////////////////////////////////////////////////////////////////

    uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    uint32_t Swapchain::GetImagesCount() const
    {
        return m_swapchainImagesCount;
    }

    Handle<Image> Swapchain::GetImage() const
    {
        return m_images[m_currentImageIndex];
    }

    Fence& Swapchain::GetCurrentFrameFence()
    {
        return *m_frameReadyFence[GetCurrentImageIndex()];
    }

} // namespace RHI