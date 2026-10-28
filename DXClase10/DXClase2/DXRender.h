#ifndef _DXRender_
#define _DXRender_

#include <Xinput.h>
#include <chrono>
#pragma comment(lib, "Xinput9_1_0.lib")

#include "Constants.h"

#include "Camara.h"
#include "Light.h"
#include "MeshPrimitives.h"

class DXRender
{
private:
	//Global Declarations//
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;

	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	D3DXMATRIX View;
	D3DXMATRIX Projection;


	MPrimitives *cubo;
	MPrimitives *esfera;
	MPrimitives *toroide;
	MPrimitives *cono;
	MPrimitives *cilindro;
	MPrimitives *billboard;
	MPrimitives *skydomo;
	MPrimitives *terreno;
	TerrainSettings terrainSettings;
	MPrimitives *casa;
	MPrimitives *mesa;
	MPrimitives *pocion;
	MPrimitives *pablito;
	MPrimitives *tree;
	MPrimitives *fence;
	MPrimitives *furnace;
	MPrimitives *pipe;
	MPrimitives *corner_pipe;

	// Individual model scales (main integration).
	float pablitoScale, casaScale, pocionScale, mesaScale;
	float treeScale, fenceScale, furnaceScale, pipeScale, cornerPipeScale;
	MPrimitives *BoundingSphere1;
	MPrimitives *BoundingSphere2;

	Camara *camara;
	Camara *skyCamara;

	float rotPerFrame;
	float sphereSpeed;
	bool isJumping;
	float jumpVelocity;
	float jumpOffset;
	bool jumpWasDown;
	float frameSeconds;
	std::chrono::steady_clock::time_point lastFrameTime;
	D3DXVECTOR2 BoundingSphereTranslation;
	D3DXVECTOR2 DummyBoundyEvents;
	D3DXVECTOR2 LastBoundingPosition;

public:
	DXRender()
	{
		// Pablito Pinwi: edit these values before rebuilding.
		terrainSettings.width = 255.0f;
		terrainSettings.depth = 255.0f;
		terrainSettings.maxHeight = 85.0f; // White pixel height in world units.
		terrainSettings.verticalScale = 1.0f;
		terrainSettings.horizontalScale = 3.0f;
		terrainSettings.position = D3DXVECTOR3(0, 0, 0);
		casaScale = mesaScale = pocionScale = 4.0f;
		pablitoScale = treeScale = fenceScale = furnaceScale = 1.0f;
		pipeScale = cornerPipeScale = 1.0f;
		rotPerFrame = 0.0f;
		isJumping = false;
		jumpVelocity = 0.0f;
		jumpOffset = 0.0f;
		jumpWasDown = false;
		frameSeconds = 0.0f;
		lastFrameTime = std::chrono::steady_clock::now();
	}

