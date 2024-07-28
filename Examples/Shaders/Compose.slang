static float4 ScreenQuadVertexPositions[] =
{
	{-0.9,  0.9, 0.0, 1.0},
	{-0.9, -0.9, 0.0, 1.0},
	{ 0.9, -0.9, 0.0, 1.0},
	{-0.9,  0.9, 0.0, 1.0},
	{ 0.9, -0.9, 0.0, 1.0},
	{ 0.9,  0.9, 0.0, 1.0}
};

struct VSInput
{
    uint32_t vertexID : SV_VertexID;
};

struct PSInput
{
    float4 csPosition : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target;
};

struct ShaderInput
{
    Texture2D colorTexture;
};

ParameterBlock<ShaderInput> g_inputs;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.csPosition = ScreenQuadVertexPositions[input.vertexID];
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
    let uv = uint3((uint2)input.csPosition.xy, 0);

	PSOutput output;
    output.color = g_inputs.colorTexture.Load(uv);
	return output;
}
