# Guía actualizada de capturas del segundo avance

En Visual Studio abre el archivo indicado y usa Ctrl+G para ir a la línea inicial.
Muestra el intervalo completo, el nombre del archivo y los números de línea.
Estos bloques ya existen en el proyecto; no necesitas pegarlos.
El ciclo dura 5 minutos y usa las texturas de luna y estrellas entregadas;
los controles F6/F7 y los ajustes están en `DXClase10/DXClase2/DIA_NOCHE.md`.

La escena contiene nueve modelos distintos: casa, mesa, poción, Pablito, árbol,
cerca, horno, tubo y codo. Pablito tiene su atlas propio `pablito_diffuse.png` y
UV separadas en `Models/pablitopinwi_textured.txt`. El cielo usa un shader propio
sin iluminación de superficies. El terreno mezcla pasto, tierra y roca y ahora
incluye iluminación direccional y sombras del ciclo solar y lunar.

## Tabla de código

| Página de referencia | Parte | Archivo | Líneas |
| --- | --- | --- | --- |
| 2 | Declaración del terreno | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:41) | 41–43 |
| 2 | Dimensiones, altura y posición | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:87) | 87–92 |
| 2 | Carga del heightmap y texturas | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:202) | 202–204 |
| 2 | Transformación del terreno | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:589) | 589–591 |
| 2 | Mezcla por altura y aplicación de sombras | [pixelShaderMultitextura.ps](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/pixelShaderMultitextura.ps:21) | 21–41 |
| 5 | Creación del cielo | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:201) | 201–201 |
| 5 | Orientación del cielo | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:584) | 584–586 |
| 5 | Cielo sin sombreado por luz | [pixelShaderSky.ps](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/pixelShaderSky.ps:2) | 2–65 |
| 7 | Declaración de modelos y escalas | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:48) | 48–60 |
| 7 | Escalas individuales | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:93) | 93–95 |
| 7 | Carga de los nueve modelos | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:207) | 207–229 |
| 8 | SRT y apoyo de modelos en el terreno | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:473) | 473–480 |
| 8 | Ubicación de casa, mesa, poción y modelos nuevos | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:483) | 483–514 |
| 9 | Dibujo de los nueve modelos | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:658) | 658–668 |
| 2 / extra | Lista de modelos que proyectan sombra | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:634) | 634–636 |
| 2 / extra | Configuración y proyección del sol | [SunLight.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/SunLight.h:58) | 58–111 |
| 2 / extra | Filtrado y bordes suaves de sombras | [SunLighting.hlsli](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/SunLighting.hlsli:29) | 29–47 |
| 2 / extra | Restauración de escena y enlace del sol | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:640) | 640–641 |
| 13 | Movimiento WASD | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:314) | 314–329 |
| 13 | Mouse y cámara | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:369) | 369–387 |
| 13 | Correr y saltar | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:293) | 293–304 |
| 14 | Detección del mando | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:278) | 278–281 |
| 14 | Sticks y zona muerta | [DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:348) | 348–365 |

## Capturas durante la ejecución

| Página | Qué mostrar |
| --- | --- |
| 1 | Portada con el juego, materia y equipo |
| 3 | Pasto `Meshpart1_diff.png`, tierra `dirt_diff.png`, roca `rock.jpg` y `PablitoPinwi_Heightmap_256.bmp` de Texturas/ |
| 4 | Terreno con las distintas elevaciones y sombras bajo los modelos |
| 6 | Horizonte y cielo; recurso `Texturas/skybox.png` sobre una esfera |
| 10–12 | Modelos individuales dentro del juego; hay nueve, falta uno para acreditar diez distintos |
| 15 | Video propio usando teclado o mando y mostrando la respuesta del juego |

Las sombras usan un mapa de profundidad desde el sol y reproducen las siluetas
de los modelos sobre terreno y objetos. La configuración del sol y la luna se explica
en `DXClase10/DXClase2/SOL.md` y `DXClase10/DXClase2/SOMBRAS.md`.

## Código completo para las capturas

### Declaración del terreno

DXRender.h, líneas 41–43.

```cpp
MPrimitives *skydomo;
MPrimitives *terreno;
TerrainSettings terrainSettings;
```

