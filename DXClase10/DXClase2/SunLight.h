#ifndef GC_SUN_LIGHT_H
#define GC_SUN_LIGHT_H

#include "ShaderCompiler.h"
#include "DayNightCycle.h"

// Celestial lighting: the existing sun/shadow map, with a moon and night sky.
class SunLight
{
public:
    D3DXVECTOR3 directionToSun;
    D3DXVECTOR3 color;
    D3DXVECTOR3 ambient;
    float intensity;
    float shadowStrength;
    float shadowCoverage;
    float shadowDepth;
    float diskRadiusDegrees;
    float moonRadiusDegrees;
    float nightSkyExposure;
    enum { ShadowResolution = 2048 };

    SunLight() : directionToSun(-0.35f, 0.65f, 0.68f), color(1.0f, 0.95f, 0.84f),
        ambient(0.30f, 0.35f, 0.42f), intensity(0.95f), shadowStrength(1.0f),
        shadowCoverage(180.0f), shadowDepth(2200.0f), diskRadiusDegrees(0.65f),
        moonRadiusDegrees(1.15f), nightSkyExposure(1.0f), dynamicSky(false),
        shadowTexture(NULL), shadowDSV(NULL), shadowSRV(NULL), comparisonSampler(NULL),
        shadowRaster(NULL), shadowDepthState(NULL), shadowVS(NULL), shadowPS(NULL),
        shadowLayout(NULL), sunBuffer(NULL), moonTexture(NULL), nightSkyTexture(NULL), skySampler(NULL),
        savedTarget(NULL), savedDepth(NULL),
        savedRaster(NULL), savedDepthState(NULL), savedStencilRef(0)
    {
        ZeroMemory(&data, sizeof(data));
        D3DXMatrixIdentity(&viewProjection);
    }

    bool Init(ID3D11Device* device)
    {
        if (!CreateResources(device))
        {
            Release();
            return false;
        }
        return true;
    }

    void Update(ID3D11DeviceContext* context, const D3DXVECTOR3& focus, const DayNightState& state)
    {
        dynamicSky = true;
        skyState = state;
        directionToSun = state.sunDirection;
        color = state.lightColor;
        ambient = state.ambient;
        intensity = state.lightIntensity;
        Update(context, focus);
    }

    void Update(ID3D11DeviceContext* context, const D3DXVECTOR3& focus)
    {
        D3DXVECTOR3 toSun = dynamicSky ? skyState.lightDirection : directionToSun;
        if (D3DXVec3LengthSq(&toSun) < 0.000001f) toSun = D3DXVECTOR3(0, 1, 0);
        D3DXVec3Normalize(&toSun, &toSun);
        // The active light is the sun by day and the moon by night.
        light.direction = toSun;
        light.ambient = D3DXVECTOR4(ambient.x, ambient.y, ambient.z, 1);
        light.diffuse = D3DXVECTOR4(color.x * intensity, color.y * intensity, color.z * intensity, 1);

        const float coverage = shadowCoverage > 1.0f ? shadowCoverage : 1.0f;
        const float depth = shadowDepth > 10.0f ? shadowDepth : 10.0f;
        D3DXVECTOR3 up = fabsf(toSun.y) > 0.98f ? D3DXVECTOR3(0, 0, 1) : D3DXVECTOR3(0, 1, 0);
        const D3DXVECTOR3 origin(0, 0, 0);
        D3DXVECTOR3 eye = toSun * (depth * 0.5f);
        D3DXMATRIX basis, inverseBasis, view, projection;
        D3DXMatrixLookAtLH(&basis, &eye, &origin, &up);
        D3DXMatrixInverse(&inverseBasis, NULL, &basis);
        D3DXVECTOR3 snapped;
        D3DXVec3TransformCoord(&snapped, &focus, &basis);
        const float texelWorld = coverage / ShadowResolution;
        snapped.x = floorf(snapped.x / texelWorld + 0.5f) * texelWorld;
        snapped.y = floorf(snapped.y / texelWorld + 0.5f) * texelWorld;
        D3DXVec3TransformCoord(&snapped, &snapped, &inverseBasis);
        eye = snapped + toSun * (depth * 0.5f);
        D3DXMatrixLookAtLH(&view, &eye, &snapped, &up);
        D3DXMatrixOrthoLH(&projection, coverage, coverage, 0.1f, depth);
        viewProjection = view * projection;
        D3DXMatrixTranspose(&data.viewProjection, &viewProjection);
        D3DXVECTOR3 solarDirection = directionToSun;
        if (D3DXVec3LengthSq(&solarDirection) < 0.000001f) solarDirection = toSun;
        D3DXVec3Normalize(&solarDirection, &solarDirection);
        data.direction = D3DXVECTOR4(solarDirection.x, solarDirection.y, solarDirection.z, 0);
        data.color = dynamicSky ? D3DXVECTOR4(skyState.sunDiskColor.x, skyState.sunDiskColor.y,
            skyState.sunDiskColor.z, skyState.sunVisibility) : D3DXVECTOR4(color.x, color.y, color.z, 1);
        // texel size, receiver depth bias, normal offset (world units), shadow strength
        data.shadow = D3DXVECTOR4(1.0f / ShadowResolution, 0.00001f, 0.035f,
            shadowStrength < 0 ? 0 : (shadowStrength > 1 ? 1 : shadowStrength));
        const float disk = D3DXToRadian(diskRadiusDegrees);
        data.sky = D3DXVECTOR4(cosf(disk), cosf(disk * 1.25f), 256.0f, 0.18f);
        const D3DXVECTOR3 moon = dynamicSky ? skyState.moonDirection : -solarDirection;
        const D3DXVECTOR3 worldUp = fabsf(moon.y) > 0.98f ? D3DXVECTOR3(0, 0, 1) : D3DXVECTOR3(0, 1, 0);
        D3DXVECTOR3 right, moonUp;
        D3DXVec3Cross(&right, &worldUp, &moon);
        D3DXVec3Normalize(&right, &right);
        D3DXVec3Cross(&moonUp, &moon, &right);
        D3DXVec3Normalize(&moonUp, &moonUp);
        data.moonDirection = D3DXVECTOR4(moon.x, moon.y, moon.z, dynamicSky ? skyState.moonVisibility : 0);
        data.moonRight = D3DXVECTOR4(right.x, right.y, right.z, tanf(D3DXToRadian(moonRadiusDegrees)));
        data.moonUp = D3DXVECTOR4(moonUp.x, moonUp.y, moonUp.z, nightSkyExposure);
        data.night = dynamicSky ? D3DXVECTOR4(skyState.daylight, skyState.starsVisibility,
            skyState.twilight, skyState.starRotation) : D3DXVECTOR4(1, 0, 0, 0);
        context->UpdateSubresource(sunBuffer, 0, NULL, &data, 0, 0);
    }

