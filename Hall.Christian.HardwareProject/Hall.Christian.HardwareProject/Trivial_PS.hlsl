
texture2D baseTexture : register(t0);
SamplerState filter : register(s0);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float4 norm : NORMAL;
	float2 uv : TEXTCOORD;
};

struct PS_OUT
{
};

float4 main(PS_IN input) : SV_TARGET
{
	return baseTexture.Sample(filter, input.uv) *input.color;
}