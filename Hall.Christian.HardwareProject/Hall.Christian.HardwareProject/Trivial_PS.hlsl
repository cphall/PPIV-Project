
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
	//int lighttype;
};

cbuffer perFrame : register(b0)
{
	Light light;
};

struct PS_OUT
{
};

float4 main(PS_IN input) : SV_TARGET
{
	return baseTexture.Sample(filter, input.uv) *input.color;

	//point light
	/*input.norm = normalize(input.norm);
	float4 diffuse = baseTexture.Sample(filter, input.uv);
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);
	float3 lightToPixelVec = light.position - input.posH;
	float distance = length(lightToPixelVec);
	float3 ambient = diffuse * light.ambient;
	
	if (distance > light.range)
	return float4(ambient, diffuse.a);
	
	lightToPixelVec /= distance;
	
	float howMuch = dot(lightToPixelVec, input.norm);
	if (howMuch > 0.0f)
	{
		finalColor += howMuch * diffuse * light.diffuse;
	
		finalColor /= light.attenuation[0] + (light.attenuation[1] * distance) + (light.attenuation[2] * (distance*distance));
	}
	
	finalColor = saturate(finalColor + ambient);
	return float4(finalColor, diffuse.a);*/
}