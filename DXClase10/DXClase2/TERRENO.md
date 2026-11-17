# Terreno de Pablito Pinwi

La solución activa es `../DXClase2.sln`; `../Objetos/plantilla` es otra plantilla.

## Valle y montañas actuales

El mundo ocupa **1,152 × 1,152 unidades**, con alturas de **28.00 a 312.75**.
Combina una terraza de pueblo a Y=30, praderas onduladas, estribaciones,
cumbres de distintas alturas y pasos entre montañas. Las montañas se elevan
sobre los bordes y al fondo del valle. El comienzo, la casa y los objetos
conservan sus coordenadas X/Z y descansan sobre la nueva terraza.

Las montañas forman macizos alargados y asimétricos: cumbres anchas, mesetas
verdes, crestas conectadas y contrafuertes de roca. Se combinan volúmenes
redondeados mediante uniones suaves y desplazamiento irregular del relieve.
Una pasada de drenaje acumula el flujo cuesta abajo y erosiona canales que
se ramifican por las laderas. Así se evitan los conos aislados y las puntas
simétricas de la versión anterior, conservando una lectura estilizada.

Hay una ruta principal que sale del inicio, rodea la casa por el este y llega
al paso norte; ramales hacia el noroeste y el este; un recorrido que vuelve por
el sureste; y un acceso corto a la plaza. Los anchos son de 12 a 18 unidades.
Los caminos tienen alturas propias y transiciones amplias hacia las laderas:
no se limitan a pintar una carretera encima de una pendiente impracticable.
Las pendientes longitudinales del trazado están por debajo del 18 %.

## Vegetación de las praderas

`Vegetation.h` distribuye las tres imágenes originales de `Texturas/Vegetacion/`
sobre la misma máscara de materiales que utiliza el terreno. Solo admite pasto,
dejando libres caminos, bordes de grava, roca, pendientes fuertes y objetos.
La base de cada mata sigue la altura y la inclinación del terreno.

Las posiciones usan una semilla fija: parecen aleatorias y se mantienen entre
partidas. Hay claros y grupos de distinta densidad, con tamaños, tonos y reflejos
horizontales variados. Las probabilidades iniciales son 50 % para el pastito
pequeño, 40 % para el segundo y 10 % para las flores; los arbustos se reducen
adicionalmente al exigir 10.5 unidades entre ellos. La distribución actual tiene
11,822 matas pequeñas, 8,164 del segundo tipo y 561 arbustos con flores:
20,547 elementos en total. Se prueban posiciones cada 3.5 unidades con una
probabilidad variable del 80 al 100 %, conservando variación y agrupaciones.

Cada candidato revisa el centro y ocho puntos de su huella antes de colocarse.
Las matas dejan al menos 0.45 unidades libres entre sus radios; si interviene un
arbusto, dejan 1.1. Los anchos son de 2.1–3.3, 2.4–3.9 y 3.6–5.0 unidades,
respectivamente. Estos valores, la densidad y la semilla se ajustan en `Scatter`.

Las imágenes conservan sus píxeles y transparencia originales. El shader
`vegetation.hlsl` omite el margen transparente mediante coordenadas UV y orienta
las matas hacia la cámara como billboards que giran sobre el eje vertical.
Reciben la iluminación de día/noche y las sombras de
los objetos, sin bloquear al jugador ni al vehículo. Se dibujan en hasta tres
lotes, con descarte fuera de cámara y desvanecimiento entre 260 y 350 unidades.

## Archivos y regeneración

- `Texturas/Pablito_Valley_513.bmp`: altura de 513 × 513 muestras, BI_RGB32.
- `Texturas/Pablito_Valley_Paths.png`: control de materiales de 1025 × 1025.
  R es camino, G es borde de grava/tierra y B es variación de la pradera.
- `Tools/generate_valley.py`: diseño reproducible de montañas, terraza,
  curvas, niveles y anchos de caminos. Ambas imágenes salen del mismo diseño.
- `Tools/check_valley.py`: valida precisión, alturas, terraza, pendientes y
  conectividad de los caminos desde el punto inicial.

Para regenerar desde la carpeta del proyecto:

```powershell
python -m pip install numpy pillow
python Tools/generate_valley.py
python Tools/check_valley.py
```

Si solo se cambia la forma de las montañas, este comando regenera la altura
conservando la máscara de los caminos:

```powershell
python Tools/generate_valley.py --height-only
```

Al modificar los trazados o sus anchos, ejecutar el comando completo.

