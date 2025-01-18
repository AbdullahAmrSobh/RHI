struct DirectionalLight
{
    float3 direction;
    float3 color;
    float intensity;
};

struct PointLight
{
    float3 position;
    float3 color;
    float intensity;
    float radius;
    float attenuation;
};

struct SpotLight
{
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
    float radius;
    float angle;
    float attenuation;
};

enum LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

struct LightData
{
    float3 position; // The position of the light (for point and spot lights).
    float3 direction; // The direction of the light (for directional and spot lights).
    float3 color; // The color of the light.
    float intensity; // The intensity of the light.
    float radius; // The radius of the light (for point and spot lights).
    float angle; // The angle of the light (for spot lights).
    float attenuation; // The attenuation of the light.
    LightType type; // The type of the light (0 = directional, 1 = point, 2 = spot).


    __init(DirectionalLight dirLight)
    {
        this.position = float3(0, 0, 0);
        this.direction = dirLight.direction;
        this.color = dirLight.color;
        this.intensity = dirLight.intensity;
        this.radius = 0;
        this.angle = 0;
        this.attenuation = 0;
        this.type = LightType::Directional;
    }

    __init(PointLight pointLight)
    {
        this.position = pointLight.position;
        this.direction = float3(0, 0, 0);
        this.color = pointLight.color;
        this.intensity = pointLight.intensity;
        this.radius = pointLight.radius;
        this.angle = 0;
        this.attenuation = pointLight.attenuation;
        this.type = LightType::Point;
    }

    __init(SpotLight spotLight)
    {
        this.position = spotLight.position;
        this.direction = spotLight.direction;
        this.color = spotLight.color;
        this.intensity = spotLight.intensity;
        this.radius = spotLight.radius;
        this.angle = spotLight.angle;
        this.attenuation = spotLight.attenuation;
        this.type = LightType::Spot;
    }
};
