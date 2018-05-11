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
#include <windowsx.h>
#include <DirectXMath.h>
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
using namespace std;


using namespace DirectX;


#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define _DEBUG 0
#define NUMVERTS 369
#define NUMTVERTS 2400
#define CAMERAEYEX 0
#define CAMERAEYEY 0
#define CAMERAEYEZ -5

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	
	ID3D11Device *device;
	IDXGISwapChain *swapChain;
	ID3D11RenderTargetView *targetView;
	ID3D11DeviceContext *context;
	D3D11_VIEWPORT viewport;
	ID3D11Texture2D* BackBuffer;
	
	ID3D11Buffer *vertBuffer;
	ID3D11InputLayout *inputLayout;
	unsigned int numVerts = NUMVERTS;
	
	ID3D11Buffer *triangleVertBuffer;
	ID3D11InputLayout *inputLayoutTriangle;
	unsigned int numTriangleVerts = NUMTVERTS;
	
	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixShader;
	
	ID3D11Buffer *constBuffer;
	ID3D11Buffer *constBuffer2;
	XTime timer;

	XMMATRIX matrixTranslate;
	XMMATRIX matrixRotateX;

	float fovAngleY = 65.0f;
	float AspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float NearZ = 0.1f;
	float FarZ = 100.0f;

	struct WM_TO_VRAM
	{
		/*XMFLOAT4 constantColor;
		XMFLOAT2 constantOffset;
		XMFLOAT2 padding;*/
		XMMATRIX worldMatrix;
		XMFLOAT4 constantColor;
	};
	struct VPM_TO_VRAM
	{
		XMMATRIX viewMatrix;
		XMMATRIX projMatrix;
	};

	WM_TO_VRAM WMToShader;
	VPM_TO_VRAM VPMToShader;
	
public:
	
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 pos;
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
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

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
	swapDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapDesc.BufferDesc.Width = SCREEN_WIDTH;

	UINT flag = 0;
