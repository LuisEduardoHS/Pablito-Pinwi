# Guía de capturas para el segundo avance

Basada en la presentación proporcionada de 15 páginas y en el código actual del proyecto.
Las líneas son las verificadas en esta revisión; pueden desplazarse si editas el código.
No necesitas pegar este código: ya existe en los archivos indicados.

Abre `DXClase10/DXClase2.sln`. En Visual Studio abre el archivo indicado, usa Ctrl+G
para ir a la línea y encuadra el intervalo completo. Activa los números de línea y,
si una instrucción larga no cabe, el ajuste de línea. Conserva visible la pestaña
con el nombre del archivo y no incluyas todo el escritorio en una captura de código.

## Página 1: portada

Coloca el nombre de tu juego, equipo, materia y avance. No requiere captura de código.

## Página 2: terreno con multitextura y heightmap

La referencia reúne cuatro recortes: declaración, creación, transformación y dibujo.
Estos son los equivalentes actuales; usa cuatro capturas pequeñas en la misma diapositiva.

### Declaración del terreno

Archivo: `DXRender.h`, líneas **39–41**.



```cpp
MPrimitives *skydomo;
MPrimitives *terreno;
TerrainSettings terrainSettings;
```

### Carga del heightmap y texturas

Archivo: `DXRender.h`, líneas **169–171**.

La tierra va en t0, el pasto en t1; la roca se carga dentro de MPrimitives.

```cpp
terreno = new MPrimitives(d3d11Device, d3d11DevCon, terrainSettings, L"Texturas/dirt_diff.png", L"Texturas/Meshpart1_diff.png", "Texturas/PablitoPinwi_Heightmap_256.bmp", ShaderMultitextura);

if (!terreno->IsTerrainLoaded()) return false;
```

### Transformación del terreno

Archivo: `DXRender.h`, líneas **400–402**.



```cpp
D3DXMatrixTranslation(&terrMat, terrainSettings.position.x,
	terrainSettings.position.y, terrainSettings.position.z);
terreno->UpdateScene(terrMat);
```

### Dibujo del terreno

Archivo: `DXRender.h`, líneas **457–458**.



```cpp
terreno->DrawScene(camara);
skydomo->DrawScene(skyCamara);
```

La nota sobre un bosque de la presentación no tiene equivalente en esta escena:
no hay un sistema de árboles instanciados implementado.

### Capturas adicionales para explicar tu integración

### Tamaño, elevación y posición

Archivo: `DXRender.h`, líneas **65–70**.



```cpp
terrainSettings.width = 255.0f;
terrainSettings.depth = 255.0f;
terrainSettings.maxHeight = 85.0f; // White pixel height in world units.
terrainSettings.verticalScale = 1.0f;
terrainSettings.horizontalScale = 3.0f;
terrainSettings.position = D3DXVECTOR3(0, 0, 0);
```

### Mezcla gradual por altura

Archivo: `pixelShaderMultitextura.ps`, líneas **31–50**.

