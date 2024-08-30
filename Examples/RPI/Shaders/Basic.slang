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


struct ViewCB
{
	float4 color;
};


struct ShaderParamters
{
	ConstantBuffer<ViewCB> viewCB;
};

ParameterBlock<ShaderParamters> bindGroup0;

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
	PSOutput output;
    output.color = bindGroup0.viewCB.color;
	return output;
}