	bool InitDevice(HWND hwnd, int Width, int Height)
	{
		//Describe our Buffer
		DXGI_MODE_DESC bufferDesc;

		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

		bufferDesc.Width = Width;
		bufferDesc.Height = Height;
		bufferDesc.RefreshRate.Numerator = 60;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		//Describe our SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc;

		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		//Create our SwapChain
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
			D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

		//Create our BackBuffer
		ID3D11Texture2D* BackBuffer;
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

		//Create our Render Target
		d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
		BackBuffer->Release();

		//Crea la descripcion del raster, el cual determina como se dibujaran los poligonos
		D3D11_RASTERIZER_DESC rasterDesc;
		ID3D11RasterizerState* rasterState;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;//se puede manipular para transparencias, cambiar a D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0;

		d3d11Device->CreateRasterizerState(&rasterDesc, &rasterState);
		d3d11DevCon->RSSetState(rasterState);

		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ID3D11Texture2D* depthStencilBuffer;

		depthStencilDesc.Width     = Width;
		depthStencilDesc.Height    = Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0; 
		depthStencilDesc.MiscFlags      = 0;

		//Create the Depth/Stencil View
		d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
		d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

		//Set our Render Target
		d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

		cubo = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		esfera = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		toroide = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, 0.5f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		cilindro = new MPrimitives(d3d11Device, d3d11DevCon, 16, 1.0f, 1.0f, 2.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		cono = new MPrimitives(d3d11Device, d3d11DevCon, 16, 0.0f, 1.0f, 2.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		billboard = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"Texturas/ave.png");
		
		skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/skybox.png", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		terreno = new MPrimitives(d3d11Device, d3d11DevCon, terrainSettings, L"Texturas/dirt_diff.png", L"Texturas/Meshpart1_diff.png", "Texturas/PablitoPinwi_Heightmap_256.bmp", ShaderMultitextura);

		if (!terreno->IsTerrainLoaded()) return false;

		// Three imported OBJ meshes, converted to the template's text format.
		casa = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/casa_pablo.txt",
			L"Models/PablitoPinwi/casa.png", L"", ShaderDiffuse);
		mesa = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/Mesa.txt",
			L"Models/PablitoPinwi/Mesa.png", L"", ShaderDiffuse);
		pocion = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/Pocion.txt",
			L"Models/PablitoPinwi/Poci\u00f3n.png", L"", ShaderDiffuse);
		if (!casa->IsModelLoaded() || !mesa->IsModelLoaded() || !pocion->IsModelLoaded()) return false;

		// Additional models from origin/main. Recenter their exported origins at the base.
		pablito = new MPrimitives(d3d11Device, d3d11DevCon, "Models/pablitopinwi.txt",
			L"Texturas/escudo.jpg", L"", ShaderDiffuse, true);
		tree = new MPrimitives(d3d11Device, d3d11DevCon, "Models/tree.txt",
			L"Texturas/tree.png", L"", ShaderDiffuse, true);
		fence = new MPrimitives(d3d11Device, d3d11DevCon, "Models/fence.txt",
			L"Texturas/fence.png", L"", ShaderDiffuse, true);
		furnace = new MPrimitives(d3d11Device, d3d11DevCon, "Models/furnace.txt",
			L"Texturas/furnace.png", L"", ShaderDiffuse, true);
		pipe = new MPrimitives(d3d11Device, d3d11DevCon, "Models/pipe.txt",
			L"Texturas/pipe.png", L"", ShaderDiffuse, true);
		corner_pipe = new MPrimitives(d3d11Device, d3d11DevCon, "Models/corner_pipe.txt",
			L"Texturas/corner_pipe.png", L"", ShaderDiffuse, true);
		if (!pablito->IsModelLoaded() || !tree->IsModelLoaded() || !fence->IsModelLoaded()
			|| !furnace->IsModelLoaded() || !pipe->IsModelLoaded() || !corner_pipe->IsModelLoaded()) return false;

		BoundingSphere1 = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		BoundingSphere2 = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		BoundingSphereTranslation = D3DXVECTOR2(0, 0);
		
		return true;
	}

	bool InitDirectInput(HINSTANCE hInstance, HWND hwnd)
	{
		DirectInput8Create(hInstance,
			DIRECTINPUT_VERSION,
			IID_IDirectInput8,
			(void**)&DirectInput,
			NULL); 

		DirectInput->CreateDevice(GUID_SysKeyboard,
			&DIKeyboard,
			NULL);

		DirectInput->CreateDevice(GUID_SysMouse,
			&DIMouse,
			NULL);

		DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
		DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

		DIMouse->SetDataFormat(&c_dfDIMouse);
		DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

		return true;
	}

	void DetectInput()
	{
		DIMOUSESTATE mouseCurrState = {};

		BYTE keyboardState[256] = {};

		DIKeyboard->Acquire();
		DIMouse->Acquire();

		DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

		DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);

		XINPUT_STATE gamepadState = {};
		const bool gamepadConnected = XInputGetState(0, &gamepadState) == ERROR_SUCCESS;
		const WORD buttons = gamepadConnected ? gamepadState.Gamepad.wButtons : 0;

		// Velocidades en unidades por segundo. Correr mientras se sostiene Shift o L3.
		const bool running = keyboardState[DIK_LSHIFT] || keyboardState[DIK_RSHIFT]
			|| (buttons & XINPUT_GAMEPAD_LEFT_THUMB);
		const float speed = (running ? 16.0f : 8.0f) * frameSeconds;

		// Un salto por pulsacion: Espacio o boton A del mando.
		const bool jumpDown = keyboardState[DIK_SPACE] || (buttons & XINPUT_GAMEPAD_A);
		if (jumpDown && !jumpWasDown && !isJumping)
		{
			isJumping = true;
			jumpVelocity = 5.0f;
		}
		jumpWasDown = jumpDown;

		float moveLeftRight = 0;
		float moveBackForward = 0;

		float camYaw = 0;
		float camPitch = 0;

		

		if(keyboardState[DIK_A])
		{
			moveLeftRight -= speed;
		}
		if(keyboardState[DIK_D])
		{
			moveLeftRight += speed;
		}
		if(keyboardState[DIK_W])
		{
			moveBackForward += speed;
		}
		if(keyboardState[DIK_S])
		{
			moveBackForward -= speed;
		}
		if (keyboardState[DIK_LEFT])
		{
			DummyBoundyEvents.x = -.01f;
		}
		if (keyboardState[DIK_RIGHT])
		{
			DummyBoundyEvents.x = .01f;
		}
		if (keyboardState[DIK_UP])
		{
			DummyBoundyEvents.y = .01f;
		}
		if (keyboardState[DIK_DOWN])
		{
			DummyBoundyEvents.y = -.01f;
		}

		// Mando 1: stick izquierdo para moverse, derecho para mirar.
		if (gamepadConnected)
		{
			const short deadzone = 8000;
			const float cameraSensitivity = 1.8f * frameSeconds;
			const float thumbLX = (float)gamepadState.Gamepad.sThumbLX;
			const float thumbLY = (float)gamepadState.Gamepad.sThumbLY;
			const float thumbRX = (float)gamepadState.Gamepad.sThumbRX;
			const float thumbRY = (float)gamepadState.Gamepad.sThumbRY;

			if (thumbLX > deadzone || thumbLX < -deadzone)
				moveLeftRight += (thumbLX / (thumbLX < 0 ? 32768.0f : 32767.0f)) * speed;
			if (thumbLY > deadzone || thumbLY < -deadzone)
				moveBackForward += (thumbLY / (thumbLY < 0 ? 32768.0f : 32767.0f)) * speed;
			if (thumbRX > deadzone || thumbRX < -deadzone)
				camYaw += (thumbRX / (thumbRX < 0 ? 32768.0f : 32767.0f)) * cameraSensitivity;
			if (thumbRY > deadzone || thumbRY < -deadzone)
				camPitch -= (thumbRY / (thumbRY < 0 ? 32768.0f : 32767.0f)) * cameraSensitivity;
		}

		// DirectInput entrega desplazamientos relativos en cada lectura.
		// Sumarlos permite usar el mouse y el mando al mismo tiempo.
		{
			float smoothness = 0.0005f;
			camYaw += mouseCurrState.lX * smoothness;

			camPitch += mouseCurrState.lY * smoothness;

			mouseLastState = mouseCurrState;
		}

		camara->UpdateCamera(moveBackForward, moveLeftRight, camYaw, camPitch); 
		skyCamara->setSkyCamara(camara);
	}

	void ReleaseObjects()
	{
		casa->ReleaseModelResources();
		mesa->ReleaseModelResources();
		pocion->ReleaseModelResources();
		delete casa;
		delete mesa;
		delete pocion;
		MPrimitives* additionalModels[] = { pablito, tree, fence, furnace, pipe, corner_pipe };
		for (MPrimitives* model : additionalModels)
		{
			model->ReleaseModelResources();
			delete model;
		}
		//Release the COM Objects we created
		SwapChain->Release();
		d3d11Device->Release();
		d3d11DevCon->Release();
		renderTargetView->Release();
		depthStencilView->Release();
	}

	bool InitScene(int Width, int Height)
	{
		//Create the Viewport
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = Width;
		viewport.Height = Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//set light values
		light.direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		light.ambient = D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
		light.diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

		//Set the Viewport
		d3d11DevCon->RSSetViewports(1, &viewport);

		//Create the camera
		// Start in the flatter village area, facing the imported models.
		D3DXVECTOR3 Position(384.0f, terreno->getTerrainCollision(D3DXVECTOR2(384.0f, 380.0f)) + 3.0f, 380.0f);
		D3DXVECTOR3 Target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		camara = new Camara(Position,Target,Up, Width, Height);
		skyCamara = new Camara(Position,Target,Up, Width, Height);

		return true;
	}

	// Centered model base rests on the terrain; yaw is in degrees.
	void PlaceModel(MPrimitives* model, float x, float z, float size, float yaw = 0.0f)
	{
		D3DXMATRIX scaling, rotation, translation;
		D3DXMatrixScaling(&scaling, size, size, size);
		D3DXMatrixRotationY(&rotation, D3DXToRadian(yaw));
		D3DXMatrixTranslation(&translation, x, terreno->getTerrainCollision(D3DXVECTOR2(x, z)), z);
		model->UpdateScene(scaling * rotation * translation);
	}

	// Edit X/Z, scale and yaw here to arrange the imported models.
	void UpdateImportedModels()
	{
		const D3DXVECTOR2 houseXZ(384.0f, 435.0f);
		const D3DXVECTOR2 tableXZ(384.0f, 403.0f);
		const float houseYaw = 65.0f; // Degrees.
		const float houseY = terreno->getTerrainCollision(houseXZ) - 0.15f;
		const float tableY = terreno->getTerrainCollision(tableXZ);
		D3DXMATRIX scaling, rotation, translation;
		D3DXMatrixScaling(&scaling, casaScale, casaScale, casaScale);
		D3DXMatrixRotationY(&rotation, D3DXToRadian(houseYaw));
		D3DXMatrixTranslation(&translation, houseXZ.x, houseY, houseXZ.y);
		casa->UpdateScene(scaling * rotation * translation);
		D3DXMatrixScaling(&scaling, mesaScale, mesaScale, mesaScale);
		D3DXMatrixTranslation(&translation, tableXZ.x, tableY, tableXZ.y);
		mesa->UpdateScene(scaling * translation);
		// Centered on the tabletop, using its measured height instead of a fixed Y.
		D3DXMatrixScaling(&scaling, pocionScale, pocionScale, pocionScale);
		D3DXMatrixTranslation(&translation, tableXZ.x,
			tableY + mesa->GetModelHeight() * mesaScale, tableXZ.y);
		pocion->UpdateScene(scaling * translation);

		PlaceModel(pablito, 378.0f, 399.0f, pablitoScale, 180.0f);
		PlaceModel(tree, 360.0f, 438.0f, treeScale);
		PlaceModel(fence, 403.0f, 427.0f, fenceScale);
		PlaceModel(furnace, 394.0f, 412.0f, furnaceScale);
		PlaceModel(pipe, 399.0f, 407.0f, pipeScale);
		PlaceModel(corner_pipe, 402.0f, 415.0f, cornerPipeScale);
	}

	void UpdateScene()
	{
		const auto now = std::chrono::steady_clock::now();
		frameSeconds = std::chrono::duration<float>(now - lastFrameTime).count();
		lastFrameTime = now;
		// Evita avances bruscos al pausar o arrastrar la ventana.
		if (frameSeconds > 0.05f)
			frameSeconds = 0.05f;

		LastBoundingPosition = BoundingSphereTranslation;
		BoundingSphereTranslation += DummyBoundyEvents;

	
		rotPerFrame += 0.1f;

		D3DXMATRIX rotMatY, northMat, westMat, eastMat, southMat, scale, rotMatX,MatAcomodo;

		D3DXMATRIX Bounding1, Bounding2, BoundinMatTranslation;

		D3DXMatrixRotationY(&rotMatY, D3DXToRadian(rotPerFrame));
		D3DXMatrixTranslation(&MatAcomodo, 50, 3, 50);
		D3DXMatrixTranslation(&northMat, 0, 3, 0);
		D3DXMatrixTranslation(&southMat, 0, -3, 0);
		D3DXMatrixTranslation(&eastMat, 3, 0, 0);
		D3DXMatrixTranslation(&westMat, -3, 0, 0);
		D3DXMatrixTranslation(&Bounding1, 3, 6, 0);
		D3DXMatrixTranslation(&Bounding2, -3, 6, 0);
		D3DXMatrixTranslation(&BoundinMatTranslation, BoundingSphereTranslation.x, BoundingSphereTranslation.y, 0);
		D3DXMatrixScaling(&scale, 0.03f, 0.03f, 0.03f);

		cubo->UpdateScene(rotMatY * MatAcomodo);
		esfera->UpdateScene(rotMatY * northMat * MatAcomodo);
		toroide->UpdateScene(rotMatY * eastMat* MatAcomodo);
		cilindro->UpdateScene(rotMatY * eastMat * northMat* MatAcomodo);
		cono->UpdateScene(rotMatY * westMat* MatAcomodo);
		
		billboard->UpdateScene(westMat * northMat* MatAcomodo);
		

		D3DXMATRIX skyRotMat, terrMat;
		D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
		skydomo->UpdateScene(skyRotMat);


		D3DXMatrixTranslation(&terrMat, terrainSettings.position.x,
			terrainSettings.position.y, terrainSettings.position.z);
		terreno->UpdateScene(terrMat);
		UpdateImportedModels();

		float groundHeight = terreno->getTerrainCollision(camara->getXZ()) + 3.0f;

		if (isJumping)
		{
			const float gravity = 12.0f; // Unidades por segundo al cuadrado.
			jumpOffset += jumpVelocity * frameSeconds - 0.5f * gravity * frameSeconds * frameSeconds;
			jumpVelocity -= gravity * frameSeconds;

			if (jumpOffset <= 0.0f)
			{
				jumpOffset = 0.0f;
				jumpVelocity = 0.0f;
				isJumping = false;
			}
		}

		camara->Position.y = groundHeight + jumpOffset;

		BoundingSphere1->UpdateScene(Bounding1 * MatAcomodo * BoundinMatTranslation);
		BoundingSphere2->UpdateScene(Bounding2 * MatAcomodo);


		if (CheckBoundingSphereCollision(BoundingSphere1->PrimitiveBoundings->Radius,
			BoundingSphere1->PrimitiveBoundings->PositionM, BoundingSphere1->getWorldMatrix(),
			BoundingSphere2->PrimitiveBoundings->Radius,
			BoundingSphere2->PrimitiveBoundings->PositionM, BoundingSphere2->getWorldMatrix())){

			BoundingSphereTranslation = LastBoundingPosition;
		}


		DummyBoundyEvents.x = 0;
		DummyBoundyEvents.y = 0;
	}

	void DrawScene()
	{
		//Clear our backbuffer to the updated color
		D3DXCOLOR bgColor(0.0f, 0.0f, 0.25f, 1.0f);
		d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
		//Refresh the Depth/Stencil view
		d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		cubo->DrawScene(camara);
		esfera->DrawScene(camara);
		toroide->DrawScene(camara);
		cilindro->DrawScene(camara);
		
		cono->DrawScene(camara);
		billboard->DrawScene(camara);

		

		casa->DrawScene(camara);
		mesa->DrawScene(camara);
		pocion->DrawScene(camara);
		pablito->DrawScene(camara);
		tree->DrawScene(camara);
		fence->DrawScene(camara);
		furnace->DrawScene(camara);
		pipe->DrawScene(camara);
		corner_pipe->DrawScene(camara);

		terreno->DrawScene(camara);
		skydomo->DrawScene(skyCamara);

		BoundingSphere1->DrawScene(camara);
		BoundingSphere2->DrawScene(camara);



		//Present the backbuffer to the screen
		SwapChain->Present(0, 0);
	}

	
	bool CheckBoundingSphereCollision(float firstSphereRadius,
		D3DXVECTOR3 firstSpherePos,
		D3DXMATRIX firstObjWorldSpace,
		float secondSphereRadius,
		D3DXVECTOR3 secondSpherePos,
		D3DXMATRIX secondObjWorldSpace)
	{
		//Declare local variables
		D3DXVECTOR3 world_1 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 world_2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		float objectsDistance = 0.0f;

		//Transform the objects world space to objects REAL center in world space
		D3DXVec3TransformCoord(&world_1, &firstSpherePos, &firstObjWorldSpace);
		D3DXVec3TransformCoord(&world_2, &secondSpherePos, &secondObjWorldSpace);

		// Create Vector to verify distance
		world_1 -= world_2;

		//Get the distance between the two objects
		objectsDistance = D3DXVec3Length(&world_1);
		//objectsDistance = XMVectorGetX(XMVector3Length(world_1 - world_2));

		//If the distance between the two objects is less than the sum of their bounding spheres...
		if (objectsDistance <= (firstSphereRadius + secondSphereRadius))
			return true;

		//If the bounding spheres are not colliding, return false
		return false;
	}
	
	
};

#endif
