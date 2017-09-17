
cbuffer cbChangesEveryFrame : register(b0)
{
	matrix WorldViewProj;
	matrix World;
};

struct VsInputContext
{
	float4 Pos		: POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float2 UV		: TEXCOORD0;
};


struct PsInputContext
{
	float4 Pos		: SV_POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float2 UV		: TEXCOORD0;
};


/*
PsInputContext main(VsInputContext input)
{

	PsInputContext output = (PsInputContext)0;
	output.Pos = mul(input.Pos, WorldViewProj);
	output.Normal = mul(input.Normal, (float3x3)World);
	output.Tangent = mul(input.Tangent, (float3x3)World);

	//output.Pos = input.Pos;
	//output.Normal = input.Normal;
	//output.Tangent = input.Tangent;

	//output.UV = input.UV;

	//float4 p = mul(input.Pos, World);
	//output.UV = p.xyz / p.w;
	return input;// output;
}*/

/*
PsInputContext main(VsInputContext input)
{
	PsInputContext output = (PsInputContext)0;
	 output.Pos = input.Pos;
	output.Normal = input.Normal;
	output.Tangent = input.Tangent;

	output.UV = input.UV;

	//float4 p = mul(input.Pos, World);
	//output.UV = p.xyz / p.w;

	return output;
}*/

float4 main(float4 pos : POSITION) : SV_POSITION
{
	return pos;
}