#include "Renderer/MeshDrawProcessor.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

namespace Engine
{
    // --------------------------------------------------------------
    // DrawList
    // --------------------------------------------------------------

    void DrawList::init(uint32_t capacity)
    {
        m_capacity = capacity;
        m_count    = 0;

        auto& pool = GpuSceneData::ptr->getStructuredBuffersPool();
        m_transformAlloc .init(pool, capacity);
        m_drawAlloc      .init(pool, capacity);

        m_entries.reserve(capacity);
    }

    void DrawList::shutdown(RHI::Device* device)
    {
        if (m_capacity == 0)
            return;

        auto& pool = GpuSceneData::ptr->getStructuredBuffersPool();
        m_transformAlloc.shutdown(pool);
        m_drawAlloc.shutdown(pool);

        m_entries.clear();
        m_capacity = 0;
        m_count    = 0;
    }

    uint32_t DrawList::push(const StaticMeshLOD* mesh,
        const Material*                          material,
        glm::mat4x4                              transform,
        uint32_t                                 viewMask)
    {
        TL_ASSERT(m_count < m_capacity);

        auto transformAlloc = m_transformAlloc.allocate(1);
        auto drawAlloc      = m_drawAlloc.allocate(1);

        TL_ASSERT(transformAlloc.valid() && drawAlloc.valid());

        GPU::DrawRequest req{
            .meshID     = mesh->m_sbDrawArgs.getOffsetElements(),
            .materialID = 0, // TODO: hook up material->id
            .instanceID = transformAlloc.getOffsetElements(),
            .viewMask   = viewMask,
        };

        Entry e{
            .transformCPU = transform,
            .drawCPU      = req,
            .transformGPU = transformAlloc,
            .drawGPU      = drawAlloc,
        };

        m_entries.push_back(e);
        return m_count++;
    }

    void DrawList::remove(uint32_t id)
    {
        TL_ASSERT(id < m_count);

        auto& entry = m_entries[id];
        m_transformAlloc.free(entry.transformGPU);
        m_drawAlloc.free(entry.drawGPU);

        if (id != m_count - 1)
        {
            m_entries[id] = m_entries.back();
        }
        m_entries.pop_back();
        --m_count;
    }

    void DrawList::onUpdate(RHI::Device* device)
    {
        if (m_count == 0)
            return;

        auto* frame = device->GetCurrentFrame();

        for (auto& e : m_entries)
        {
            frame->BufferWrite(
                e.transformGPU.getBuffer().getBuffer(),
                e.transformGPU.getOffsetElementsRaw(),
                TL::Block::create(&e.transformCPU, sizeof(glm::mat4x4)));

            frame->BufferWrite(
                e.drawGPU.getBuffer().getBuffer(),
                e.drawGPU.getOffsetElementsRaw(),
                TL::Block::create(&e.drawCPU, sizeof(GPU::DrawRequest)));
        }
    }

    // --------------------------------------------------------------
    // MeshVisibilityPass
    // --------------------------------------------------------------

    TL::Error MeshVisibilityPass::init(RHI::Device* device)
    {
        m_device          = device;
        m_bindGroupLayout = sig::CullParams::createBindGroupLayout(device);
        m_bindGroup       = device->CreateBindGroup({.name = "CULL-BG", .layout = m_bindGroupLayout});

        m_pipelineLayout = device->CreatePipelineLayout({.name = "CULL-PL", .layouts = m_bindGroupLayout});

        PipelineLibrary::ptr->acquireComputePipeline(
            "I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/Cull.spirv.CSMain",
            [this](RHI::Device* device, RHI::ShaderModule* cs)
            {
                if (m_pipeline)
                {
                    device->DestroyComputePipeline(m_pipeline);
                }
                RHI::ComputePipelineCreateInfo createInfo{
                    .name         = "CULL-CS",
                    .shaderName   = "main", // TODO: Should be csmain
                    .shaderModule = cs,
                    .layout       = m_pipelineLayout,
                };
                m_pipeline = device->CreateComputePipeline(createInfo);
            });

        m_shaderParams.drawCount.m_dirty        = true;
        m_shaderParams.drawIndirectArgs.m_dirty = true;
        m_shaderParams.staticMeshOffsets.m_dirty = true;

        return TL::NoError;
    }

