struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : TEXCOORD;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
};

VSOutput VSMain(uint index: SV_VertexID)
{
	float4 trianglePositions[3] = { float4(0, -1, 0, 1), float4(-1, 1, 0, 1), float4(1, 1, 0, 1) };
	float4 triangleColors[3]    = { float4(1, 0, 0, 1), float4(0, 1, 0, 1), float4(0, 0, 1, 1) };

	VSOutput output; 
	output.position = trianglePositions[index];
	output.color    = triangleColors[index];
	return output;
}

PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color; 
	return output;
}
