#ifndef _TerrainShadows_
#define _TerrainShadows_

// Artistic contact shadows. The terrain pixel shader evaluates these ellipses
// at the actual surface position, so no floating decals or extra meshes are needed.
class TerrainShadows
{
public:
    enum { MaxShadows = 24 };

    float strength;
    float softness;
    float lengthScale;
    D3DXVECTOR2 direction;

    TerrainShadows() : buffer(NULL), strength(1.0f), softness(0.60f),
        lengthScale(0.55f), direction(-0.75f, -0.66f)
    {
        ZeroMemory(&data, sizeof(data));
        data.tint = D3DXVECTOR4(0.30f, 0.38f, 0.50f, 1.0f);
    }

    bool Init(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(Data);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        D3D11_SUBRESOURCE_DATA initial = {};
        initial.pSysMem = &data;
        return SUCCEEDED(device->CreateBuffer(&desc, &initial, &buffer));
    }

    void Update(ID3D11DeviceContext* context, MPrimitives* terrain,
        MPrimitives* const* models, UINT count)
    {
        data.count = 0;
        data.strength = Clamp(strength, 0.0f, 1.0f);
        const D3DXMATRIX ground = terrain->getWorldMatrix();
        D3DXVECTOR2 castDirection = direction;
        if (D3DXVec2LengthSq(&castDirection) < 0.000001f)
            castDirection = D3DXVECTOR2(-1, 0);
        D3DXVec2Normalize(&castDirection, &castDirection);

        for (UINT i = 0; i < count && data.count + 2 <= MaxShadows; ++i)
        {
            MPrimitives* model = models[i];
            if (!model || !model->IsModelLoaded()) continue;
            const D3DXMATRIX world = model->getWorldMatrix();
            const D3DXVECTOR3 size = model->GetModelSize();
            const D3DXVECTOR3 xAxis(world._11, world._12, world._13);
            const D3DXVECTOR3 yAxis(world._21, world._22, world._23);
            const D3DXVECTOR3 zAxis(world._31, world._32, world._33);
            const float scaleX = D3DXVec3Length(&xAxis);
            const float scaleZ = D3DXVec3Length(&zAxis);
            const float radiusX = Larger(0.35f, size.x * scaleX * 0.65f);
            const float radiusZ = Larger(0.35f, size.z * scaleZ * 0.65f);
            const float height = size.y * D3DXVec3Length(&yAxis);
            const float reach = height * Clamp(lengthScale, 0.0f, 2.0f);
            const float x = world._41 - ground._41;
            const float z = world._43 - ground._43;
            D3DXVECTOR2 footprintAxis(world._11, world._13);
            if (D3DXVec2LengthSq(&footprintAxis) < 0.000001f)
                footprintAxis = D3DXVECTOR2(1, 0);
            D3DXVec2Normalize(&footprintAxis, &footprintAxis);

            // Contact area under the footprint, plus a softer sunward projection.
            Add(x, z, radiusX, radiusZ, footprintAxis, 0.60f);
            Add(x + castDirection.x * reach * 0.45f,
                z + castDirection.y * reach * 0.45f,
                Larger(radiusX, radiusZ) * 0.85f + reach * 0.5f,
                Larger(0.35f, Smaller(radiusX, radiusZ) * 0.8f), castDirection, 0.45f);
        }
        context->UpdateSubresource(buffer, 0, NULL, &data, 0, 0);
    }

    void Bind(ID3D11DeviceContext* context)
    {
        context->PSSetConstantBuffers(1, 1, &buffer);
    }

    void Release()
    {
        if (buffer) { buffer->Release(); buffer = NULL; }
    }

private:
    struct Shadow
    {
        D3DXVECTOR4 centerRadius; // local terrain X/Z, ellipse radius X/Z
        D3DXVECTOR4 axisOpacity;  // normalized X/Z axis, opacity, edge softness
    };

    struct Data
    {
        int count;
        float strength;
        D3DXVECTOR2 padding;
        D3DXVECTOR4 tint;
        Shadow shadows[MaxShadows];
    } data;

    ID3D11Buffer* buffer;

    static float Larger(float a, float b) { return a > b ? a : b; }
    static float Smaller(float a, float b) { return a < b ? a : b; }
    static float Clamp(float x, float low, float high) { return Smaller(high, Larger(low, x)); }

    void Add(float x, float z, float radiusX, float radiusZ,
        const D3DXVECTOR2& axis, float opacity)
    {
        Shadow& shadow = data.shadows[data.count++];
        shadow.centerRadius = D3DXVECTOR4(x, z, radiusX, radiusZ);
        shadow.axisOpacity = D3DXVECTOR4(axis.x, axis.y, opacity, Clamp(softness, 0.05f, 0.95f));
    }
};

#endif
