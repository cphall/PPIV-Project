textureCUBE env : register(t1);

SamplerState envFilter : register(s0);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float4 norm : NORMAL;
	float3 uv : TEXTCOORD;
};

float4 main(PS_IN input) : SV_TARGET
{
	return env.Sample(envFilter, input.uv);
}