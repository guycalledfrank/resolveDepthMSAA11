struct vi
{
	float3 Position : SV_POSITION;
};

struct pi
{
    float4 Position : SV_POSITION;
	float2 TexCoords: TEXCOORD0;
};

void main( in vi IN, out pi OUT)
{
	OUT.Position = float4(IN.Position.xy, 0.5, 1);
	OUT.TexCoords = IN.Position.xy * 0.5 + 0.5;
}
