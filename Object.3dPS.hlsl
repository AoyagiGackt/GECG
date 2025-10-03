#include "Object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    int shadingType; // 0: Lambert, 1: HalfLambert
    float2 padding;
    float4x4 uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float4 baseColor = gMaterial.color * textureColor;

    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);

        float lighting = 1.0f;
        if (gMaterial.shadingType == 0)
        { // Lambert
            lighting = max(NdotL, 0.0f);
        }
        else
        { // HalfLambert
            lighting = NdotL * 0.5f + 0.5f;
        }

        //output.color = baseColor * gDirectionalLight.color * lighting * gDirectionalLight.intensity;
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * lighting * gDirectionalLight.intensity;
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb  * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = baseColor;
    }
    
    return output;
}