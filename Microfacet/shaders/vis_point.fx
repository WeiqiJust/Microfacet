//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//#include "vis_point_config.txt"
#define VIS_SHADOW_MAP_SIZE 512
#define NUM_VIS_SAMPLES 8
#define SHADOW_EPSILON	0.00015f

Texture2D		texPoint : register(t0),
				texNormal : register(t1),
				texShadow[NUM_VIS_SAMPLES] : register(t2);
SamplerState	samContext : register(s0),
				samShadow : register(s1);

struct single_light
{
	float4	dir;
	matrix	LightViewProj;
};

cbuffer cbLights : register(b1)
{
	single_light	lights[NUM_VIS_SAMPLES];
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
	float2 Tex		: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VS( VS_INPUT input )
{
    return input;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

uint PS( PS_INPUT input) : SV_Target
{
	float4 normal = texNormal.Sample(samContext, input.Tex);
	if (normal.w <= 0)
		return 0;
		
	uint result = 0;
	
	float3 pos = texPoint.Sample(samContext, input.Tex).xyz;
	for (int i = 0; i < NUM_VIS_SAMPLES; i++)
		if (result == 0 && dot(normal.xyz, lights[i].dir.xyz) > 0)
		{   
			float4 lPos			= mul(float4(pos, 1), lights[i].LightViewProj);   
			float2 shadowTexC	= 0.5 * lPos.xy / lPos.w + float2(0.5, 0.5);
			shadowTexC.y = 1.0 - shadowTexC.y;
			float2 texelPos = VIS_SHADOW_MAP_SIZE * shadowTexC;

			if (texShadow[i].Sample(samShadow, shadowTexC).x+SHADOW_EPSILON > lPos.z/lPos.w)
				result = 255;
		}
    
	return result;
}

uint PSMask(PS_INPUT input) : SV_Target
{
	float4 normal = texNormal.Sample(samContext, input.Tex);
	if (normal.w <= 0)
		return 0;
		
	uint result = 0, mask = 1;
	
	float3 pos = texPoint.Sample(samContext, input.Tex).xyz;
	for (int i = 0; i < NUM_VIS_SAMPLES; i++)
	{
		if (lights[i].dir.w > 0 && dot(normal.xyz, lights[i].dir.xyz) > 0)
		{   
			float4 lPos			= mul(float4(pos, 1), lights[i].LightViewProj);   
			float2 shadowTexC	= 0.5 * lPos.xy / lPos.w + float2(0.5, 0.5);
			shadowTexC.y = 1.0 - shadowTexC.y;
			float2 texelPos = VIS_SHADOW_MAP_SIZE * shadowTexC;

			if (texShadow[i].Sample(samShadow, shadowTexC).x+SHADOW_EPSILON > lPos.z/lPos.w)
				result += mask;
		}
		mask *= 2;
    }
	return result;
	
}
/*
float2 PSMask() : SV_Target
{
	return float2(1.0f, 1.0f);
}*/