    void MeshVisibilityPass::shutdown()
    {
        m_device->DestroyComputePipeline(m_pipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
        m_device->DestroyBindGroup(m_bindGroup);
        m_device->DestroyBindGroupLayout(m_bindGroupLayout);
    }

    RHI::RGPass* MeshVisibilityPass::addPass(RHI::RenderGraph* rg, const MeshVisibilityPassParams& input)
    {
        // sig::CullParams::CB cb{
        //     .viewProjection   = {},
        //     .viewportSize     = {},
        //     .cameraPosition   = {},
        //     .drawRequestCount = {},
        //     .indexedMeshCount = {},
        // };
        // m_constants.update();

        return rg->AddPass({
            .name          = input.name.data(),
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                auto argsBufferSize = TL::AlignUp<uint32_t>(sizeof(uint32_t), m_device->GetLimits().minStorageBufferOffsetAlignment) + (sizeof(RHI::DrawIndexedParameters) * input.capacity);
                m_drawIndirectArgs  = builder.CreateBuffer(
                    "mdiArgs",
                    argsBufferSize,
                    RHI::BufferUsage::Storage,
                    RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [=, this](RHI::CommandList& cmd)
            {
                m_shaderParams.drawIndirectArgs.bindingInfo         = input.drawList->getDrawRequests();
                m_shaderParams.staticMeshOffsets.bindingInfo        = GpuSceneData::ptr->getSBPoolRenderables();

                m_shaderParams.drawCount.bindingInfo        = getCountBuffer(rg);
                m_shaderParams.drawIndirectArgs.bindingInfo = getArgBuffer(rg);
                m_shaderParams.updateBindGroup(m_device, m_bindGroup);

                cmd.BindComputePipeline(m_pipeline, RHI::BindGroupBindingInfo{m_bindGroup});
                const uint32_t groupSize = 64; // must match [numthreads(x,y,z)] in shader
                uint32_t       numGroups = (input.capacity + groupSize - 1) / groupSize;
                cmd.Dispatch({numGroups, 1, 1});
            },
        });
    }

    RHI::BufferBindingInfo MeshVisibilityPass::getCountBuffer(RHI::RenderGraph* rg)
    {
        return {rg->GetBufferHandle(m_drawIndirectArgs)};
    }

    RHI::BufferBindingInfo MeshVisibilityPass::getArgBuffer(RHI::RenderGraph* rg)
    {
        auto bindingInfo = getCountBuffer(rg);
        bindingInfo.offset += m_device->GetLimits().minStorageBufferOffsetAlignment;
        return bindingInfo;
    }

    void MeshVisibilityPass::setup(RHI::RenderGraphBuilder& builder)
    {
        builder.ReadBuffer(m_drawIndirectArgs, RHI::BufferUsage::Indirect, RHI::PipelineStage::DrawIndirect);
    }

    void MeshVisibilityPass::draw(RHI::RenderGraph* rg, RHI::CommandList& cmd)
    {
        cmd.BindIndexBuffer(GpuSceneData::ptr->getIndexPool(), RHI::IndexType::uint32);
        cmd.BindVertexBuffers(
            0,
            {
                GpuSceneData::ptr->getVertexPoolPositions(),
                GpuSceneData::ptr->getVertexPoolNormals(),
                GpuSceneData::ptr->getVertexPoolUVs(),
                // {rg->GetBufferHandle(m_instanceBuffer), 0},
            });
        cmd.DrawIndexedIndirect(
            getArgBuffer(rg),
            getCountBuffer(rg),
            1,
            sizeof(RHI::DrawIndexedParameters));
    }

} // namespace Engine