    const D3DXMATRIX& GetViewProjection() const { return viewProjection; }

    void BeginShadowPass(ID3D11DeviceContext* context)
    {
        context->OMGetRenderTargets(1, &savedTarget, &savedDepth);
        context->RSGetState(&savedRaster);
        context->OMGetDepthStencilState(&savedDepthState, &savedStencilRef);
        savedViewportCount = 1;
        context->RSGetViewports(&savedViewportCount, &savedViewport);
        // A texture cannot be a depth target and a shader input at the same time.
        ID3D11ShaderResourceView* empty = NULL;
        context->PSSetShaderResources(3, 1, &empty);
        context->OMSetRenderTargets(0, NULL, shadowDSV);
        context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
        D3D11_VIEWPORT viewport = {};
        viewport.Width = viewport.Height = static_cast<float>(ShadowResolution);
        viewport.MaxDepth = 1.0f;
        context->RSSetViewports(1, &viewport);
        context->RSSetState(shadowRaster);
        context->OMSetDepthStencilState(shadowDepthState, 0);
        context->IASetInputLayout(shadowLayout);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(shadowVS, NULL, 0);
        context->PSSetShader(shadowPS, NULL, 0);
    }

    void EndShadowPass(ID3D11DeviceContext* context)
    {
        context->OMSetRenderTargets(1, &savedTarget, savedDepth);
        context->RSSetViewports(savedViewportCount, &savedViewport);
        context->RSSetState(savedRaster);
        context->OMSetDepthStencilState(savedDepthState, savedStencilRef);
        Free(savedTarget); Free(savedDepth); Free(savedRaster); Free(savedDepthState);
    }

    void Bind(ID3D11DeviceContext* context)
    {
        context->PSSetConstantBuffers(2, 1, &sunBuffer);
        context->PSSetShaderResources(3, 1, &shadowSRV);
        context->PSSetSamplers(1, 1, &comparisonSampler);
        context->PSSetShaderResources(4, 1, &nightSkyTexture);
        context->PSSetShaderResources(5, 1, &moonTexture);
        context->PSSetSamplers(2, 1, &skySampler);
    }

