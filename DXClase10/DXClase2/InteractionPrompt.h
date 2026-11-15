#ifndef GC_INTERACTION_PROMPT_H
#define GC_INTERACTION_PROMPT_H

#include <cmath>
#include "Camara.h"
#include "ShaderCompiler.h"

// Shared last-pass marker: any interactable can supply its anchor and availability.
// It stays attached to a world position, while its size/color remain readable UI.
class InteractionPrompt
{
    ID3D11DeviceContext* context = NULL; // Borrowed from the scene.
    ID3D11VertexShader* vertexShader = NULL;
    ID3D11PixelShader* pixelShader = NULL;
    ID3D11Buffer* drawBuffer = NULL;
    ID3D11ShaderResourceView* texture = NULL;
    ID3D11SamplerState* sampler = NULL;
    ID3D11BlendState* alphaBlend = NULL;
    ID3D11DepthStencilState* overlayDepth = NULL;
    ID3D11RasterizerState* rasterizer = NULL;

    struct DrawData
    {
        D3DXVECTOR4 anchorClip;
        D3DXVECTOR4 sizeAndOffset;
    };

    template<typename T> static void Free(T*& resource)
    {
        if (resource) { resource->Release(); resource = NULL; }
    }

public:
    InteractionPrompt() = default;
    InteractionPrompt(const InteractionPrompt&) = delete;
    InteractionPrompt& operator=(const InteractionPrompt&) = delete;
    ~InteractionPrompt() { Release(); }

    bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LPCWSTR imagePath)
    {
        Release();
        context = deviceContext;
        ID3DBlob* vs = NULL;
        ID3DBlob* ps = NULL;
        bool compiled = CompileSceneShader(L"interactionPrompt.hlsl", "VS", "vs_4_0", &vs)
            && CompileSceneShader(L"interactionPrompt.hlsl", "PS", "ps_4_0", &ps);
        HRESULT result = E_FAIL;
        if (compiled)
        {
            result = device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &vertexShader);
            if (SUCCEEDED(result))
                result = device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pixelShader);
        }
        Free(vs);
        Free(ps);
        if (FAILED(result)) { Release(); return false; }

        D3D11_BUFFER_DESC buffer = {};
        buffer.ByteWidth = sizeof(DrawData);
        buffer.Usage = D3D11_USAGE_DEFAULT;
        buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        D3D11_SAMPLER_DESC sample = {};
        sample.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sample.AddressU = sample.AddressV = sample.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sample.MaxLOD = D3D11_FLOAT32_MAX;
        D3D11_BLEND_DESC blend = {};
        blend.RenderTarget[0].BlendEnable = TRUE;
        blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        D3D11_DEPTH_STENCIL_DESC depth = {};
        depth.DepthEnable = FALSE;
        depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depth.DepthFunc = D3D11_COMPARISON_ALWAYS;
        D3D11_RASTERIZER_DESC raster = {};
        raster.FillMode = D3D11_FILL_SOLID;
        raster.CullMode = D3D11_CULL_NONE;
        raster.DepthClipEnable = TRUE;

        if (FAILED(device->CreateBuffer(&buffer, NULL, &drawBuffer))
            || FAILED(D3DX11CreateShaderResourceViewFromFile(device, imagePath, NULL, NULL, &texture, NULL))
            || FAILED(device->CreateSamplerState(&sample, &sampler))
            || FAILED(device->CreateBlendState(&blend, &alphaBlend))
            || FAILED(device->CreateDepthStencilState(&depth, &overlayDepth))
            || FAILED(device->CreateRasterizerState(&raster, &rasterizer)))
        {
            Release();
            return false;
        }
        return true;
    }

    // Call after the world, before Present. False means no marker was drawn.
    bool Draw(const Camara& camera, const D3DXVECTOR3& worldAnchor, bool available, float pixels = 64.0f)
    {
        if (!available || !texture || !context || !std::isfinite(pixels) || pixels <= 0) return false;
        const D3DXMATRIX viewProjection = camera.View * camera.Projection;
        const D3DXVECTOR4 anchor(worldAnchor.x, worldAnchor.y, worldAnchor.z, 1);
        DrawData data;
        D3DXVec4Transform(&data.anchorClip, &anchor, &viewProjection);
        const D3DXVECTOR4& clip = data.anchorClip;
        if (!std::isfinite(clip.w) || !std::isfinite(clip.x) || !std::isfinite(clip.y)
            || !std::isfinite(clip.z) || clip.w <= 0 || clip.z < 0 || clip.z > clip.w) return false;
        D3D11_VIEWPORT viewport = {};
        UINT count = 1;
        context->RSGetViewports(&count, &viewport);
        if (count == 0 || viewport.Width <= 0 || viewport.Height <= 0) return false;
        // Lift the badge slightly off its anchor, with a consistent pixel size.
        data.sizeAndOffset = D3DXVECTOR4(2*pixels/viewport.Width, 2*pixels/viewport.Height,
            0, 24.0f/viewport.Height);
        const float screenX = clip.x/clip.w, screenY = clip.y/clip.w + data.sizeAndOffset.w;
        if (fabsf(screenX) > 1+data.sizeAndOffset.x*0.5f
            || fabsf(screenY) > 1+data.sizeAndOffset.y*0.5f) return false;

        ID3D11BlendState* oldBlend = NULL;
        ID3D11DepthStencilState* oldDepth = NULL;
        ID3D11RasterizerState* oldRaster = NULL;
        FLOAT factors[4];
        UINT mask, stencilRef;
        context->OMGetBlendState(&oldBlend, factors, &mask);
        context->OMGetDepthStencilState(&oldDepth, &stencilRef);
        context->RSGetState(&oldRaster);
        context->OMSetBlendState(alphaBlend, NULL, 0xffffffff);
        context->OMSetDepthStencilState(overlayDepth, 0);
        context->RSSetState(rasterizer);
        context->IASetInputLayout(NULL);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context->VSSetShader(vertexShader, NULL, 0);
        context->PSSetShader(pixelShader, NULL, 0);
        context->UpdateSubresource(drawBuffer, 0, NULL, &data, 0, 0);
        context->VSSetConstantBuffers(0, 1, &drawBuffer);
        context->PSSetShaderResources(0, 1, &texture);
        context->PSSetSamplers(0, 1, &sampler);
        context->Draw(4, 0);
        context->OMSetBlendState(oldBlend, factors, mask);
        context->OMSetDepthStencilState(oldDepth, stencilRef);
        context->RSSetState(oldRaster);
        Free(oldBlend);
        Free(oldDepth);
        Free(oldRaster);
        return true;
    }

    void Release()
    {
        Free(vertexShader);
        Free(pixelShader);
        Free(drawBuffer);
        Free(texture);
        Free(sampler);
        Free(alphaBlend);
        Free(overlayDepth);
        Free(rasterizer);
        context = NULL;
    }
};

#endif