### Dimensiones, altura y posición

DXRender.h, líneas 87–92.

```cpp
terrainSettings.width = 255.0f;
terrainSettings.depth = 255.0f;
terrainSettings.maxHeight = 85.0f; // White pixel height in world units.
terrainSettings.verticalScale = 1.0f;
terrainSettings.horizontalScale = 3.0f;
terrainSettings.position = D3DXVECTOR3(0, 0, 0);
```

### Carga del heightmap y texturas

DXRender.h, líneas 202–204.

```cpp
terreno = new MPrimitives(d3d11Device, d3d11DevCon, terrainSettings, L"Texturas/dirt_diff.png", L"Texturas/Meshpart1_diff.png", "Texturas/PablitoPinwi_Heightmap_256.bmp", ShaderMultitextura);

if (!terreno->IsTerrainLoaded()) return false;
```

### Transformación del terreno

DXRender.h, líneas 589–591.

```cpp
D3DXMatrixTranslation(&terrMat, terrainSettings.position.x,
	terrainSettings.position.y, terrainSettings.position.z);
terreno->UpdateScene(terrMat);
```

### Mezcla por altura y aplicación de sombras

pixelShaderMultitextura.ps, líneas 21–41.

```cpp
float4 PS(VS_OUTPUT input) : SV_Target
{
    float3 grassColor = ObjTexture2.Sample(ObjSamplerState, input.TexCoord).rgb;
    float3 sandColor = ObjTexture.Sample(ObjSamplerState, input.TexCoord).rgb;
    float3 rockColor = ObjTexture3.Sample(ObjSamplerState, input.TexCoord).rgb;

    float height = saturate(input.vectPos.y / max(input.maxHeight, 0.0001f));
    // Fractions of the mesh's peak height. Larger intervals soften the changes.
    const float sandStart = 0.45f;
    const float sandEnd = 0.80f;
    const float rockStart = 0.80f;
    const float rockEnd = 1.00f;
    float sandBlend = smoothstep(sandStart, sandEnd, height);
    float rockBlend = smoothstep(rockStart, rockEnd, height);

    // Convex blends preserve the source colors without additive brightening.
    float3 terrainColor = lerp(grassColor, sandColor, sandBlend);
    terrainColor = lerp(terrainColor, rockColor, rockBlend);
    terrainColor = SunLitColor(terrainColor, input.normal, input.worldPosition);
    return float4(terrainColor, 1.0f);
}
```

### Creación del cielo

DXRender.h, líneas 201–201.

```cpp
skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/skybox.png", L"", ShaderSky);
```

### Orientación del cielo

DXRender.h, líneas 584–586.

```cpp
D3DXMATRIX skyRotMat, terrMat;
D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
skydomo->UpdateScene(skyRotMat);
```

### Cielo sin sombreado por luz

pixelShaderSky.ps, líneas 2–65.

