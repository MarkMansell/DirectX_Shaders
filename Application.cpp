#include "Application.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool Application::HandleKeyboard(MSG msg)
{
	XMFLOAT3 cameraPosition = _camera->GetPosition();

	switch (msg.wParam)
	{
	case VK_ESCAPE:
		PostQuitMessage(WM_QUIT);
		return true;
		break;
	}

	return false;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_renderTargetView = nullptr;
	_ShadowTexture = nullptr;
	_ShadowTargetView = nullptr;
	_ShadowShaderTexture = nullptr;
	_depthTargetView = nullptr;
	_renderToTexture = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
    _pVertexShaderNormalMapping = nullptr;
    _pPixelShaderNormalMapping = nullptr;
	_pVertexShaderStandardParallax = nullptr;
	_pPixelShaderStandardParallax = nullptr;
	_pVertexShaderRenderToTexture = nullptr;
	_pPixelShaderRenderToTexture = nullptr;
	_pVertexLayout = nullptr;
	_pVertexLayoutQuad = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
	_pShadowShader = nullptr;
	_pDepthShader = nullptr;

	DSLessEqual = nullptr;
	RSCullNone = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_COLOR.dds", nullptr, &_pTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_NRM.dds", nullptr, &_pTextureNormal);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_DISP.dds", nullptr, &_pTextureDisplacement);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Rocks\\RocksTexture_COLOR.dds", nullptr, &_pRockTexture);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Rocks\\RocksTexture_NRM.dds", nullptr, &_pRockNormal);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Rocks\\RocksTexture_DISP.dds", nullptr, &_pRockDisplacement);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\TutTextures\\rock_colormap_COLOR.dds", nullptr, &_pTutRockDiffuse);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\TutTextures\\rock_normalmap_NRM.dds", nullptr, &_pTutRockNormal);

	_bRenderExampleTexture = false;
	_bDepthScene = false;
	_bApplyBlur = false;
	_bShadowScene = false;
	_iCurrentObject = 5;
	

    // Setup Camera
	XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -1.0f);
	XMFLOAT3 at = XMFLOAT3(0.45f, -0.44f, 0.77f);
	XMFLOAT3 up = XMFLOAT3(0.22f, 0.89f, 0.38f);

	_camera = new Camera();
	_camera->SetPosition(eye);
	_camera->SetLens(0.25f * XM_PI, (float)_renderWidth / (float)_renderHeight, 1.0f, 200.0f);

	XMFLOAT3 lightpos = XMFLOAT3(-5.15f, 3.73f, -7.54f);
	// Create the light object.
	_pLight = new LightClass;
	_pLight->GetCamera()->SetPosition(lightpos);
	_pLight->GetCamera()->LookAt(lightpos, at, up);
	_pLight->GetCamera()->SetLens(0.25f * XM_PI, (float)_renderWidth / (float)_renderHeight, 1.0f, 200.0f);


	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.LightVecW = XMFLOAT3(10.0f, 10.0f, -5.0f);

	// Initialize the light object.
	_pLight->SetAmbientColor(0.5f, 0.5f, 0.5f, 1.0f);
	_pLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);

	Geometry cubeGeometry;
	cubeGeometry.indexBuffer = _pIndexBuffer;
	cubeGeometry.vertexBuffer = _pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry planeGeometry;
	planeGeometry.indexBuffer = _pPlaneIndexBuffer;
	planeGeometry.vertexBuffer = _pPlaneVertexBuffer;
	planeGeometry.numberOfIndices = 6;
	planeGeometry.vertexBufferOffset = 0;
	planeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry quadGeometry;
	quadGeometry.indexBuffer = _pScreenQuadIndexBuffer;
	quadGeometry.vertexBuffer = _pScreenQuadVertexBuffer;
	quadGeometry.numberOfIndices = 6;
	quadGeometry.vertexBufferOffset = 0;
	quadGeometry.vertexBufferStride = sizeof(QuadVertex);

	Material shinyMaterial;
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	Material noSpecMaterial;
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;
	
	GameObject * gameObject = new GameObject("Floor", planeGeometry, shinyMaterial);
	gameObject->SetPosition(0.0f, 0.0f, 0.0f);
	gameObject->SetScale(15.0f, 15.0f, 15.0f);
	gameObject->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);
	gameObject->SetDiffuseTextureRV(_pRockTexture);
	gameObject->SetNormalTextureRV(_pRockNormal);
	gameObject->SetDisplacementTextureRV(_pRockDisplacement);
	gameObject->SetPixelShader(_pPixelShader);
	gameObject->SetVertexShader(_pVertexShader);


	_gameObjects.push_back(gameObject);

	gameObject = new GameObject("Quad", quadGeometry, shinyMaterial);
	gameObject->SetPosition(0.0f, 0.0f, 0.0f);
	gameObject->SetScale(1.0f, 1.0f, 1.0f);
	gameObject->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);
	gameObject->SetDiffuseTextureRV(_D2DTexture);
	gameObject->SetPixelShader(_pPixelShaderRenderToTexture);
	gameObject->SetVertexShader(_pVertexShaderRenderToTexture);

	_gameObjects.push_back(gameObject);

	for (auto i = 0; i < 5; i++)
	{
		gameObject = new GameObject("Cube " + i, cubeGeometry, shinyMaterial);
		gameObject->SetScale(0.5f, 0.5f, 0.5f);
		gameObject->SetPosition(-4.0f + (i * 2.0f), 0.5f, 0.0f);
		gameObject->SetRotation(0.0f, 0.0f, 0.0f);


		switch (i)
		{
		case 1:
			gameObject->SetDiffuseTextureRV(_pRockTexture);
			gameObject->SetNormalTextureRV(_pRockNormal);
			gameObject->SetDisplacementTextureRV(_pRockDisplacement);
			gameObject->SetPixelShader(_pPixelShader);
			gameObject->SetVertexShader(_pVertexShader);
			break;

		case 2:
			gameObject->SetDiffuseTextureRV(_pTutRockDiffuse);
			gameObject->SetNormalTextureRV(_pTutRockNormal);
			gameObject->SetDisplacementTextureRV(_pRockDisplacement);
			gameObject->SetPixelShader(_pPixelShader);
			gameObject->SetVertexShader(_pVertexShader);
			break;
		case 3:
			gameObject->SetDiffuseTextureRV(_pTextureRV);
			gameObject->SetNormalTextureRV(_pTextureNormal);
			gameObject->SetDisplacementTextureRV(_pTextureDisplacement);
			gameObject->SetPixelShader(_pPixelShaderNormalMapping);
			gameObject->SetVertexShader(_pVertexShaderNormalMapping);
			break;
		case 4:
			gameObject->SetDiffuseTextureRV(_pRockTexture);
			gameObject->SetNormalTextureRV(_pRockNormal);
			gameObject->SetDisplacementTextureRV(_pRockDisplacement);
			gameObject->SetPixelShader(_pPixelShaderStandardParallax);
			gameObject->SetVertexShader(_pVertexShaderStandardParallax);
			break;

		default:
				gameObject->SetDiffuseTextureRV(_pTextureRV);
				gameObject->SetNormalTextureRV(_pTextureNormal);
				gameObject->SetDisplacementTextureRV(_pTextureDisplacement);
				gameObject->SetPixelShader(_pPixelShader);
				gameObject->SetVertexShader(_pVertexShader);
			break;
		}


		_gameObjects.push_back(gameObject);
	}

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;
	//_pVertexShaderStandardParallax
		//_pPixelShaderStandardParallax;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"ParallaxMapping.fx", "VS", "vs_4_0", &pVSBlob);

	ID3DBlob* pVSBlob1 = nullptr;
	hr = CompileShaderFromFile(L"RenderToTexture.fx", "VS", "vs_4_0", &pVSBlob1);

	ID3DBlob* pVSBlob2 = nullptr;
	hr = CompileShaderFromFile(L"RenderBlur.fx", "VS", "vs_4_0", &pVSBlob2);

	ID3DBlob* pVSBlob3 = nullptr;
	hr = CompileShaderFromFile(L"NormalMapping.fx", "VS", "vs_4_0", &pVSBlob3);
	
	ID3DBlob* pVSBlob4 = nullptr;
	hr = CompileShaderFromFile(L"StandardParallaxMapping.fx", "VS", "vs_4_0", &pVSBlob4);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob1->GetBufferPointer(), pVSBlob1->GetBufferSize(), nullptr, &_pVertexShaderRenderToTexture);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &_pVertexShaderBlur);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob3->GetBufferPointer(), pVSBlob3->GetBufferSize(), nullptr, &_pVertexShaderNormalMapping);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob3->GetBufferPointer(), pVSBlob3->GetBufferSize(), nullptr, &_pVertexShaderStandardParallax);
	
	if (FAILED(hr))
	{	
		pVSBlob->Release();
		pVSBlob1->Release();
		pVSBlob2->Release();
		pVSBlob3->Release();
        return hr;
	}


	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"ParallaxMapping.fx", "PS", "ps_4_0", &pPSBlob);

	ID3DBlob* pPSBlob1 = nullptr;
	hr = CompileShaderFromFile(L"RenderToTexture.fx", "D2D_PS", "ps_4_0", &pPSBlob1);

	ID3DBlob* pPSBlob2 = nullptr;
	hr = CompileShaderFromFile(L"RenderBlur.fx", "PS", "ps_4_0", &pPSBlob2);
	
	ID3DBlob* pPSBlob3 = nullptr;
	hr = CompileShaderFromFile(L"NormalMapping.fx", "PS", "ps_4_0", &pPSBlob3);

	ID3DBlob* pPSBlob4 = nullptr;
	hr = CompileShaderFromFile(L"StandardParallaxMapping.fx", "PS", "ps_4_0", &pPSBlob4);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

	hr = _pd3dDevice->CreatePixelShader(pPSBlob1->GetBufferPointer(), pPSBlob1->GetBufferSize(), nullptr, &_pPixelShaderRenderToTexture);
	pPSBlob1->Release();

	hr = _pd3dDevice->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &_pPixelShaderBlur);
	pPSBlob2->Release();

	hr = _pd3dDevice->CreatePixelShader(pPSBlob3->GetBufferPointer(), pPSBlob3->GetBufferSize(), nullptr, &_pPixelShaderNormalMapping);
	pPSBlob3->Release();

	hr = _pd3dDevice->CreatePixelShader(pPSBlob4->GetBufferPointer(), pPSBlob4->GetBufferSize(), nullptr, &_pPixelShaderStandardParallax);
	pPSBlob3->Release();

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	_pShadowShader = new ShadowShaderClass;
	if (!_pShadowShader)
	{
		return false;
	}

	// Initialize the shadow shader object.
	hr = _pShadowShader->Initialize(_pd3dDevice, nullptr);
	if (!hr)
	{
		//MessageBox(L"Could not initialize the shadow shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the depth shader object.
	_pDepthShader = new DepthShaderClass;
	if (!_pDepthShader)
	{
		return false;
	}

	// Initialize the depth shader object.
	hr = _pDepthShader->Initialize(_pd3dDevice, nullptr);
	if (!hr)
	{
		//MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
		return false;
	}





    if (FAILED(hr))
        return hr;
	
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();


	D3D11_INPUT_ELEMENT_DESC quadLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements1 = ARRAYSIZE(quadLayout);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(quadLayout, numElements1, pVSBlob1->GetBufferPointer(),
		pVSBlob1->GetBufferSize(), &_pVertexLayoutQuad);

	pVSBlob1->Release();

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		// Fill in the front face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),   XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		// Fill in the back face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		// Fill in the top face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		// Fill in the bottom face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),   XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),  XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		
		// Fill in the left face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),   XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },

		// Fill in the right face vertex data.
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),   XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),    XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),   XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };


    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	// Create vertex buffer
	SimpleVertex planeVertices[] =
	{
		//--------------Position--------------------Normal----------------------TextCoord--------------Tangent--------
		{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 15.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 1.0f),   XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(15.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, -1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(15.0f, 15.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

	
	QuadVertex ScreenQuadVerts[] =
	{
		//--------------Position-----------TextCoord-----------
		{ XMFLOAT2(-1.0f, -1.0f),  XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT2(-1.0f, 1.0f),   XMFLOAT2(0.0f,0.0f) },
		{ XMFLOAT2(1.0f, 1.0f),    XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT2(1.0f, -1.0f),   XMFLOAT2(1.0f, 1.0f) },
	};



	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(QuadVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = ScreenQuadVerts;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pScreenQuadVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
	  0, 1, 2,
	  0, 2, 3,

	  4, 5, 6,
	  4, 6, 7,

	  8, 9, 10,
	  8, 10, 11,

	  12, 13, 14,
	  12, 14, 15,

	  16, 17, 18,
	  16, 18, 19,

	  20, 21, 22,
	  20, 22, 23
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	// Create plane index buffer
	WORD planeIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);

	WORD screenQuadIndices[] =
	{
	/*	0, 1, 2,
		3, 4, 5,*/
		0, 1, 2,
		0, 2, 3,
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = screenQuadIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pScreenQuadIndexBuffer);


	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 960, 540};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Parallax Mapping", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 4;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _renderWidth;
    sd.BufferDesc.Height = _renderHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;

	hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView); 

	pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	
	D3D11_TEXTURE2D_DESC renderToTextureDesc;

	ZeroMemory(&renderToTextureDesc, sizeof(renderToTextureDesc));

	renderToTextureDesc.Width = _renderWidth;
	renderToTextureDesc.Height = _renderHeight;
	renderToTextureDesc.MipLevels = 1;
	renderToTextureDesc.ArraySize = 1;
	renderToTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderToTextureDesc.SampleDesc.Count = 1;
	renderToTextureDesc.SampleDesc.Quality = 0;
	renderToTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderToTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderToTextureDesc.CPUAccessFlags = 0;
	renderToTextureDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&renderToTextureDesc, nullptr, &_renderToTexture);
	_pd3dDevice->CreateTexture2D(&renderToTextureDesc, nullptr, &_depthTexture);
	_pd3dDevice->CreateTexture2D(&renderToTextureDesc, nullptr, &_ShadowTexture);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = renderToTextureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	_pd3dDevice->CreateRenderTargetView(_renderToTexture, &renderTargetViewDesc, &_renderTargetView);
	_pd3dDevice->CreateRenderTargetView(_depthTexture, &renderTargetViewDesc, &_depthTargetView);
	_pd3dDevice->CreateRenderTargetView(_ShadowTexture, &renderTargetViewDesc, &_ShadowTargetView);

	_pd3dDevice->CreateShaderResourceView(_renderToTexture, nullptr, &_D2DTexture);
	_pd3dDevice->CreateShaderResourceView(_depthTexture, nullptr, &_depthShaderTexture);
	_pd3dDevice->CreateShaderResourceView(_ShadowTexture, nullptr, &_ShadowShaderTexture);


    if (FAILED(hr))
        return hr;

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_renderWidth;
    vp.Height = (FLOAT)_renderHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitIndexBuffer();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _renderWidth;
	depthStencilDesc.Height = _renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView); // for tutorial 5


	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	_pd3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode);

    return S_OK;
}