Python, NumPy y Pillow solo se usan al generar o verificar los archivos. El
juego carga los recursos ya generados y no requiere estas dependencias.

El BMP anterior `PablitoPinwi_Heightmap_256.bmp` permanece disponible, junto con
`heightmap01.bmp`. El archivo proporcionado originalmente conserva el SHA-256
`A364D973C9F9DBA28A0E52EB66030E91F5FD957DD439147D827210256302729A`.

## Escala y precisión

Se configura en el constructor de `DXRender.h`:

| Parámetro | Valor | Uso |
| --- | --- | --- |
| `width`, `depth` | 384 | Extensión base |
| `horizontalScale` | 3 | Extensión física de 1,152 por eje |
| `maxHeight` | 360 | Rango completo de codificación de altura |
| `verticalScale` | 1 | Multiplicador vertical |
| `position` | (0, 0, 0) | Traslación del terreno |
| `highPrecisionHeightmap` | true | Altura de 16 bits en BMP de 32 bits |
| `blendMapPath` | `Texturas/Pablito_Valley_Paths.png` | Máscara de materiales |

La modalidad de alta precisión codifica el byte alto en RGB gris y el byte
bajo en alfa: `altura = (R * 256 + A) / 65535 * maxHeight * verticalScale`.
La resolución vertical es de unas 0.0055 unidades. Así se evitan los saltos
visibles de más de una unidad que produciría ampliar un heightmap de 8 bits
hasta estas alturas. No guardar el BMP nuevo con un editor que descarte alfa.

Para usar un BMP gris convencional de 24/32 bits, desactivar
`highPrecisionHeightmap`. Se mantiene la fórmula `gris / 255 * maxHeight` y
la validación de formato, dimensiones, relleno de filas y longitud del archivo.
El cargador admite BI_RGB de 24/32 bits, entre 2 y 1024 muestras por eje.

La malla tiene 263,169 vértices y 524,288 triángulos con índices de 32 bits.
La separación es de 2.25 unidades. Las normales se calculan a partir de sus
triángulos; la consulta de altura interpola los mismos triángulos e incluye
la traslación. Pablito, la nube y los objetos consultan esa misma superficie.
La nube sigue limitada al interior del terreno. La cámara comprueba el terreno
para acortar su brazo cuando una ladera se interpone.

## Materiales y profundidad del paisaje

`pixelShaderMultitextura.ps` usa las texturas existentes de pasto
(`Meshpart1_diff.png`), tierra (`dirt_diff.png`) y roca (`rock.jpg`).
El pasto mezcla dos escalas y variación amplia de color para reducir la
repetición; los caminos usan tierra cálida, con bordes irregulares y una
transición de grava hacia la pradera. Se conserva el detalle de las texturas
con un contraste más suave para acompañar el estilo de Pablito.

La roca aparece principalmente en las pendientes fuertes y los escarpes;
la influencia de la altitud es leve para conservar pasto sobre las cumbres y
las mesetas. Su proyección triplanar evita estirarla por paredes altas.
Una neblina suave por distancia separa las montañas del primer plano y adapta
su color al día y la noche. Los materiales siguen recibiendo la iluminación
y las sombras comunes de `SunLighting.hlsli`, `SunLight.h` y `DayNightCycle.h`.

La máscara se aplica una sola vez sobre todo el mapa, con UV independientes de
las texturas repetidas. Usa t6/s3 para respetar los recursos del sol, la luna
y las estrellas en t3-t5/s1-s2. Un sampler CLAMP evita repetir los caminos en
los extremos. La cámara tiene alcance de 2,500 unidades y el cielo radio 2,000
para que el fondo no corte las montañas distantes.

## Verificación

1. Abrir `../DXClase2.sln`, seleccionar `Debug | Win32` y compilar.
2. Ejecutar desde la carpeta del proyecto, que contiene shaders y recursos.
3. Recorrer las rutas caminando y con la nube; mirar las laderas, las cumbres,
   la plaza y los bordes de los caminos. C alterna cámara; E sube/baja.
4. `python Tools/check_valley.py` comprueba los recursos generados.

Se verificaron la compilación y el enlace con MSBuild, la compilación real de
los shaders al ejecutar, las alturas y normales del terreno, la carga GPU de
la máscara y el sampler, y la terraza del pueblo. La prueba de los archivos
comprueba los 36,741 píxeles conectados de camino, el uso de los 256 valores
del byte bajo y las pendientes de los trazados. El SDK DirectX de junio de
2010 mantiene advertencias de compilación heredadas de la plantilla.
