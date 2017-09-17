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
	float3	inten;
	float	id;
	matrix	LightViewProj;
};

//--------------------------------------------------------------------------------------
struct VsInput
{
    float4 Pos		: POSITION;
    float3 Normal	: NORMAL;
};

struct PsInput
{
    float4 Pos		: SV_POSITION;
    float3 Normal	: NORMAL;
};

struct PsOutput
{
	float	id				: SV_Target0;
	float3	Normal			: SV_Target1;
};

//--------------------------------------------------------------------------------------
// Shaders for generating context
//--------------------------------------------------------------------------------------
PsInput VS(VsInput input)
{
    PsInput output = (PsInput)0;
    output.Pos		= mul(input.Pos, WorldViewProj);
    output.Normal	= mul(input.Normal, (float3x3)World);    

    return output;
}

PsOutput PS(PsInput input)
{
	PsOutput output = (PsOutput)0;
	
	output.id		= id;
	output.Normal	= normalize(input.Normal);
	
	return output;
}
