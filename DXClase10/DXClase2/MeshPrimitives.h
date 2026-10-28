#ifndef _MeshPrimitives_
#define _MeshPrimitives_

#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>

struct TerrainSettings
{
	float width, depth;       // Physical extent, independent of image resolution.
	float maxHeight;          // Height of white (255), before verticalScale.
	float verticalScale;
	float horizontalScale;
	D3DXVECTOR3 position;

	TerrainSettings() : width(255.0f), depth(255.0f), maxHeight(17.0f),
		verticalScale(1.0f), horizontalScale(1.0f), position(0, 0, 0) {}
};

class MPrimitives {
private:
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;
	ID3D11Buffer* cbBuffer;
	ID3D11Buffer* cbBufferPS;

	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;

	ID3D11ShaderResourceView* Texture;
	ID3D11ShaderResourceView* Texture2;
	ID3D11ShaderResourceView* NormalTexture;
	ID3D11SamplerState* TexSamplerState;

	//Bitmap
	ID3D11Texture2D* renderTargetTextureGUI;

	D3DXMATRIX mapView;
	D3DXMATRIX mapProjection;

	UINT numVertex;
	UINT numIndex;

	D3DXMATRIX World;

	float Size;

	int Stacks;
	int Slices;
	int tWidth, tHeight;
	float Radius;
	float radius;
	float Height;
	float Width;



	int CantOfSections;
	int TEXTURE_REPEAT;
	bool faceToCam;

	//The constant buffer Structure
	struct ConstantBuffer
	{
		D3DXMATRIX WorldViewProjection;
		D3DXMATRIX World;
		D3DXVECTOR3 CameraPos;
		float MaxH;
	} consBuff;

	struct ConstantBufferPS
	{
		Light  light;
	} consBuffPS;

	//The vertex Structure
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 tex;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 biTangent;
	} ;

	std::vector<SimpleVertex> terrainVertices;
	TerrainSettings terrainSettings;
	bool terrainLoaded = false;
	bool modelLoaded = false;
	float modelHeight = 0.0f;

	

