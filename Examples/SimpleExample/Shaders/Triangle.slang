// Test bind groups
struct CBSceneView
{
    float4x4 modelToView;
    float4x4 viewToClip;
    float4x4 modelToClip;
    float4x4 clipToView;
    float4x4 clipToLocal;
};


struct BindGroup0
{
    ConstantBuffer<CBSceneView> sceneView;
    StructuredBuffer<uint32_t> ib;
    StructuredBuffer<float3> positionsVB;
    StructuredBuffer<float3> normalsVB;
    StructuredBuffer<float3> texcoordsVB;
    SamplerState             sampler;
    Texture2D                textures[];
};

ParameterBlock<BindGroup0> block0;

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

[[shader("vertex")]]
VS_OUTPUT VSMain(uint vertexID : SV_VertexID)
{
    float2 positions[3] = {
        float2(-0.5, -0.5),
        float2( 0.5, -0.5),
        float2( 0.0,  0.5)
    };
    float4 colors[3] = {
        float4(1,0,0,1),
        float4(0,1,0,1),
        float4(0,0,1,1)
    };
    float2 uvs[3] = {
        float2(0,0),
        float2(1,0),
        float2(0.5,1)
    };

    VS_OUTPUT output;
    output.pos = float4(positions[vertexID], 0.0, 1.0);
    output.col = colors[vertexID];
    output.uv  = uvs[vertexID];
    return output;
}

struct PSOut
{
    float4 col1 : SV_Target;
    float4 col2 : SV_Target;
};

[[shader("fragment")]]
PSOut PSMain(VS_OUTPUT input)
{
    var sceneView = block0.sceneView.clipToLocal[0].xyz;
    var positionsVB = block0.positionsVB[0];
    var normalsVB = block0.normalsVB[0];
    var texcoordsVB = block0.texcoordsVB[0];
    var sampler = block0.sampler;

    var texture = block0.textures[1].Sample(sampler, input.uv).rgb;

    var sum = texture +
               sceneView +
               positionsVB +
               normalsVB +
               texcoordsVB;

    return {input.col, sum.rgbb};
}