void Application::moveForward(int objectNumber)
{
	XMFLOAT3 position = _gameObjects[objectNumber]->GetPosition();
	position.z -= 0.1f;
	_gameObjects[objectNumber]->SetPosition(position);

	XMFLOAT3 lightvect = _pLight->GetCamera()->GetPosition();	
	lightvect.x += 0.1f;
	_pLight->GetCamera()->SetPosition(lightvect);
}
void Application::moveBackwards(int objectNumber)
{
	XMFLOAT3 position = _gameObjects[objectNumber]->GetPosition();
	position.z += 0.1f;
	_gameObjects[objectNumber]->SetPosition(position);


	XMFLOAT3 lightvect = _pLight->GetCamera()->GetPosition();
	lightvect.x -= 0.1f;
	_pLight->GetCamera()->SetPosition(lightvect);
}

void Application::moveLeft(int objectNumber)
{
	XMFLOAT3 position = _gameObjects[objectNumber]->GetPosition();
	position.x -= 0.1f;
	_gameObjects[objectNumber]->SetPosition(position);
}
void Application::moveRight(int objectNumber)
{
	XMFLOAT3 position = _gameObjects[objectNumber]->GetPosition();
	position.x += 0.1f;
	_gameObjects[objectNumber]->SetPosition(position);
}

