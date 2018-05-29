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
#include "WolfOBJ.h"
#include "DDSTextureLoader.h"
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
#define CAMERAEYEY 1.5f
#define CAMERAEYEZ 3
#define LAYOUTSIZE 3
#define FOV 45.0f
#define CAMERASPEED 20.0f
#define ZOOMMIN 10.0f
#define ZOOMMAX 100.0f

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
	ID3D11Buffer *indexBuffer;
	ID3D11InputLayout *inputLayout;
	unsigned int numVerts = NUMVERTS;
	
	ID3D11Buffer *triangleVertBuffer;
	ID3D11InputLayout *inputLayoutTriangle;
	unsigned int numTriangleVerts = NUMTVERTS;
	
	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixShader;
	
	ID3D11Buffer *constBuffer;
	ID3D11Buffer *constBuffer2;

	//textures
	ID3D11DepthStencilView *zBuffer;
	ID3D11Texture2D *depthBuffer;
	ID3D11Texture2D *wolfTexture;
	ID3D11ShaderResourceView *wolfTextureView;

	//camera
	FXMVECTOR eye = { CAMERAEYEX, CAMERAEYEY, CAMERAEYEZ };
	FXMVECTOR at = { 0.0f, 0.0f, 0.0f };
	FXMVECTOR up = { 0.0f, 1.0f, 0.0f };
	XMFLOAT4X4 cameraWM;

	XTime timer;
	HRESULT result;

	XMMATRIX matrixTranslate;
	XMMATRIX matrixRotateX;

	float fovAngleY = FOV;
	float AspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float NearZ = 0.1f;
	float FarZ = 100.0f;
	unsigned int indexCount;
	unsigned int cubeIndices[36] =
	{
		0,1,2,
		1,3,2,
		4,6,5,
		5,6,7,
		0,5,1,
		0,4,5,
		2,7,6,
		2,3,7,
		0,6,4,
		0,2,6,
		1,7,3,
		1,5,7,
	};

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
		XMMATRIX rotMatrix;
		XMFLOAT4 lightVector;
		XMFLOAT4 lightClr;
		XMFLOAT4 ambientClr;
	};

	WM_TO_VRAM WMToShader;
	VPM_TO_VRAM VPMToShader;
	
