#include "RHI/Resources.hpp"

#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    void BindGroupData::BindImages(uint32_t index, TL::Span<Handle<ImageView>> handles, uint32_t arrayOffset)
    {
        BindGroupData::ResourceImageBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views = { handles.begin(), handles.end() };
        m_bindings[index] = binding;
    }

    void BindGroupData::BindBuffers(uint32_t index, TL::Span<Handle<Buffer>> handles, bool dynamic, size_t element_size, uint32_t arrayOffset)
    {
        BindGroupData::ResourceBufferBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views = { handles.begin(), handles.end() };
        binding.dynamic = dynamic;
        binding.elementSize = element_size;
        m_bindings[index] = binding;
    }

    void BindGroupData::BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset)
    {
        BindGroupData::ResourceSamplerBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.samplers = { samplers.begin(), samplers.end() };
        m_bindings[index] = binding;
    }

    /// TODO: move to a new Swapchain.cpp file

    Handle<ImageView> Swapchain::GetImageView(Context* context, const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewsLRU.find(createInfo); it != m_imageViewsLRU.end())
            return it->second;

        auto imageView = context->CreateImageView(createInfo);
        m_imageViewsLRU[createInfo] = imageView;
        return imageView;
    }
} // namespace RHI