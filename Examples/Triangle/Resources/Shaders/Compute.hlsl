[[vk::binding(0)]] RWTexture2D<float> outputImage : register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID) {
    uint height;
    uint width;
    uint levels;
    outputImage.GetDimensions(width, height);

    // Calculate the center and radius of the circle
    float2 center = float2(width / 2.0f, height / 2.0f);
    float radius = min(width, height) / 4.0f;

    // Calculate the distance from the current pixel to the center of the circle
    float2 pixelPos = float2(dispatchThreadId.xy);
    float distance = length(pixelPos - center);

    // If the distance is less than the radius, the pixel is inside the circle
    // Otherwise, it's outside
    if (distance < radius) {
        outputImage[dispatchThreadId.xy] = 1.0f;
    } else {
        outputImage[dispatchThreadId.xy] = 0.0f;
    }
}
