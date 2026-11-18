# Ciclo de día y noche

El juego comienza a las **09:00** y recorre automáticamente las 24 horas en
**300 segundos reales (5 minutos)**. La hora aparece en el título de la ventana.
El sol sale a las 06:00, alcanza 65 grados de elevación al mediodía y se pone a
las 18:00. La luna llena recorre el lado opuesto del cielo durante la noche.

## Controles

| Acción | Teclado | Gamepad XInput |
| --- | --- | --- |
| Pausar / reanudar el reloj | F6 | — |
| Acelerar el tiempo 30 veces | Mantener F7 | Mantener → de la cruceta |
| Cambiar primera / tercera persona | C | ↑ de la cruceta |

F6 congela la hora y permite seguir moviendo al personaje y la cámara. El título
indica «Tiempo pausado». Al soltar F7 o la cruceta derecha vuelve la velocidad
normal. Una vuelta completa con aceleración tarda unos 10 segundos.

## Transiciones

- El amanecer y el atardecer mezclan gradualmente el cielo diurno con tonos
  cálidos cerca del horizonte. La luz directa cambia de color y de intensidad.
- Al oscurecer, desaparece el disco solar y se desvanece el cielo diurno. Las
  estrellas pintadas aparecen con una edición nocturna del mismo cielo diurno.
  Ambas imágenes comparten las coordenadas UV para conservar las nubes.
- La luna utiliza una edición pintada de la textura entregada y una máscara circular para evitar que
  sus esquinas negras tapen un rectángulo del cielo. Tiene un halo suave.
- Durante la noche, la luna ilumina con un tono azul suave y proyecta sombras
  reales usando el mismo sistema de profundidad del sol. El ambiente nocturno
  elevado permite ver el terreno, los caminos y los objetos incluso en sombra.
  El cielo y la bruma también son más claros para distinguir las montañas.
- El cambio de luz solar a lunar ocurre en el horizonte con ambas intensidades
  directas en cero, de modo que no aparece un salto visible de sombras.
- Las estrellas forman parte de la pintura nocturna y permanecen con sus nubes.
  Al cruzar medianoche, el reloj y las órbitas continúan sin un reinicio visible.

## Texturas integradas

`Texturas/skybox.png` se conserva como cielo de día. `Texturas/skybox-night.png`
es una edición de esa misma pintura: azul índigo, nubes lavanda y estrellas
sencillas, sin galaxias fotográficas. `Texturas/T_Moon.png` es una edición de la
luna existente, con pinceladas amplias y manchas simplificadas. La fotografía
`Texturas/T_Stars.png` se conserva en el repositorio pero ya no se carga.
Las dos ediciones se hicieron con la herramienta integrada de edición de imágenes,
usando los archivos existentes como objetivos de edición, sin crear escenas desde cero.

## Ajustes

En `DayNightCycle.h`:

| Ajuste | Valor inicial | Uso |
| --- | --- | --- |
| `hour` | 9.0 | Hora inicial del juego |
| `dayLengthSeconds` | 300.0 | Segundos reales por vuelta completa |
| `paused` | false | Estado inicial del reloj |
| Inclinación de la órbita | 25° | Elevación máxima del sol y la luna de 65° |
| Intensidad solar máxima | 0.95 | Luz directa de día, en `GetState()` |
| Intensidad lunar máxima | 0.60 | Luz directa de noche, en `GetState()` |
| Ambiente nocturno | (0.32, 0.38, 0.48) | Claridad y tono de las zonas sin luz directa |

En `SunLight.h`, `moonRadiusDegrees = 1.15` ajusta el tamaño de la luna (23 %
menor en diámetro que antes) y `nightSkyExposure = 1.0` el brillo de la pintura
nocturna. Se mantienen los ajustes
`diskRadiusDegrees`, `shadowCoverage`, `shadowDepth`, `shadowStrength` y
`ShadowResolution` del sistema de sombras existente. La dirección, color,
ambiente e intensidad de la luz activa los calcula ahora `DayNightCycle`.

Para cambiar una hora desde código, usar `dayNight.SetHour(18.0)`. El método
normaliza horas positivas o negativas a [0, 24). No se usa la hora del sistema.
El reloj utiliza tiempo real transcurrido; el límite de paso de la física del
personaje no ralentiza el ciclo cuando hay pocos cuadros por segundo.

## Archivos y verificación

`DayNightCycle.h` calcula el reloj, las órbitas y las curvas de transición.
`SunLight.h` sincroniza las luces, el mapa de sombras y las texturas del cielo.
`pixelShaderSky.ps` compone cielo diurno, crepúsculo, estrellas, sol y luna.

Se compiló `Debug | Win32` y se revisaron las vistas nocturna, lunar y de
atardecer en ejecución. `Tools/check_day_night.cpp` es una prueba independiente
de CPU: verifica día/noche, medianoche, pausa, duración, independencia de FPS
y 1,440 muestras de transición. Se compila por separado con las mismas rutas
de Include/Lib x86 del SDK de DirectX que usa el proyecto.

Una prueba temporal de Direct3D 11 WARP leyó píxeles reales para verificar luz
y sombra de la luna, máscara circular, aparición/desaparición de estrellas y
varias horas del día. La capa de depuración no reportó advertencias ni errores.
También pasaron las regresiones de personaje y cámaras, F6 con repetición
ignorada y aceleración mediante entrada XInput simulada.

## Indicaciones de las ediciones de imagen

Herramienta: editor de imágenes integrado (`image_gen`), modo edición de archivos
existentes. Objetivos: `skybox.png` y `T_Moon.png`; el cielo diurno también sirve
como referencia de estilo para la luna. Indicaciones enviadas:

**Cielo nocturno**

> Use case: lighting-weather. EDIT TARGET: the supplied existing 2048x1024 game skybox texture. Convert THIS SAME painting to night; do not create a different scene. Preserve exactly the panoramic composition, cloud silhouettes and locations, horizon at middle, texture orientation, painterly brushwork, and unadorned bottom hemisphere. Recolor the blue sky to deep navy/indigo and the cream clouds to muted dusty blue/lavender moonlit clouds. Keep clouds legible against dark sky but subdued. Add sparse, small simple painted ivory/pale-blue stars ONLY in the clear sky between clouds, not on clouds, no galaxies or Milky Way or photographic noise. Keep a friendly stylized storybook game look consistent with the source. No moon and no sun embedded, those are separately rendered by the engine. Preserve 2:1 image and original UV layout; avoid added scenery, text, trees, mountains or any new cloud formations. Left/right wrap should be unobtrusive. This is an edit of the existing texture, not a new sky.

**Luna**

> Use case: style-transfer. EDIT TARGET image1 existing circular full Moon texture. Image2 is supporting STYLE reference only, the painterly existing game sky. Simplify and repaint THIS SAME full Moon: keep centered circular full disk, exact silhouette/size filling the square edge to edge, and existing broad dark lunar marking locations. Remove photographic detail, fine craters, sharp realistic surface grain. Use soft broad painterly brush strokes and a few large simplified moon markings, friendly stylized game art matching image2. Pale ivory/light muted blue disk with subtle lavender blue-gray markings, bright but with gentle contrast; no face, no extra objects, no halo, no stars. Outside the exact circular disk retain pure black, no extra padding. The engine will reduce on-screen size separately. Do not generate a different moon or change the full circle to a crescent. Output square texture.