    void Release()
    {
        Free(shadowDSV); Free(shadowSRV); Free(shadowTexture); Free(comparisonSampler);
        Free(shadowRaster); Free(shadowDepthState); Free(shadowVS); Free(shadowPS);
        Free(shadowLayout); Free(sunBuffer);
        Free(moonTexture); Free(nightSkyTexture); Free(skySampler);
    }

private:
    struct Data
    {
        D3DXMATRIX viewProjection;
        D3DXVECTOR4 direction, color, shadow, sky;
        D3DXVECTOR4 moonDirection, moonRight, moonUp, night;
    } data;
    bool dynamicSky;
    DayNightState skyState;
    D3DXMATRIX viewProjection;
    ID3D11Texture2D* shadowTexture;
    ID3D11DepthStencilView* shadowDSV;
    ID3D11ShaderResourceView* shadowSRV;
    ID3D11SamplerState* comparisonSampler;
    ID3D11RasterizerState* shadowRaster;
    ID3D11DepthStencilState* shadowDepthState;
    ID3D11VertexShader* shadowVS;
    ID3D11PixelShader* shadowPS;
    ID3D11InputLayout* shadowLayout;
    ID3D11Buffer* sunBuffer;
    ID3D11ShaderResourceView* moonTexture;
    ID3D11ShaderResourceView* nightSkyTexture;
    ID3D11SamplerState* skySampler;
    ID3D11RenderTargetView* savedTarget;
    ID3D11DepthStencilView* savedDepth;
    ID3D11RasterizerState* savedRaster;
    ID3D11DepthStencilState* savedDepthState;
    UINT savedStencilRef, savedViewportCount;
    D3D11_VIEWPORT savedViewport;

    template<class T> static void Free(T*& value) { if (value) { value->Release(); value = NULL; } }

    bool CreateResources(ID3D11Device* device)
    {
        D3D11_TEXTURE2D_DESC texture = {};
        texture.Width = texture.Height = ShadowResolution;
        texture.MipLevels = texture.ArraySize = texture.SampleDesc.Count = 1;
        texture.Format = DXGI_FORMAT_R32_TYPELESS;
        texture.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(device->CreateTexture2D(&texture, NULL, &shadowTexture))) return false;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if (FAILED(device->CreateDepthStencilView(shadowTexture, &dsv, &shadowDSV))) return false;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
        srv.Format = DXGI_FORMAT_R32_FLOAT;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = 1;
        if (FAILED(device->CreateShaderResourceView(shadowTexture, &srv, &shadowSRV))) return false;
        D3D11_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        sampler.BorderColor[0] = sampler.BorderColor[1] = sampler.BorderColor[2] = sampler.BorderColor[3] = 1;
        sampler.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        sampler.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device->CreateSamplerState(&sampler, &comparisonSampler))) return false;
        D3D11_RASTERIZER_DESC raster = {};
        raster.FillMode = D3D11_FILL_SOLID;
        raster.CullMode = D3D11_CULL_NONE;
        raster.DepthClipEnable = TRUE;
        raster.DepthBias = 100;
        // The 3x3 comparison filter reaches 1.5 texels; cover that depth slope.
        raster.SlopeScaledDepthBias = 2.0f;
        if (FAILED(device->CreateRasterizerState(&raster, &shadowRaster))) return false;
        D3D11_DEPTH_STENCIL_DESC depth = {};
        depth.DepthEnable = TRUE;
        depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(device->CreateDepthStencilState(&depth, &shadowDepthState))) return false;
        D3D11_BUFFER_DESC buffer = {};
        buffer.ByteWidth = sizeof(Data);
        buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->CreateBuffer(&buffer, NULL, &sunBuffer))) return false;

        ID3DBlob* vs = NULL;
        ID3DBlob* ps = NULL;
        if (!CompileSceneShader(L"sunShadow.hlsl", "VS", "vs_4_0", &vs)) return false;
        if (!CompileSceneShader(L"sunShadow.hlsl", "PS", "ps_4_0", &ps)) { Free(vs); return false; }
        HRESULT result = device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &shadowVS);
        if (SUCCEEDED(result))
            result = device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &shadowPS);
        const D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        if (SUCCEEDED(result))
            result = device->CreateInputLayout(layout, ARRAYSIZE(layout), vs->GetBufferPointer(), vs->GetBufferSize(), &shadowLayout);
        Free(vs); Free(ps);
        if (FAILED(result)) return false;
        const LPCWSTR textures[] = { L"Texturas/skybox-night.png", L"Texturas/T_Moon.png" };
        ID3D11ShaderResourceView** views[] = { &nightSkyTexture, &moonTexture };
        for (int i = 0; i < 2; ++i)
        {
            if (FAILED(D3DX11CreateShaderResourceViewFromFile(device, textures[i], NULL, NULL, views[i], NULL)))
            {
                MessageBoxW(NULL, textures[i], L"No se pudo cargar la textura del cielo", MB_OK | MB_ICONERROR);
                return false;
            }
        }
        D3D11_SAMPLER_DESC skySampling = {};
        skySampling.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        skySampling.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        skySampling.AddressV = skySampling.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        skySampling.MaxLOD = D3D11_FLOAT32_MAX;
        return SUCCEEDED(device->CreateSamplerState(&skySampling, &skySampler));
    }
};

#endif
