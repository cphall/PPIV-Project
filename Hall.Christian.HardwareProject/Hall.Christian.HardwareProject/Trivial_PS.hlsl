
texture2D baseTexture : register(t0);
SamplerState filter : register(s0);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float4 norm : NORMAL;
	float2 uv : TEXTCOORD;
};

struct Light
{
	float3 direction;
	float3 position;
	float range;
	float3 attenuation;
	float4 ambient;
	float4 diffuse;
	float lighttype;
};

cbuffer perFrame
{
	Light light;
};

struct PS_OUT
{
};

float4 main(PS_IN input) : SV_TARGET
{
	return baseTexture.Sample(filter, input.uv) *input.color;
}