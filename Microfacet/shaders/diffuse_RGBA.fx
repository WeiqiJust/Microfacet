//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//#include "shadow_config.txt"
#define SHADOW_MAP_SIZE 512
#define SHADOW_EPSILON	0.00005f

Texture2D		texShadow : register( t0 );
SamplerState	samShadow : register( s0 );

cbuffer cbChangesEveryFrame : register(b0)
{
    matrix WorldViewProj;
    matrix World;
};

cbuffer cbLights : register(b1)
{
	float4	dir[4];
	float4	inten[4];
	matrix	LightViewProj[4];
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
    float3 Normal	: NORMAL;
    float4 lPos[4]	: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, WorldViewProj);
    output.Normal = mul(input.Normal, (float3x3)World);
    
    for (int i = 0; i < 4; i++)
		output.lPos[i] = mul(mul(input.Pos, World), LightViewProj[i]);   

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float3 normal = normalize(input.Normal);
	float4 output = float4(0, 0, 0, 1);
	
	for (int i = 0; i < 4; i++)
	{
		float d;
		d = dot(normal, (float3)dir[i]);
		if (d > 0)
		{   
			float2 shadowTexC = 0.5 * input.lPos[i].xy / input.lPos[i].w + float2(0.5, 0.5);
			shadowTexC.y = 1.0 - shadowTexC.y;
			float2 texelPos = SHADOW_MAP_SIZE * shadowTexC;
	        
			float light = (texShadow.Sample(samShadow, shadowTexC)[i]+SHADOW_EPSILON < input.lPos[i].z/input.lPos[i].w) ? 0.0f : 1.0f;
				output.xyz += light * saturate(d) * (float3)inten[i];
		}
	}
	    
    return output;
}
