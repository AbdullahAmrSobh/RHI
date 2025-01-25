

#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <RHI-Vulkan/Loader.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tracy/Tracy.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "Camera.hpp"
#include "Examples-Base/ApplicationBase.hpp"
#include "ImGuiRenderer.hpp"
#include "dds_image/dds.hpp"
#include "stb_image.h"


using namespace Examples;

template<typename T>
using Handle = RHI::Handle<T>;


// GpuSceneManager class to manage GPU resources for the scene
class GpuSceneManager
{
public:
    // Total number of indices
    size_t m_totalIndexCount;
    // Total number of vertices
    size_t m_totalVertexCount;

    // Buffer pool for geometry data
    Handle<RHI::Buffer> m_geometryBufferPool;
    // Buffer pool for uniform data
    Handle<RHI::Buffer> m_uniformBufferPool;
    // Buffer pool for scene lights
    Handle<RHI::Buffer> m_sceneLightsPool;

    // Sampler handle
    Handle<RHI::Sampler>           m_sampler;
    // Vector of image handles
    TL::Vector<Handle<RHI::Image>> m_images;

    // Bind group layout for scene
    Handle<RHI::BindGroupLayout> m_sceneBindGroupLayout;
    // Bind group for scene
    Handle<RHI::BindGroup>       m_sceneBindGroup;
    // Bind group layout for bindless textures
    Handle<RHI::BindGroupLayout> m_bindlessTexturesBindGroupLayout;
    // Bind group for bindless textures
    Handle<RHI::BindGroup>       m_bindlessTexturesBindGroup;

    // CPU Scene management
    GpuSceneManager* m_scene;

    // Initialize the GpuSceneManager with the given device and buffer sizes
    RHI::ResultCode Init(RHI::Device& device, const fastgltf::Asset asset)
    {

        // Helper function to load primitives
        auto loadPrimitives = [&](const fastgltf::Primitive& primitive, glm::mat4 modelToWorldMatrix) -> SceneDrawable
        {
            auto positionAccessorIndex = primitive.findAttribute("POSITION")->accessorIndex;
            auto normalAccessorIndex   = primitive.findAttribute("NORMAL")->accessorIndex;
            auto texcoordAccessorIndex = primitive.findAttribute("TEXCOORD_0")->accessorIndex;
            auto indicesAccessor       = asset.accessors[primitive.indicesAccessor.value()];
            auto positionsAccessor     = asset.accessors[positionAccessorIndex];
            auto normalsAccessor       = asset.accessors[normalAccessorIndex];
            auto texcoordsAccessor     = asset.accessors[texcoordAccessorIndex];

            // If the position accessor is already in the drawCalls map, reuse existing data
            if (auto it = drawCalls.find(positionAccessorIndex); it != drawCalls.end())
            {
                return SceneDrawable{
                    .drawIndirectArgs = {
                        .indexCount   = static_cast<uint32_t>(indicesAccessor.count),
                        .firstIndex   = static_cast<uint32_t>(it->second.drawIndirectArgs.firstIndex),
                        .vertexOffset = static_cast<int32_t>(it->second.drawIndirectArgs.vertexOffset),
                    },
                    .materialIndex      = static_cast<uint32_t>(primitive.materialIndex.value()),
                    .modelToWorldMatrix = modelToWorldMatrix,
                };
            }

            // Calculate the current offsets in the respective buffers
            auto indexBufferCursor = indexBufferData.size();
            auto vbPositionsCursor = vertexBufferPositionsData.size();
            auto vbNormalsCursor   = vertexBufferNormalsData.size();
            auto vbTexcoordsCursor = vertexBufferTexcoordsData.size();

            // Resize buffers and copy data from accessors
            indexBufferData.resize(indexBufferData.size() + indicesAccessor.count);
            fastgltf::copyFromAccessor<uint32_t>(asset, indicesAccessor, indexBufferData.data() + indexBufferCursor);

            vertexBufferPositionsData.resize(vertexBufferPositionsData.size() + positionsAccessor.count);
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, positionsAccessor, vertexBufferPositionsData.data() + vbPositionsCursor);

            vertexBufferNormalsData.resize(vertexBufferNormalsData.size() + normalsAccessor.count);
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, normalsAccessor, vertexBufferNormalsData.data() + vbNormalsCursor);

