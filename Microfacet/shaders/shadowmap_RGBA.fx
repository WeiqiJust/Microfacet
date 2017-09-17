//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
//Texture2D txDiffuse : register( t0 );
//SamplerState samLinear : register( s0 );

cbuffer cbChangesEveryFrame : register(b0)
{
    matrix WorldViewProj;
    matrix WorldView;
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
    float2 Depth	: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos	= mul(input.Pos, WorldViewProj);
    output.Depth= output.Pos.zw;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    float z = input.Depth.x / input.Depth.y;
    return float4(z, z, z, z);
}
