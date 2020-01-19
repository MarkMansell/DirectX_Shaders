#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "Camera.h"
#include <iostream>

#include <vector>
#include "GameObject.h"
#include "ShadowClass.h"
#include "LightClass.h"
#include "DepthShaderClass.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 PosL;
	XMFLOAT3 NormL;
	XMFLOAT2 Tex;
	XMFLOAT3 Tan;
};
struct QuadVertex
{
	XMFLOAT2 PosL;
	XMFLOAT2 Tex;
};

struct SurfaceInfo
{
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 SpecularMtrl;
};

struct Light
{
	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;

	float SpecularPower;
	XMFLOAT3 LightVecW;
	//XMFLOAT4 LightPos;
};

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	
	SurfaceInfo surface;

	Light light;

	XMFLOAT3 EyePosW;
	float HasTexture;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	LightClass*             _pLight;
	ShadowShaderClass*		_pShadowShader;
	DepthShaderClass* _pDepthShader;
	
	ID3D11Texture2D* _renderToTexture;
	ID3D11RenderTargetView* _renderTargetView;
	ID3D11ShaderResourceView *_D2DTexture;

	ID3D11Texture2D* _depthTexture;
	ID3D11RenderTargetView* _depthTargetView;
	ID3D11ShaderResourceView *_depthShaderTexture;

	ID3D11Texture2D* _ShadowTexture;
	ID3D11RenderTargetView* _ShadowTargetView;
	ID3D11ShaderResourceView *_ShadowShaderTexture;

	ID3D11VertexShader*     _pVertexShaderNormalMapping;
	ID3D11PixelShader*      _pPixelShaderNormalMapping;

	ID3D11VertexShader*     _pVertexShaderStandardParallax;
	ID3D11PixelShader*      _pPixelShaderStandardParallax;

	ID3D11VertexShader*     _pVertexShaderRenderToTexture;
	ID3D11PixelShader*      _pPixelShaderRenderToTexture;

	ID3D11VertexShader*     _pVertexShaderBlur;
	ID3D11PixelShader*      _pPixelShaderBlur;

	ID3D11InputLayout*      _pVertexLayoutQuad;

	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

	ID3D11Buffer*           _pPlaneVertexBuffer;
	ID3D11Buffer*           _pPlaneIndexBuffer;

	ID3D11Buffer*           _pScreenQuadVertexBuffer;
	ID3D11Buffer*           _pScreenQuadIndexBuffer;

	ID3D11Buffer*           _pConstantBuffer;

	ID3D11DepthStencilView* _depthStencilView = nullptr;
	ID3D11Texture2D* _depthStencilBuffer = nullptr;

	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11ShaderResourceView * _pTextureNormal = nullptr;
	ID3D11ShaderResourceView * _pTextureDisplacement = nullptr;

	ID3D11ShaderResourceView * _pRockTexture = nullptr;
	ID3D11ShaderResourceView * _pRockNormal = nullptr;
	ID3D11ShaderResourceView * _pRockDisplacement = nullptr;

	ID3D11ShaderResourceView * _pTutRockDiffuse= nullptr;
	ID3D11ShaderResourceView * _pTutRockNormal = nullptr;

	ID3D11SamplerState * _pSamplerLinear = nullptr;

	Light basicLight;

	vector<GameObject *> _gameObjects;

	Camera * _camera;

	// Our map camera's view and projection matrices
	XMMATRIX mapView;
	XMMATRIX mapProjection;

	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = -90.0f;
	float _cameraSpeed = 3.0f;
	float _cameraPivotSpeed = 0.5f;

	float _CursorX;
	float _CursorY;

	bool _RightMouseDown = false;
	bool _bRenderExampleTexture;
	bool _bDepthScene;
	bool _bApplyBlur;
	bool _bShadowScene;
	int _iCurrentObject;


	// Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;

	ID3D11RasterizerState* CCWcullMode;
	ID3D11RasterizerState* CWcullMode;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	void moveForward(int objectNumber);
	void moveBackwards(int objectNumber);
	void moveLeft(int objectNumber);
	void moveRight(int objectNumber);
	void HandleInput(float delta);

public:

	UINT _WindowHeight;
	UINT _WindowWidth;

	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update(double deltatime);
	void Draw();

};