            vertexBufferTexcoordsData.resize(vertexBufferTexcoordsData.size() + texcoordsAccessor.count);
            fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordsAccessor, vertexBufferTexcoordsData.data() + vbTexcoordsCursor);

            // Add to drawCalls and return the SceneDrawable
            return drawCalls[positionAccessorIndex] = {
                    .drawIndirectArgs = {
                        .indexCount   = static_cast<uint32_t>(indicesAccessor.count),
                        .firstIndex   = static_cast<uint32_t>(indexBufferCursor), // Correct offset in the index buffer
                        .vertexOffset = static_cast<int32_t>(vbPositionsCursor),  // Correct starting vertex in the vertex buffer
                    },
                    .materialIndex      = static_cast<uint32_t>(primitive.materialIndex.value()),
                    .modelToWorldMatrix = modelToWorldMatrix,
                };
        };


        {
            // Create and upload buffers
            RHI::BufferCreateInfo gbCI{
                .usageFlags = RHI::BufferUsage::Index | RHI::BufferUsage::Vertex,
                .byteSize   = geometryBufferSize // indexBufferData.size() * (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(uint32_t)),
            };
            m_geometryBufferPool = device.CreateBuffer(gbCI).GetValue();

            RHI::BufferCreateInfo ubCI{
                .usageFlags = RHI::BufferUsage::Uniform,
                .byteSize   = uniformBufferSize,
            };
            m_uniformBufferPool = device.CreateBuffer(ubCI).GetValue();

            RHI::BufferCreateInfo ssboCI{
                .usageFlags = RHI::BufferUsage::Uniform,
                .byteSize   = sceneLightSize,
            };
            m_sceneLightsPool = device.CreateBuffer(ssboCI).GetValue();
        }

        {
            m_sampler = device.CreateSampler({.name = "Sampler"});
        }

        {
            auto sceneSubregions   = RHI::BufferSubregion{0, Shader::kAlignedSceneSize};
            auto perDrawSubregions = RHI::BufferSubregion{Shader::kAlignedSceneSize, Shader::kAlignedPerDrawSize};

            m_sceneBindGroup = device.CreateBindGroup(m_sceneBindGroupLayout);
            device.UpdateBindGroup(
                m_sceneBindGroup,
                RHI::BindGroupUpdateInfo{
                    .buffers = {
                        {
                            .dstBinding = 0,
                            .buffers    = {m_uniformBufferPool},
                            .subregions = sceneSubregions,
                        },
                        {
                            .dstBinding = 1,
                            .buffers    = {m_uniformBufferPool},
                            .subregions = perDrawSubregions,
                        },
                    },
                });
        }

        {
            m_bindlessTexturesBindGroup = device.CreateBindGroup(m_bindlessTexturesBindGroupLayout);
            device.UpdateBindGroup(
                m_bindlessTexturesBindGroup,
                RHI::BindGroupUpdateInfo{
                    .images = {
                        {
                            .dstBinding = 1,
                            .images     = m_images,
                        },
                    },
                    .samplers = {
                        {

                            .dstBinding = 0,
                            .samplers   = m_sampler,
                        },
                    },
                });
        }
        // Initialization logic here
        return RHI::ResultCode::Success;
    }

    // Shutdown the GpuSceneManager and release resources
    void Shutdown(RHI::Device& device)
    {
        device.DestroyBindGroupLayout(m_bindlessTexturesBindGroupLayout);
        device.DestroyBindGroup(m_bindlessTexturesBindGroup);
        device.DestroyBindGroupLayout(m_sceneBindGroupLayout);
        device.DestroyBindGroup(m_sceneBindGroup);
        for (auto image : m_images)
            device.DestroyImage(image);
        device.DestroySampler(m_sampler);
        device.DestroyBuffer(m_sceneLightsPool);
        device.DestroyBuffer(m_uniformBufferPool);
        device.DestroyBuffer(m_geometryBufferPool);
    }
};
