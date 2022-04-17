Rendering Hardware Interface (RHI) is a thin abstraction layer over graphics APIs


RHI Objects
--
- IFactory: A Factory Object that is responsible for creating all RHI objects. 
- IDevice: an Object that represents a GPU device. 
- ISwapChain: 
- IResourceMemoryPool: A GPU Memory Pool for other resources to be allocated from.
- IBuffer: A Buffer Resource
- ITexture: A Texture Resource
- ITextureView: A view for the Texture Resource 
- IBufferView: A view for the Buffer Resource
- IShaderModule: Represents a Binary Shader Program. 
- IPipelineLayout: Describe the inputs for the pipline's shaders. 
- IPiplineState: 
- ISampler:
- IDescriptorPool: 
- IDescriptorSetLayout: 
- IDescriptorSet: 
- IQueue: 
- IFence: 
- IRenderTarget: 
- IRenderGraphBuilder: 
- IRenderGraph: 
- ICommandContext:

Goals
-- 
- Create Factory 
- Basic Rendering Pipeline (No RenderGraph Required)
- RenderGraph implementation

RenderGraph Features
--
- Import Persistent Resources 
- Create Transient Resources
- Manage Synchronization
- Present To SwapChains
- Stream Resources 

RHI Shader Compiler 
--
HLSL Resource Binding Abstraction layer

RHI Shaders Refelections 
--
The goal is to be able to describe the Pipeline States, as well as Pipeline Layout in a text file, that is consumed to generate GraphicsPipelineStateDesc, and PipelineLayoutDesc, and DescriptorSetLayoutDesc objects.



RenderGraph Components 
RenderGraphBuilder
RenderGraphAttachment
RenderGraphAttachmentDesc
RenderGraphPass
RenderGraphPassExecuteCallbacks
