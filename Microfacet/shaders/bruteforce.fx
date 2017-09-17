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

cbuffer cbChangesEveryFrame
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
    float3 Normal	: NORMAL;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, WorldViewProj);
    output.Pos = -output.Pos;
    output.Normal = normalize(mul(float4(input.Normal, 1.0f), WorldView));
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    return float4((input.Normal + float3(1,1,1))/2, 0);
}
