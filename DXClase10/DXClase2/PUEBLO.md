# Casa, entregas, río y vehículo

## Nube

El punto de subir usa el asiento real de `pinwinube.txt`: se calcula con un
rayo vertical sobre el centro de su malla. Antes se usaba el máximo de una nube
lateral y se añadían 1.65 unidades al objetivo de E, que quedaba fuera del alcance
vertical del selector. La holgura bajo la nube es ahora 0.25 y el alcance se
calcula con su tamaño real.

Pablito mantiene los pies sobre el asiento al conducir. La cámara exige tercera
persona durante el trayecto y recupera la perspectiva anterior al bajar. Se
restauraron las transiciones visuales de subir/bajar y el giro suave del modelo.
La bajada busca una posición libre con soporte y comprueba el recorrido de la
cápsula. El volumen de choque abarca tanto la nube como al pasajero.

El vehículo sigue el terreno y los tableros del puente, y flota sobre el agua.
Las colisiones con paredes, barandillas y otros modelos permanecen activas.

## Casa principal y cocina

Se mantiene `Models/pinwichoza.txt`, con origen (384, 30.08, 435) y giro de 65°.
Su entrada real está en el extremo +X local, aproximadamente (395.8, 411.3) en
el mapa. Todas las paredes conservan sus colisiones y la puerta queda libre.

Los dos suministros de ingredientes, la mesa de frascos, el mortero, el caldero,
el fogón y la embotelladora se colocan con coordenadas locales de la casa en
`AlchemyScene` y `DXRender::UpdateImportedModels`. Las tuberías se reducen para
caber en la zona del fogón. El pasillo central queda despejado. Un suelo de
madera y una contribución ambiental cálida limitada al interior permiten leer
los modelos bajo el techo sin cambiar la iluminación del exterior.

## Casas secundarias y entregas

El modelo pequeño cerrado `Models/PablitoPinwi/casa_pablo.txt` se instancia tres
veces a escala 2.2, orientado hacia el camino. Cada parcela tiene su propia
señal, letrero y objetivo de entrega junto a la fachada:

| Destino | Centro X/Z de la casa | Ramal |
| --- | --- | --- |
| Vecina | 940 / 1063 | Norte |
| Mensajero | 1039.5 / 760.3 | Este |
| Guardabosques | 166.8 / 987.7 | Oeste |

Se conservan las tres recetas, sus destinos, los plazos y las reglas de entrega.
Reiniciar repone ingredientes y estaciones en la nueva distribución.

## Río y puente

El río recorre las 1,152 unidades del mapa de norte a sur. Su anchura varía de
24 a 34, con un lecho situado 1.25 unidades bajo el agua y orillas excavadas en
el terreno. El agua conserva ondas, corrientes y reflejos. Su textura avanza
hacia el sur, siguiendo la pendiente del cauce. La vegetación deja libres el
agua, las orillas y las nuevas construcciones.

El puente del ramal occidental mide 86 por 16 unidades. Usa tablones y
barandillas con colisiones. Sus rampas están integradas en el terreno y su
pendiente coincide con el camino. Se corrigieron las normales de las caras de
los cubos para que las piezas anchas no presenten triángulos oscuros.

## Edición y verificación

`WorldLayout.json` es la fuente de coordenadas de casas, entregas, río y puente.
`Tools/world_layout.py` genera `WorldLayout.h`, compartido por el renderizador,
la escena y el agua. El encabezado y los mapas generados se incluyen en el repo;
Python no es necesario para ejecutar o compilar el juego.

Para modificar el trazado y reconstruir los mapas:

```powershell
python Tools/generate_valley.py
python Tools/check_valley.py
```

La verificación usa DirectX real: E para subir y bajar, bloqueo de C al conducir,
entrada física por la puerta, conectividad de cada objetivo del taller, recetas
y entregas, cruce del puente a pie y en vehículo, y liberación de los recursos.
La prueba de terreno valida las parcelas niveladas, el cauce y la conexión de
todos los ramales a través del puente.

Resultados: Debug y Release Win32 correctos; 36 comprobaciones de distribución,
puente y nube; 90 comprobaciones de jugabilidad y alquimia; y 428 comprobaciones
de trayectos reales en vehículo por los tres caminos, con llegada y bajada en
cada destino. Los mapas y las pendientes también pasan `Tools/check_valley.py`.
