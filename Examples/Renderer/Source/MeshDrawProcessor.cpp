#include "Renderer/MeshDrawProcessor.hpp"
#include "Renderer/PipelineLibrary.hpp"

namespace Engine
{
    // --------------------------------------------------------------
    // DrawList
    // --------------------------------------------------------------

    // DrawList::DrawList(uint32_t capacity)
    // {
    //     auto& pool = GpuSceneData::ptr->getSBPoolRenderables();
    //     m_drawList = pool.allocate<DrawList>(capacity);

    //     m_transformData.clear();
    //     m_drawListData.clear();

    //     m_drawListData.resize(capacity, GPU::DrawRequest{});
    // }

    // DrawList::~DrawList()
    // {
    //     auto& pool = GpuSceneData::ptr->getSBPoolRenderables();
    //     pool.free<DrawList>(m_drawList);
    // }

    // DrawList::DrawList(uint32_t capacity)
    //     : m_capacity(capacity)
    //     , m_count(0)
    // {
    //     m_transformData.reserve(capacity);
    //     m_drawListData.reserve(capacity);
    // }

    // DrawList::~DrawList()
    // {
    // }

    // uint32_t DrawList::push(const StaticMeshLOD* mesh, const Material* material, glm::mat4x4 transform, uint32_t viewMask)
    // {
    //     TL_ASSERT(m_count < m_capacity, "DrawList overflow");

    //     uint32_t instanceID = m_count++;

    //     // record transform
    //     m_transformData.push_back(transform);

    //     // record GPU draw request
    //     GPU::DrawRequest req{};
    //     req.meshID     = meshID;
    //     req.materialID = materialID;
    //     req.instanceID = instanceID;
    //     req.viewMask   = viewMask;
    //     m_drawListData.push_back(req);

    //     return instanceID;
    // }

    // void DrawList::remove(uint32_t instanceID)
    // {
    //     TL_ASSERT(instanceID < m_count);

    //     // swap with last element
    //     m_transformData[instanceID] = m_transformData.back();
    //     m_transformData.pop_back();

    //     m_drawListData[instanceID] = m_drawListData.back();
    //     m_drawListData.pop_back();

    //     --m_count;

    //     // fix moved element’s instanceID
    //     if (instanceID < m_count)
    //     {
    //         m_drawListData[instanceID].instanceID = instanceID;
    //     }
    // }

    // void DrawList::onUpdate(RHI::Device* device)
    // {
    //     if (!m_transform.valid())
    //     {
    //         // allocate GPU buffers lazily
    //         auto& meshPool   = GpuSceneData::ptr->(device); // global/shared pool
    //         auto& structPool = GpuSceneData::ptr->(device); // global/shared pool

    //         m_transform = meshPool.allocate<glm::mat4x4>(m_capacity);
    //         m_drawList  = structPool.allocate<GPU::DrawRequest>(m_capacity);
    //     }

    //     if (m_count > 0)
    //     {
    //         device->GetCurrentFrame()->BufferWrite(
    //             m_transform.getBuffer(),
    //             m_transform.getOffset(),
    //             TL::Block::create(m_transformData.data(),
    //                 m_count * sizeof(glm::mat4x4)));

    //         device->GetCurrentFrame()->BufferWrite(
    //             m_drawList.getBuffer(),
    //             m_drawList.getOffset(),
    //             TL::Block::create(m_drawListData.data(),
    //                 m_count * sizeof(GPU::DrawRequest)));
    //     }
    // }

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
        cmd.BindIndexBuffer(GpuSceneData::ptr->getIndexPool().getBaseBinding(), RHI::IndexType::uint32);
        cmd.BindVertexBuffers(
            0,
            {
                GpuSceneData::ptr->getVertexPoolPositions().getBaseBinding(),
                GpuSceneData::ptr->getVertexPoolNormals().getBaseBinding(),
                GpuSceneData::ptr->getVertexPoolUVs().getBaseBinding(),
                // {rg->GetBufferHandle(m_instanceBuffer), 0},
            });
        cmd.DrawIndexedIndirect(
            getArgBuffer(rg),
            getCountBuffer(rg),
            1,
            sizeof(RHI::DrawIndexedParameters));
    }

} // namespace Engine
