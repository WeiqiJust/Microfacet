//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//#include "T:/Microfacet/shaders/shadow_config.txt"

#define SHADOW_EPSILON	0.0010f//0.0005f//0.00005f
#define SHADOW_MAP_SIZE 512

Texture2D		texShadow : register( t0 );
SamplerState	samShadow : register( s0 );

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
    float3 Normal	: NORMAL;
    float4 lPos		: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, WorldViewProj);
    output.Normal = mul(input.Normal, (float3x3)World);
    
    output.lPos = mul(mul(input.Pos, World), LightViewProj);   
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float3 normal = normalize(input.Normal);
	
    if (dot(normal, (float3)dir) > 0)
    {   
        //transform from RT space to texture space.
        float2 shadowTexC = 0.5 * input.lPos.xy / input.lPos.w + float2(0.5, 0.5);
        shadowTexC.y = 1.0 - shadowTexC.y;
		float2 texelPos = SHADOW_MAP_SIZE * shadowTexC;
        
        float lightAmount = (texShadow.Sample(samShadow, shadowTexC)+SHADOW_EPSILON < input.lPos.z/input.lPos.w)? 0.0f: 1.0f;
	    return float4(lightAmount * saturate(dot(normal, (float3)dir)) * (float3)inten, 1);
    }
    
    return float4(0, 0, 0, 1);
}

float4 PS_fence(PS_INPUT input) : SV_Target
{
    return float4(241.0/255.0, 86.0/255.0, 52.0/255.0, 0.4);
}


float4 PS_constant(PS_INPUT input) : SV_Target
{
    return float4((float3)inten, 0.5);
}