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
  estrellas aparecen suavemente y el fondo pasa a un gradiente azul oscuro.
- La luna utiliza la textura entregada y una máscara circular para evitar que
  sus esquinas negras tapen un rectángulo del cielo. Tiene un halo suave.
- Durante la noche, la luna ilumina en azul y proyecta sombras reales usando
  el mismo sistema de profundidad del sol. La luz ambiente conserva visibilidad
  del terreno y los objetos.
- El cambio de luz solar a lunar ocurre en el horizonte con ambas intensidades
  directas en cero, de modo que no aparece un salto visible de sombras.
- Las estrellas giran lentamente con la hora. Al cruzar medianoche, el reloj,
  las órbitas y la textura panorámica continúan sin un reinicio visible.

## Texturas integradas

`Texturas/T_Moon.png` (1024 × 1024, RGBA) y `Texturas/T_Stars.png`
(8192 × 4096, RGB) son copias exactas de los archivos proporcionados.
Se conservan sus resoluciones, colores y datos originales. El brillo se ajusta
en el shader, sin modificar las imágenes.

## Ajustes

En `DayNightCycle.h`:

| Ajuste | Valor inicial | Uso |
| --- | --- | --- |
| `hour` | 9.0 | Hora inicial del juego |
| `dayLengthSeconds` | 300.0 | Segundos reales por vuelta completa |
| `paused` | false | Estado inicial del reloj |
| Inclinación de la órbita | 25° | Elevación máxima del sol y la luna de 65° |
| Intensidad solar máxima | 0.95 | Luz directa de día, en `GetState()` |
| Intensidad lunar máxima | 0.25 | Luz directa de noche, en `GetState()` |
| Ambiente nocturno | (0.08, 0.11, 0.18) | Claridad y tono de las zonas sin luz directa |

En `SunLight.h`, `moonRadiusDegrees = 1.5` ajusta el tamaño de la luna y
`starsExposure = 1.5` el brillo de las estrellas. Se mantienen los ajustes
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
