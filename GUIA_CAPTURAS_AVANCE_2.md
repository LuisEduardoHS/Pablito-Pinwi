# Capturas del segundo avance: integración actualizada de main

Se integró el commit remoto `50b86e5` (Modelooos) con el trabajo local de terreno,
texturas, controles y modelos. Esta guía corresponde al código final de esa integración.
Las líneas pueden cambiar cuando se editen los archivos.

La escena tiene nueve modelos importados distintos: casa, mesa, poción, Pablito,
árbol, cerca, horno, tubo y codo. Casa, mesa y poción conservan los archivos locales
convertidos desde los OBJ originales; los seis adicionales provienen de main.
Pablito conserva la textura provisional `escudo.jpg` del commit remoto, que no
incluye una textura propia para él. No se añadió el círculo de interfaz eliminado.

## Tabla para preparar las capturas

En Visual Studio abre el archivo y usa Ctrl+G para ir a la línea inicial.
Muestra el intervalo completo, la pestaña del archivo y los números de línea.
Para instrucciones largas, activa el ajuste de línea.

| Página de referencia | Evidencia | Archivo | Líneas actuales |
| --- | --- | --- | --- |
| 1 | Portada | Sin código | Nombre del juego y equipo |
| 2 | Declaración del terreno | DXRender.h | 39–41 |
| 2 | Parámetros físicos del terreno | DXRender.h | 77–82 |
| 2 | Heightmap y texturas utilizados | DXRender.h | 184–186 |
| 2 | Transformación del terreno | DXRender.h | 490–493 |
| 2 | Dibujo del terreno | DXRender.h | 558 |
| 2, extra | Mezcla por altura de pasto, tierra y roca | pixelShaderMultitextura.ps | 31–50 |
| 3 | Imágenes de las tres texturas y el heightmap | Texturas/ | Ver lista abajo |
| 4 | Terreno funcionando | Juego en ejecución | Mostrar ladera y montaña |
| 5 | Creación del nuevo cielo | DXRender.h | 183 |
| 5 | Orientación del skydomo | DXRender.h | 485–487 |
| 5 | Dibujo del cielo | DXRender.h | 559 |
| 6 | Cielo funcionando | Juego en ejecución | Mirar al horizonte y al cielo |
| 7 | Declaración de modelos y escalas | DXRender.h | 42–54 y 83–85 |
| 7 | Carga de modelos y texturas | DXRender.h | 188–211 |
| 8 | Helper de escala, rotación y traslación | DXRender.h | 406–414 |
| 8 | Colocación de casa, mesa y poción | DXRender.h | 417–436 |
| 8 | Posiciones y escalas de los modelos nuevos | DXRender.h | 438–443 |
| 9 | Dibujo de los nueve modelos | DXRender.h | 548–556 |
| 10–12 | Modelos visibles e individuales | Juego en ejecución | Nueve modelos disponibles; falta uno para acreditar diez |
| 13 | Movimiento con WASD | DXRender.h | 284–299 |
| 13 | Mouse y actualización de cámara | DXRender.h | 337–349 |
| 13, extra | Correr y saltar | DXRender.h | 262–274 |
| 14 | Detección del mando | DXRender.h | 258–260 |
| 14 | Sticks, zona muerta y movimiento | DXRender.h | 317–335 |
| 14 | Aplicación a la cámara | DXRender.h | 348–349 |
| 15 | Video propio utilizando el mando | Sin código | Grabar la respuesta real del juego |

## Código para las capturas

Los siguientes bloques ya están implementados: no necesitas pegarlos.

### Terreno: declaración

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:39), líneas **39–41**.

```cpp
MPrimitives *skydomo;
MPrimitives *terreno;
TerrainSettings terrainSettings;
```

### Terreno: dimensiones y altura

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:77), líneas **77–82**.

```cpp
terrainSettings.width = 255.0f;
terrainSettings.depth = 255.0f;
terrainSettings.maxHeight = 85.0f; // White pixel height in world units.
terrainSettings.verticalScale = 1.0f;
terrainSettings.horizontalScale = 3.0f;
terrainSettings.position = D3DXVECTOR3(0, 0, 0);
```

### Terreno: carga

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:184), líneas **184–186**.

```cpp
terreno = new MPrimitives(d3d11Device, d3d11DevCon, terrainSettings, L"Texturas/dirt_diff.png", L"Texturas/Meshpart1_diff.png", "Texturas/PablitoPinwi_Heightmap_256.bmp", ShaderMultitextura);

if (!terreno->IsTerrainLoaded()) return false;
```

### Terreno: posición

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:490), líneas **490–493**.

```cpp
D3DXMatrixTranslation(&terrMat, terrainSettings.position.x,
	terrainSettings.position.y, terrainSettings.position.z);
terreno->UpdateScene(terrMat);
UpdateImportedModels();
```

### Terreno y cielo: dibujo

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:558), líneas **558–559**.

```cpp
terreno->DrawScene(camara);
skydomo->DrawScene(skyCamara);
```

### Terreno: mezcla por altura

[pixelShaderMultitextura.ps](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/pixelShaderMultitextura.ps:31), líneas **31–50**.

