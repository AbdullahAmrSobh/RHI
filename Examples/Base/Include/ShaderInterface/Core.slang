#include "ShaderInterface/ShaderMacros.h"

#define re_max_point_lights_count 16

re_shader_struct SceneTransform
{
    re_mat4 viewProjectionMatrix;
    re_mat4 projectionMatrix;
    re_mat4 viewMatrix;
    re_mat4 inverseViewMatrix;
};

re_shader_struct DirectionalLight
{
    re_vec3 direction;
    re_float intensity;

    re_vec3 ambientColor;
    re_vec3 diffuseColor;
    re_vec3 specularColor;
};

re_shader_struct PointLight
{
    re_vec3 position;
    re_float intensity;
    re_vec3 color;
    re_float radius;

    re_vec3 ambientColor;
    re_vec3 diffuseColor;
    re_vec3 specularColor;
    re_float attuation;
};

re_shader_struct SpotLight
{
    re_vec3 position;
    re_float radius;
    re_vec3 direction;
    re_float innerAngle;
    re_vec3 ambientColor;
    re_vec3 specularColor;
    re_vec3 diffuseColor;
    re_float attuation;
};

re_shader_struct ObjectTransform
{
    re_mat4 modelMatrix;

    // DirectionalLight directionalLight[re_max_point_lights_count];
};

re_enum PipelineVertexInputRate
{
    None,
    PerInstance,
    PerVertex,
};

re_enum PipelineRasterizerStateCullMode
{
    None,
    FrontFace,
    BackFace,
    Discard,
};

re_enum PipelineRasterizerStateFillMode
{
    Point,
    Triangle,
    Line
};

re_enum PipelineTopologyMode
{
    Points,
    Lines,
    Triangles,
};

re_enum PipelineRasterizerStateFrontFace
{
    Clockwise,
    CounterClockwise,
};

re_enum CompareOperator
{
    Never,
    Equal,
    NotEqual,
    Greater,
    GreaterOrEqual,
    Less,
    LessOrEqual,
    Always,
};

re_enum SamplerFilter
{
    Point,
    Linear,
};

re_enum SamplerAddressMode
{
    Repeat,
    Clamp,
};

re_enum SamplerCompareOperation
{
    Never,
    Equal,
    NotEqual,
    Always,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

re_enum BlendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

re_enum BlendEquation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

re_shader_struct ColorAttachmentBlendStateDesc
{
    bool                  blendEnable  = false;
    BlendEquation         colorBlendOp = BlendEquation::Add;
    BlendFactor           srcColor     = BlendFactor::One;
    BlendFactor           dstColor     = BlendFactor::Zero;
    BlendEquation         alphaBlendOp = BlendEquation::Add;
    BlendFactor           srcAlpha     = BlendFactor::One;
    BlendFactor           dstAlpha     = BlendFactor::Zero;
    // Flags<ColorWriteMask> writeMask    = ColorWriteMask::All;
};

re_shader_struct PipelineRasterizerStateDesc
{
    PipelineRasterizerStateCullMode  cullMode;
    PipelineRasterizerStateFillMode  fillMode;
    PipelineRasterizerStateFrontFace frontFace;
    float                            lineWidth;
};

re_shader_struct PipelineDepthStencilStateDesc
{
    bool            depthTestEnable;
    bool            depthWriteEnable;
    CompareOperator compareOperator;
    bool            stencilTestEnable;
};

re_shader_struct PipelineColorBlendStateDesc
{
    // ColorAttachmentBlendStateDesc blendStates[c_MaxRenderTargetAttachmentsCount];
    float                         blendConstants[4];
};


re_shader_struct GraphicsPipelineCreateInfo
{
    re_name                         name;
    PipelineColorBlendStateDesc     colorBlendState;
    PipelineTopologyMode            topologyMode;
    PipelineRasterizerStateDesc     rasterizationState;
    PipelineDepthStencilStateDesc   depthStencilState;
};
