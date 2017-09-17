//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//#include "shadow_config.txt"

//#define SHADOW_EPSILON	0.00005f
#define SHADOW_EPSILON	0.0003f
#define SHADOW_MAP_SIZE 512
Texture2D		texShadow : register(t0);
SamplerState	samShadow : register(s0);

cbuffer cbChangesEveryFrame : register(b0)
{
    matrix WorldViewProj;
    matrix World;
};

cbuffer cbLights : register(b1)
{
	float4	dir;
	float3	inten;
	float	vis_mask;
	matrix	LightViewProj;
};

//--------------------------------------------------------------------------------------
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

struct PsOutputContext
{
	float2	UV				: SV_Target0;
	//float4	NormalTangent	: SV_Target1;
	float3	Normal			: SV_Target1;
	float3	Tangent			: SV_Target2;
};

//--------------------------------------------------------------------------------------
// Shaders for generating context
//--------------------------------------------------------------------------------------
PsInputContext VSContext(VsInputContext input)
{
    PsInputContext output = (PsInputContext)0;
	output.Pos = mul(input.Pos, WorldViewProj);
	output.Normal = mul(input.Normal, (float3x3)World);
	output.Tangent = mul(input.Tangent, (float3x3)World);

	output.UV = input.UV;
    
    //float4 p = mul(input.Pos, World);
    //output.UV = p.xyz / p.w;
    
    return output;
}

PsOutputContext PSContext(PsInputContext input)
{
	PsOutputContext output = (PsOutputContext)0;
	
	output.Normal	= normalize(input.Normal);
	output.Tangent	= normalize(input.Tangent - dot(input.Tangent, output.Normal)*output.Normal);
	
	output.UV = input.UV;

    //float3 p = normalize(float3(0, 0, 1) - input.UV);
    //output.UV.x = abs(p.x);
    //output.UV.y = abs(p.y);	
	
	return output;
}

PsOutputContext PSContextBackground(PsInputContext input) : SV_Target
{
	PsOutputContext output = (PsOutputContext)0;
	output.UV = float2(-1.0f, -1.0f);
	return output;
}


//--------------------------------------------------------------------------------------
struct VsInputShadow
{
    float4 Pos		: POSITION;
    float3 Normal	: NORMAL;
};

struct PsInputShadow
{
    float4 Pos		: SV_POSITION;
    float3 Normal	: NORMAL;
    float4 lPos		: TEXCOORD0;    
};

//--------------------------------------------------------------------------------------
// Shaders for shadow tests
//--------------------------------------------------------------------------------------
PsInputShadow VSShadow(VsInputShadow input)
{
    PsInputShadow output = (PsInputShadow)0;
    output.Pos = mul(input.Pos, WorldViewProj);
    output.Normal = mul(input.Normal, (float3x3)World);
    output.lPos = mul(mul(input.Pos, World), LightViewProj);   
    return output;
}

float PSShadow(PsInputShadow input) : SV_Target
{
	float3 normal = normalize(input.Normal);
	float3 dir3 = float3(dir.x, dir.y, dir.z);
	float d = dot(normal, dir3);
    if (d > 0)
    {   
        //transform from RT space to texture space.
        float2 shadowTexC = 0.5 * input.lPos.xy / input.lPos.w + float2(0.5, 0.5);
        shadowTexC.y = 1.0 - shadowTexC.y;
        float2 texelPos = SHADOW_MAP_SIZE * shadowTexC;
        
        if (texShadow.Sample(samShadow, shadowTexC).r+SHADOW_EPSILON > input.lPos.z/input.lPos.w)
			return vis_mask;
    }
    
    return 0;
}
