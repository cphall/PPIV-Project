// CGS HW Project A "Line Land".
// Author: L.Norri CD CGS, FullSail University

// Introduction:
// Welcome to the hardware project of the Computer Graphics Systems class.
// This assignment is fully guided but still requires significant effort on your part. 
// Future classes will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this project and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include <Windows.h>

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
// TODO: PART 1 STEP 1b
#include <DirectXMath.h>
using namespace DirectX;
// TODO: PART 2 STEP 6

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500
#define _DEBUG 0
#define NUMVERTS 369
#define NUMTVERTS 2400

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	// TODO: PART 1 STEP 2
	ID3D11Device *device;
	IDXGISwapChain *swapChain;
	ID3D11RenderTargetView *targetView;
	ID3D11DeviceContext *context;
	D3D11_VIEWPORT viewport;
	ID3D11Texture2D* BackBuffer;
	// TODO: PART 2 STEP 2
	ID3D11Buffer *vertBuffer;
	ID3D11InputLayout *inputLayout;
	unsigned int numVerts = NUMVERTS;
	// BEGIN PART 5
	// TODO: PART 5 STEP 1
	ID3D11Buffer *triangleVertBuffer;
	ID3D11InputLayout *inputLayoutTriangle;
	unsigned int numTriangleVerts = NUMTVERTS;
	// TODO: PART 2 STEP 4
	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixShader;
	// BEGIN PART 3
	// TODO: PART 3 STEP 1
	ID3D11Buffer *constBuffer;
	XTime timer;
	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		XMFLOAT4 constantColor;
		XMFLOAT2 constantOffset;
		XMFLOAT2 padding;
	};
	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;
	SEND_TO_VRAM triangleSend;
public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		XMFLOAT2 pos;
		XMFLOAT4 rgba;
	};
	bool reverseX = false;
	bool reverseY = false;
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst; 
	appWndProc = proc; 

	WNDCLASSEX  wndClass;
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"CGS Hardware Project",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
	//********************* END WARNING ************************//

	// TODO: PART 1 STEP 3a
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 1; //no aa
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.OutputWindow = window;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.Height = BACKBUFFER_HEIGHT;
	swapDesc.BufferDesc.Width = BACKBUFFER_WIDTH;

	// TODO: PART 1 STEP 3b
	UINT flag = 0;
