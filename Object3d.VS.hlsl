float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

// float32_t4
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = input.position;
    return output;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    return output;
}

