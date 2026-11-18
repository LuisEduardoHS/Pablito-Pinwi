# Efectos de shaders

Se conservan `vertexShader.vs`, `pixelShader.ps`, los materiales de terreno,
cielo, vegetación y el pase de sombras. Los archivos HLSL nuevos se compilan
con `CompileSceneShader`, usando Shader Model 4.0, igual que la plantilla.

## Pociones y objetos cercanos

`materialEffects.ps` mantiene textura, iluminación solar/lunar, transparencia
recortada y sombras. Añade dos componentes independientes:

- Emisión pulsante de dos segundos, con borde Fresnel y una banda que recorre
  el objeto. Se aplica a las pociones embotelladas y al líquido preparado del
  caldero. Energía usa dorado, Velocidad azul y Vitalidad verde.
- Resaltado cian que aumenta al acercarse. Ingredientes, estaciones, destinos
  utilizables y nube reciben el efecto mediante sus identificadores existentes.
  Se utiliza la misma lista de candidatos, alcance y visibilidad que la tecla E;
  no se crean interacciones, colisiones ni cambios de reglas.

`SceneEffects.h` carga un único pixel shader y un buffer de parámetros en `b3`.
Los colores, intensidad, tiempo y fase se actualizan por objeto. El parámetro
opcional `materialOverride` de `MPrimitives::DrawScene` permite usar ese material;
los objetos sin efecto siguen dibujándose con su shader original. La emisión es
del material: no requiere otro sistema de luces ni un pase de bloom.

## Río del valle

`WaterStream.h` crea una superficie subdividida de norte a sur, entre Z=0 y
Z=1152, con 24–34 unidades de ancho. `WorldLayout.json` define la curva y las
alturas; `Tools/generate_valley.py` excava su cauce en el heightmap. La vegetación
deja libre el agua y los márgenes. Un puente con colisiones mantiene conectado
el camino occidental. La nube usa también la superficie del agua como apoyo.

`waterStream.hlsl` desplaza los vértices con ondas pequeñas y desplaza dos capas
de UV a distinta velocidad. Una textura periódica generada de 128×128, con
mipmaps, aporta corrientes, variación de normales y espuma. La superficie usa
color turquesa, reflejo aproximado del cielo, brillo especular y bordes suaves.
Se dibuja tras la geometría opaca, con mezcla alfa, prueba de profundidad y sin
escribir profundidad; restaura los estados de mezcla, profundidad y rasterizado.

## Integración y ajustes

- `DXRender`: inicialización/liberación, reloj de efectos, candidatos y arroyo.
- `AlchemyScene`: emisión por poción y asociación de las piezas de cada estación.
- `SceneEffects`: colores y fuerza del resaltado; `b0` y `b2` siguen reservados
  a iluminación/sombras, `b3` contiene los efectos de material.
- `WaterStream::waveAmplitude`: amplitud de las ondas; `WorldLayout.json` define trazado/ancho.
- `waterStream.hlsl`: velocidad de UV, colores, espuma y reflejos; su buffer usa
  `b1` y sus recursos `t0/s0`, sin ocupar los de sol, luna o estrellas.

El reloj visual avanza por segundos durante la partida enfocada y se detiene
en los menús. Reiniciar restablece el tiempo y los resaltados. No se modifican
recetas, temporizadores, posiciones de objetos, movimiento o controles.

`Tools/check_scene_effects.cpp` comprueba alcance, visibilidad, énfasis de la
selección, transición de proximidad, reinicio y reloj a distintas tasas de FPS.
Se compila como ejecutable independiente desde una consola x86 de Visual Studio,
con los directorios Include y Lib/x86 del DirectX SDK de junio de 2010:

```bat
cl /EHsc /std:c++14 /utf-8 Tools\check_scene_effects.cpp /Fe:check_scene_effects.exe /link d3d11.lib d3dx11.lib d3dx10.lib
check_scene_effects.exe
```

La verificación visual compara una poción en el máximo y mínimo del pulso,
ingredientes cerca/lejos y dos tiempos del arroyo con cámara fija. Se verifica
además el ciclo de jugabilidad completo y la compilación Debug/Release Win32.
