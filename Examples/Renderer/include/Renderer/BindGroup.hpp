#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>

#include "Renderer/Renderer.hpp"

#include <memory>

namespace Engine
{
    template<typename T>
    class BindGroupLayoutRAII
    {
    public:
        BindGroupLayoutRAII()
        {
            m_layout = T::createBindGroupLayout(Renderer::ptr->GetDevice());
        }

        ~BindGroupLayoutRAII()
        {
            auto device = Renderer::ptr->GetDevice();
            device->DestroyBindGroupLayout(m_layout);
        }

        RHI::BindGroupLayout* get() const
        {
            return m_layout;
        }

        operator RHI::BindGroupLayout*() const
        {
            return m_layout;
        }

    private:
        RHI::BindGroupLayout* m_layout = nullptr;
    };

    template<typename T>
    using BindGroupLayoutRef = std::shared_ptr<BindGroupLayoutRAII<T>>;

    template<typename T>
    class ShaderBindGroup final : public T
    {
    public:
        void init(RHI::Device* device, uint32_t bindlessArrayCount)
        {
            m_bindGroup = device->CreateBindGroup(RHI::BindGroupCreateInfo{
                .name               = typeid(T).name(),
                .layout             = getLayout()->get(),
                .bindlessArrayCount = bindlessArrayCount,
            });
        }

        void shutdown(RHI::Device* device)
        {
            device->DestroyBindGroup(m_bindGroup);
            s_bindGroupLayout.reset();
        }

        void update(RHI::Device* device)
        {
            T::updateBindGroup(device, m_bindGroup);
        }

        RHI::BindGroupBindingInfo bind(uint32_t dynamicOffset) const
        {
            return {
                .bindGroup      = m_bindGroup,
                .dynamicOffsets = dynamicOffset,
            };
        }

        RHI::BindGroupBindingInfo bind() const
        {
            return {
                .bindGroup      = m_bindGroup,
                .dynamicOffsets = {},
            };
        }


        static BindGroupLayoutRef<T> getLayout()
        {
            if (s_bindGroupLayout == nullptr)
            {
                s_bindGroupLayout = std::make_shared<BindGroupLayoutRAII<T>>();
            }

            return s_bindGroupLayout;
        }

    private:
        inline static BindGroupLayoutRef<T> s_bindGroupLayout = nullptr;
        RHI::BindGroup*                     m_bindGroup;
    };

} // namespace Engine