```hlsl
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
    return float4(terrainColor, 1.0f);
}
```

### Cielo: textura nueva

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:183), líneas **183–183**.

```cpp
skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/skybox.png", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
```

### Cielo: orientación

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:485), líneas **485–487**.

```cpp
D3DXMATRIX skyRotMat, terrMat;
D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
skydomo->UpdateScene(skyRotMat);
```

### Modelos: declaraciones y escalas

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:42), líneas **42–54**.

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

### Modelos: valores de escala

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:83), líneas **83–85**.

```cpp
casaScale = mesaScale = pocionScale = 4.0f;
pablitoScale = treeScale = fenceScale = furnaceScale = 1.0f;
pipeScale = cornerPipeScale = 1.0f;
```

### Modelos: archivos y texturas

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:188), líneas **188–211**.

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
```

### Modelos: helper SRT sobre el terreno

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:406), líneas **406–414**.

```cpp
// Centered model base rests on the terrain; yaw is in degrees.
void PlaceModel(MPrimitives* model, float x, float z, float size, float yaw = 0.0f)
{
	D3DXMATRIX scaling, rotation, translation;
	D3DXMatrixScaling(&scaling, size, size, size);
	D3DXMatrixRotationY(&rotation, D3DXToRadian(yaw));
	D3DXMatrixTranslation(&translation, x, terreno->getTerrainCollision(D3DXVECTOR2(x, z)), z);
	model->UpdateScene(scaling * rotation * translation);
}
```

### Modelos: transformaciones y colocación

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:417), líneas **417–444**.

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

	PlaceModel(pablito, 378.0f, 399.0f, pablitoScale, 180.0f);
	PlaceModel(tree, 360.0f, 438.0f, treeScale);
	PlaceModel(fence, 403.0f, 427.0f, fenceScale);
	PlaceModel(furnace, 394.0f, 412.0f, furnaceScale);
	PlaceModel(pipe, 399.0f, 407.0f, pipeScale);
	PlaceModel(corner_pipe, 402.0f, 415.0f, cornerPipeScale);
}
```

### Modelos: dibujo

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:548), líneas **548–556**.

```cpp
casa->DrawScene(camara);
mesa->DrawScene(camara);
pocion->DrawScene(camara);
pablito->DrawScene(camara);
tree->DrawScene(camara);
fence->DrawScene(camara);
furnace->DrawScene(camara);
pipe->DrawScene(camara);
corner_pipe->DrawScene(camara);
```

### Teclado: WASD

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:284), líneas **284–299**.

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

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:337), líneas **337–349**.

```cpp
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
```

### Correr y saltar

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:262), líneas **262–274**.

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

### Gamepad: detección

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:258), líneas **258–260**.

```cpp
XINPUT_STATE gamepadState = {};
const bool gamepadConnected = XInputGetState(0, &gamepadState) == ERROR_SUCCESS;
const WORD buttons = gamepadConnected ? gamepadState.Gamepad.wButtons : 0;
```

### Gamepad: sticks

[DXRender.h](D:/projects/GC_PLANTILLA_2/DXClase10/DXClase2/DXRender.h:317), líneas **317–335**.

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

## Archivos de imagen para la página 3

Dentro de `DXClase10/DXClase2/Texturas/`:

- Pasto: `Meshpart1_diff.png`.
- Tierra: `dirt_diff.png`.
- Roca: `rock.jpg`.
- Heightmap: `PablitoPinwi_Heightmap_256.bmp`.
- Cielo de la integración: `skybox.png`.

## Carga interna y comprobaciones técnicas adicionales

| Función | Archivo | Líneas |
| --- | --- | --- |
| Repetición de texturas: 64 por eje | MeshPrimitives.h | 177 |
| Lectura y validación del BMP | MeshPrimitives.h | 924–988 |
| Vértices, índices y normales del terreno | MeshPrimitives.h | 990–1048 |
| Lectura del modelo convertido y centrado opcional | MeshPrimitives.h | 1051–1101 |
| Creación de buffers de modelos | MeshPrimitives.h | 1103–1148 |
| Consulta de altura para cámara y modelos | MeshPrimitives.h | 1291–1314 |
| Punto de inicio del jugador | DXRender.h | 395–403 |

## Qué mostrar y qué queda pendiente

- El recurso llamado `skybox.png` se dibuja sobre el skydomo esférico de la plantilla;
  no es un cubemap ni un cielo con rotación animada.
- Hay nueve modelos distintos cargados. Para replicar la evidencia de diez modelos
  de la presentación todavía hace falta integrar un modelo adicional diferente.
- Pablito usa una textura provisional. La geometría sí se carga, pero esa imagen
  no es un material propio del personaje.
- Para teclado y gamepad, una captura de código no sustituye una prueba física:
  graba tu propio video para la página 15.
- La integración se compiló y enlazó en Debug | Win32. La escena se revisó en
  ejecución. Se verificaron buffers, índices, normales, tangentes y texturas de
  los nueve modelos mediante Direct3D 11 WARP, y se repitieron las pruebas del
  cargador del terreno. Release y el mando físico no se verificaron en esta revisión.
