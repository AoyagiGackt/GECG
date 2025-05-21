
struct Vector4
{
    float x, y, z, w;
};

struct Material
{
    Vector4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
    float4 color : SV_TARGET;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    return output;
}