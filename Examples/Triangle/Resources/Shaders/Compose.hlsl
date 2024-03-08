[[vk::binding(0)]] Texture2D<float> image1 : register(t0);
[[vk::binding(1)]] Texture2D<float4> image2 : register(t1);
[[vk::binding(2)]] SamplerState sampler0 : register(s0);

[[vk::binding(3)]] cbuffer Constants : register(b0) {
    float blurStrength;
}

struct VertexOutput {
    float2 texCoord : TEXCOORD0;
};

float Gaussian(float x, float sigma) {
    return exp(-(x * x) / (2 * sigma * sigma)) / (sqrt(2 * 3.14159 * sigma * sigma));
}

float4 Blur(float2 uv, float2 direction, float sigma) {
    float4 color = 0;
    float totalWeight = 0;

    for (int i = -5; i <= 5; ++i) {
        float offset = i * 0.1;
        color += image2.SampleLevel(sampler0, uv + direction * offset, 0) * Gaussian(offset, sigma);
        totalWeight += Gaussian(offset, sigma);
    }

    return color / totalWeight;
}

VertexOutput VSMain(uint vertexID : SV_VERTEXID) {
    VertexOutput output;
    
    // Generate quad vertices
    float2 position = float2(vertexID == 0 || vertexID == 1 ? -1 : 1,
                              vertexID == 0 || vertexID == 2 ? -1 : 1);
    output.texCoord = 0.5 * (float2(1, -1) * position + 1);
    
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET {
    float intensity1 = image1.SampleLevel(sampler0, input.texCoord, 0).r;
    float4 color2 = image2.SampleLevel(sampler0, input.texCoord, 0);

    float4 blurred = Blur(input.texCoord, float2(1, 0), blurStrength) + Blur(input.texCoord, float2(0, 1), blurStrength);

    return color2 + (blurred - color2) * intensity1;
}