#if _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flag, NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, NULL, &context);
	// TODO: PART 1 STEP 4
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
	device->CreateRenderTargetView(BackBuffer, NULL, &targetView);
	
	// TODO: PART 1 STEP 5
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	viewport.Height = 500;
	viewport.Width = 500;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// TODO: PART 2 STEP 3a
	const float PI = 3.1415927;
	SIMPLE_VERTEX vertArr[NUMVERTS];
	for (unsigned int i = 0; i < NUMVERTS; i++)
	{
		float sin = sinf(i * ( PI/ 180));
		float cos = cosf(i * (PI / 180));
		vertArr[i].pos = { cos, sin };
		vertArr[i].rgba = { 0.0f, 0.0f, 0.0f, 0.0f };
	}
	// BEGIN PART 4
	// TODO: PART 4 STEP 1
	for (unsigned int i = 0; i < NUMVERTS; i++)
	{
		vertArr[i].pos = { vertArr[i].pos.x * .20f, vertArr[i].pos.y * .20f };
	}
	// TODO: PART 2 STEP 3b
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL; //may need to be D3D11_CPU_ACCESS_WRITE later
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * numVerts;
	bufferDesc.MiscFlags = 0;
    // TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = vertArr;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	// TODO: PART 2 STEP 3d
	hr = device->CreateBuffer(&bufferDesc, &InitData, &vertBuffer);
	// TODO: PART 5 STEP 2a
	SIMPLE_VERTEX vertTArr[NUMTVERTS];
	for (unsigned int i = 0; i < NUMTVERTS; i++)
	{
		vertTArr[i].rgba = { 1.0f,0.0f, 0.0f,0.0f };
	}
	// TODO: PART 5 STEP 2b
	unsigned int vertexCount = 0;
	for (unsigned int y = 0; y < 20; y++) //going horizontally each row at a time
	{
		unsigned int x = 0;
		for (; x < 20; x++)
		{
			if ((x+y) % 2 != 0) //if even then we stagger
			{
				//fill vertices for both triangles in the square
				vertTArr[vertexCount].pos = { -1 + (0.1f * x), 1 - (0.1f * y) };
				vertTArr[vertexCount+1].pos = { -0.9f + (0.1f * x) , 1 - (0.1f * y) };
				vertTArr[vertexCount+2].pos = { -1 + (0.1f *x), 0.9f - (0.1f * y) };
				vertTArr[vertexCount+3].pos = { -1 + (0.1f *x), 0.9f - (0.1f * y) };
				vertTArr[vertexCount+4].pos = { -0.9f + (0.1f * x) , 1 - (0.1f * y) };
				vertTArr[vertexCount+5].pos = { -0.9f + (0.1f * x), 0.9f - (0.1f * y) };
				vertexCount += 6;
			}
		}
	}
	// TODO: PART 5 STEP 3
	D3D11_BUFFER_DESC trianglebufferDesc;
	ZeroMemory(&trianglebufferDesc, sizeof(D3D11_BUFFER_DESC));
	trianglebufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	trianglebufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	trianglebufferDesc.CPUAccessFlags = NULL; //may need to be D3D11_CPU_ACCESS_WRITE later
	trianglebufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * NUMTVERTS;
	trianglebufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA triangleInitData;
	ZeroMemory(&triangleInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	triangleInitData.pSysMem = vertTArr;
	triangleInitData.SysMemPitch = 0;
	triangleInitData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&trianglebufferDesc, &triangleInitData, &triangleVertBuffer);
	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pixShader);
	// TODO: PART 2 STEP 8a
	D3D11_INPUT_ELEMENT_DESC vLayout[2] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// TODO: PART 2 STEP 8b
	device->CreateInputLayout(vLayout, 2, Trivial_VS, sizeof(Trivial_VS), &inputLayout);
	// TODO: PART 3 STEP 3
	D3D11_BUFFER_DESC cbufferDesc;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allows cpu to modify our data at runtime
	cbufferDesc.MiscFlags = 0;
	cbufferDesc.ByteWidth = sizeof(SEND_TO_VRAM); //May need to redo this stuff for this buffer?

	D3D11_SUBRESOURCE_DATA cbInitData;
	ZeroMemory(&cbInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbInitData.pSysMem = &toShader;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&cbufferDesc, &InitData, &constBuffer);

	// TODO: PART 3 STEP 4b
	toShader.constantOffset = { 0.0f, 0.0f };
	toShader.constantColor = { 1.0f, 1.0f, 0.0f, 0.0f };
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	timer.Signal();
	// TODO: PART 4 STEP 3
	XMFLOAT2 velocity = { 1.0f, 0.5f };
	velocity.x *= timer.SmoothDelta();
	velocity.y *= timer.SmoothDelta();
	// TODO: PART 4 STEP 5

	if (toShader.constantOffset.x >= 1)
		reverseX = true;
	else if (toShader.constantOffset.x <= -1)
		reverseX = false;
	if (toShader.constantOffset.y >= 1)
		reverseY = true;
	else if (toShader.constantOffset.y <= -1)
		reverseY = false;

	if (reverseX)
		toShader.constantOffset.x -= velocity.x;
	else if (!reverseX)
		toShader.constantOffset.x += velocity.x;

	if (reverseY)
		toShader.constantOffset.y -= velocity.y;
	else if (!reverseY)
		toShader.constantOffset.y += velocity.y;
	// END PART 4

	// TODO: PART 1 STEP 7a
	context->OMSetRenderTargets(1, &targetView, NULL);
	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);
	// TODO: PART 1 STEP 7c
	float clearColor[4] = { 0,0,1,0 };
	context->ClearRenderTargetView(targetView, clearColor);
	// TODO: PART 5 STEP 4
	triangleSend.constantColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	triangleSend.constantOffset = { 0.0f, 0.0f };

	// TODO: PART 5 STEP 5
	D3D11_MAPPED_SUBRESOURCE tsubResource;
	ZeroMemory(&tsubResource, sizeof(tsubResource));
	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &tsubResource);
	memcpy(tsubResource.pData, &triangleSend, sizeof(SEND_TO_VRAM));
	context->Unmap(constBuffer, NULL);
	context->VSSetConstantBuffers(0, 1, &constBuffer); //do I need this?
	// TODO: PART 5 STEP 6
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &triangleVertBuffer, &stride, &offset);
	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);												  
	context->Draw(NUMTVERTS, 0);
	// TODO: PART 5 STEP 7
	// END PART 5
	
	// TODO: PART 3 STEP 5
	D3D11_MAPPED_SUBRESOURCE subResource;
	ZeroMemory(&subResource, sizeof(subResource));
	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &toShader, sizeof(SEND_TO_VRAM));
	context->Unmap(constBuffer, NULL);
	// TODO: PART 3 STEP 6
	context->VSSetConstantBuffers(0, 1, &constBuffer);
	// TODO: PART 2 STEP 9a
	context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	// TODO: PART 2 STEP 9b
	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);
	// TODO: PART 2 STEP 9c
	context->IASetInputLayout(inputLayout);
	// TODO: PART 2 STEP 9d
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP); //or linestrip
	// TODO: PART 2 STEP 10
	context->Draw(numVerts, 0);
	// END PART 2

	// TODO: PART 1 STEP 8
	swapChain->Present(0, 0);
	// END OF PART 1
	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6
	device->Release();
	swapChain->Release();
	targetView->Release();
	context->Release();
	BackBuffer->Release();
	vertBuffer->Release();
	constBuffer->Release();
	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//