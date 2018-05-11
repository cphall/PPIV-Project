
struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 clr : COLOR;
	float2 uv : TEXTCOORD;
	float3 norm : NORMAL;
};

float4 main(PS_IN input) : SV_TARGET
{
	return input.clr;
}