void Application::HandleInput(float delta)
{
	if (GetAsyncKeyState('1')) // Default
	{
		_bRenderExampleTexture = false;
		_bDepthScene = false;
		_bApplyBlur = false;
		_bShadowScene = false;
	}
	if (GetAsyncKeyState('2')) // Render example texture 
	{
		_bRenderExampleTexture = true;
	}
	if (GetAsyncKeyState('3'))
	{
		_bShadowScene = true;
	}
	if (GetAsyncKeyState('4')) 
	{
		_bApplyBlur = true;
	}
	if (GetAsyncKeyState('5')) 
	{
		_bDepthScene = true;
	}
	if (GetAsyncKeyState('6'))
	{
		_iCurrentObject = 6;
	}

	//------------------------------Move Camera-----------------------------
	if (GetAsyncKeyState('W'))
	{
		_camera->Walk(_cameraSpeed * delta);
	}
	if (GetAsyncKeyState('S'))
	{
		_camera->Walk(-_cameraSpeed * delta);
	}
	if (GetAsyncKeyState('D'))
	{
		_camera->Strafe(_cameraSpeed * delta);
	}
	if (GetAsyncKeyState('A'))
	{
		_camera->Strafe(-_cameraSpeed * delta);
	}

	//------------------------------Pivot Camera-----------------------------
	if (GetAsyncKeyState(VK_UP))
	{
		moveBackwards(_iCurrentObject);
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		moveForward(_iCurrentObject);
	}
	if (GetAsyncKeyState(VK_LEFT))
	{
		moveLeft(_iCurrentObject);
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		moveRight(_iCurrentObject);
	}


	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) && (_RightMouseDown == false)) 
	{
		POINT cursor;
		GetCursorPos(&cursor);
		_CursorX = cursor.x;
		_CursorY = cursor.y;
		_RightMouseDown = true;//reset the flag
	}
	else if (GetAsyncKeyState(VK_RBUTTON) == 0)
	{
		_RightMouseDown = false;//reset the flag
	}
	else if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) && (_RightMouseDown == true))
	{
		POINT cursor;
		GetCursorPos(&cursor);

		_camera->Pitch((cursor.y - _CursorY) * delta * _cameraPivotSpeed);
		_camera->RotateY((cursor.x - _CursorX) * delta * _cameraPivotSpeed);

		_RightMouseDown = false;
	}
		
}

