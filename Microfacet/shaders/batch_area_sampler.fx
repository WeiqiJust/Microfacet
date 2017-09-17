//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//#include "batch_area_sampler_config.txt"
#define MAX_TARGETS 4
cbuffer cbViewProj : register(b0)
{
    matrix ViewProj[MAX_TARGETS];
};

cbuffer cbWorld : register(b1)
{
	matrix World;
}

cbuffer cbAttr : register(b2)
{
	float4	attr;
};

//--------------------------------------------------------------------------------------
struct VS_IN
{
	float4 pos	: POSITION;
};

struct GS_IN
{
	float4 pos	: SV_POSITION;
};

struct PS_IN
{
    float4 pos	: SV_POSITION;
    uint RTIndex: SV_RenderTargetArrayIndex;
};

// Vertex Shader
GS_IN VS(VS_IN input)
{
	GS_IN output;
	output.pos = mul(input.pos, World);
	return output;
}

//Geometry Shader
[maxvertexcount(3*MAX_TARGETS)]
void GS(triangle GS_IN input[3], inout TriangleStream<PS_IN> out_stream)
{
    for( int f = 0; f < MAX_TARGETS; ++f )
    {
        // Compute screen coordinates
        PS_IN output;

		output.RTIndex = f;
        for( int v = 0; v < 3; v++ )
        {
            output.pos = mul(input[v].pos, ViewProj[f]);
            out_stream.Append(output);
        }
        out_stream.RestartStrip();
    }
}

// Pixel Shader
float4 PS(PS_IN input) : SV_TARGET
{
    return attr.w;
}
