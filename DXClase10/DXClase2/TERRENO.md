# Terreno de Pablito Pinwi

La solución activa es `../DXClase2.sln`. La carpeta `../Objetos/plantilla`
contiene otra plantilla y no interviene en esta solución.

## Archivo integrado

`Texturas/PablitoPinwi_Heightmap_256.bmp` es una copia exacta del BMP proporcionado:
256 × 256 píxeles, 24 bits, BI_RGB sin compresión, en escala de grises.
SHA-256: `A364D973C9F9DBA28A0E52EB66030E91F5FD957DD439147D827210256302729A`.
El archivo original `heightmap01.bmp` permanece disponible.

## Configuración

Editar los valores `terrainSettings` en el constructor de `DXRender.h` y recompilar:

| Parámetro | Valor inicial | Efecto |
| --- | --- | --- |
| `width` | 255 | Ancho físico en X |
| `depth` | 255 | Largo físico en Z |
| `horizontalScale` | 3 | Multiplica ancho y largo |
| `maxHeight` | 85 | Altura de un píxel blanco (255) |
| `verticalScale` | 1 | Multiplica todas las elevaciones |
| `position` | (0, 0, 0) | Traslación del terreno en el mundo |

Para este archivo gris, la altura local es:

`altura = (gris / 255.0f) * maxHeight * verticalScale`

No se normaliza el histograma, no se recorta el relieve y no se redondean las
alturas a enteros. Como la imagen contiene valores entre 41 y 237, la configuración
actual produce alturas entre 13.6667 y 79 unidades. `maxHeight = 85` define la
altura del blanco, no obliga al punto más alto de esta imagen a llegar a 85.
Para que el pico de este archivo alcance una altura H, usar
`maxHeight = H * 255 / 237`, con `verticalScale = 1`.

El ancho real anterior era 255 × 255 unidades: el argumento `100.0f` de la
plantilla se ignoraba. Se conserva como tamaño base; con `horizontalScale = 3` el terreno ocupa
765 × 765 unidades, independientemente de la resolución de futuros archivos.
La distancia lejana de la cámara se amplió de 1000 a 1500 unidades para abarcarlo. La posición no afecta a la mezcla de texturas por altura.
Las escalas se incorporan a los vértices y la matriz de mundo aplica la traslación;
la consulta de colisión admite esa configuración, sin rotación adicional.

## Carga y malla

`MPrimitives::LoadHeightMap` valida las cabeceras, dimensiones, desplazamiento y
longitud del archivo. Lee filas con su relleno a múltiplos de cuatro bytes y
reconoce BMP con filas almacenadas de abajo arriba o de arriba abajo. El borde
inferior de la imagen corresponde a Z=0, manteniendo la convención de la plantilla.
Para BMP de color se usa luminancia; en 32 bits se ignora el alfa.

La malla usa un vértice por píxel e índices de 32 bits: para este archivo hay
65,536 vértices y 390,150 índices (130,050 triángulos). Los índices utilizan el
ancho como paso de fila, por lo que también se admiten imágenes rectangulares.
Se calculan normales con los triángulos del terreno. Las UV repiten las texturas
64 veces por eje mediante el sampler WRAP existente. Para ajustar el tamaño
visual, cambiar `TEXTURE_REPEAT` en el constructor del terreno de
`MeshPrimitives.h`: un valor mayor produce detalles más pequeños. Este ajuste
afecta a las tres texturas, que comparten las UV.

Se conserva `vertexShader.vs`. El shader `pixelShaderMultitextura.ps` mezcla
el pasto `Meshpart1_diff.png` (t1), tierra `dirt_diff.png` (t0) y roca (t2) según la altura local.
El pasto domina hasta el 45% de la altura máxima de la malla. La tierra entra
suavemente entre el 45% y el 80%, y la roca entre el 80% y el 100%. Con el pico
actual de 79 unidades, los intervalos son 35.55–63.2 y 63.2–79 unidades.
Las transiciones usan `smoothstep` y `lerp`, sin sumar colores ni aplicar
multiplicadores de iluminación que apaguen el verde original en las zonas bajas.
Para retrasar los cambios, aumentar `sandStart` y `rockStart`; para suavizarlos,
ampliar la separación respecto a `sandEnd` y `rockEnd`, manteniendo los valores
entre 0 y 1 y el inicio menor que el fin. La repetición continúa en 64 por eje.

La consulta de altura interpola sobre los mismos triángulos que se dibujan y
contempla la traslación; fuera del terreno devuelve la altura del plano base.
No se añadieron límites al movimiento.

Los datos temporales se gestionan con vectores y el archivo se cierra por RAII.
Un error de lectura o de creación de buffers muestra un mensaje y detiene la
inicialización de la escena.

## Futuros heightmaps

Colocarlos en `Texturas/`, junto a este BMP, y cambiar la ruta del constructor de
`terreno` en `DXRender.h`. El cargador admite BMP BI_RGB de 24 o 32 bits,
entre 2 y 1024 píxeles por eje, sin exigir dimensiones cuadradas ni potencias de
dos. El límite controla el consumo de memoria de la aplicación Win32.
PNG, BMP indexados de 8 bits, 16 bits, RLE y BI_BITFIELDS requieren conversión
previa o ampliar el cargador. No se remuestrea el archivo.

## Verificación en Visual Studio

1. Abrir `../DXClase2.sln` y seleccionar `DXClase2` como proyecto de inicio.
2. Elegir `Debug | Win32` (puede mostrarse como x86) y compilar la solución.
3. Ejecutar con F5 o Ctrl+F5. El proyecto establece el directorio de trabajo en
   `$(ProjectDir)` para encontrar los shaders y `Texturas/`.
4. Recorrer el terreno y revisar relieve, textura de pasto y seguimiento de la
   altura. Probar cambios de escala o posición después de recompilar.

La plantilla usa el SDK DirectX de junio de 2010 y el toolset v143.
Para ejecutar el EXE manualmente se debe usar como directorio de trabajo la
carpeta del proyecto, donde están los shaders.

## Comprobaciones realizadas

- Compilación y enlace reales de `Debug | Win32` con MSBuild: correctos.
  Hay advertencias en el SDK antiguo y en código de la plantilla; no se realizó
  una limpieza de advertencias ajena al terreno. Release no se verificó.
- Ejecución de la aplicación y captura de su ventana: terreno y texturas visibles.
- Prueba temporal en C++ contra `MPrimitives` con un dispositivo Direct3D 11 WARP:
  comprobó índices leídos del buffer GPU, alturas, normales y carga de los tres
  recursos de textura y ambos shaders.
- Casos pequeños de prueba: BMP rectangulares de 24/32 bits, relleno de filas,
  ambas orientaciones, escalas, traslación e interpolación en ambos triángulos.
  Se comprobó el rechazo de archivos ausentes, truncados o de formato incompatible.
- Hash del archivo copiado idéntico al proporcionado y `git diff --check` correcto.

Estas comprobaciones no sustituyen una prueba extensa de jugabilidad ni validan
otras configuraciones de compilación.

La textura de tierra `Texturas/dirt_diff.png` es una copia del archivo
`Downloads/dirt/Meshpart1_diff.png` proporcionado. Se renombró para evitar
sobrescribir el archivo de pasto, que tiene el mismo nombre de origen.
