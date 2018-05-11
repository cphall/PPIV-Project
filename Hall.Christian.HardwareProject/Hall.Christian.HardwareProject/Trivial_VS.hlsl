#pragma pack_matrix( row_major )
struct VS_IN
{
	float3 posL : POSITION;
	float4 rgba : COLOR;
};

struct VS_OUT
{
	float4 posH : SV_POSITION;
	float4 clr : COLOR;
};

cbuffer OBJECT_DATA : register( b0 )
{
	float4x4 worldMatrix;
	float4 rgba;
};


cbuffer SCENE : register( b1 )
{
	/*float4 constantColor;
	float2 constantOffset;
	float2 padding;*/
	float4x4 viewMatrix;
	float4x4 projMatrix;
};

VS_OUT main( VS_IN input )
{
	VS_OUT sendToRasterizer = (VS_OUT)0;
	
	float4 localH = float4(input.posL, 1);
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);

	sendToRasterizer.posH = localH;
	sendToRasterizer.clr = input.rgba;

	return sendToRasterizer;
}