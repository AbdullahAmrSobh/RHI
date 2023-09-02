#include "RHI/ShaderBindGroup.hpp"

namespace RHI
{

void ShaderBindGroup::BindImages(uint32_t index, TL::Span<const Handle<ImageView>> images, uint32_t arrayOffset)
{
    BindingData data {};
    data.index                = index;
    data.bindArrayStartOffset = arrayOffset;
    data.type                 = RHI::ShaderBindingType::Image;
    data.resources            = std::vector<Handle<ImageView>>(images.begin(), images.end());
    m_data.push_back(data);
}

void ShaderBindGroup::BindBuffers(uint32_t index, TL::Span<const Handle<BufferView>> buffers, uint32_t arrayOffset)
{
    BindingData data {};
    data.index                = index;
    data.bindArrayStartOffset = arrayOffset;
    data.type                 = RHI::ShaderBindingType::Buffer;
    data.resources            = std::vector<Handle<BufferView>>(buffers.begin(), buffers.end());
    m_data.push_back(data);
}

void ShaderBindGroup::BindSamplers(uint32_t index, TL::Span<const Handle<Sampler>> samplers, uint32_t arrayOffset)
{
    BindingData data {};
    data.index                = index;
    data.bindArrayStartOffset = arrayOffset;
    data.type                 = RHI::ShaderBindingType::Sampler;
    data.resources            = std::vector<Handle<Sampler>>(samplers.begin(), samplers.end());
    m_data.push_back(data);
}

}  // namespace RHI