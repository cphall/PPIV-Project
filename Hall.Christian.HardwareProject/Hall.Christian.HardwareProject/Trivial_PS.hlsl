
struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	//float4 norm : NORMAL;
	//float2 uv : TEXTCOORD;
};

float4 main(PS_IN input) : SV_TARGET
{
	return input.color;
}