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

        auto& pool = RenderContext::ptr->getStructuredBuffersPool();
        m_transformAlloc.init(pool, capacity);
        m_drawAlloc.init(pool, capacity);

        m_entries.reserve(capacity);
    }

    void DrawList::shutdown(RHI::Device* device)
    {
        if (m_capacity == 0)
            return;

        auto& pool = RenderContext::ptr->getStructuredBuffersPool();
        m_transformAlloc.shutdown();
        m_drawAlloc.shutdown();

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
            .meshID     = mesh->m_sbDrawArgs.getOffsetIndex(),
            .materialID = 0, // TODO: hook up material->id
            .instanceID = transformAlloc.getOffsetIndex(),
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

        auto& pool = RenderContext::ptr->getStructuredBuffersPool();

        for (auto& e : m_entries)
        {
            m_transformAlloc.update(e.transformGPU, e.transformCPU);
            m_drawAlloc.update(e.drawGPU, e.drawCPU);
        }
    }

    // --------------------------------------------------------------
    // MeshVisibilityPass
    // --------------------------------------------------------------

    TL::Error MeshVisibilityPass::init(RHI::Device* device)
    {
        m_device = device;

        auto layout = m_shaderParams.getLayout()->get();
        m_shaderParams.init(device, 0);

        m_shader = PipelineLibrary::ptr->acquireComputePipeline<GPU::CullParams>(
            "I:/repos/repos3/RHI/Examples/Renderer/Shaders/source/Cull.json");
        m_shaderParams.cb.m_dirty                = true;
        m_shaderParams.staticMeshOffsets.m_dirty = true;
        m_shaderParams.drawRequests.m_dirty      = true;
        m_shaderParams.drawCountOut.m_dirty      = true;
        m_shaderParams.drawIndirectArgs.m_dirty  = true;

        auto& pool  = RenderContext::ptr->getConstantBuffersPool();
        m_constants = pool.allocate<GPU::CullParams::CB>();

        return TL::NoError;
    }

    void MeshVisibilityPass::shutdown()
    {
        // m_device->DestroyComputePipeline(m_pipeline);
        // m_device->DestroyPipelineLayout(m_pipelineLayout);
        m_shaderParams.shutdown(m_device);
    }

    RHI::RGPass* MeshVisibilityPass::addPass(RHI::RenderGraph* rg, const MeshVisibilityPassParams& input)
    {
        GPU::CullParams::CB cb{
            .drawCount = input.drawList->getCount(),
        };
        auto& pool = RenderContext::ptr->getConstantBuffersPool();
        pool.update(m_constants, cb);

        return rg->AddPass({
            .name          = input.name.data(),
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                auto argsBufferSize = TL::AlignUp<uint32_t>(sizeof(uint32_t), m_device->GetLimits().minStorageBufferOffsetAlignment) + (sizeof(RHI::DrawIndexedParameters) * input.capacity);
                m_drawIndirectArgs  = builder.CreateBuffer(
                    "mdiArgs",
                    input.drawList->getCapacity() * sizeof(RHI::DrawIndexedParameters) + argsBufferSize,
                    RHI::BufferUsage::Storage,
                    RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [=, this](RHI::CommandList& cmd)
            {
                m_shaderParams.cb                = this->m_constants;
                m_shaderParams.drawRequests      = input.drawList->getDrawRequests();
                m_shaderParams.staticMeshOffsets = RenderContext::ptr->getSBPoolRenderables();
                m_shaderParams.drawCountOut      = getCountBuffer(rg);
                m_shaderParams.drawIndirectArgs  = getArgBuffer(rg);
                m_shaderParams.update(m_device);

                cmd.BindComputePipeline(m_shader->getPipeline(), m_shaderParams.bind());

                const uint32_t groupSize = 64; // must match [numthreads(x,y,z)] in shader
                uint32_t       numGroups = (groupSize) / groupSize;
                cmd.Dispatch({groupSize, 1, 1});
            },
        });
    }

    RHI::BufferBindingInfo MeshVisibilityPass::getCountBuffer(RHI::RenderGraph* rg)
    {
        return {
            .buffer = rg->GetBufferHandle(m_drawIndirectArgs),
            .offset = 0,
            .range  = sizeof(uint32_t),
        };
    }

    RHI::BufferBindingInfo MeshVisibilityPass::getArgBuffer(RHI::RenderGraph* rg)
    {
        auto bindingInfo   = getCountBuffer(rg);
        bindingInfo.offset = m_device->GetLimits().minStorageBufferOffsetAlignment;
        bindingInfo.range  = RHI::RemainingSize;
        return bindingInfo;
    }

    void MeshVisibilityPass::setup(RHI::RenderGraphBuilder& builder)
    {
        builder.ReadBuffer(m_drawIndirectArgs, RHI::BufferUsage::Indirect, RHI::PipelineStage::DrawIndirect);
    }

    void MeshVisibilityPass::draw(RHI::RenderGraph* rg, RHI::CommandList& cmd, uint32_t maxDrawCount)
    {
        cmd.BindIndexBuffer(RenderContext::ptr->getIndexPool(), RHI::IndexType::uint32);
        cmd.BindVertexBuffers(
            0,
            {
                RenderContext::ptr->getVertexPoolPositions(),
                RenderContext::ptr->getVertexPoolNormals(),
                RenderContext::ptr->getVertexPoolUVs(),
                // {rg->GetBufferHandle(m_instanceBuffer), 0},
            });
        cmd.DrawIndexedIndirect(
            getArgBuffer(rg),
            getCountBuffer(rg),
            maxDrawCount,
            sizeof(RHI::DrawIndexedParameters));
    }

} // namespace Engine
