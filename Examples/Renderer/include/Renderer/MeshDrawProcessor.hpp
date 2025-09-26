#pragma once

#include <RHI/RHI.hpp>
#include <Shaders/GpuCommonStructs.h>

#include "Renderer/Resources.hpp"
#include "Renderer/Scene.hpp"

#include "Renderer-Shaders/Cull.hpp"

namespace Engine
{
    class Material;

    // --------------------------------------
    // DrawList
    // --------------------------------------
    // class DrawList
    // {
    // public:
    //     DrawList(uint32_t capacity);
    //     ~DrawList();

    //     uint32_t getCapacity() const { return m_capacity; }

    //     uint32_t getCount() const { return m_count; }

    //     /// Push new element into the draw list.
    //     /// Returns the assigned instanceID.
    //     uint32_t push(const StaticMeshLOD* mesh, const Material* material, glm::mat4x4 transform, uint32_t viewMask = 0x1u);

    //     /// Removes element by instanceID (swap-remove).
    //     void remove(uint32_t id);

    //     /// Uploads CPU-side draw data into GPU buffers.
    //     void onUpdate(RHI::Device* device);

    // private:
    //     uint32_t m_capacity;
    //     uint32_t m_count;

    //     TL::Vector<glm::mat4x4>      m_transformData;
    //     TL::Vector<GPU::DrawRequest> m_drawListData;

    //     MeshBuffer<glm::mat4x4>            m_transform;
    //     StructuredBuffer<GPU::DrawRequest> m_drawList;
    // };

    // --------------------------------------
    // Mesh visibility
    // --------------------------------------
    struct MeshVisibilityPassParams
    {
        TL::StringView name;
        uint32_t       capacity;
        // glm::mat4x4                        viewProjection;
        // RHI::ImageSize2D                   viewportSize;
        // glm::vec3                          cameraPosition;
        // StructuredBuffer<GPU::DrawRequest> drawRequests;
        // DrawList*                          drawList;
    };

    class MeshVisibilityPass
    {
    public:
        TL::Error              init(RHI::Device* device);
        void                   shutdown();
        RHI::RGPass*           addPass(RHI::RenderGraph* rg, const MeshVisibilityPassParams& input);
        RHI::BufferBindingInfo getCountBuffer(RHI::RenderGraph* rg);
        RHI::BufferBindingInfo getArgBuffer(RHI::RenderGraph* rg);
        void                   setup(RHI::RenderGraphBuilder& builder);
        void                   draw(RHI::RenderGraph* rg, RHI::CommandList& cmd);

    private:
        RHI::Device* m_device;

        RHI::RGBuffer* m_drawIndirectArgs = nullptr;

        RHI::BindGroupLayout* m_bindGroupLayout;

        sig::CullParams m_shaderParams;
        RHI::BindGroup* m_bindGroup;

        RHI::PipelineLayout*  m_pipelineLayout;
        RHI::ComputePipeline* m_pipeline;
    };
} // namespace Engine
