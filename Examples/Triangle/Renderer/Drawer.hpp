#pragma once

#include <RHI/RHI.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "PipelineLibrary.hpp"

namespace Engine
{
    // The idea is to have all draws go through this interface,
    // each platform will override this interface to suit its needs,
    // e.g. on web and old mobile hardware it would fallback to not use bindless

    struct DrawRequest
    {
        TL::Span<const glm::mat4> transforms;
        uint32_t                  geometryID; // id into the Drawable storage buffer
        uint32_t                  materialID; // id into the materials storage buffer
    };

    class DrawList
    {
    public:
        virtual void Draw() = 0;
    };

    PipelineLibrary* GetPipelineLibrary();

    struct DrawArguments
    {
        const char*                          name;
        TL::Optional<RHI::BufferBindingInfo> ib;
        TL::Optional<RHI::BufferBindingInfo> positionVB;
        TL::Optional<RHI::BufferBindingInfo> normalVB;
        TL::Optional<RHI::BufferBindingInfo> texcoordVB;
        TL::Optional<RHI::BufferBindingInfo> drawListVB;
        RHI::Handle<RHI::GraphicsPipeline>   pipeline;
        TL::Span<RHI::BindGroupBindingInfo>  bindGroups;
        uint32_t                             drawCount;
        uint32_t                             stride;

        // DrawList
        RHI::BufferBindingInfo drawListBuffer; // array of DrawRequests
    };

    class DrawerDirect
    {
    };

    class DrawerIndirect
    {
    private:
        RHI::Handle<RHI::BindGroup> m_cullBindGroup; // bind group used for culling!

    public:
        //
        void Setup(RHI::Device& device, RHI::RenderGraph& renderGraph, const DrawArguments& args)
        {
            RHI::RenderGraphBuffer* indirectBuffer;
            renderGraph.AddPass(
                {
                    .name          = "Dispatch-Cull",
                    .queue         = RHI::QueueType::Compute,
                    .setupCallback = [&](RHI::RenderGraph& rg, RHI::Pass& pass)
                    {
                        rg.UseBuffer(pass, indirectBuffer, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader, RHI::Access::Write);
                    },
                    .compileCallback = [&](RHI::RenderGraph& rg, RHI::Pass& pass)
                    {
                        // UpdateBindGroup(device, args.drawListBuffer, {indirectDrawBuffer})
                    },
                    .executeCallback = [&](RHI::CommandList& commandList)
                    {
                        Cull(commandList, args.name);
                    },
                });

            renderGraph.AddPass({
                .name  = "",
                .queue = RHI::QueueType::Graphics,
                // .setupCallback =
            });
        }

        void UpdateBindGroup(RHI::Device* device, RHI::BufferBindingInfo drawListBuffer, RHI::BufferBindingInfo indirectDrawBuffer)
        {
            RHI::BindGroupBuffersUpdateInfo buffersBindings[] = {
                {
                 .dstBinding = 0,
                 .buffers    = {drawListBuffer.buffer},
                 .subregions = {{drawListBuffer.offset, RHI::WholeSize}},
                 },
                {
                 .dstBinding = 1,
                 .buffers    = {indirectDrawBuffer.buffer},
                 .subregions = {{indirectDrawBuffer.offset, RHI::WholeSize}},
                 },
            };
            device->UpdateBindGroup(m_cullBindGroup, {.buffers = buffersBindings});
        }

        void Cull(RHI::CommandList& cmd, const char* passName)
        {
            auto cullingPipeline = GetPipelineLibrary();

            if (passName)
                cmd.DebugMarkerPush(passName, {});
            cmd.BindComputePipeline(cullingPipeline->GetComputePipeline("ComputeCullPipeline"), {{m_cullBindGroup}});
            cmd.Dispatch({32, 1, 1});
            if (passName)
                cmd.DebugMarkerPop();
        }

        // Problem allow recording a another pass from within another pass?
        void Draw(RHI::CommandList& cmd, const DrawArguments& args, RHI::BufferBindingInfo indirectBufferBindingInfo)
        {
            if (args.name)
                cmd.DebugMarkerPush(args.name, {});

            cmd.BindGraphicsPipeline(args.pipeline, args.bindGroups);

            if (args.ib)
                cmd.BindIndexBuffer(*args.ib, RHI::IndexType::uint32);

            uint32_t               slot = 0;
            RHI::BufferBindingInfo vbs[4];

            if (args.positionVB) vbs[slot++] = *args.positionVB;
            if (args.normalVB) vbs[slot++] = *args.normalVB;
            if (args.texcoordVB) vbs[slot++] = *args.texcoordVB;

            if (slot != 0)
                cmd.BindVertexBuffers(0, vbs);

            auto [indirectBuffer, offset] = indirectBufferBindingInfo;
            cmd.DrawIndexedIndirect(
                {.buffer = indirectBuffer, .offset = offset + sizeof(uint32_t)}, // first 4 bytes are used as count!
                {.buffer = indirectBuffer, .offset = offset},
                args.drawCount,
                args.stride);

            if (args.name)
                cmd.DebugMarkerPop();
        }
    };

    template<typename DrawerType>
    class Drawer
    {
    public:
        virtual void Setup(RHI::RenderGraph& renderGraph) = 0;

        virtual void Execute(RHI::RenderGraph& renderGraph, RHI::CommandList& commandList, const DrawList& drawList) = 0;
    };
} // namespace Engine