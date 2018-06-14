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
#include "SKYMAP_VS.csh"
#include "SKYMAP_PS.csh"
#include "MAlienPlanet.h"
#include "aircraft.h"
#include "MAlienP2.h"
#include "Moon.h"
#include "DDSTextureLoader.h"
#include <Windows.h>
#include <windowsx.h>
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
//#include "D3DX11asyc.h"

using namespace std;
using namespace DirectX;

#define SCREEN_WIDTH	1600.0f
#define SCREEN_HEIGHT	900.0f
#define _DEBUG 0
#define NUMVERTS 369
#define NUMTVERTS 2400
#define CAMERAEYEX 0
#define CAMERAEYEY 1.5f
#define CAMERAEYEZ 3
#define LAYOUTSIZE 3
#define FOV 90.0f
#define CAMERASPEED 25.0f
#define ZOOMMIN 10.0f
#define ZOOMMAX 100.0f
#define NEARMIN 0.1f
#define NEARMAX 100.0f
#define FARMIN 1001.0f
#define FARMAX 1500.0f
#define NEARFARMOD 0.025f
#define APARRAYSIZE 1841
#define APINDEXSIZE 7800
#define ASPECTRATIO SCREEN_WIDTH/SCREEN_HEIGHT

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
	
	//buffers
	ID3D11Texture2D *BackBuffer;
	ID3D11Buffer *vertBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11Buffer *constBuffer;
	ID3D11Buffer *constBuffer2;
	ID3D11Buffer *triangleVertBuffer;
	ID3D11Buffer *cubeIndexBuffer;
	ID3D11Buffer *cubeVertBuffer;

	//input layouts
	ID3D11InputLayout *inputLayout;
	ID3D11InputLayout *inputLayout2;
	ID3D11InputLayout *inputLayoutTriangle;
	unsigned int numVerts = NUMVERTS;
	unsigned int numTriangleVerts = NUMTVERTS;
	
	//shader vars
	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixShader;
	ID3D11VertexShader *cubeMap_VS;
	ID3D11PixelShader *cubeMap_PS;
	ID3D11Buffer* perFrameBuffer;
	//ID3D11PixelShader *pl_PS;
	
	//textures
	ID3D11DepthStencilView *zBuffer;
	ID3D11DepthStencilState *DSLessEqual;
	ID3D11RasterizerState *ccwCullMode;
	ID3D11RasterizerState *cwCullMode;
	ID3D11RasterizerState *RSCullNone;
	ID3D11Texture2D *depthBuffer;
	ID3D11Texture2D *environmentTexture;
	ID3D11Texture2D *alienPlanetTexture;
	ID3D11Texture2D *alienPlanet2Texture;
	ID3D11Texture2D *sunTexture;
	ID3D11Texture2D *aircraftTexture;

	//SRV's
	ID3D11ShaderResourceView *environmentView;
	ID3D11ShaderResourceView *alienPlanetTextureView;
	ID3D11ShaderResourceView *alienPlanet2TextureView;
	ID3D11ShaderResourceView *sunTextureView;
	ID3D11ShaderResourceView *aircraftTextureView;

	//camera
	FXMVECTOR eye = { CAMERAEYEX, CAMERAEYEY, CAMERAEYEZ };
	FXMVECTOR at = { 0.0f, 0.0f, 0.0f };
	FXMVECTOR up = { 0.0f, 1.0f, 0.0f };
	XMFLOAT4X4 cameraWM;

	//timer & debug
	XTime timer;
	HRESULT result;

	//Object matricies
	XMMATRIX matrixTranslate;
	XMMATRIX matrixRotateX;
	XMMATRIX matrixScaling;
	XMFLOAT4X4 cubeWorld; //skymap world matrix
	XMMATRIX planetWorld = XMMatrixIdentity();
	XMMATRIX worldMat = XMMatrixIdentity();
	//structs

	struct WM_TO_VRAM
	{
		XMMATRIX worldMatrix;
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

	//vars
	WM_TO_VRAM WMToShader;
	WM_TO_VRAM cubeWMToShader;
	VPM_TO_VRAM VPMToShader;
	float fovAngleY = FOV;
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	float aspectRatio = width / height;
	RECT window_size;
	float NearZ = NEARMIN;
	float FarZ = FARMAX;
	unsigned int indexCount;
	unsigned int cubeIndices[36] =
	{
		//front
		0,1,2,
		0,2,3,
		//back
		4,5,6,
		4,6,7,
		//top
		8,9,10,
		8,10,11,
		//bottom
		12,13,14,
		12,14,15,
		//left
		16,17,18,
		16,18,19,
		//right
		20,21,22,
		20,22,23
	};


public:
	
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT2 uv;
	};
	SIMPLE_VERTEX cubeVerts[24] =
	{ 
		//front
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{{1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{{1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},

		//back          
		{{1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{ { 1.0f,  1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },

		//top 
		{ { -1.0f, 1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, 1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },

		//bottom
		{ { -1.0f, -1.0f,  -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },

		//left 
		{ { -1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },

		//right
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }
	};

	struct Light
	{
		Light()
		{
			ZeroMemory(this, sizeof(Light));
		}
		XMFLOAT3 direction;
		float padding1;
		XMFLOAT3 position;
		float range;
		XMFLOAT3 attenuation;
		float padding2;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
	};

	struct perFrame
	{
		Light light;
	};

	perFrame constBufferPerFrame;

	//lights
	Light pointLight;

	SIMPLE_VERTEX alienPlanetModel[APARRAYSIZE];
	SIMPLE_VERTEX alienPlanet2[4997];
	SIMPLE_VERTEX sun[83952];
	SIMPLE_VERTEX aircraft[33192];
	bool reverseX = false;
	bool reverseY = false;
	bool fullscreen = false;
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
	//wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	window_size = { 0, 0, (long)SCREEN_WIDTH, (long)SCREEN_HEIGHT};
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//
	for (unsigned int i = 0; i < APARRAYSIZE; i++)
	{
		alienPlanetModel[i].pos.x = MAlienPlanet_data[i].pos[0];
		alienPlanetModel[i].pos.y = MAlienPlanet_data[i].pos[1];
		alienPlanetModel[i].pos.z = MAlienPlanet_data[i].pos[2];
		//uvw
		alienPlanetModel[i].uv.x = MAlienPlanet_data[i].uvw[0];
		alienPlanetModel[i].uv.y = MAlienPlanet_data[i].uvw[1];
		//wolfModel[i].uvw.z = WolfOBJ_data[i].uvw[2];
		//norms
		alienPlanetModel[i].norm.x = MAlienPlanet_data[i].nrm[0];
		alienPlanetModel[i].norm.y = MAlienPlanet_data[i].nrm[1];
		alienPlanetModel[i].norm.z = MAlienPlanet_data[i].nrm[2];
	}

	pointLight.position = XMFLOAT3(0.0f,0.0f,0.0f);
	pointLight.range = 200.0f;
	pointLight.attenuation = XMFLOAT3(0.0f, 0.2f, 0.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 4; 
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
	
	//planet
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * APARRAYSIZE;
	bufferDesc.MiscFlags = 0;
    
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = alienPlanetModel;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	
	hr = device->CreateBuffer(&bufferDesc, &InitData, &vertBuffer);
	

	D3D11_BUFFER_DESC ibufferDesc;
	ZeroMemory(&ibufferDesc, sizeof(D3D11_BUFFER_DESC));
	ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufferDesc.ByteWidth = sizeof(unsigned int) * APINDEXSIZE;
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufferDesc.MiscFlags = 0;

	device->CreateBuffer(&ibufferDesc, NULL, &indexBuffer);

	//cube
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 24;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA bufferData;
	ZeroMemory(&bufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferData.pSysMem = cubeVerts;
	bufferData.SysMemPitch = 0;
	bufferData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &bufferData, &cubeVertBuffer);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * 36;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	indexBufferDesc.MiscFlags = 0;

	device->CreateBuffer(&indexBufferDesc, NULL, &cubeIndexBuffer);

	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(perFrame);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	//shaders
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pixShader);
	device->CreateVertexShader(SKYMAP_VS, sizeof(SKYMAP_VS), NULL, &cubeMap_VS);
	device->CreatePixelShader(SKYMAP_PS, sizeof(SKYMAP_PS), NULL, &cubeMap_PS);
	//device->CreatePixelShader(PL_PS, sizeof(PL_PS), NULL, &pl_PS);
	

	//TODO: changed to float4 now so we need to work with that as well as adding in uv as third element in array
	D3D11_INPUT_ELEMENT_DESC vLayout[LAYOUTSIZE] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = device->CreateInputLayout(vLayout, LAYOUTSIZE, Trivial_VS, sizeof(Trivial_VS), &inputLayout);
	hr = device->CreateInputLayout(vLayout, LAYOUTSIZE, SKYMAP_VS, sizeof(SKYMAP_VS), &inputLayout2);

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

	WMToShader.worldMatrix = XMMatrixIdentity();
	matrixTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	XMMATRIX id = XMMatrixIdentity();
	XMStoreFloat4x4(&cameraWM, id);
	XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
	XMMATRIX translate = XMMatrixTranslation(0.0f, 1.0f, -10.0f);
	XMMATRIX result = XMMatrixMultiply(translate, tempCam);
	XMStoreFloat4x4(&cameraWM, result);
	XMStoreFloat4x4(&cubeWorld, id);

	VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);

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

	device->CreateDepthStencilView(depthBuffer, &dsvDesc, &zBuffer);

	D3D11_RASTERIZER_DESC rasDSC;
	ZeroMemory(&rasDSC, sizeof(D3D11_RASTERIZER_DESC));
	rasDSC.FillMode = D3D11_FILL_SOLID;
	rasDSC.CullMode = D3D11_CULL_BACK;
	rasDSC.FrontCounterClockwise = true;
	device->CreateRasterizerState(&rasDSC, &ccwCullMode);

	rasDSC.FrontCounterClockwise = false;

	device->CreateRasterizerState(&rasDSC, &cwCullMode);

	rasDSC.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rasDSC, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, &DSLessEqual);

	//DDS Loader
	CreateDDSTextureFromFile(device, L"alienplanet_tex.dds", nullptr, &alienPlanetTextureView);
	CreateDDSTextureFromFile(device, L"planet2.dds", nullptr, &alienPlanet2TextureView);
	CreateDDSTextureFromFile(device, L"sun.dds", nullptr, &sunTextureView);
	CreateDDSTextureFromFile(device, L"aircraft_tex.dds", nullptr, &aircraftTextureView);
	CreateDDSTextureFromFile(device, L"CubeMap.dds", (ID3D11Resource**)&environmentTexture, &environmentView);

	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();

	//if (GetAsyncKeyState(VK_MENU) & 0x8000 && GetAsyncKeyState(VK_RETURN) & 0x8000)
	//{
	//	if (!fullscreen) //windowed -> fullscreen
	//	{
	//		fullscreen = true;
	//		swapChain->SetFullscreenState(true, NULL);
	//		context->OMSetRenderTargets(0, 0, 0);
	//		targetView->Release();

	//	}
	//	else //fullscreen -> windowed
	//	{
	//		fullscreen = false;
	//		swapChain->SetFullscreenState(false, NULL);
	//	}
	//}

	matrixRotateX = XMMatrixIdentity();
	matrixRotateX = XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 20));

	//planetWorld = XMMatrixScaling(0.5f, 1.0f, 1.0f);
	planetWorld = matrixTranslate * matrixRotateX;
	WMToShader.worldMatrix = planetWorld;
	VPMToShader.rotMatrix = matrixRotateX;
	VPMToShader.lightVector = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
	VPMToShader.lightClr = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	VPMToShader.ambientClr = XMFLOAT4(0.2, 0.2f, 0.2f, 1.0f);

	XMVECTOR pointLightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	pointLightVec = XMVector3TransformCoord(pointLightVec, planetWorld);
	pointLight.position.x = XMVectorGetX(pointLightVec);
	pointLight.position.y = XMVectorGetX(pointLightVec);
	pointLight.position.z = XMVectorGetX(pointLightVec);

	if (GetAsyncKeyState('W') & 0x8000)//w
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, CAMERASPEED * timer.SmoothDelta());
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM,result);
	}

	if (GetAsyncKeyState('S') & 0x8000)//s
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, -CAMERASPEED * timer.SmoothDelta());
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState('A') & 0x8000)//a
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(-CAMERASPEED * timer.SmoothDelta(), 0.0f, 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState('D') & 0x8000)//d
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(CAMERASPEED * timer.SmoothDelta(), 0.0f, 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState('X') & 0x8000)//x
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, -CAMERASPEED * timer.SmoothDelta(), 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)//space
	{
		XMMATRIX tempCam = XMLoadFloat4x4(&cameraWM);
		XMMATRIX translate = XMMatrixTranslation(0.0f, CAMERASPEED * timer.SmoothDelta(), 0.0f);
		XMMATRIX result = XMMatrixMultiply(translate, tempCam);
		XMStoreFloat4x4(&cameraWM, result);
	}

	if (GetAsyncKeyState('E') & 0x8000) //e
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

	if (GetAsyncKeyState('Q') & 0x8000) //q
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

	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) //zoom in (FOV goes down)
	{
		
		if (fovAngleY < ZOOMMAX)
		{
			fovAngleY += 0.05f;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState(VK_ADD) & 0x8000) //zoom out
	{
		
		if (fovAngleY > ZOOMMIN)
		{
			fovAngleY -= 0.05f;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState('U') & 0x8000) //near -
	{
		if (NearZ - NEARFARMOD >= NEARMIN)
		{
			NearZ -= NEARFARMOD;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState('I') & 0x8000) //near +
	{
		if (NearZ + NEARFARMOD <= NEARMAX)
		{
			NearZ += NEARFARMOD;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState('J') & 0x8000) //far -
	{
		if (FarZ - NEARFARMOD >= FARMIN)
		{
			FarZ -= NEARFARMOD;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	if (GetAsyncKeyState('K') & 0x8000) //far +
	{
		if (FarZ + NEARFARMOD <= FARMAX)
		{
			FarZ += NEARFARMOD;
			VPMToShader.projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), ASPECTRATIO, NearZ, FarZ);
		}
	}

	XMFLOAT4 cameraPos = { cameraWM._41, cameraWM._42, cameraWM._43, cameraWM._44 };
	cubeWorld._41 = cameraPos.x;
	cubeWorld._42 = cameraPos.y;
	cubeWorld._43 = cameraPos.z;
	cubeWorld._44 = cameraPos.w;
	XMMATRIX cube = XMLoadFloat4x4(&cubeWorld);
	XMMATRIX camera = XMLoadFloat4x4(&cameraWM);

	//store the 4th row of the camera now and save an identity matrix to it
	//set the cube WM to this new matrix

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

	////point light mapping
	//constBufferPerFrame.light = pointLight;
	//context->UpdateSubresource(perFrameBuffer, 0, NULL, &constBufferPerFrame, 0, 0);
	////D3D11_MAPPED_SUBRESOURCE mappedResource;
	////ZeroMemory(&mappedResource, sizeof(mappedResource));
	////context->Map(perFrameBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);
	////memcpy(mappedResource.pData, &constBufferPerFrame, sizeof(perFrame));
	////context->Unmap(perFrameBuffer, NULL);
	//context->PSSetConstantBuffers(0, 1, &perFrameBuffer);
	//context->VSSetShader(vertShader, 0, 0);
	//context->PSSetShader(pl_PS, 0, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(mappedResource));
	context->Map(indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);
	memcpy(mappedResource.pData, MAlienPlanet_indicies, sizeof(MAlienPlanet_indicies));
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

	
	context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);
	
	context->IASetInputLayout(inputLayout);
	
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //or linestrip
	
	ID3D11ShaderResourceView* texViews[] = { alienPlanetTextureView, environmentView };
	context->PSSetShaderResources(0, 2, texViews);

	context->DrawIndexed(APINDEXSIZE,0, 0);

	//cube WM math
	WMToShader.worldMatrix = XMMatrixIdentity();
	matrixScaling = XMMatrixScaling(1000.0f, 1000.0f, 1000.0f);
	cube =  matrixScaling;
	WMToShader.worldMatrix = cube;

	//update cuberesource
	D3D11_MAPPED_SUBRESOURCE cubeResource;
	ZeroMemory(&cubeResource, sizeof(cubeResource));
	context->Map(cubeIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cubeResource);
	memcpy(cubeResource.pData, cubeIndices, sizeof(cubeIndices));
	context->Unmap(cubeIndexBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);
	context->VSSetConstantBuffers(1, 1, &constBuffer2);
	context->IASetVertexBuffers(0, 1, &cubeVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(cubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->IASetInputLayout(inputLayout2);
	//cube topology
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
/*
	ID3D11ShaderResourceView* texViews2[] = { environmentView };
	context->PSSetShaderResources(0, 1, texViews2)*/;

	//update constant buffer for cube
	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetShader(cubeMap_VS, NULL, 0);
	context->PSSetShader(cubeMap_PS, NULL, 0);
	context->OMSetDepthStencilState(DSLessEqual, 0);
	context->RSSetState(RSCullNone);
	context->DrawIndexed(36, 0, 0);
	context->OMSetDepthStencilState(NULL, 0);
	context->ClearDepthStencilView(zBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

	swapChain->Present(0, 0);
	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6

	swapChain->SetFullscreenState(FALSE, NULL);

	device->Release();
	swapChain->Release();
	targetView->Release();
	context->Release();

	BackBuffer->Release();
	vertBuffer->Release();
	constBuffer->Release();
	constBuffer2->Release();
	indexBuffer->Release();
	depthBuffer->Release();
	zBuffer->Release();
	cubeIndexBuffer->Release();
	cubeVertBuffer->Release();
	perFrameBuffer->Release();

	cubeMap_VS->Release();
	cubeMap_PS->Release();
	pixShader->Release();
	vertShader->Release();

	//skymapRV->Release();
	inputLayout->Release();
	inputLayout2->Release();

	DSLessEqual->Release();
	RSCullNone->Release();

	/*alienPlanetTexture->Release();*/
	alienPlanetTextureView->Release();
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