void Application::Update(double deltatime)
{
    // Update our time
    static float timeSinceStart = 0.0f;
    static DWORD dwTimeStart = 0;

    DWORD dwTimeCur = GetTickCount();

    if (dwTimeStart == 0)
        dwTimeStart = dwTimeCur;

	timeSinceStart = (dwTimeCur - dwTimeStart) / 1000.0f;

	HandleInput(deltatime);

	_camera->UpdateViewMatrix();
	_pLight->GetCamera()->UpdateViewMatrix();
    int objectindex = 5;
	                                                                                                                                                                                                                                                                                        
	float rot = XMConvertToRadians(3 * 0.0166f);
	_gameObjects[objectindex]->SetRotation(_gameObjects[objectindex]->GetRotation().x, _gameObjects[objectindex]->GetRotation().y + rot, _gameObjects[objectindex]->GetRotation().z);
	
	objectindex = 2;

	_gameObjects[objectindex]->SetRotation(_gameObjects[objectindex]->GetRotation().x + rot, _gameObjects[objectindex]->GetRotation().y + rot, _gameObjects[objectindex]->GetRotation().z);

	objectindex = 3;

	_gameObjects[objectindex]->SetRotation(_gameObjects[objectindex]->GetRotation().x, _gameObjects[objectindex]->GetRotation().y + rot * 0.5f, _gameObjects[objectindex]->GetRotation().z);
	

	// Update objects
	for (auto gameObject : _gameObjects)
	{
		gameObject->Update(timeSinceStart);
	}
}

