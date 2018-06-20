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
#include "AlienPlanet2.h"
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
#define FOV 60.0f
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
#define ASPECTRATIO SCREEN_WIDTH/(SCREEN_HEIGHT * 0.5f)

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
	D3D11_VIEWPORT viewport2;
	
	//buffers
	ID3D11Texture2D *BackBuffer;
	ID3D11Buffer *vertBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11Buffer *constBuffer;
	ID3D11Buffer *constBuffer2;
	ID3D11Buffer *cbPerFrame;
	ID3D11Buffer *triangleVertBuffer;
	ID3D11Buffer *cubeIndexBuffer;
	ID3D11Buffer *cubeVertBuffer;
	ID3D11Buffer *planet2IndexBuffer;
	ID3D11Buffer *planet2VertBuffer;
	ID3D11Buffer *sunIndexBuffer;
	ID3D11Buffer *sunVertBuffer;
	ID3D11Buffer *aircraftIndexBuffer;
	ID3D11Buffer *aircraftVertBuffer;

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
	XMFLOAT4X4 aircraft4X4;
	XMMATRIX planetWorld = XMMatrixIdentity();
	XMMATRIX planet2World = XMMatrixIdentity();
	XMMATRIX sunWorld = XMMatrixIdentity();
	XMMATRIX aircraftWorld = XMMatrixIdentity();
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
	float height = SCREEN_HEIGHT / 2;
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
		XMFLOAT4 ambient2;
		XMFLOAT4 diffuse2;
		float padding3;
		XMFLOAT3 position2;
	};

	struct perFrame
	{
		Light light;
	};

	perFrame constBufferPerFrame;

	//lights
	/*Light pointLight;
	Light dirLight;*/

	SIMPLE_VERTEX alienPlanetModel[APARRAYSIZE];
	SIMPLE_VERTEX *alienPlanet2 = new SIMPLE_VERTEX[10962];
	SIMPLE_VERTEX *sun = new SIMPLE_VERTEX[10962];
	SIMPLE_VERTEX *aircraft = new SIMPLE_VERTEX[23089];
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
	//alien planet 
	for (unsigned int i = 0; i < APARRAYSIZE; i++)
	{
		alienPlanetModel[i].pos.x = MAlienPlanet_data[i].pos[0];
		alienPlanetModel[i].pos.y = MAlienPlanet_data[i].pos[1];
		alienPlanetModel[i].pos.z = MAlienPlanet_data[i].pos[2];
		//uvw
		alienPlanetModel[i].uv.x = MAlienPlanet_data[i].uvw[0];
		alienPlanetModel[i].uv.y = MAlienPlanet_data[i].uvw[1];
		//norms
		alienPlanetModel[i].norm.x = MAlienPlanet_data[i].nrm[0];
		alienPlanetModel[i].norm.y = MAlienPlanet_data[i].nrm[1];
		alienPlanetModel[i].norm.z = MAlienPlanet_data[i].nrm[2];
	}

	//alien planet 2
	for (unsigned int i = 0; i < 10962; i++)
	{
		alienPlanet2[i].pos.x  =AlienPlanet2_data[i].pos[0];
		alienPlanet2[i].pos.y  =AlienPlanet2_data[i].pos[1];
		alienPlanet2[i].pos.z  =AlienPlanet2_data[i].pos[2];
		//uvw
		alienPlanet2[i].uv.x   =AlienPlanet2_data[i].uvw[0];
		alienPlanet2[i].uv.y   =AlienPlanet2_data[i].uvw[1];
		//norms
		alienPlanet2[i].norm.x = AlienPlanet2_data[i].nrm[0];
		alienPlanet2[i].norm.y = AlienPlanet2_data[i].nrm[1];
		alienPlanet2[i].norm.z = AlienPlanet2_data[i].nrm[2];
	}

	//sun
	for (unsigned int i = 0; i < 10962; i++)
	{
		sun[i].pos.x = AlienPlanet2_data[i].pos[0];
		sun[i].pos.y = AlienPlanet2_data[i].pos[1];
		sun[i].pos.z = AlienPlanet2_data[i].pos[2];
		//uvw
		sun[i].uv.x = AlienPlanet2_data[i].uvw[0];
		sun[i].uv.y = AlienPlanet2_data[i].uvw[1];
		//norms
		sun[i].norm.x = AlienPlanet2_data[i].nrm[0];
		sun[i].norm.y = AlienPlanet2_data[i].nrm[1];
		sun[i].norm.z = AlienPlanet2_data[i].nrm[2];
	}

	//spaceship
	for (unsigned int i = 0; i < 23089; i++)
	{
		aircraft[i].pos.x = aircraft_data[i].pos[0];
		aircraft[i].pos.y = aircraft_data[i].pos[1];
		aircraft[i].pos.z = aircraft_data[i].pos[2];
		//uvw
		aircraft[i].uv.x = aircraft_data[i].uvw[0];
		aircraft[i].uv.y = aircraft_data[i].uvw[1];
		//norms
		aircraft[i].norm.x = aircraft_data[i].nrm[0];
		aircraft[i].norm.y = aircraft_data[i].nrm[1];
		aircraft[i].norm.z = aircraft_data[i].nrm[2];
	}

	constBufferPerFrame.light.position = XMFLOAT3(0.0f,0.0f,0.0f);
	constBufferPerFrame.light.range = 200.0f;
	constBufferPerFrame.light.attenuation = XMFLOAT3(0.0f, 0.2f, 0.0f);
	constBufferPerFrame.light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	constBufferPerFrame.light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	constBufferPerFrame.light.position2 = XMFLOAT3(0.0f, 0.0f, 30.0f);
	constBufferPerFrame.light.direction = XMFLOAT3(0.25f, 1.0f, -1.0f);
	constBufferPerFrame.light.ambient2 = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	constBufferPerFrame.light.diffuse2 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

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
	viewport.Height = SCREEN_HEIGHT/2;
	viewport.Width = SCREEN_WIDTH;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	ZeroMemory(&viewport2, sizeof(D3D11_VIEWPORT));
	viewport2.MaxDepth = 1;
	viewport2.MinDepth = 0;
	viewport2.Height = SCREEN_HEIGHT / 2;
	viewport2.Width = SCREEN_WIDTH;
	viewport2.TopLeftX = 0;
	viewport2.TopLeftY = 450;

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

	//planet 2
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 10962;
	bufferDesc.MiscFlags = 0;

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = alienPlanet2;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&bufferDesc, &InitData, &planet2VertBuffer);


	ZeroMemory(&ibufferDesc, sizeof(D3D11_BUFFER_DESC));
	ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufferDesc.ByteWidth = sizeof(unsigned int) * 26232;
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufferDesc.MiscFlags = 0;

	device->CreateBuffer(&ibufferDesc, NULL, &planet2IndexBuffer);

	//sun
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 10962;
	bufferDesc.MiscFlags = 0;

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = sun;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&bufferDesc, &InitData, &sunVertBuffer);


	ZeroMemory(&ibufferDesc, sizeof(D3D11_BUFFER_DESC));
	ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufferDesc.ByteWidth = sizeof(unsigned int) * 26232;
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufferDesc.MiscFlags = 0;

	device->CreateBuffer(&ibufferDesc, NULL, &sunIndexBuffer);

	//spaceship
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 23089;
	bufferDesc.MiscFlags = 0;

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = aircraft;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&bufferDesc, &InitData, &aircraftVertBuffer);


	ZeroMemory(&ibufferDesc, sizeof(D3D11_BUFFER_DESC));
	ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufferDesc.ByteWidth = sizeof(unsigned int) * 50316;
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufferDesc.MiscFlags = 0;

	device->CreateBuffer(&ibufferDesc, NULL, &aircraftIndexBuffer);

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

	/*ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(perFrame);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;*/

	//shaders
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pixShader);
	device->CreateVertexShader(SKYMAP_VS, sizeof(SKYMAP_VS), NULL, &cubeMap_VS);
	device->CreatePixelShader(SKYMAP_PS, sizeof(SKYMAP_PS), NULL, &cubeMap_PS);
	

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

	//pixel shader
	//point light
	D3D11_BUFFER_DESC cbufferDesc3;
	ZeroMemory(&cbufferDesc3, sizeof(D3D11_BUFFER_DESC));
	cbufferDesc3.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDesc3.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allows cpu to modify our data at runtime
	cbufferDesc3.MiscFlags = 0;
	cbufferDesc3.ByteWidth = sizeof(perFrame); //May need to redo this stuff for this buffer?

	D3D11_SUBRESOURCE_DATA cbInitData3;
	ZeroMemory(&cbInitData3, sizeof(D3D11_SUBRESOURCE_DATA));
	cbInitData3.pSysMem = &constBufferPerFrame;
	cbInitData3.SysMemPitch = 0;
	cbInitData3.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&cbufferDesc3, &cbInitData3, &cbPerFrame);

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
	CreateDDSTextureFromFile(device, L"alienplanet_tex.dds", (ID3D11Resource**)&alienPlanetTexture, &alienPlanetTextureView);
	CreateDDSTextureFromFile(device, L"planet2.dds", (ID3D11Resource**)&alienPlanet2Texture, &alienPlanet2TextureView);
	CreateDDSTextureFromFile(device, L"sun.dds", (ID3D11Resource**)&sunTexture, &sunTextureView);
	CreateDDSTextureFromFile(device, L"aircraft_tex.dds", (ID3D11Resource**)&aircraftTexture, &aircraftTextureView);
	CreateDDSTextureFromFile(device, L"CubeMap.dds", (ID3D11Resource**)&environmentTexture, &environmentView);

	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();

	matrixRotateX = XMMatrixIdentity();
	matrixRotateX = XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 5));

	planetWorld = XMMatrixScaling(10.0f, 10.0f, 10.0f);
	planetWorld = matrixTranslate * matrixRotateX;
	WMToShader.worldMatrix = planetWorld;
	VPMToShader.rotMatrix = matrixRotateX;
	VPMToShader.lightVector = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
	VPMToShader.lightClr = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	VPMToShader.ambientClr = XMFLOAT4(0.2, 0.2f, 0.2f, 1.0f);

	//point light follow sun
	XMVECTOR pointLightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	pointLightVec = XMVector3TransformCoord(pointLightVec, sunWorld);
	constBufferPerFrame.light.position.x = XMVectorGetX(pointLightVec);
	constBufferPerFrame.light.position.y = XMVectorGetY(pointLightVec);
	constBufferPerFrame.light.position.z = XMVectorGetZ(pointLightVec);

		//directional light follow moon
	XMVECTOR dirLightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	dirLightVec = XMVector3TransformCoord(dirLightVec, planet2World);
	constBufferPerFrame.light.position2.x = XMVectorGetX(dirLightVec);
	constBufferPerFrame.light.position2.y = XMVectorGetY(dirLightVec);
	constBufferPerFrame.light.position2.z = XMVectorGetZ(dirLightVec);


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
	aircraft4X4._41 = cameraPos.x;
	aircraft4X4._42 = cameraPos.y;
	aircraft4X4._43 = cameraPos.z;
	aircraft4X4._44 = cameraPos.w;
	/*aircraftWorld = XMLoadFloat4x4(&aircraft4X4);
	aircraftWorld*/
	XMMATRIX cube = XMLoadFloat4x4(&cubeWorld);
	XMMATRIX camera = XMLoadFloat4x4(&cameraWM);
	//XMMATRIX preInverseCam = XMLoadFloat4x4(&cameraWM);

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


	//alien planet 1 
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
	
	//map lighting
	D3D11_MAPPED_SUBRESOURCE subResource3;
	ZeroMemory(&subResource3, sizeof(subResource3));
	context->Map(cbPerFrame, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource3);
	memcpy(subResource3.pData, &constBufferPerFrame, sizeof(perFrame));
	context->Unmap(cbPerFrame, NULL);

	context->PSSetConstantBuffers(0, 1, &cbPerFrame);

	context->IASetInputLayout(inputLayout);
	
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //or linestrip
	
	ID3D11ShaderResourceView* planet1TexViews[] = { alienPlanetTextureView, environmentView };
	context->PSSetShaderResources(0, 2, planet1TexViews);

	context->DrawIndexed(APINDEXSIZE,0, 0);

	//planet 2
	WMToShader.worldMatrix = XMMatrixIdentity();
	planet2World = XMMatrixTranslation(20.0f, 5.0f, 20.0f);
	planet2World = planet2World * XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 0.5f)), planetWorld);
	matrixScaling = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	planet2World = planet2World * matrixScaling;
	WMToShader.worldMatrix = planet2World;

	D3D11_MAPPED_SUBRESOURCE planet2Resource;
	ZeroMemory(&planet2Resource, sizeof(planet2Resource));
	context->Map(planet2IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &planet2Resource);
	memcpy(planet2Resource.pData, AlienPlanet2_indicies, sizeof(AlienPlanet2_indicies));
	context->Unmap(planet2IndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &planet2VertBuffer, &stride, &offset);
	context->IASetIndexBuffer(planet2IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* planet2TexView[] = { alienPlanet2TextureView };
	context->PSSetShaderResources(0, 1, planet2TexView);

	context->DrawIndexed(26232, 0, 0);

	//sun

	WMToShader.worldMatrix = XMMatrixIdentity();
	sunWorld = XMMatrixTranslation(10.0f, 0.0f, 0.0f);
	sunWorld = sunWorld * XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 0.2f)), planetWorld);
	matrixScaling = XMMatrixScaling(10.0f, 10.0f, 10.0f);
	sunWorld = sunWorld * matrixScaling;
	WMToShader.worldMatrix = sunWorld;

	D3D11_MAPPED_SUBRESOURCE sunResource;
	ZeroMemory(&sunResource, sizeof(sunResource));
	context->Map(sunIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &sunResource);
	memcpy(sunResource.pData, AlienPlanet2_indicies, sizeof(AlienPlanet2_indicies));
	context->Unmap(sunIndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &planet2VertBuffer, &stride, &offset);
	context->IASetIndexBuffer(sunIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* sunTexView[] = { sunTextureView };
	context->PSSetShaderResources(0, 1, sunTexView);

	context->DrawIndexed(26232, 0, 0);

	//aircraft
	WMToShader.worldMatrix = XMMatrixIdentity();
	aircraftWorld = XMMatrixTranslation(-20.0f, 0.0f, 5.0f);
	matrixScaling = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	aircraftWorld = aircraftWorld * matrixScaling;
	WMToShader.worldMatrix = aircraftWorld;

	D3D11_MAPPED_SUBRESOURCE aircraftResource;
	ZeroMemory(&aircraftResource, sizeof(aircraftResource));
	context->Map(aircraftIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &aircraftResource);
	memcpy(aircraftResource.pData, aircraft_indicies, sizeof(aircraft_indicies));
	context->Unmap(aircraftIndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &aircraftVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(aircraftIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* aircraftTexView[] = { aircraftTextureView };
	context->PSSetShaderResources(0, 1, aircraftTexView);

	context->DrawIndexed(50316, 0, 0);




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

	//second viewport draws
	context->RSSetViewports(1, &viewport2);

	//alien planet 1 

	WMToShader.worldMatrix = XMMatrixIdentity(); //we need to set all of our objects to the proper view
	matrixRotateX = XMMatrixIdentity();
	matrixRotateX = XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 5));

	//TODO: Scale everything down so we look like the bottom is an overview of solar system. divide by 10
	XMMATRIX scale = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	planetWorld = matrixTranslate * matrixRotateX;
	planetWorld = planetWorld * scale;
	WMToShader.worldMatrix = planetWorld;
	VPMToShader.rotMatrix = matrixRotateX;

	D3D11_MAPPED_SUBRESOURCE mappedResource2;
	ZeroMemory(&mappedResource2, sizeof(mappedResource2));
	context->Map(indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource2);
	memcpy(mappedResource2.pData, MAlienPlanet_indicies, sizeof(MAlienPlanet_indicies));
	context->Unmap(indexBuffer, NULL);


	D3D11_MAPPED_SUBRESOURCE subResource4;
	ZeroMemory(&subResource4, sizeof(subResource4));
	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource4);
	memcpy(subResource4.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	D3D11_MAPPED_SUBRESOURCE subResource5;
	ZeroMemory(&subResource5, sizeof(subResource5));
	context->Map(constBuffer2, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource5);
	memcpy(subResource5.pData, &VPMToShader, sizeof(VPM_TO_VRAM));
	context->Unmap(constBuffer2, NULL);

	context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->VSSetShader(vertShader, NULL, 0);
	context->PSSetShader(pixShader, NULL, 0);

	//lighting for second viewport
	D3D11_MAPPED_SUBRESOURCE subResource6;
	ZeroMemory(&subResource6, sizeof(subResource6));
	context->Map(cbPerFrame, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource6);
	memcpy(subResource6.pData, &constBufferPerFrame, sizeof(perFrame));
	context->Unmap(cbPerFrame, NULL);

	context->PSSetConstantBuffers(0, 1, &cbPerFrame);

	context->IASetInputLayout(inputLayout);

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //or linestrip

	ID3D11ShaderResourceView* planet1TexViews2[] = { alienPlanetTextureView, environmentView };
	context->PSSetShaderResources(0, 2, planet1TexViews2);

	context->DrawIndexed(APINDEXSIZE, 0, 0);

	//planet 2
	WMToShader.worldMatrix = XMMatrixIdentity();
	planet2World = XMMatrixTranslation(20.0f, 5.0f, 20.0f);
	planet2World = planet2World * XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 0.5f)), planetWorld);
	matrixScaling = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	planet2World = planet2World * matrixScaling;
	WMToShader.worldMatrix = planet2World;

	D3D11_MAPPED_SUBRESOURCE planet2Resource2;
	ZeroMemory(&planet2Resource2, sizeof(planet2Resource2));
	context->Map(planet2IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &planet2Resource2);
	memcpy(planet2Resource2.pData, AlienPlanet2_indicies, sizeof(AlienPlanet2_indicies));
	context->Unmap(planet2IndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &planet2VertBuffer, &stride, &offset);
	context->IASetIndexBuffer(planet2IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* planet2TexView2[] = { alienPlanet2TextureView };
	context->PSSetShaderResources(0, 1, planet2TexView2);

	context->DrawIndexed(26232, 0, 0);

	//sun

	WMToShader.worldMatrix = XMMatrixIdentity();
	sunWorld = XMMatrixTranslation(10.0f, 0.0f, 0.0f);
	sunWorld = sunWorld * XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(timer.TotalTime() * 0.2f)), planetWorld);
	matrixScaling = XMMatrixScaling(2.5f, 2.5f, 2.5f);
	sunWorld = sunWorld * matrixScaling;
	WMToShader.worldMatrix = sunWorld;

	D3D11_MAPPED_SUBRESOURCE sunResource2;
	ZeroMemory(&sunResource2, sizeof(sunResource2));
	context->Map(sunIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &sunResource2);
	memcpy(sunResource2.pData, AlienPlanet2_indicies, sizeof(AlienPlanet2_indicies));
	context->Unmap(sunIndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &planet2VertBuffer, &stride, &offset);
	context->IASetIndexBuffer(sunIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* sunTexView2[] = { sunTextureView };
	context->PSSetShaderResources(0, 1, sunTexView2);

	context->DrawIndexed(26232, 0, 0);

	//aircraft
	WMToShader.worldMatrix = XMMatrixIdentity();
	aircraftWorld = XMMatrixTranslation(-30.0f, 0.0f, 5.0f);
	matrixScaling = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	aircraftWorld = aircraftWorld * matrixScaling;
	WMToShader.worldMatrix = aircraftWorld;

	D3D11_MAPPED_SUBRESOURCE aircraftResource2;
	ZeroMemory(&aircraftResource2, sizeof(aircraftResource2));
	context->Map(aircraftIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &aircraftResource2);
	memcpy(aircraftResource2.pData, aircraft_indicies, sizeof(aircraft_indicies));
	context->Unmap(aircraftIndexBuffer, NULL);

	context->Map(constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subResource);
	memcpy(subResource.pData, &WMToShader, sizeof(WM_TO_VRAM));
	context->Unmap(constBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);

	context->IASetVertexBuffers(0, 1, &aircraftVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(aircraftIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* aircraftTexView2[] = { aircraftTextureView };
	context->PSSetShaderResources(0, 1, aircraftTexView2);

	context->DrawIndexed(50316, 0, 0);


	//cube WM math
	WMToShader.worldMatrix = XMMatrixIdentity();
	matrixScaling = XMMatrixScaling(1000.0f, 1000.0f, 1000.0f);
	cube = matrixScaling;
	WMToShader.worldMatrix = cube;

	//update cuberesource
	D3D11_MAPPED_SUBRESOURCE cubeResource2;
	ZeroMemory(&cubeResource2, sizeof(cubeResource2));
	context->Map(cubeIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cubeResource2);
	memcpy(cubeResource2.pData, cubeIndices, sizeof(cubeIndices));
	context->Unmap(cubeIndexBuffer, NULL);

	context->VSSetConstantBuffers(0, 1, &constBuffer);
	context->VSSetConstantBuffers(1, 1, &constBuffer2);
	context->IASetVertexBuffers(0, 1, &cubeVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(cubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->IASetInputLayout(inputLayout2);
	//cube topology
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
	delete[] aircraft;
	delete[] sun;
	delete[] alienPlanet2;

	device->Release();
	swapChain->Release();
	targetView->Release();
	context->Release();

	BackBuffer->Release();
	vertBuffer->Release();
	constBuffer->Release();
	constBuffer2->Release();
	cbPerFrame->Release();
	indexBuffer->Release();
	depthBuffer->Release();
	zBuffer->Release();
	cubeIndexBuffer->Release();
	cubeVertBuffer->Release();
	planet2IndexBuffer->Release();
	planet2VertBuffer->Release();
	sunIndexBuffer->Release();
	sunVertBuffer->Release();
	aircraftIndexBuffer->Release();
	aircraftVertBuffer->Release();

	cubeMap_VS->Release();
	cubeMap_PS->Release();
	pixShader->Release();
	vertShader->Release();

	inputLayout->Release();
	inputLayout2->Release();

	DSLessEqual->Release();
	RSCullNone->Release();

	alienPlanetTextureView->Release();
	alienPlanet2TextureView->Release();
	sunTextureView->Release();
	aircraftTextureView->Release();
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