```cpp
// The sky is a background image: its brightness must not depend on sphere normals.
#include "SunLighting.hlsli"
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXTURE;
    float3 normal : NORMAL;
    float3 viewDirection : COORDS;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 vectPos : POSITION;
    float maxHeight : HEIGHT;
    float3 worldPosition : TEXCOORD1;
};

Texture2D SkyTexture : register(t0);
Texture2D StarsTexture : register(t4);
Texture2D MoonTexture : register(t5);
SamplerState SkySampler : register(s0);
SamplerState CelestialSampler : register(s2);

float4 PS(VS_OUTPUT input) : SV_Target
{
    float3 direction = normalize(input.worldPosition);
    float3 daytime = SkyTexture.Sample(SkySampler, input.TexCoord).rgb;
    float aboveHorizon = smoothstep(-0.025f, 0.04f, direction.y);
    float horizon = pow(saturate(1.0f - abs(direction.y)), 6.0f);
    float3 night = lerp(float3(0.025f, 0.040f, 0.080f), float3(0.004f, 0.009f, 0.025f),
        pow(saturate(direction.y), 0.35f));
    daytime *= lerp(float3(1, 1, 1), float3(1.15f, 0.65f, 0.42f), NightSky.z * 0.65f);
    float3 sky = lerp(night, daytime, NightSky.x);
    float3 sunsetDirection = normalize(float3(SunDirection.x, 0.08f, SunDirection.z));
    float sunsetFacing = pow(saturate(dot(direction, sunsetDirection)), 4.0f);
    sky += float3(0.65f, 0.14f, 0.025f) * NightSky.z * horizon * (0.25f + 0.75f * sunsetFacing);

    // The provided 2:1 panorama wraps in longitude and clamps at the poles.
    float longitude = atan2(direction.z, direction.x) + NightSky.w;
    float2 starsUV = float2(longitude / 6.283185307f + 0.5f,
        acos(clamp(direction.y, -1.0f, 1.0f)) / 3.141592654f);
    float2 starsDx = ddx(starsUV), starsDy = ddy(starsUV);
    starsDx.x -= round(starsDx.x);
    starsDy.x -= round(starsDy.x);
    float3 stars = StarsTexture.SampleGrad(CelestialSampler, starsUV, starsDx, starsDy).rgb;
    sky += stars * MoonUp.w * NightSky.y * aboveHorizon;

    float cosine = dot(direction, SunDirection.xyz);
    float disk = smoothstep(SunSky.y, SunSky.x, cosine) * SunColor.w * aboveHorizon;
    float halo = pow(saturate(cosine), SunSky.z) * SunSky.w * SunColor.w * aboveHorizon;
    sky += SunColor.rgb * halo;
    sky = lerp(sky, SunColor.rgb, disk);

    // Project the moon as a circular sky billboard; the black PNG corners never
    // cover a rectangle of sky. Its texture remains the original supplied file.
    float moonFacing = dot(direction, MoonDirection.xyz);
    float2 moonLocal = float2(dot(direction, MoonRight.xyz), -dot(direction, MoonUp.xyz))
        / max(moonFacing, 0.001f) / max(MoonRight.w, 0.001f);
    float radius = length(moonLocal);
    float moonMask = (1.0f - smoothstep(0.975f, 1.0f, radius))
        * step(0.0f, moonFacing) * MoonDirection.w * aboveHorizon;
    float3 moonColor = MoonTexture.Sample(CelestialSampler, moonLocal * 0.5f + 0.5f).rgb;
    sky += float3(0.035f, 0.045f, 0.075f) * pow(saturate(moonFacing), 512.0f)
        * MoonDirection.w * aboveHorizon;
    sky = lerp(sky, saturate(moonColor * float3(1.10f, 1.18f, 1.30f)), moonMask);
    return float4(sky, 1.0f);
}
```

### Declaración de modelos y escalas

DXRender.h, líneas 48–60.

```cpp
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
```

### Escalas individuales

DXRender.h, líneas 93–95.

```cpp
casaScale = mesaScale = pocionScale = 4.0f;
pablitoScale = treeScale = fenceScale = furnaceScale = 1.0f;
pipeScale = cornerPipeScale = 1.0f;
```

### Carga de los nueve modelos

DXRender.h, líneas 207–229.

```cpp
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
```

### SRT y apoyo de modelos en el terreno

DXRender.h, líneas 473–480.

```cpp
void PlaceModel(MPrimitives* model, float x, float z, float size, float yaw = 0.0f)
{
	D3DXMATRIX scaling, rotation, translation;
	D3DXMatrixScaling(&scaling, size, size, size);
	D3DXMatrixRotationY(&rotation, D3DXToRadian(yaw));
	D3DXMatrixTranslation(&translation, x, terreno->getTerrainCollision(D3DXVECTOR2(x, z)), z);
	model->UpdateScene(scaling * rotation * translation);
}
```

### Ubicación de casa, mesa, poción y modelos nuevos

DXRender.h, líneas 483–514.

```cpp
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
```

### Dibujo de los nueve modelos

DXRender.h, líneas 658–668.

```cpp
casa->DrawScene(camara);
mesa->DrawScene(camara);
pocion->DrawScene(camara);
if (camara->IsThirdPerson())
	pablito->DrawScene(camara);
tree->DrawScene(camara);
fence->DrawScene(camara);
furnace->DrawScene(camara);
pipe->DrawScene(camara);
corner_pipe->DrawScene(camara);
```

