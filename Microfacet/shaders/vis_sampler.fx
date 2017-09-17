//
// Constant Buffer Variables
//

cbuffer cbConstant : register(b0)
{
    matrix visViewProj[6];
};

cbuffer cbUserChanges : register(b1)
{
    matrix visWorld;
};

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
	GS_IN output = (GS_IN)0;
    
	output.pos = mul(input.pos, visWorld);

	return output;
}

//Geometry Shader
[maxvertexcount(18)]
void GS(triangle GS_IN input[3], inout TriangleStream<PS_IN> out_stream)
{
    for( int f = 0; f < 6; ++f )
    {
        // Compute screen coordinates
        PS_IN output;

		output.RTIndex = f;
        for( int v = 0; v < 3; v++ )
        {
            output.pos = mul(input[v].pos, visViewProj[f]);
            out_stream.Append(output);
        }
        out_stream.RestartStrip();
    }
}

// Pixel Shader
float4 PS(PS_IN input) : SV_TARGET
{
    return float4(0, 0, 0, 1);
}