public:
	
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT2 uvw;
	};
	SIMPLE_VERTEX wolfModel[1981];
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
	for (unsigned int i = 0; i < 1981; i++)
	{
		wolfModel[i].pos.x = WolfOBJ_data[i].pos[0];
		wolfModel[i].pos.y = WolfOBJ_data[i].pos[1];
		wolfModel[i].pos.z = WolfOBJ_data[i].pos[2];
		//uvw
		wolfModel[i].uvw.x = WolfOBJ_data[i].uvw[0];
		wolfModel[i].uvw.y = WolfOBJ_data[i].uvw[1];
		//wolfModel[i].uvw.z = WolfOBJ_data[i].uvw[2];
		//norms
		wolfModel[i].norm.x = WolfOBJ_data[i].nrm[0];
		wolfModel[i].norm.y = WolfOBJ_data[i].nrm[1];
		wolfModel[i].norm.z = WolfOBJ_data[i].nrm[2];
	}


	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 4; //no aa
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
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flag, NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, NULL, &context);

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

	static const SIMPLE_VERTEX floorVerts[] =
	{
	{ XMFLOAT3(-0.5f, -0.5f, -0.5f) },
	{ XMFLOAT3(-0.5f, -0.5f,  0.5f) },
	{ XMFLOAT3(-0.5f,  0.5f, -0.5f) },
	{ XMFLOAT3(-0.5f,  0.5f,  0.5f) },
	{ XMFLOAT3(0.5f, -0.5f, -0.5f)},
	{ XMFLOAT3(0.5f, -0.5f,  0.5f)},
	{ XMFLOAT3(0.5f,  0.5f, -0.5f)},
	{ XMFLOAT3(0.5f,  0.5f,  0.5f)},
	};
	
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 1981;
	bufferDesc.MiscFlags = 0;
    
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = /*cubeVerts*/ wolfModel;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	
	hr = device->CreateBuffer(&bufferDesc, &InitData, &vertBuffer);
	
	D3D11_BUFFER_DESC ibufferDesc;
	ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufferDesc.ByteWidth = sizeof(unsigned int) * 6114;
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufferDesc.MiscFlags = 0;

	device->CreateBuffer(&ibufferDesc, NULL, &indexBuffer);

	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pixShader);
	//TODO: changed to float4 now so we need to work with that as well as adding in uv as third element in array
	D3D11_INPUT_ELEMENT_DESC vLayout[LAYOUTSIZE] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = device->CreateInputLayout(vLayout, LAYOUTSIZE, Trivial_VS, sizeof(Trivial_VS), &inputLayout);

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

	//XMStoreFloat4x4(&VPMToShader.viewMatrix, XMMatrixIdentity());
	/*toShader.matScale = XMMatrixIdentity();
	toShader.matRotateX = XMMatrixIdentity();
	toShader.matRotateY = XMMatrixIdentity();
	toShader.matRotateZ = XMMatrixIdentity();
	toShader.matTranslate = XMMatrixIdentity();
	toShader.matRotateX = XMMatrixRotationX(XMConvertToRadians(90.0f));
	toShader.matScale = XMMatrixScaling(1.5f, 1.5f, 1.5f);*/
	matrixTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	XMMATRIX id = XMMatrixIdentity();
	XMStoreFloat4x4(&cameraWM, id);

	//VPMToShader.viewMatrix = XMMatrixLookAtLH(eye, at, up);
	VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), AspectRatio, NearZ, FarZ);
	//toShader.matFinal = toShader.worldMatrix * toShader.viewMatrix * toShader.projMatrix;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = SCREEN_WIDTH;
	texDesc.Height = SCREEN_HEIGHT;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 4;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	device->CreateTexture2D(&texDesc, NULL, &depthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	result = device->CreateDepthStencilView(depthBuffer, &dsvDesc, &zBuffer);

	//DDS Loader
	result = CreateDDSTextureFromFile(device, L"alphaBlackB.dds", nullptr, &wolfTextureView);

	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();
	//do whatever we need to do with the view matrix 
	matrixRotateX = XMMatrixIdentity();
	//matrixRotateX = XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 20));
	WMToShader.worldMatrix = matrixTranslate * matrixRotateX;
	VPMToShader.rotMatrix = matrixRotateX;
	VPMToShader.lightVector = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
	VPMToShader.lightClr = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	VPMToShader.ambientClr = XMFLOAT4(0.2, 0.2f, 0.2f, 1.0f);

	if (GetAsyncKeyState(0x57) & 0x8000)//w
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, CAMERASPEED * timer.SmoothDelta());
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM,result);
	}

	if (GetAsyncKeyState(0x53) & 0x8000)//s
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, -CAMERASPEED * timer.SmoothDelta());
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(0x41) & 0x8000)//a
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(-CAMERASPEED * timer.SmoothDelta(), 0.0f, 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(0x44) & 0x8000)//d
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(CAMERASPEED * timer.SmoothDelta(), 0.0f, 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(0x58) & 0x8000)//x
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, -CAMERASPEED * timer.SmoothDelta(), 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)//space
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, CAMERASPEED * timer.SmoothDelta(), 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		//cameraWM = XMMatrixMultiply(translate, cameraWM);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(0x45) & 0x8000) //e
	{
		//rotate clockwise
		XMFLOAT4 pos = XMFLOAT4(cameraWM._41,cameraWM._42, cameraWM._43, cameraWM._44);
		cameraWM._41 = 0;
		cameraWM._42 = 0;
		cameraWM._43 = 0;

		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(timer.SmoothDelta() * CAMERASPEED));
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		tempCam = XMMatrixMultiply(rotate, tempCam);

		XMStoreFloat4x4(&cameraWM, tempCam);

		cameraWM._41 = pos.x;
		cameraWM._42 = pos.y;
		cameraWM._43 = pos.z;

	}

	if (GetAsyncKeyState(0x51) & 0x8000) //q
	{
		//rotate counterclockwise
		XMFLOAT4 pos = XMFLOAT4(cameraWM._41, cameraWM._42, cameraWM._43, cameraWM._44);
		cameraWM._41 = 0;
		cameraWM._42 = 0;
		cameraWM._43 = 0;

		XMMATRIX rotate = XMMatrixRotationY(-XMConvertToRadians(timer.SmoothDelta() * CAMERASPEED));
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		tempCam = XMMatrixMultiply(rotate, tempCam);

		XMStoreFloat4x4(&cameraWM, tempCam);

		cameraWM._41 = pos.x;
		cameraWM._42 = pos.y;
		cameraWM._43 = pos.z;
	}

	if (GetAsyncKeyState(VK_ADD) & 0x8000) //+ sign
	{
		//zoom in (FOV goes down)
		if (!fovAngleY >= ZOOMMAX)
		{
			fovAngleY += 1.0f;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), AspectRatio, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) //- sign
	{
		//zoom out

	}

	XMMATRIX camera = XMLoadFloat4x4(&cameraWM);

	//now inverse the view matrix and store in the VPMToShader.viewMatrix
	VPMToShader.viewMatrix = XMMatrixInverse(&XMMatrixDeterminant(camera), camera);
	//XMStoreFloat4x4(&VPMToShader.viewMatrix, XMMatrixInverse(,VPMToShader.viewMatrix))
	context->OMSetRenderTargets(1, &targetView, zBuffer);
	context->RSSetViewports(1, &viewport);
	
	float clearColor[4] = { 0.26f,0.53f,0.96f,0.0f };
	context->ClearRenderTargetView(targetView, clearColor);
	context->ClearDepthStencilView(zBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);
	memcpy(mappedResource.pData, WolfOBJ_indicies, sizeof(WolfOBJ_indicies));
	context->Unmap(indexBuffer, NULL);
	
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
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);
	
	context->IASetInputLayout(inputLayout);
	
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //or linestrip
	
	ID3D11ShaderResourceView* texViews[] = { wolfTextureView };
	context->PSSetShaderResources(0, 1, texViews);

	context->DrawIndexed(6114,0, 0);
	
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
	indexBuffer->Release();
	pixShader->Release();
	vertShader->Release();
	depthBuffer->Release();
	zBuffer->Release();
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