### Lista de modelos que proyectan sombra

DXRender.h, líneas 634–636.

```cpp
MPrimitives* shadowCasters[] = { terreno, casa, mesa, pocion, pablito, tree,
	fence, furnace, pipe, corner_pipe, cubo, esfera, toroide, cilindro, cono,
	BoundingSphere1, BoundingSphere2 };
```

### Configuración y proyección del sol

SunLight.h, líneas 58–111.

```cpp
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
        data.moonUp = D3DXVECTOR4(moonUp.x, moonUp.y, moonUp.z, starsExposure);
        data.night = dynamicSky ? D3DXVECTOR4(skyState.daylight, skyState.starsVisibility,
            skyState.twilight, skyState.starRotation) : D3DXVECTOR4(1, 0, 0, 0);
        context->UpdateSubresource(sunBuffer, 0, NULL, &data, 0, 0);
    }
```

### Filtrado y bordes suaves de sombras

SunLighting.hlsli, líneas 29–47.

```cpp
float SunVisibility(float3 worldPosition, float3 normal)
{
    float facing = saturate(dot(normal, light.direction));
    float3 receiver = worldPosition + normal * SunShadow.z * (1.0f - facing);
    float4 projected = mul(float4(receiver, 1), SunViewProjection);
    float3 lightPosition = projected.xyz / projected.w;
    float edge = max(abs(lightPosition.x), abs(lightPosition.y));
    if (edge >= 1.0f || lightPosition.z <= 0 || lightPosition.z >= 1) return 1;
    float2 uv = lightPosition.xy * float2(0.5f, -0.5f) + 0.5f;
    float visibility = 0;
    [unroll] for (int y = -1; y <= 1; ++y)
    [unroll] for (int x = -1; x <= 1; ++x)
        visibility += SunShadowMap.SampleCmpLevelZero(SunShadowSampler,
            uv + float2(x, y) * SunShadow.x, lightPosition.z - SunShadow.y);
    visibility /= 9.0f;
    // Fade out at the map's perimeter rather than showing a square boundary.
    visibility = lerp(visibility, 1, smoothstep(0.85f, 0.98f, edge));
    return lerp(1, visibility, SunShadow.w);
}
```

### Restauración de escena y enlace del sol

DXRender.h, líneas 640–641.

```cpp
sun.EndShadowPass(d3d11DevCon);
sun.Bind(d3d11DevCon);
```

### Movimiento WASD

DXRender.h, líneas 314–329.

```cpp
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
```

### Mouse y cámara

DXRender.h, líneas 369–387.

```cpp
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
playerPosition += movement;
if (!camara->IsThirdPerson())
	playerYaw = camara->Yaw;
else if (distance > 0.00001f)
	playerYaw = atan2f(movement.x, movement.z);
```

Pablito es el personaje controlable. **C** o **D-pad ↑** alternan primera y tercera
persona. `UpdatePlayerCamera()` sigue sus pies y el salto antes de dibujar.
Los controles y ajustes se documentan en
[CAMARAS.md](DXClase10/DXClase2/CAMARAS.md).

### Correr y saltar

DXRender.h, líneas 293–304.

```cpp
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
```

### Detección del mando

DXRender.h, líneas 278–281.

```cpp
XINPUT_STATE gamepadState = {};
const bool gamepadConnected = GetForegroundWindow() == gameWindow
	&& XInputGetState(0, &gamepadState) == ERROR_SUCCESS;
const WORD buttons = gamepadConnected ? gamepadState.Gamepad.wButtons : 0;
```

### Sticks y zona muerta

DXRender.h, líneas 348–365.

```cpp
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

```

## Funciones internas

| Parte | Archivo | Líneas |
| --- | --- | --- |
| Carga BMP | MeshPrimitives.h | 929–993 |
| Vértices e índices del terreno | MeshPrimitives.h | 995–1053 |
| Carga de modelos | MeshPrimitives.h | 1056–1107 |
| Buffers de modelos | MeshPrimitives.h | 1109–1154 |
| Altura de terreno / colisión | MeshPrimitives.h | 1297–1320 |
