#ifndef _DXRender_
#define _DXRender_

#include <Xinput.h>
#include <chrono>
#include <cwchar>
#pragma comment(lib, "Xinput9_1_0.lib")

#include "Constants.h"

#include "Camara.h"
#include "Light.h"
#include "MeshPrimitives.h"
#include "SunLight.h"
#include "InteractionPrompt.h"
#include "Gameplay/AlchemyScene.h"
#include "Gameplay/AlchemyHud.h"

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
	SunLight sun;
	DayNightCycle dayNight;
	InteractionPrompt interactionPrompt;
	Alchemy::Game alchemy;
	Alchemy::Scene alchemyScene;
	Alchemy::Hud alchemyHud;
	Alchemy::Target interactionTarget;
	bool hasInteractionTarget=false,dropRequested=false,recipeRequested=false,restartRequested=false;
	bool windowFocused=false;
	bool fastForwardTime;
	int lastClockMinute;
	MPrimitives *casa;
	MPrimitives *mesa;
	MPrimitives *pocion;
	MPrimitives *pablito;
	MPrimitives *cloudVehicleMesh;
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
	HWND gameWindow;
	D3DXVECTOR3 playerPosition; // Feet, independent of either camera mode.
	float playerYaw;
	bool cameraPadWasDown;
	D3DXVECTOR3 cloudPosition; // Center of the vehicle; the rider stands on its top.
	D3DXVECTOR3 cloudSize;
	float cloudYaw;
	float cloudHoverHeight;
	float cloudSpeed;
	bool ridingCloud;
	bool cloudInteractionRequested;
	bool perspectiveBeforeCloud;
	int lastCloudHint;
	D3DXVECTOR3 requestedMovement;
	bool playerGrounded;

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
		terrainSettings.width = 384.0f;
		terrainSettings.depth = 384.0f;
		terrainSettings.maxHeight = 360.0f; // Precision heightmap's full range.
		terrainSettings.verticalScale = 1.0f;
		terrainSettings.horizontalScale = 3.0f;
		terrainSettings.position = D3DXVECTOR3(0, 0, 0);
		terrainSettings.highPrecisionHeightmap = true;
		terrainSettings.blendMapPath = L"Texturas/Pablito_Valley_Paths.png";
		casaScale = mesaScale = pocionScale = 4.0f;
		pablitoScale = treeScale = fenceScale = furnaceScale = 1.0f;
		pipeScale = cornerPipeScale = 1.0f;
		rotPerFrame = 0.0f;
		isJumping = false;
		jumpVelocity = 0.0f;
		jumpOffset = 0.0f;
		jumpWasDown = false;
		cameraPadWasDown = false;
		playerPosition = D3DXVECTOR3(384.0f, 0.0f, 380.0f);
		playerYaw = 0.0f;
		cloudVehicleMesh = NULL;
		cloudPosition = D3DXVECTOR3(384.0f, 0.0f, 384.0f);
		cloudSize = D3DXVECTOR3(4.0f, 0.6f, 5.0f);
		cloudYaw = 0.0f;
		cloudHoverHeight = 0.8f; // Clearance below the provisional gray platform.
		cloudSpeed = 28.0f; // Walking: 8; running: 16 world units per second.
		ridingCloud = cloudInteractionRequested = false;
		perspectiveBeforeCloud = true;
		lastCloudHint = -1;
		requestedMovement = D3DXVECTOR3(0,0,0);
		playerGrounded = true;
		camara = skyCamara = NULL;
		gameWindow = NULL;
		fastForwardTime = false;
		lastClockMinute = -1;
		DummyBoundyEvents = D3DXVECTOR2(0, 0);
		frameSeconds = 0.0f;
		lastFrameTime = std::chrono::steady_clock::now();
	}

	bool InitDevice(HWND hwnd, int Width, int Height)
	{
		gameWindow = hwnd;
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
		
		skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 2000.0f, L"Texturas/skybox.png", L"", ShaderSky);
		terreno = new MPrimitives(d3d11Device, d3d11DevCon, terrainSettings, L"Texturas/dirt_diff.png", L"Texturas/Meshpart1_diff.png", "Texturas/Pablito_Valley_513.bmp", ShaderMultitextura);

		if (!terreno->IsTerrainLoaded()) return false;
		cloudVehicleMesh = new MPrimitives(d3d11Device, d3d11DevCon,
			1.0f, L"", L"", ShaderDiffuse);
		if (!cloudVehicleMesh->SetSolidColor(165, 165, 165)) return false;

		// Three imported OBJ meshes, converted to the template's text format.
		casa = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/casa_pablo.txt",
			L"Models/PablitoPinwi/casa.png", L"", ShaderDiffuse);
		mesa = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/Mesa.txt",
			L"Models/PablitoPinwi/Mesa.png", L"", ShaderDiffuse);
		pocion = new MPrimitives(d3d11Device, d3d11DevCon, "Models/PablitoPinwi/Pocion.txt",
			L"Models/PablitoPinwi/Poci\u00f3n.png", L"", ShaderDiffuse);
		if (!casa->IsModelLoaded() || !mesa->IsModelLoaded() || !pocion->IsModelLoaded()) return false;

		// Additional models from origin/main. Recenter their exported origins at the base.
		pablito = new MPrimitives(d3d11Device, d3d11DevCon, "Models/pablitopinwi_textured.txt",
			L"Texturas/pablito_diffuse.png", L"", ShaderDiffuse, true);
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
		// Pablito is the controlled capsule, not an obstacle made from his own mesh.
		pablito->SetCollisionEnabled(false);

		BoundingSphere1 = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		BoundingSphere2 = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1.0f, L"Texturas/escudo.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		BoundingSphere1->SetCollisionEnabled(false);
		BoundingSphere2->SetCollisionEnabled(false);
		if (!sun.Init(d3d11Device)) return false;
		if (!interactionPrompt.Init(d3d11Device, d3d11DevCon, L"Texturas/Tecla-E.png")) return false;
		if (!alchemyHud.Init(d3d11Device,Width,Height)) return false;
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
		lastFrameTime = std::chrono::steady_clock::now();

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
		const bool gamepadConnected = GetForegroundWindow() == gameWindow
			&& XInputGetState(0, &gamepadState) == ERROR_SUCCESS;
		const WORD buttons = gamepadConnected ? gamepadState.Gamepad.wButtons : 0;
		fastForwardTime = (keyboardState[DIK_F7] & 0x80) != 0
			|| (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

		// Keyboard C is handled by WM_KEYDOWN so even short taps are received.
		// The gamepad toggles on the press edge, ignoring a held D-pad Up.
		const bool cameraPadDown = (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
		if (cameraPadDown && !cameraPadWasDown)
			ToggleCameraPerspective();
		cameraPadWasDown = cameraPadDown;
		// WM_KEYDOWN queues one interaction per E press, without key auto-repeat.
		ProcessGameplayActions();

		// Velocidades en unidades por segundo. Correr mientras se sostiene Shift o L3.
		const bool running = keyboardState[DIK_LSHIFT] || keyboardState[DIK_RSHIFT]
			|| (buttons & XINPUT_GAMEPAD_LEFT_THUMB);
		const float speed = (ridingCloud ? cloudSpeed : (running ? 16.0f : 8.0f)) * frameSeconds;

		// Un salto por pulsacion: Espacio o boton A del mando.
		const bool jumpDown = keyboardState[DIK_SPACE] || (buttons & XINPUT_GAMEPAD_A);
		if (jumpDown && !jumpWasDown && !isJumping && !ridingCloud)
		{
			isJumping = true;
			playerGrounded = false;
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

		camara->UpdateRotation(camYaw, camPitch);
		D3DXVECTOR3 movement = camara->Forward * moveBackForward + camara->Right * moveLeftRight;
		const float distance = D3DXVec3Length(&movement);
		if (distance > speed && distance > 0.0f)
			movement *= speed / distance;
		requestedMovement = movement;
		if(alchemy.state!=Alchemy::MatchState::Playing)requestedMovement=D3DXVECTOR3(0,0,0);
		if (ridingCloud)
		{
			playerYaw = cloudYaw;
		}
		else
		{
			if (!camara->IsThirdPerson())
				playerYaw = camara->Yaw;
			else if (distance > 0.00001f)
				playerYaw = atan2f(movement.x, movement.z);
		}
	}

	void ToggleCameraPerspective()
	{
		if (camara && !ridingCloud)
			camara->TogglePerspective();
	}

	void RequestCloudInteraction()
	{
		cloudInteractionRequested = true;
	}
	void RequestInteraction() {cloudInteractionRequested=true;}
	void RequestDrop() {dropRequested=true;}
	void RequestRecipe() {recipeRequested=true;}
	void RequestRestart() {restartRequested=true;}

	void RefreshInteractionTarget()
	{
		std::vector<Alchemy::Target> targets=ridingCloud?std::vector<Alchemy::Target>():alchemy.Candidates();
		if(alchemy.state==Alchemy::MatchState::Playing && (ridingCloud || CanMountCloudVehicle()))
			targets.emplace_back(Alchemy::TargetKind::Cloud,0,-10,
				Alchemy::Position(cloudPosition.x,cloudPosition.y+cloudSize.y*0.5f+1.65f,cloudPosition.z),ridingCloud?L"E - Bajar de la nube":L"E - Subir a la nube",4.5f);
		for(auto& target:targets)
		{
			if(target.kind==Alchemy::TargetKind::Cloud)continue;
			float fraction;
			target.visible=!SceneCollision::Raycast(playerPosition+D3DXVECTOR3(0,PlayerHeight()*0.7f,0),
				D3DXVECTOR3(target.position.x,target.position.y,target.position.z),fraction,alchemyScene.IgnoreFor(target));
		}
		const int selected=Alchemy::SelectTarget({playerPosition.x,playerPosition.y,playerPosition.z},
			{camara->Forward.x,0,camara->Forward.z},targets);
		hasInteractionTarget=selected>=0;
		if(hasInteractionTarget)interactionTarget=targets[selected];
	}

	void DropCarriedObject()
	{
		if(!alchemy.Carried())return;
		const float offset=static_cast<int>(alchemy.Carried()->type)<6?0.45f:0.0f;
		for(float angle:{0.0f,0.65f,-0.65f,1.4f,-1.4f,3.14f})
		{
			const float distance=ridingCloud?4.0f:2.1f;
			D3DXVECTOR3 point=playerPosition+D3DXVECTOR3(sinf(playerYaw+angle)*distance,0,cosf(playerYaw+angle)*distance);
			point.y=SupportHeight(point,PlayerHeight(),8.0f)+offset;
			float hit;
			if(SceneCollision::Raycast(playerPosition+D3DXVECTOR3(0,PlayerHeight()*0.7f,0),point,hit,ridingCloud?cloudVehicleMesh->GetCollider():NULL))continue;
			D3DXVECTOR3 bottom=point-D3DXVECTOR3(0,offset,0);
			if(!SceneCollision::CapsuleFree(bottom,0.6f,0.25f,ridingCloud?cloudVehicleMesh->GetCollider():NULL))continue;
			alchemy.Drop({point.x,point.y,point.z});return;
		}
		alchemy.message=L"No hay espacio libre para soltar aquí. Muévete un poco.";alchemy.messageSeconds=4;
	}

	void ProcessGameplayActions()
	{
		if(GetForegroundWindow()==gameWindow)
		{
			if(recipeRequested)alchemy.showRecipe=!alchemy.showRecipe;
			if(restartRequested && alchemy.state!=Alchemy::MatchState::Playing)
			{
				alchemy.Restart();ridingCloud=false;cloudPosition=D3DXVECTOR3(384,0,384);cloudYaw=0;UpdateCloudVehicle();
				playerPosition=D3DXVECTOR3(384,terreno->getTerrainCollision(D3DXVECTOR2(384,380)),380);
				playerGrounded=true;isJumping=false;jumpVelocity=jumpOffset=0;playerYaw=0;
			}
			RefreshInteractionTarget();
			if(cloudInteractionRequested && alchemy.state==Alchemy::MatchState::Playing)
			{
				if(ridingCloud)ToggleCloudVehicle();
				else if(hasInteractionTarget)
				{
					if(interactionTarget.kind==Alchemy::TargetKind::Cloud)ToggleCloudVehicle();
					else alchemy.Interact(interactionTarget);
				}
			}
			if(dropRequested)DropCarriedObject();
			if(cloudInteractionRequested || dropRequested || recipeRequested || restartRequested)alchemyHud.Invalidate();
		}
		cloudInteractionRequested=dropRequested=recipeRequested=restartRequested=false;
	}

	void UpdateCarriedObject()
	{
		D3DXVECTOR3 point;
		if(camara->IsThirdPerson()) point=playerPosition+D3DXVECTOR3(sinf(playerYaw)*0.8f+cosf(playerYaw)*1.8f,2.0f,cosf(playerYaw)*0.8f-sinf(playerYaw)*1.8f);
		else point=camara->Position+camara->Forward*2.5f+camara->Right*0.75f-D3DXVECTOR3(0,0.65f,0);
		alchemy.UpdateCarry({point.x,point.y,point.z},camara->IsThirdPerson()?playerYaw:camara->Yaw);
	}

	bool CanMountCloudVehicle() const
	{
		const float dx = playerPosition.x - cloudPosition.x;
		const float dz = playerPosition.z - cloudPosition.z;
		const float top = cloudPosition.y + cloudSize.y * 0.5f;
		if (ridingCloud || dx * dx + dz * dz > 4.5f * 4.5f
			|| fabsf(playerPosition.y - top) > 3.0f) return false;
		float hit;
		return !SceneCollision::Raycast(playerPosition + D3DXVECTOR3(0,PlayerHeight()*0.7f,0),
			cloudPosition + D3DXVECTOR3(0,cloudSize.y*0.5f,0),hit,cloudVehicleMesh->GetCollider());
	}

	void ToggleCloudVehicle()
	{
		if (!camara) return;
		if (ridingCloud)
		{
			D3DXVECTOR3 landing;
			if (!FindCloudExit(landing)) return;
			playerPosition = landing;
			ridingCloud = false;
			if (camara->IsThirdPerson() != perspectiveBeforeCloud)
				camara->TogglePerspective();
		}
		else
		{
			if (!CanMountCloudVehicle()) return;
			perspectiveBeforeCloud = camara->IsThirdPerson();
			ridingCloud = true;
			if (!camara->IsThirdPerson()) camara->TogglePerspective();
			playerYaw = cloudYaw;
			playerPosition = cloudPosition + D3DXVECTOR3(0, cloudSize.y * 0.5f, 0);
		}
		isJumping = false;
		jumpVelocity = jumpOffset = 0.0f;
		playerGrounded = true;
		lastCloudHint = -1;
	}

	void UpdateCloudVehicle()
	{
		// Sample its footprint, so slopes do not bury the edges of the platform.
		const float cosine = cosf(cloudYaw), sine = sinf(cloudYaw);
		float floor = terreno->getTerrainCollision(D3DXVECTOR2(cloudPosition.x, cloudPosition.z));
		for (int x = -1; x <= 1; ++x)
			for (int z = -1; z <= 1; ++z)
			{
				const float localX = x * cloudSize.x * 0.5f;
				const float localZ = z * cloudSize.z * 0.5f;
				const D3DXVECTOR2 sample(cloudPosition.x + localX * cosine + localZ * sine,
					cloudPosition.z - localX * sine + localZ * cosine);
				floor = fmaxf(floor, terreno->getTerrainCollision(sample));
			}
		cloudPosition.y = floor + cloudHoverHeight + cloudSize.y * 0.5f;
		D3DXMATRIX scaling, rotation, translation;
		D3DXMatrixScaling(&scaling, cloudSize.x, cloudSize.y, cloudSize.z);
		D3DXMatrixRotationY(&rotation, cloudYaw);
		D3DXMatrixTranslation(&translation, cloudPosition.x, cloudPosition.y, cloudPosition.z);
		cloudVehicleMesh->UpdateScene(scaling * rotation * translation);
		if (ridingCloud)
			playerPosition = cloudPosition + D3DXVECTOR3(0, cloudSize.y * 0.5f, 0);
	}

	float PlayerHeight() const { return pablito->GetModelHeight()*pablitoScale; }
	float PlayerRadius() const { return fmaxf(0.30f,fminf(0.65f,PlayerHeight()*0.18f)); }

	float SupportHeight(const D3DXVECTOR3& feet,float above,float below,
		const SceneCollision::MeshCollider* ignore=NULL) const
	{
		float support=terreno->getTerrainCollision(D3DXVECTOR2(feet.x,feet.z));
		const D3DXVECTOR3 start=feet+D3DXVECTOR3(0,above,0),end=feet-D3DXVECTOR3(0,below,0);
		float fraction;
		if(SceneCollision::Raycast(start,end,fraction,ignore,NULL,true))
			support=fmaxf(support,start.y+(end.y-start.y)*fraction);
		return support;
	}

	bool FindCloudExit(D3DXVECTOR3& landing) const
	{
		const D3DXVECTOR3 directions[]={D3DXVECTOR3(1,0,0),D3DXVECTOR3(-1,0,0),
			D3DXVECTOR3(0,0,-1),D3DXVECTOR3(0,0,1),D3DXVECTOR3(1,0,1),
			D3DXVECTOR3(-1,0,1),D3DXVECTOR3(1,0,-1),D3DXVECTOR3(-1,0,-1)};
		for(D3DXVECTOR3 direction:directions)
		{
			D3DXVec3Normalize(&direction,&direction);
			const float distance=fabsf(direction.x)*cloudSize.x*0.5f+fabsf(direction.z)*cloudSize.z*0.5f+PlayerRadius()+0.5f;
			D3DXVECTOR3 candidate=playerPosition+SceneCollision::RotateY(direction,cloudYaw)*distance;
			candidate.y=SupportHeight(candidate,0.3f,10.0f,cloudVehicleMesh->GetCollider());
			if(playerPosition.y-candidate.y>3.0f || candidate.y-playerPosition.y>0.35f) continue;
			if(!SceneCollision::CapsuleFree(candidate,PlayerHeight(),PlayerRadius(),cloudVehicleMesh->GetCollider())) continue;
			SceneCollision::Contacts contacts;
			D3DXVECTOR3 arrived=SceneCollision::MoveCapsule(playerPosition,candidate-playerPosition,PlayerHeight(),
				PlayerRadius(),contacts,cloudVehicleMesh->GetCollider());
			if(SceneCollision::Length(arrived-candidate)>0.1f) continue;
			landing=candidate;return true;
		}
		return false;
	}

	void MovePlayer(const D3DXVECTOR3& movement,float seconds)
	{
		const float radius=PlayerRadius(),height=PlayerHeight(),gravity=12.0f;
		D3DXVECTOR3 displacement=movement;
		displacement.y=jumpVelocity*seconds-0.5f*gravity*seconds*seconds;
		jumpVelocity-=gravity*seconds;
		const int steps=(std::max)(1,static_cast<int>(ceilf(SceneCollision::Length(displacement)/(radius*0.35f))));
		D3DXVECTOR3 delta=displacement/static_cast<float>(steps);
		bool grounded=playerGrounded;
		for(int i=0;i<steps;++i)
		{
			const D3DXVECTOR3 previous=playerPosition;
			D3DXVECTOR3 next=previous+delta;
			const float support=SupportHeight(next,0.35f,0.40f);
			if(grounded && delta.y<=0 && support>=previous.y-0.35f && support<=previous.y+0.35f)
			{
				D3DXVECTOR3 stepped(next.x,support,next.z);
				if(SceneCollision::CapsuleFree(stepped,height,radius)) next=stepped;
			}
			SceneCollision::Contacts contacts=SceneCollision::ResolveCapsule(next,height,radius,previous);
			const float terrainHeight=terreno->getTerrainCollision(D3DXVECTOR2(next.x,next.z));
			if(next.y<terrainHeight) next.y=terrainHeight;
			// A low ceiling plus terrain can leave no valid separation direction.
			// Keep the last safe step rather than forcing the capsule through either.
			if(!SceneCollision::CapsuleFree(next,height,radius)) next=previous;
			const float floor=SupportHeight(next,0.04f,0.08f);
			grounded=delta.y<=0 && (contacts.ground || fabsf(next.y-floor)<0.05f);
			if(grounded) {jumpVelocity=0;delta.y=0;}
			if(contacts.ceiling && delta.y>0) {jumpVelocity=0;delta.y=0;}
			playerPosition=next;
		}
		playerGrounded=grounded;
		isJumping=!grounded;
		jumpOffset=fmaxf(0,playerPosition.y-terreno->getTerrainCollision(D3DXVECTOR2(playerPosition.x,playerPosition.z)));
	}

	void MoveCloud(const D3DXVECTOR3& movement)
	{
		const float length=SceneCollision::Length(movement);
		float turn=length>0.00001f?atan2f(movement.x,movement.z)-cloudYaw:0;
		while(turn>D3DX_PI) turn-=2*D3DX_PI;
		while(turn< -D3DX_PI) turn+=2*D3DX_PI;
		const int steps=(std::max)(1,static_cast<int>(ceilf(fmaxf(length/0.18f,fabsf(turn)/0.12f))));
		const D3DXVECTOR3 delta=movement/static_cast<float>(steps);
		const float margin=SceneCollision::Length(cloudSize)*0.5f+0.5f;
		const D3DXVECTOR3 half(cloudSize.x*0.5f,(cloudSize.y+PlayerHeight())*0.5f,cloudSize.z*0.5f);
		for(int i=0;i<steps;++i)
		{
			const D3DXVECTOR3 previous=cloudPosition;
			const float oldYaw=cloudYaw;
			cloudPosition+=delta;cloudYaw+=turn/steps;
			cloudPosition.x=SceneCollision::Clamp(cloudPosition.x,terrainSettings.position.x+margin,
				terrainSettings.position.x+terrainSettings.width*terrainSettings.horizontalScale-margin);
			cloudPosition.z=SceneCollision::Clamp(cloudPosition.z,terrainSettings.position.z+margin,
				terrainSettings.position.z+terrainSettings.depth*terrainSettings.horizontalScale-margin);
			bool accepted=false;
			for(int pass=0;pass<4;++pass)
			{
				UpdateCloudVehicle();
				D3DXVECTOR3 center=cloudPosition+D3DXVECTOR3(0,PlayerHeight()*0.5f,0);
				const D3DXVECTOR3 desired=center;
				SceneCollision::Contacts hit=SceneCollision::ResolveBox(center,half,cloudYaw,
					previous+D3DXVECTOR3(0,PlayerHeight()*0.5f,0),cloudVehicleMesh->GetCollider());
				if(!hit.blocked) {accepted=true;break;}
				if(fabsf(center.y-desired.y)>0.05f) break;
				cloudPosition.x+=center.x-desired.x;cloudPosition.z+=center.z-desired.z;
			}
			if(!accepted) {cloudPosition=previous;cloudYaw=oldYaw;}
		}
		UpdateCloudVehicle();playerYaw=cloudYaw;
	}

	void ToggleDayNightPause()
	{
		dayNight.paused = !dayNight.paused;
		lastClockMinute = -1;
	}

	void UpdateClockTitle()
	{
		const int minute = static_cast<int>(dayNight.hour * 60.0) % (24 * 60);
		const int cloudHint = ridingCloud ? 2 : (CanMountCloudVehicle() ? 1 : 0);
		if (minute == lastClockMinute && cloudHint == lastCloudHint) return;
		lastClockMinute = minute;
		lastCloudHint = cloudHint;
		wchar_t title[192];
		swprintf_s(title, L"Pablito Pinwi | %02d:%02d%s%s", minute / 60, minute % 60,
			dayNight.paused ? L" | Tiempo pausado" : L"",
			cloudHint == 2 ? L" | Nube: WASD manejar, E bajar" :
			(cloudHint == 1 ? L" | E: subir a la nube" : L""));
		SetWindowTextW(gameWindow, title);
	}

	void ReleaseObjects()
	{
		alchemyHud.Release();alchemyScene.Release();
		interactionPrompt.Release();
		sun.Release();
		MPrimitives* models[] = { terreno,casa,mesa,pocion,pablito,cloudVehicleMesh,tree,fence,
			furnace,pipe,corner_pipe,cubo,esfera,toroide,cono,cilindro,billboard,skydomo,BoundingSphere1,BoundingSphere2 };
		for (MPrimitives* model : models)
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
		delete camara;
		delete skyCamara;
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

		//Set the Viewport
		d3d11DevCon->RSSetViewports(1, &viewport);

		// Spawn Pablito in the village; both views share his feet and look angles.
		D3DXMATRIX terrainWorld;
		D3DXMatrixTranslation(&terrainWorld, terrainSettings.position.x,
			terrainSettings.position.y, terrainSettings.position.z);
		terreno->UpdateScene(terrainWorld);
		playerPosition.y = terreno->getTerrainCollision(D3DXVECTOR2(playerPosition.x, playerPosition.z));
		UpdateCloudVehicle();
		UpdateImportedModels(); // Register final object transforms before the first input.
		if(!alchemyScene.Init(d3d11Device,d3d11DevCon,terreno,pocion,furnace,alchemy))return false;
		D3DXVECTOR3 Position = playerPosition;
		D3DXVECTOR3 Target = Position + D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		camara = new Camara(Position,Target,Up, Width, Height);
		skyCamara = new Camara(Position,Target,Up, Width, Height);
		camara->UpdateRotation(0.0f, D3DXToRadian(10.0f));
		UpdatePlayerCamera();
		sun.Update(d3d11DevCon, playerPosition, dayNight.GetState());
		UpdateClockTitle();
		RefreshInteractionTarget();

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
		// The existing potion mesh is now owned visually by the carry/bottling system.

		// The exported mesh faces +X. Rotate it to the player's horizontal heading.
		D3DXMatrixScaling(&scaling, pablitoScale, pablitoScale, pablitoScale);
		D3DXMatrixRotationY(&rotation, playerYaw - D3DX_PI * 0.5f);
		D3DXMatrixTranslation(&translation, playerPosition.x, playerPosition.y, playerPosition.z);
		pablito->UpdateScene(scaling * rotation * translation);
		PlaceModel(tree, 360.0f, 438.0f, treeScale);
		PlaceModel(fence, 403.0f, 427.0f, fenceScale);
		PlaceModel(furnace, 394.0f, 412.0f, furnaceScale);
		PlaceModel(pipe, 399.0f, 407.0f, pipeScale);
		PlaceModel(corner_pipe, 402.0f, 415.0f, cornerPipeScale);
	}

	void UpdatePlayerCamera()
	{
		if (ridingCloud && !camara->IsThirdPerson()) camara->TogglePerspective();
		const float playerHeight = pablito->GetModelHeight() * pablitoScale;
		const D3DXVECTOR3 eyes = playerPosition + D3DXVECTOR3(0, playerHeight * 0.86f, 0);
		const float followDistance = ridingCloud ? fmaxf(playerHeight * 2.5f, cloudSize.z * 2.0f) : playerHeight * 2.5f;
		camara->FollowPlayer(eyes, followDistance);
		if (camara->IsThirdPerson())
		{
			// Shorten the camera arm at hills, checking the whole path from the player.
			const D3DXVECTOR3 arm = camara->Position - eyes;
			float hit;
			if(SceneCollision::Raycast(eyes,camara->Position,hit,cloudVehicleMesh->GetCollider()))
			{
				camara->Position=eyes+arm*fmaxf(0,hit-0.3f/fmaxf(0.001f,SceneCollision::Length(arm)));
				camara->UpdateView();
			}
			const D3DXVECTOR3 safeArm=camara->Position-eyes;
			const int samples = 48;
			for (int i = 1; i <= samples; ++i)
			{
				const D3DXVECTOR3 probe = eyes + safeArm * (static_cast<float>(i) / samples);
				if (probe.y < terreno->getTerrainCollision(D3DXVECTOR2(probe.x, probe.z)) + 0.4f)
				{
					camara->Position = eyes + safeArm * (static_cast<float>(i - 1) / samples);
					camara->UpdateView();
					break;
				}
			}
		}
		skyCamara->setSkyCamara(camara);
	}

	void UpdateScene()
	{
		const auto now = std::chrono::steady_clock::now();
		const double elapsedSeconds = std::chrono::duration<double>(now - lastFrameTime).count();
		frameSeconds = static_cast<float>(elapsedSeconds);
		lastFrameTime = now;
		// Evita avances bruscos al pausar o arrastrar la ventana.
		if (frameSeconds > 0.05f)
			frameSeconds = 0.05f;
		// Input, player physics, model transforms and cameras use this same frame.
		windowFocused=GetForegroundWindow()==gameWindow;
		if(windowFocused)alchemy.Tick(static_cast<float>(elapsedSeconds));
		UpdateImportedModels();
		DetectInput();
		// The clock uses elapsed real time even when movement needs a capped step.
		dayNight.Advance(elapsedSeconds * (fastForwardTime ? 30.0 : 1.0));

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
		if(ridingCloud) MoveCloud(requestedMovement);
		else {MovePlayer(requestedMovement,frameSeconds);UpdateCloudVehicle();}
		UpdatePlayerCamera();
		UpdateImportedModels();
		sun.Update(d3d11DevCon, playerPosition, dayNight.GetState());
		UpdateClockTitle();
		UpdateCarriedObject();alchemyScene.Sync(alchemy);RefreshInteractionTarget();

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
		// Real silhouettes, including Pablito when his mesh is hidden in first person.
		MPrimitives* shadowCasters[] = { terreno, casa, mesa, pablito, cloudVehicleMesh, tree,
			fence, furnace, pipe, corner_pipe, cubo, esfera, toroide, cilindro, cono,
			BoundingSphere1, BoundingSphere2 };
		sun.BeginShadowPass(d3d11DevCon);
		for (MPrimitives* model : shadowCasters)
			model->DrawShadow(sun.GetViewProjection());
		alchemyScene.DrawShadow(alchemy,sun.GetViewProjection());
		sun.EndShadowPass(d3d11DevCon);
		sun.Bind(d3d11DevCon);
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
		alchemyScene.Draw(alchemy,camara);
		cloudVehicleMesh->DrawScene(camara);
		// Keep his world transform and shadow in first person, but hide the mesh.
		if (camara->IsThirdPerson())
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
		// Future interactables can use this same renderer with their own anchor/range.
		if(hasInteractionTarget)
			interactionPrompt.Draw(*camara,D3DXVECTOR3(interactionTarget.position.x,interactionTarget.position.y+0.5f,interactionTarget.position.z),true,42.0f);
		alchemyHud.Draw(d3d11Device,d3d11DevCon,interactionPrompt,alchemy,
			hasInteractionTarget?interactionTarget.label:L"",windowFocused,frameSeconds,hasInteractionTarget?&interactionTarget:nullptr);
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