public:

	struct SphereBoundingInfo{
		D3DXVECTOR3 PositionM;
		float Radius;
	};

	SphereBoundingInfo* PrimitiveBoundings;

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Size, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Size = Size;
		light.specularPower = 2;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createCube();
		
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Stacks, int Slices, float Radius, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		PrimitiveBoundings = new SphereBoundingInfo();
		PrimitiveBoundings->PositionM = D3DXVECTOR3(0, 0, 0);
		PrimitiveBoundings->Radius = Radius;
		this->Stacks = Stacks;
		this->Slices = Slices;
		this->Radius = Radius;

		int numVertexInTwoTriangles = 6;
		numVertex = Slices * Stacks;
		numIndex = Slices * Stacks * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		
		createSphere();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Stacks, int Slices, float Radius, float radius, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Stacks = Stacks;
		this->Slices = Slices;
		this->Radius = Radius;
		this->radius = radius;

		int numVertexInTwoTriangles = 6;
		numVertex = Slices * Stacks;
		numIndex = Slices * Stacks * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createTorus();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Slices, float Radius, float radius, float Height, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Slices = Slices;
		this->Radius = Radius;
		this->radius = radius;
		this->Height = Height;

		int numVertexInTwoTriangles = 6;
		CantOfSections = 4;
		numVertex = Slices * CantOfSections;
		numIndex = Slices * CantOfSections * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createCilinder();
	}

	MPrimitives(ID3D11Device* device, ID3D11DeviceContext* context,
		const TerrainSettings& settings, LPCWSTR texturePath, LPCWSTR textureNormalPath,
		const char* heightMapFilename, int ShaderType)
	{
		terrainSettings = settings;
		faceToCam = false;
		VertexBuffer = NULL;
		IndexBuffer = NULL;
		numVertex = numIndex = 0;
		TEXTURE_REPEAT = 64; // More repetitions make terrain texture details smaller.
		InitScene(device, context, texturePath, textureNormalPath, ShaderType);
		try
		{
			LoadHeightMap(heightMapFilename);
			terrainLoaded = true;
		}
		catch (const std::exception& error)
		{
			terrainVertices.clear();
			numVertex = numIndex = 0;
			MessageBoxA(NULL, error.what(), "Error al cargar el terreno", MB_OK | MB_ICONERROR);
		}
	}

	bool IsTerrainLoaded() const { return terrainLoaded; }

	// Imported meshes use the converted Vertex Count/Data format of the template.
	MPrimitives(ID3D11Device* device, ID3D11DeviceContext* context, const char* modelObjectPath,
		LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		faceToCam = false;
		VertexBuffer = NULL;
		IndexBuffer = NULL;
		Texture = NormalTexture = Texture2 = NULL;
		numVertex = numIndex = 0;
		consBuff.MaxH = 1.0f;
		InitScene(device, context, texturePath, textureNormalPath, ShaderType);
		modelLoaded = LoadModel(modelObjectPath) && Texture != NULL;
		if (!modelLoaded)
			MessageBoxA(NULL, modelObjectPath, "No se pudo cargar el modelo o su textura", MB_OK | MB_ICONERROR);
	}

	bool IsModelLoaded() const { return modelLoaded; }
	float GetModelHeight() const { return modelHeight; }

	// Only used to release imported models before shutting down the D3D device.
	void ReleaseModelResources()
	{
		if (VertexBuffer) { VertexBuffer->Release(); VertexBuffer = NULL; }
		if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = NULL; }
		if (VS) { VS->Release(); VS = NULL; }
		if (PS) { PS->Release(); PS = NULL; }
		if (VS_Buffer) { VS_Buffer->Release(); VS_Buffer = NULL; }
		if (PS_Buffer) { PS_Buffer->Release(); PS_Buffer = NULL; }
		if (vertLayout) { vertLayout->Release(); vertLayout = NULL; }
		if (cbBuffer) { cbBuffer->Release(); cbBuffer = NULL; }
		if (cbBufferPS) { cbBufferPS->Release(); cbBufferPS = NULL; }
		if (Texture) { Texture->Release(); Texture = NULL; }
		if (Texture2) { Texture2->Release(); Texture2 = NULL; }
		if (NormalTexture) { NormalTexture->Release(); NormalTexture = NULL; }
		if (TexSamplerState) { TexSamplerState->Release(); TexSamplerState = NULL; }
		modelLoaded = false;
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Height, float Width, LPCWSTR texturePath)
	{
		this->Height = Height;
		this->Width = Width;
		faceToCam = true;

		InitScene(d3d11Device, d3d11DevCon, texturePath, L"", ShaderDiffuse);
		createBillboard();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Height, float Width, LPCWSTR texturePath, Camara* camara){


		
		this->Height = Height;
		this->Width = Width;

		InitScene(d3d11Device, d3d11DevCon, texturePath, L"", ShaderGUI);
		createBitMap(camara);
	}

	bool InitScene(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		//Inicializacion de variables
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		//The input-layout description
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);
		LPCWSTR pixelShaderFile = L"pixelShader.ps";

		switch (ShaderType)
		{
		case ShaderSpecular:
			pixelShaderFile = L"pixelShaderSpecular.ps";
			break;
		case ShaderNormalmap:
			pixelShaderFile = L"pixelShaderNormalMap.ps";
			break;
		case ShaderGUI:
			pixelShaderFile = L"pixelShaderGUI.ps";
			break;
		case ShaderMultitextura:
			pixelShaderFile = L"pixelShaderMultitextura.ps";
			break;
		default:
			break;
		}

		

		//Compile Shaders from shader file
		D3DX11CompileFromFile(L"vertexShader.vs", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
		D3DX11CompileFromFile(pixelShaderFile, 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

		//Create the Shader Objects
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),
			PS_Buffer->GetBufferSize(), NULL, &PS);

		//Create the Input Layout
		d3d11Device->CreateInputLayout(layout, numElements,
			VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Set the Input Layout
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Set Primitive Topology
		d3d11DevCon->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



		//Create the buffer to send to the cbuffer in effect file
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(ConstantBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbd, NULL,
			&cbBuffer);

		// Set the texture
		D3DX11CreateShaderResourceViewFromFile(d3d11Device, texturePath,
			NULL, NULL, &Texture, NULL);

		// Set the normal texture
		D3DX11CreateShaderResourceViewFromFile(d3d11Device, textureNormalPath,
			NULL, NULL, &NormalTexture, NULL);

		D3DX11CreateShaderResourceViewFromFile(d3d11Device, L"Texturas/rock.jpg",
			NULL, NULL, &Texture2, NULL);

		// Describe the Sample State
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		//Create the Sample State
		d3d11Device->CreateSamplerState(&sampDesc, &TexSamplerState);

		//Create the pixel constant buffer
		D3D11_BUFFER_DESC cbbdps;
		ZeroMemory(&cbbdps, sizeof(D3D11_BUFFER_DESC));

		cbbdps.Usage = D3D11_USAGE_DEFAULT;
		cbbdps.ByteWidth = sizeof(ConstantBufferPS);
		cbbdps.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbdps.CPUAccessFlags = 0;
		cbbdps.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbdps, NULL,
			&cbBufferPS);

		return true;
	}

	void createBillboard()
	{
		Height = Height / 2.0f;
		Width = Width / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			{ D3DXVECTOR3(-Width, -Height, 0), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-Width, Height, 0), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(Width, Height, 0), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(Width, -Height, 0), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		numIndex = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createCube()
	{
		float cubeSize = Size / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			// Front Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Back Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },

			// Top Face
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Bottom Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },

			// Left Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Right Face
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			// Front Face
			0, 1, 2,
			0, 2, 3,

			// Back Face
			4, 5, 6,
			4, 6, 7,

			// Top Face
			8, 9, 10,
			8, 10, 11,

			// Bottom Face
			12, 13, 14,
			12, 14, 15,

			// Left Face
			16, 17, 18,
			16, 18, 19,

			// Right Face
			20, 21, 22,
			20, 22, 23
		};
		numIndex = ARRAYSIZE(indices);
		setTangentBinormal(vertex, indices, 3);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createSphere()
	{
		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		const float thetax = D3DX_PI;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < Stacks; i++)
		{
			float y = (float)i / (Stacks - 1);
			for (int j = 0; j < Slices; j++)
			{
				float x = (float)j / (Slices - 1);

				vertex[index].pos.x = Radius * sin(y*thetax) * cos(x*phix);
				vertex[index].pos.y = Radius * sin(y*thetax) * sin(x*phix);
				vertex[index].pos.z = Radius * cos(y*thetax);

				vertex[index].tex.x = x;
				vertex[index].tex.y = y;

				vertex[index].normal = vertex[index].pos;

				index++;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;

		for (int i = 0; i < Stacks - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1 = (Slices *   i) + j;
				int index2 = (Slices *   i) + (j + 1);
				int index3 = (Slices * (i + 1)) + j;
				int index4 = (Slices * (i + 1)) + (j + 1);

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index4;
				indices[index++] = index2;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &IndexBuffer);
	}

	void createTorus()
	{
		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		const float thetax = D3DX_PI;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < Stacks; i++)
		{
			float y = (float)i / (Stacks - 1);
			for (int j = 0; j < Slices; j++)
			{
				float x = (float)j / (Slices - 1);

				vertex[index].pos.x = (Radius + radius * cos(x*phix)) * cos(y*phix);
				vertex[index].pos.y = (Radius + radius * cos(x*phix)) * sin(y*phix);
				vertex[index].pos.z = radius * sin(x*phix);

				vertex[index].tex.x = x;
				vertex[index].tex.y = y;

				vertex[index].normal = vertex[index].pos;

				index++;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;

		for (int i = 0; i < Stacks - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1 = (Slices *   i) + j;
				int index2 = (Slices *   i) + (j + 1);
				int index3 = (Slices * (i + 1)) + j;
				int index4 = (Slices * (i + 1)) + (j + 1);

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index4;
				indices[index++] = index2;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &IndexBuffer);
	}

	void createCilinder()
	{
		//Create the vertex buffer
		int index = 0;
		const float fullCircle = D3DX_PI * 2.0;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < CantOfSections; i++)
		{
			for (int j = 0; j < Slices; j++)
			{
				float segment = (float)j / (Slices - 1);

				switch (i)
				{
				case 0:
					vertex[index].pos.x = radius * cos(segment*fullCircle);
					vertex[index].pos.y = -Height / 2;
					vertex[index].pos.z = radius * sin(segment*fullCircle);

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 1;
					break;
				case 1:
					vertex[index].pos.x = Radius * cos(segment*fullCircle);
					vertex[index].pos.y = Height / 2;
					vertex[index].pos.z = Radius * sin(segment*fullCircle);

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 0;
					break;
				case 2:
					vertex[index].pos.x = 0;
					vertex[index].pos.y = -Height / 2;
					vertex[index].pos.z = 0;

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 0;
					break;
				case 3:
					vertex[index].pos.x = 0;
					vertex[index].pos.y = Height / 2;
					vertex[index].pos.z = 0;

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 1;
					break;
				}
				vertex[index].normal = vertex[index].pos;
				index++;
			}
		}


		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;
		int indexBottom = Slices;

		for (int i = 0; i < CantOfSections - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1, index2, index3, index4;
				int indexPoint = Slices * (i + 1);

				switch (i)
				{
				case 0:
					index1 = j;
					index2 = j + indexBottom;
					index3 = j + indexBottom + 1;
					index4 = j + 1;
					break;
				case 1:
					index1 = j + 1;
					index2 = j + indexPoint;
					index3 = j + indexPoint + 1;
					index4 = j;

					break;
				case 2:
					index1 = j + indexBottom;
					index2 = j + indexPoint;
					index3 = j + indexPoint + 1;
					index4 = j + indexBottom + 1;

					break;
				}

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index2;
				indices[index++] = index3;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createBitMap(Camara *camara){

		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Textura del bitmap para GUI 

		// Inicialisamos la descripcion de la textura.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Llenamos la descripcion de la textura
		// Vamos a hacer que nuestra textura sea un cuadrado

		textureDesc.Width = Width / 2;
		textureDesc.Height = Height / 2;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// creamos la textura con el device
		d3d11Device->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureGUI);

		
		Height = Height / 2.0f;
		Width = Width / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			{ D3DXVECTOR3(-Width, -Height, 0), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-Width, Height, 0), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(Width, Height, 0), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(Width, -Height, 0), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		numIndex = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	

	// BMP BI_RGB, 24/32 bits. Image bottom edge maps to z=0, as in the template.
	void LoadHeightMap(const char* filename)
	{
		const TerrainSettings& cfg = terrainSettings;
		const float extentX = cfg.width * cfg.horizontalScale;
		const float extentZ = cfg.depth * cfg.horizontalScale;
		const float peak = cfg.maxHeight * cfg.verticalScale;
		if (!std::isfinite(extentX) || !std::isfinite(extentZ) || !std::isfinite(peak)
			|| cfg.width <= 0 || cfg.depth <= 0 || cfg.horizontalScale <= 0
			|| cfg.maxHeight < 0 || cfg.verticalScale < 0 || extentX <= 0 || extentZ <= 0
			|| !std::isfinite(cfg.position.x) || !std::isfinite(cfg.position.y)
			|| !std::isfinite(cfg.position.z))
			throw std::runtime_error("Parametros de terreno invalidos.");

		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file) throw std::runtime_error("No se pudo abrir el heightmap. Revise Texturas y el directorio de trabajo.");
		const std::streamoff fileSize = file.tellg();
		file.seekg(0);
		BITMAPFILEHEADER header = {};
		BITMAPINFOHEADER info = {};
		if (!file.read(reinterpret_cast<char*>(&header), sizeof(header))
			|| !file.read(reinterpret_cast<char*>(&info), sizeof(info)))
			throw std::runtime_error("Cabecera BMP incompleta.");
		if (header.bfType != 0x4d42 || info.biSize < sizeof(info) || info.biPlanes != 1
			|| info.biCompression != BI_RGB || (info.biBitCount != 24 && info.biBitCount != 32))
			throw std::runtime_error("Use un BMP sin compresion (BI_RGB) de 24 o 32 bits.");
		// Bound allocations and avoid signed overflow, including LONG_MIN heights.
		if (info.biWidth < 2 || info.biWidth > 1024 || info.biHeight == 0
			|| info.biHeight < -1024 || info.biHeight > 1024 || info.biHeight == 1 || info.biHeight == -1)
			throw std::runtime_error("El heightmap debe medir entre 2 y 1024 pixeles por eje.");
		tWidth = info.biWidth;
		tHeight = info.biHeight < 0 ? -info.biHeight : info.biHeight;
		const size_t bytesPerPixel = info.biBitCount / 8;
		const size_t rowStride = (tWidth * bytesPerPixel + 3) & ~size_t(3);
		const unsigned long long dataEnd = static_cast<unsigned long long>(header.bfOffBits)
			+ rowStride * tHeight;
		if (header.bfOffBits < sizeof(header) + static_cast<unsigned long long>(info.biSize)
			|| fileSize < 0 || dataEnd > static_cast<unsigned long long>(fileSize))
			throw std::runtime_error("Datos BMP truncados o desplazamiento invalido.");
		file.seekg(header.bfOffBits);
		terrainVertices.resize(static_cast<size_t>(tWidth) * tHeight);
		std::vector<unsigned char> row(rowStride);
		float maxY = 0.0f;
		for (int storedRow = 0; storedRow < tHeight; ++storedRow)
		{
			if (!file.read(reinterpret_cast<char*>(row.data()), rowStride))
				throw std::runtime_error("No se pudo leer una fila del heightmap.");
			const int z = info.biHeight > 0 ? storedRow : tHeight - 1 - storedRow;
			for (int x = 0; x < tWidth; ++x)
			{
				const unsigned char* pixel = &row[x * bytesPerPixel];
				// Grayscale is preserved exactly; colored BMPs use luminance (BGR).
				const float intensity = (0.2126f * pixel[2] + 0.7152f * pixel[1] + 0.0722f * pixel[0]) / 255.0f;
				SimpleVertex& v = terrainVertices[z * tWidth + x];
				v.pos = D3DXVECTOR3(extentX * x / (tWidth - 1), intensity * peak,
					extentZ * z / (tHeight - 1));
				v.tex = D3DXVECTOR2(float(TEXTURE_REPEAT) * x / (tWidth - 1),
					1.0f - float(TEXTURE_REPEAT) * z / (tHeight - 1));
				v.normal = D3DXVECTOR3(0, 0, 0);
				if (v.pos.y > maxY) maxY = v.pos.y;
			}
		}
		// The existing multitexture shader divides local height by MaxH.
		consBuff.MaxH = maxY > 0.0f ? maxY : 1.0f;
		createTerrain();
	}

	void createTerrain()
	{
		numVertex = static_cast<UINT>(terrainVertices.size());
		numIndex = (tWidth - 1) * (tHeight - 1) * 6;
		std::vector<DWORD> indices;
		indices.reserve(numIndex);
		for (int z = 0; z < tHeight - 1; ++z)
		{
			for (int x = 0; x < tWidth - 1; ++x)
			{
				const DWORD a = z * tWidth + x, b = a + 1;
				const DWORD c = a + tWidth, d = c + 1;
				const DWORD cell[] = { c, d, a, a, d, b };
				indices.insert(indices.end(), cell, cell + 6);
			}
		}
		// Area-weighted normals from the actual triangles, pointing upward.
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			SimpleVertex& a = terrainVertices[indices[i]];
			SimpleVertex& b = terrainVertices[indices[i + 1]];
			SimpleVertex& c = terrainVertices[indices[i + 2]];
			D3DXVECTOR3 e1 = b.pos - a.pos, e2 = c.pos - a.pos, normal;
			D3DXVec3Cross(&normal, &e1, &e2);
			a.normal += normal;
			b.normal += normal;
			c.normal += normal;
		}
		for (int z = 0; z < tHeight; ++z)
		{
			for (int x = 0; x < tWidth; ++x)
			{
				SimpleVertex& v = terrainVertices[z * tWidth + x];
				D3DXVec3Normalize(&v.normal, &v.normal);
				const int left = x > 0 ? x - 1 : x, right = x + 1 < tWidth ? x + 1 : x;
				v.tangent = terrainVertices[z * tWidth + right].pos - terrainVertices[z * tWidth + left].pos;
				v.tangent -= v.normal * D3DXVec3Dot(&v.normal, &v.tangent);
				D3DXVec3Normalize(&v.tangent, &v.tangent);
				D3DXVec3Cross(&v.biTangent, &v.normal, &v.tangent);
			}
		}
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(SimpleVertex) * numVertex;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = terrainVertices.data();
		if (FAILED(d3d11Device->CreateBuffer(&desc, &data, &VertexBuffer)))
			throw std::runtime_error("No se pudo crear el vertex buffer del terreno.");
		desc.ByteWidth = sizeof(DWORD) * numIndex;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		data.pSysMem = indices.data();
		if (FAILED(d3d11Device->CreateBuffer(&desc, &data, &IndexBuffer)))
		{
			VertexBuffer->Release();
			VertexBuffer = NULL;
			throw std::runtime_error("No se pudo crear el index buffer del terreno.");
		}
	}

	// Converted OBJ reader. A malformed file must fail rather than loop at EOF.
	bool LoadModel(const char* filename)
	{
		std::ifstream file(filename);
		std::string header;
		UINT count = 0;
		if (!std::getline(file, header) || header.find("Vertex Count:") != 0)
			return false;
		try
		{
			const unsigned long parsed = std::stoul(header.substr(13));
			if (parsed == 0 || parsed > 1000000 || parsed % 3 != 0) return false;
			count = static_cast<UINT>(parsed);
		}
		catch (const std::exception&) { return false; }
		bool foundData = false;
		while (std::getline(file, header))
		{
			if (header == "Data:") { foundData = true; break; }
		}
		if (!foundData) return false;
		std::vector<SimpleVertex> vertices(count);
		float minY = 0.0f, maxY = 0.0f;
		for (UINT i = 0; i < count; ++i)
		{
			SimpleVertex& v = vertices[i];
			if (!(file >> v.pos.x >> v.pos.y >> v.pos.z >> v.tex.x >> v.tex.y
				>> v.normal.x >> v.normal.y >> v.normal.z)) return false;
			if (!std::isfinite(v.pos.x) || !std::isfinite(v.pos.y) || !std::isfinite(v.pos.z)
				|| !std::isfinite(v.tex.x) || !std::isfinite(v.tex.y)
				|| !std::isfinite(v.normal.x) || !std::isfinite(v.normal.y)
				|| !std::isfinite(v.normal.z) || D3DXVec3LengthSq(&v.normal) < 0.000001f)
				return false;
			D3DXVec3Normalize(&v.normal, &v.normal);
			if (i == 0 || v.pos.y < minY) minY = v.pos.y;
			if (i == 0 || v.pos.y > maxY) maxY = v.pos.y;
		}
		numVertex = numIndex = count;
		modelHeight = maxY - minY;
		return createModelObject(vertices.data());
	}

	bool createModelObject(SimpleVertex* vertices)
	{
		std::vector<DWORD> indices(numIndex);
		for (UINT i = 0; i < numIndex; ++i) indices[i] = i;
		// Populate the complete vertex before upload, including degenerate atlas UVs.
		for (UINT i = 0; i < numVertex; i += 3)
		{
			const D3DXVECTOR3 edge1 = vertices[i + 1].pos - vertices[i].pos;
			const D3DXVECTOR3 edge2 = vertices[i + 2].pos - vertices[i].pos;
			const D3DXVECTOR2 uv1 = vertices[i + 1].tex - vertices[i].tex;
			const D3DXVECTOR2 uv2 = vertices[i + 2].tex - vertices[i].tex;
			const float determinant = uv1.x * uv2.y - uv2.x * uv1.y;
			for (UINT j = i; j < i + 3; ++j)
			{
				SimpleVertex& v = vertices[j];
				D3DXVECTOR3 tangent = edge1;
				if (fabsf(determinant) > 0.000001f)
					tangent = (edge1 * uv2.y - edge2 * uv1.y) / determinant;
				tangent -= v.normal * D3DXVec3Dot(&v.normal, &tangent);
				if (D3DXVec3LengthSq(&tangent) < 0.000001f)
				{
					D3DXVECTOR3 axis = fabsf(v.normal.y) < 0.9f ? D3DXVECTOR3(0, 1, 0) : D3DXVECTOR3(1, 0, 0);
					D3DXVec3Cross(&tangent, &axis, &v.normal);
				}
				D3DXVec3Normalize(&v.tangent, &tangent);
				D3DXVec3Cross(&v.biTangent, &v.normal, &v.tangent);
			}
		}
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(SimpleVertex) * numVertex;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = vertices;
		if (FAILED(d3d11Device->CreateBuffer(&desc, &data, &VertexBuffer))) return false;
		desc.ByteWidth = sizeof(DWORD) * numIndex;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		data.pSysMem = indices.data();
		if (FAILED(d3d11Device->CreateBuffer(&desc, &data, &IndexBuffer)))
		{
			VertexBuffer->Release();
			VertexBuffer = NULL;
			return false;
		}
		return true;
	}

	void setTangentBinormal(SimpleVertex *vertex, DWORD *indices, int base) 
	{
		int numTriangles = numIndex / base;
		for(int i = 0; i < numTriangles; ++i)
		{
			UINT i0 = indices[i*3+0];
			UINT i1 = indices[i*3+1];
			UINT i2 = indices[i*3+2];

			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i0].tangent, vertex[i0].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i1].tangent, vertex[i1].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i2].tangent, vertex[i2].biTangent);
		}
	}

	void CalculateTangentBinormal(SimpleVertex vertex1, SimpleVertex vertex2, SimpleVertex vertex3,
										  D3DXVECTOR3& tangent, D3DXVECTOR3& binormal)
	{

		D3DXVECTOR3 vector1, vector2;
		float tuVector[2], tvVector[2];
		float den;
		float length;


		// Calculate the two vectors for this face.
		vector1 = vertex2.pos - vertex1.pos;
		vector2 = vertex3.pos - vertex1.pos;

		// Calculate the tu and tv texture space vectors.
		tuVector[0] = vertex2.tex.x - vertex1.tex.x;
		tvVector[0] = vertex2.tex.y - vertex1.tex.y;

		tuVector[1] = vertex3.tex.x - vertex1.tex.x;
		tvVector[1] = vertex3.tex.y - vertex1.tex.y;

		// Calculate the denominator of the tangent/binormal equation.
		den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
		tangent = (tvVector[1] * vector1 - tvVector[0] * vector2) * den;
		binormal = (tuVector[0] * vector2 - tuVector[1] * vector1) * den;

		// Calculate the length of this normal.
		length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

		// Normalize the normal and then store it
		tangent /= length;

		// Calculate the length of this normal.
		length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

		// Normalize the normal and then store it
		binormal /= length;

		return;
	}

	void UpdateScene(D3DXMATRIX World)
	{
		this->World = World;
	}

	void DrawScene(Camara *camara)
	{
		if (faceToCam)
		{
			D3DXMATRIX rotMat;
			D3DXMatrixRotationY(&rotMat, camara->Yaw);
			World = rotMat * World;
		}
		D3DXMATRIX WorldViewProjection = World * camara->View * camara->Projection;
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &World);
		consBuff.CameraPos = camara->Position;
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		//Set the vertex and the index buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		d3d11DevCon->PSSetShaderResources(0, 1, &Texture);
		d3d11DevCon->PSSetShaderResources(1, 1, &NormalTexture );

		d3d11DevCon->PSSetShaderResources(2, 1, &Texture2);
		d3d11DevCon->PSSetSamplers(0, 1, &TexSamplerState);

		consBuffPS.light = light;
		d3d11DevCon->UpdateSubresource(cbBufferPS, 0, NULL, &consBuffPS, 0, 0);
		d3d11DevCon->PSSetConstantBuffers(0, 1, &cbBufferPS);

		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}

	void DrawBitmap(Camara *camara){
		
		// Dibujar la GUI
	
		/*Aqui se dibuja el bitmap, solo necesitamos un cuadrado donde poner la textura, por eso utilizamos los
		vertices de el billboard, lo estaremos dibujando directamente sobre la pantalla, recuerden que las 
		coordenadas 0,0 son el centro de la pantalla, los lados van de -1 a 1*/
		
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		//Cargamos el buffer de indices y vertices
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Seteamos la matriz WVP a una escala y traslacion dada apara acomodarla en pantalla

		D3DXMATRIX ScaleM, TranslationM;
		D3DXMatrixScaling(&ScaleM, .5, .5, 0);
		D3DXMatrixTranslation(&TranslationM, .5, -.5, .8);

		D3DXMATRIX WorldViewProjection = ScaleM * TranslationM;
		
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &World);
		consBuff.CameraPos = camara->Position;
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		d3d11DevCon->PSSetShaderResources(0, 1, &Texture);
		d3d11DevCon->PSSetShaderResources(1, 1, &NormalTexture);
		d3d11DevCon->PSSetSamplers(0, 1, &TexSamplerState);

		//Dibujamos el bitmap
		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}

	float getTerrainCollision(D3DXVECTOR2 XZ)
	{
		if (!terrainLoaded) return 0.0f;
		// Terrain world transform is translation only; scale is baked into the mesh.
		const float extentX = terrainSettings.width * terrainSettings.horizontalScale;
		const float extentZ = terrainSettings.depth * terrainSettings.horizontalScale;
		float gx = (XZ.x - World._41) * (tWidth - 1) / extentX;
		float gz = (XZ.y - World._43) * (tHeight - 1) / extentZ;
		if (gx < 0 || gz < 0 || gx > tWidth - 1 || gz > tHeight - 1)
			return World._42; // Outside the terrain: retain the template's base-plane behavior.
		int x = static_cast<int>(gx), z = static_cast<int>(gz);
		if (x == tWidth - 1) --x;
		if (z == tHeight - 1) --z;
		const float u = gx - x, v = gz - z;
		const size_t a = z * tWidth + x;
		const float h00 = terrainVertices[a].pos.y;
		const float h10 = terrainVertices[a + 1].pos.y;
		const float h01 = terrainVertices[a + tWidth].pos.y;
		const float h11 = terrainVertices[a + tWidth + 1].pos.y;
		// Same diagonal as the rendered mesh (a--d), avoiding bilinear mismatch.
		const float height = v >= u ? h00 + (h11 - h01) * u + (h01 - h00) * v
			: h00 + (h10 - h00) * u + (h11 - h10) * v;
		return height + World._42;
	}

	D3DXMATRIX getWorldMatrix(){
		return this->World;
	}

};

#endif