#if _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, NULL, &context);

	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
	device->CreateRenderTargetView(BackBuffer, NULL, &targetView);

	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	viewport.Height = SCREEN_HEIGHT;
	viewport.Width = SCREEN_WIDTH;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	const float PI = 3.1415927;
	
	//OUR TRIANGLE
	SIMPLE_VERTEX triArr[3];
	triArr[0].pos = { 0.0f, 0.5f, 0.0f };
	triArr[0].rgba = { 1.0f, 0.0f, 0.0f,1.0f };
	triArr[1].pos = { 0.5f, -0.5f, 0.0f };
	triArr[1].rgba = { 0.0f, 1.0f, 0.0f,1.0f };
	triArr[2].pos = { -0.5f, -0.5f, 0.0f };
	triArr[2].rgba = { 0.0f, 0.0f, 1.0f,1.0f };
	
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 3;
	bufferDesc.MiscFlags = 0;
    
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = triArr;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	
	hr = device->CreateBuffer(&bufferDesc, &InitData, &vertBuffer);
	
	//D3D11_BUFFER_DESC trianglebufferDesc;
	//ZeroMemory(&trianglebufferDesc, sizeof(D3D11_BUFFER_DESC));
	//trianglebufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//trianglebufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//trianglebufferDesc.CPUAccessFlags = NULL; //may need to be D3D11_CPU_ACCESS_WRITE later
	//trianglebufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * NUMTVERTS;
	//trianglebufferDesc.MiscFlags = 0;
	//D3D11_SUBRESOURCE_DATA triangleInitData;
	//ZeroMemory(&triangleInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	//triangleInitData.pSysMem = vertTArr;
	//triangleInitData.SysMemPitch = 0;
	//triangleInitData.SysMemSlicePitch = 0;
	//hr = device->CreateBuffer(&trianglebufferDesc, &triangleInitData, &triangleVertBuffer);
	
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pixShader);
	//TODO: changed to float4 now so we need to work with that as well as adding in uv as third element in array
	D3D11_INPUT_ELEMENT_DESC vLayout[2] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	device->CreateInputLayout(vLayout, 2, Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	D3D11_BUFFER_DESC cbufferDesc;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allows cpu to modify our data at runtime
	cbufferDesc.MiscFlags = 0;
	cbufferDesc.ByteWidth = sizeof(WM_TO_VRAM); //May need to redo this stuff for this buffer?

	D3D11_SUBRESOURCE_DATA cbInitData;
	ZeroMemory(&cbInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbInitData.pSysMem = &WMToShader;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&cbufferDesc, &cbInitData, &constBuffer);

	D3D11_BUFFER_DESC cbufferDesc2;
	ZeroMemory(&cbufferDesc2, sizeof(D3D11_BUFFER_DESC));
	cbufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allows cpu to modify our data at runtime
	cbufferDesc2.MiscFlags = 0;
	cbufferDesc2.ByteWidth = sizeof(VPM_TO_VRAM); //May need to redo this stuff for this buffer?

	D3D11_SUBRESOURCE_DATA cbInitData2;
	ZeroMemory(&cbInitData2, sizeof(D3D11_SUBRESOURCE_DATA));
	cbInitData2.pSysMem = &VPMToShader;
	cbInitData2.SysMemPitch = 0;
	cbInitData2.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&cbufferDesc2, &cbInitData2, &constBuffer2);

	//toShader.constantOffset = { 0.0f, 0.0f };
	WMToShader.constantColor = { 1.0f, 1.0f, 0.0f, 0.0f };
	WMToShader.worldMatrix = XMMatrixIdentity();
	VPMToShader.viewMatrix = XMMatrixIdentity();
	/*toShader.matScale = XMMatrixIdentity();
	toShader.matRotateX = XMMatrixIdentity();
	toShader.matRotateY = XMMatrixIdentity();
	toShader.matRotateZ = XMMatrixIdentity();
	toShader.matTranslate = XMMatrixIdentity();
	toShader.matRotateX = XMMatrixRotationX(XMConvertToRadians(90.0f));
	toShader.matScale = XMMatrixScaling(1.5f, 1.5f, 1.5f);*/
	matrixTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	FXMVECTOR eye = { CAMERAEYEX, CAMERAEYEY, CAMERAEYEZ };
	FXMVECTOR at = { 0.0f, 0.0f, 0.0f };
	FXMVECTOR up = { 0.0f, 1.0f, 0.0f };
	VPMToShader.viewMatrix = XMMatrixLookAtLH(eye, at, up);
	VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), AspectRatio, NearZ, FarZ);
	//toShader.matFinal = toShader.worldMatrix * toShader.viewMatrix * toShader.projMatrix;
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();
	matrixRotateX = XMMatrixIdentity();
	matrixRotateX = XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 20));
	WMToShader.worldMatrix = matrixTranslate * matrixRotateX;

	context->OMSetRenderTargets(1, &targetView, NULL);
	context->RSSetViewports(1, &viewport);
	
	float clearColor[4] = { 0,0,0.7f,0 };
	context->ClearRenderTargetView(targetView, clearColor);
	
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;
	
	D3D11_MAPPED_SUBRESOURCE subResource;
	ZeroMemory(&subResource, sizeof(subResource));
	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);
	
	context->VSSetConstantBuffers(0, 1, &constBuffer);

	D3D11_MAPPED_SUBRESOURCE subResource2;
	ZeroMemory(&subResource2, sizeof(subResource2));
	context->Map(constBuffer2, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource2);
	memcpy(subResource2.pData, &VPMToShader, sizeof(VPM_TO_VRAM));
	context->Unmap(constBuffer2, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);
	context->VSSetConstantBuffers(1, 1, &constBuffer2);
	
	context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	
	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);
	
	context->IASetInputLayout(inputLayout);
	
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //or linestrip
	
	context->Draw(3, 0);
	
	swapChain->Present(0, 0);
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
	constBuffer2->Release();
	pixShader->Release();
	vertShader->Release();
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