Esta captura muestra por qué el pasto, la tierra y la roca cambian con la elevación.

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
    return float4(terrainColor, 1.0f);
}
```

## Página 3: imágenes de texturas y heightmap

Abre y captura directamente estas imágenes, con un título debajo de cada una:

| Título | Archivo dentro de DXClase10/DXClase2 |
| --- | --- |
| Pasto | `Texturas/Meshpart1_diff.png` |
| Tierra | `Texturas/dirt_diff.png` |
| Roca | `Texturas/rock.jpg` |
| Heightmap | `Texturas/PablitoPinwi_Heightmap_256.bmp` |

La referencia muestra dos texturas y un heightmap. Tu implementación usa tres
texturas; conviene mostrar las tres y el heightmap para describirla correctamente.

## Página 4: resultado del terreno

Ejecuta con F5 y toma una captura del juego donde se vean pasto cercano, una
ladera con tierra y una cima rocosa. No es captura de código. Evita que otra
ventana tape el juego. Tu escena no tendrá el bosque y los edificios del ejemplo.

## Página 5: skydomo o skybox

Usa tres recortes: creación, orientación y dibujo.

### Creación del skydomo y su textura

Archivo: `DXRender.h`, líneas **168–168**.



```cpp
skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/sky.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
```

### Orientación del skydomo

Archivo: `DXRender.h`, líneas **395–397**.

La rotación de -90 grados es fija. No presentes esta captura como una rotación animada: tu versión no tiene el giro continuo del ejemplo.

```cpp
D3DXMATRIX skyRotMat, terrMat;
D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
skydomo->UpdateScene(skyRotMat);
```

### Dibujo del cielo

Archivo: `DXRender.h`, líneas **458–458**.



```cpp
skydomo->DrawScene(skyCamara);
```

## Página 6: resultado del skydomo

Toma una o dos capturas del juego mirando hacia el cielo, dejando algo del horizonte
para mostrar que rodea la escena. La imagen de cielo utilizada es `Texturas/sky.jpg`.

## Páginas 7, 8 y 9: modelos OBJ, transformaciones y dibujo

No hay equivalentes implementados para los modelos del ejemplo. La escena actual
crea cubo, esfera, toroide, cilindro, cono y billboard: son primitivas, no evidencia
de modelos OBJ importados.

| Página | Lo que demuestra la referencia | Situación actual |
| --- | --- | --- |
| 7 | Declarar modelos y cargarlos con sus texturas | No hay modelos externos instanciados en DXRender |
| 8 | Aplicar escala, rotación y traslación a esos modelos | Hay transformaciones de primitivas, no de los modelos requeridos |
| 9 | Dibujar los modelos importados | DrawScene dibuja primitivas, terreno y cielo |

Como referencia técnica puedes revisar el constructor de carga de modelos en
`MeshPrimitives.h`, líneas 191–200, y `LoadModel`, líneas 1021–1070. La función
lee el formato de texto convertido de la plantilla (cabeceras separadas por dos
puntos), no analiza directamente un OBJ Wavefront. Estas funciones por sí solas
no prueban que hayas integrado diez modelos en la escena.

Si presentas el estado actual, titula las capturas siguientes como «Primitivas de
la plantilla», sin usarlas para acreditar OBJ importados:

- Declaraciones: `DXRender.h`, líneas 33–38.
- Creación: `DXRender.h`, líneas 161–166.
- Transformaciones: `DXRender.h`, líneas 367–392. El factor `scale` se calcula,
  pero no se aplica a las primitivas dibujadas en ese bloque; no afirmes SRT completo.
- Dibujo: `DXRender.h`, líneas 447–453.

## Páginas 10, 11 y 12: evidencia visual de diez modelos OBJ

Todavía no se pueden reproducir con esta escena. Primero hay que integrar y dibujar
los modelos propios del juego. Después se captura cada modelo dentro del juego,
como en la referencia. No uses imágenes de archivos sueltos como prueba de que
se renderizan en DirectX.

## Página 13: teclado y mouse

Usa una captura del código WASD y otra del mouse. Para imitar la evidencia visual,
acompaña el código con una foto o grabación real usando el teclado y el mouse.

### Movimiento WASD

Archivo: `DXRender.h`, líneas **244–259**.



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

### Mouse y actualización de cámara

Archivo: `DXRender.h`, líneas **298–310**.



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

### Correr y saltar (opcional)

Archivo: `DXRender.h`, líneas **222–234**.

Shift permite correr y Espacio inicia el salto; este bloque también incluye los botones del mando.

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

## Página 14: gamepad

Usa una captura de detección del mando y otra del manejo de sticks. Conecta un
mando compatible con XInput y demuestra el movimiento en el juego antes de
presentar una foto como evidencia de funcionamiento.

### Detección del mando

Archivo: `DXRender.h`, líneas **218–220**.



```cpp
XINPUT_STATE gamepadState = {};
const bool gamepadConnected = XInputGetState(0, &gamepadState) == ERROR_SUCCESS;
const WORD buttons = gamepadConnected ? gamepadState.Gamepad.wButtons : 0;
```

### Sticks, zona muerta y movimiento

Archivo: `DXRender.h`, líneas **277–296**.



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

	// Los ejes van de -32768 a 32767: normalizar cada signo por separado.
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

### Aplicación del movimiento a la cámara

Archivo: `DXRender.h`, líneas **309–310**.



```cpp
camara->UpdateCamera(moveBackForward, moveLeftRight, camYaw, camPitch); 
skyCamara->setSkyCamara(camara);
```

El stick izquierdo mueve, el derecho mira, L3 permite correr y A inicia el salto,
según el código actual. La revisión del código no sustituye la prueba física del mando.

## Página 15: video del gamepad

Graba un video propio donde se vea el mando y la respuesta del juego: movimiento,
giro y, si quieres, salto. Enlaza tu video en la diapositiva. No uses el enlace del
equipo del ejemplo como evidencia de tu proyecto.
