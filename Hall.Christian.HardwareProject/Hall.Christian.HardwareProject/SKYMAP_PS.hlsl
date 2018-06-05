textureCUBE env : register(t1);

SamplerState envFilter : register(s0);

float4 main(float4 posH : SV_POSITION, float4 icolor: COLOR) : SV_TARGET
{
	return env.Sample(envFilter, posH.xyz);
}