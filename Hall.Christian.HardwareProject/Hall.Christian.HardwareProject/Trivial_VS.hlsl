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

	//Matricies
	//float4x4 matWorld;
	float4x4 viewMatrix;
	/*float4x4 matRotateX;
	float4x4 matRotateY;
	float4x4 matRotateZ;
	float4x4 matScale;
	float4x4 matTranslate;*/
	float4x4 projMatrix;
	/*float4x4 matFinal;*/
};

VS_OUT main( VS_IN input )
{
	VS_OUT sendToRasterizer = (VS_OUT)0;
	/*sendToRasterizer.projectedCoordinate.w = 1;
	
	sendToRasterizer.projectedCoordinate.xy = fromVertexBuffer.coordinate.xy;
		
	sendToRasterizer.projectedCoordinate.xy += constantOffset;
	
	sendToRasterizer.colorOut = constantColor;

	sendToRasterizer.*/
	float4 localH = float4(input.posL, 1);
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);

	return sendToRasterizer;
}