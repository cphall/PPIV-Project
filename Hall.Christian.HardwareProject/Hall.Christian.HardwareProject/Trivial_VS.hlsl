#pragma pack_matrix( row_major )
struct VS_IN
{
	float3 posL : POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXTCOORD;
};

struct VS_OUT
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float4 norm : NORMAL;
	float2 uv : TEXTCOORD;
};

cbuffer OBJECT_DATA : register( b0 )
{
	float4x4 worldMatrix;
};

cbuffer SCENE : register( b1 )
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 rotMatrix;
	float4 lightVector;
	float4 lightClr;
	float4 ambientClr;
};

VS_OUT main( VS_IN input )
{
	VS_OUT sendToRasterizer = (VS_OUT)0;
	
	float4 localH = float4(input.posL, 1);
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);

	sendToRasterizer.posH = localH;

	//light stuff needs to be put in pixel shader
	sendToRasterizer.color = ambientClr;
	sendToRasterizer.uv = input.uv;
	float4 norm = normalize(mul(input.norm, worldMatrix));
	float diffuse = saturate(dot(norm, lightVector));
	sendToRasterizer.color += lightClr * diffuse;

	return sendToRasterizer;
}