void Application::Draw()
{
	
    //
    // Clear buffers
    //
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha

	//----------------------------------Render To Texture-----------------------------------------
	_pImmediateContext->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView); 
	_pImmediateContext->ClearRenderTargetView(_renderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup buffers and render scene
    //
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	
    ConstantBuffer cb;
	// Generate the light view matrix based on the light's position.
	//
	cb.View = DirectX::XMMatrixTranspose(_camera->View());
	cb.Projection = DirectX::XMMatrixTranspose(_camera->Proj());
	cb.EyePosW = _camera->GetPosition();
	
	cb.light = basicLight;


	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (_gameObjects[i]->GetName() == "Quad")
			continue;

		Material material = _gameObjects[i]->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;
		_gameObjects[i]->SetEnablePixelShader(true);
		_gameObjects[i]->SetEnableVertexShader(true);
		// Set world matrix
		cb.World = DirectX::XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		_gameObjects[i]->Draw(_pImmediateContext);
	}
	//-------------------------------------------------------------------------------------

	//----------------------------------Depth Map-----------------------------------------
	_pImmediateContext->OMSetRenderTargets(1, &_depthTargetView, _depthStencilView);
	_pImmediateContext->ClearRenderTargetView(_depthTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	cb.View = DirectX::XMMatrixTranspose(_pLight->GetCamera()->View());
	cb.Projection = DirectX::XMMatrixTranspose(_pLight->GetCamera()->Proj());
	cb.EyePosW = _pLight->GetCamera()->GetPosition();

	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (_gameObjects[i]->GetName() == "Quad")
			continue;

		// Set world matrix
		cb.World = DirectX::XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		 //Draw object
		_gameObjects[i]->SetDraw(_pImmediateContext);

		_pDepthShader->Render(_pImmediateContext, _gameObjects[i]->GetNumberOfIndices(), cb.World, cb.View, cb.Projection);
	}
	//-------------------------------------------------------------------------------------

	//----------------------------------Shadow Map-----------------------------------------
	_pImmediateContext->OMSetRenderTargets(1, &_ShadowTargetView, _depthStencilView);
	_pImmediateContext->ClearRenderTargetView(_ShadowTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	cb.View = DirectX::XMMatrixTranspose(_camera->View());
	cb.Projection = DirectX::XMMatrixTranspose(_camera->Proj());
	cb.EyePosW = _camera->GetPosition();

	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (_gameObjects[i]->GetName() == "Quad")
			continue;

		// Set world matrix
		cb.World = DirectX::XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		//Draw object
		_gameObjects[i]->SetDraw(_pImmediateContext);

		_pShadowShader->Render(_pImmediateContext, _gameObjects[i]->GetNumberOfIndices(), cb.World, cb.View, cb.Projection, _pLight->GetCamera()->View(), _pLight->GetCamera()->Proj(), _gameObjects[i]->GetDiffuseTextureRV()
		, _depthShaderTexture, _pLight->GetCamera()->GetPosition(), _pLight->GetAmbientColor(), _pLight->GetDiffuseColor());
	}

	//-------------------------------------------------------------------------------------

	//----------------------------------Render To Quad-----------------------------------------
	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr); 
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_pImmediateContext->IASetInputLayout(_pVertexLayoutQuad);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (_gameObjects[i]->GetName() != "Quad")
			continue;

		if (_bRenderExampleTexture)
		{
			_gameObjects[i]->SetDiffuseTextureRV(_pTextureRV);
		}
		else if(_bShadowScene)
		{
			_gameObjects[i]->SetDiffuseTextureRV(_ShadowShaderTexture);		
		}
		else if (_bDepthScene)
		{
			_gameObjects[i]->SetDiffuseTextureRV(_depthShaderTexture);
		} else
			_gameObjects[i]->SetDiffuseTextureRV(_D2DTexture);		

		if (_bApplyBlur)
		{
			_gameObjects[i]->SetVertexShader(_pVertexShaderBlur);
			_gameObjects[i]->SetPixelShader(_pPixelShaderBlur);
		}
		else
		{
			_gameObjects[i]->SetVertexShader(_pVertexShaderRenderToTexture);
			_gameObjects[i]->SetPixelShader(_pPixelShaderRenderToTexture);
		}


		cb.World = DirectX::XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);


		_gameObjects[i]->Draw(_pImmediateContext);
	}

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	_pImmediateContext->PSSetShaderResources(0, 1, pSRV);
}

