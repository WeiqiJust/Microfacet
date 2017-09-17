//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer cbChangesEveryFrame : register(b0)
{
    matrix WorldViewProj;
    matrix World;
};

cbuffer cbLights : register(b1)
{
	float4	dir;
	float4	inten;
	matrix	LightViewProj;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos		: POSITION;
    float3 Normal	: NORMAL;
};

struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, WorldViewProj);
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{  
    return dir.x;
}
