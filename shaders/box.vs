cbuffer perObject : register(b0)
{
	float4x4 MatWorld;
	float4x4 MatWorldViewProj;
};


struct vi
{
	float3 Position : SV_POSITION;
	float3 Normal	: NORMAL0;
	float2 TexCoords: TEXCOORD0;
};

struct pi
{
	float4 Position : SV_POSITION;
	float3 Pos	: TEXCOORD0;
	float3 Normal	: TEXCOORD1;
	float2 TexCoords: TEXCOORD2;
};

void main( in vi IN, out pi OUT)
{
	OUT.Pos = IN.Position;
	OUT.Position = mul(MatWorldViewProj, float4(IN.Position,1));

	OUT.Normal = normalize( mul((float3x3)(MatWorld), IN.Normal) );

	OUT.TexCoords = IN.TexCoords;
}