void Application::Cleanup()
{
	if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pSamplerLinear) _pSamplerLinear->Release();

	if (_pTextureRV) _pTextureRV->Release();


	if (_pConstantBuffer) _pConstantBuffer->Release();

	if (_pVertexBuffer) _pVertexBuffer->Release();
	if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
	if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();

	if (_pVertexLayout) _pVertexLayout->Release();
	if (_pVertexShader) _pVertexShader->Release();
	if (_pPixelShader) _pPixelShader->Release();
	if (_pRenderTargetView) _pRenderTargetView->Release();
	if (_renderToTexture) _renderToTexture->Release();
	if (_pSwapChain) _pSwapChain->Release();
	if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();

	if (DSLessEqual) DSLessEqual->Release();
	if (RSCullNone) RSCullNone->Release();

	if (CCWcullMode) CCWcullMode->Release();
	if (CWcullMode) CWcullMode->Release();

	// Release the light object.
	if (_pLight)
	{
		delete _pLight;
		_pLight = 0;
	}
	// Release the depth shader object.
	if (_pShadowShader)
	{
		_pShadowShader->Shutdown();
		delete _pShadowShader;
		_pShadowShader = 0;
	}
	// Release the shadow shader object.
	if (_pShadowShader)
	{
		_pShadowShader->Shutdown();
		delete _pShadowShader;
		_pShadowShader = 0;
	}

	if (_camera)
	{
		delete _camera;
		_camera = nullptr;
	}

	for (auto gameObject : _gameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}
}