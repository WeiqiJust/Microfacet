
struct PsInputContext
{
	float4 Pos		: SV_POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float2 UV		: TEXCOORD0;
};

/*
struct PsOutputContext
{
	float2	UV				: SV_Target0;
	//float4	NormalTangent	: SV_Target1;
	float3	Normal			: SV_Target1;
	float3	Tangent			: SV_Target2;
};*/

/*
PsOutputContext main(PsInputContext input)
{
	PsOutputContext output = (PsOutputContext)0;

	output.Normal = input.Normal;
	output.Tangent = normalize(input.Tangent - dot(input.Tangent, output.Normal)*output.Normal);

	output.UV = input.UV;

	return output;
}
*/

float2 main() : SV_TARGET
{
	return float2(1.0f, 1.0f);
}