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

    Swapchain::Swapchain()
        : m_currentImageIndex(0)
        , m_swapchainImagesCount(0)
        , m_images()
    {
    }

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

    Handle<Image> Swapchain::GetImage(uint32_t index) const
    {
        return m_images[index];
    }

} // namespace RHI