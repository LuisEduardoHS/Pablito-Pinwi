# Iluminación solar y lunar

El sol existente está integrado al ciclo automático de día y noche. Su dirección,
color e intensidad cambian con la hora; al anochecer, la luna toma el papel de
luz direccional. Ambos astros comparten el sistema de sombras de silueta.

Consulta [DIA_NOCHE.md](DIA_NOCHE.md) para duración, controles, texturas,
curvas de transición y ajustes de iluminación.

## Mapa de sombras

`DXRender::DrawScene()` dibuja primero la profundidad desde la luz activa.
Incluye terreno, los nueve modelos importados y las primitivas 3D. Pablito
proyecta sombra también en primera persona; la poción puede dejar sombra
sobre la mesa y los modelos reciben sombras de otros modelos.

La textura de profundidad tiene 2048 × 2048 texeles y cubre 180 × 180 unidades
alrededor del personaje en el plano de la luz. Lo sigue al caminar, con el
encuadre ajustado a texeles y un desvanecimiento en el perímetro. La iluminación
continúa en todo el mapa. Un filtro PCF de 3 × 3 suaviza los bordes. Las sombras
bloquean luz directa y conservan la luz ambiente.

En `SunLight.h`, `shadowCoverage`, `shadowDepth`, `shadowStrength` y
`ShadowResolution` ajustan cobertura, profundidad, intensidad y resolución.
El disco solar usa `diskRadiusDegrees`. La dirección, los colores y las
intensidades de día/noche se calculan en `DayNightCycle::GetState()`.

## Archivos

- `SunLight.h`: recursos de luz, texturas celestes y mapa de sombras.
- `sunShadow.hlsl`: pase de profundidad con recortes por alfa.
- `SunLighting.hlsli`: iluminación y comparación de sombras compartidas.
- `pixelShaderSky.ps`: cielo, transiciones, discos y estrellas.
- `MeshPrimitives::DrawShadow()`: geometría de cada malla en el pase de sombras.
- `ShaderCompiler.h`: includes compatibles con el SDK de junio de 2010.

La textura usa las vistas `R32_TYPELESS`, `D32_FLOAT` y `R32_FLOAT` descritas
por [Microsoft para leer un depth buffer como textura](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-depth-stencil#reading-the-depth-stencil-buffer-as-a-texture).
`TerrainShadows.h` conserva las elipses anteriores como referencia; ya no se
aplican en el renderizador.

Se verificaron compilación Debug | Win32, sombras de silueta, cambios de
posición/dirección, recortes transparentes, superficies sin rayas de
sombreado, sol cenital, luz lunar y compatibilidad de todos los shaders.
La prueba de renderizado WARP terminó sin advertencias ni errores de Direct3D.
