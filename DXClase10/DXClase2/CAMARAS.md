# Pablito y cámaras

Pablito es el personaje controlable y aparece en tercera persona al iniciar.
Se usa el modelo y la textura actuales, sin animaciones. Su posición inicial
es X=384, Z=380; los pies siguen la altura del terreno y el salto.

| Acción | Teclado y mouse | Gamepad XInput, mando 1 |
| --- | --- | --- |
| Primera / tercera persona | C | Arriba de la cruceta (D-pad ↑) |
| Mover a Pablito | WASD | Stick izquierdo |
| Mirar / orbitar | Mouse | Stick derecho |
| Correr | Mantener Shift | Mantener L3 |
| Saltar | Espacio | A |
| Subir / bajar de la nube cercana | E | — |

Cada pulsación cambia de cámara una sola vez. C usa `WM_KEYDOWN` y descarta
la repetición del teclado; la cruceta detecta el paso de suelto a presionado.
El gamepad solo actúa mientras la ventana del juego tiene el foco.

Ambas vistas comparten la posición del personaje y los ángulos de mirada:
cambiar de modo no teletransporta a Pablito ni reinicia su salto. En tercera
persona, gira hacia su dirección de movimiento y la cámara orbita a su alrededor.
En primera persona, mira en la dirección de la cámara y su malla se oculta
para que la cabeza no tape la vista; su sombra permanece en el terreno.

## Ajustes

- `DXRender::playerPosition`: posición de los pies; ya no se coloca otra
  instancia fija de Pablito junto a la mesa.
- `pablitoScale`: escala del modelo, inicialmente 1.
- `DXRender::UpdatePlayerCamera()`: ojos al 86% de la altura del modelo y
  distancia de tercera persona de 2.5 veces esa altura.
- `DXRender::InitScene()`: inclinación inicial de 10 grados hacia abajo.
- `Camara::UpdateRotation()`: límite vertical de ±85 grados; el movimiento
  siempre usa los ejes horizontales aunque se mire arriba o abajo.

La cámara de tercera persona comprueba los modelos entre los ojos y su posición
deseada, y se acerca con un margen de 0.3 unidades si encuentra una pared.
Después comprueba el terreno con 48 muestras y un margen de 0.4 unidades.
`COLISIONES.md` describe la generación automática de colisiones de los modelos.

La malla exportada mira hacia +X. Su matriz aplica un desfase de −90 grados
respecto al rumbo de Pablito para alinearla con el avance. La actualización
procesa entrada, salto, cámara, modelo y sombra antes de dibujar el mismo cuadro.

## Vehículo nube provisional

Una plataforma rectangular gris aparece frente a Pablito, en X=384, Z=384,
al alcance de E desde el inicio. E sube cuando está a un máximo de 4.5 unidades
del centro y a menos de 3 unidades de diferencia vertical respecto a la cubierta;
otra pulsación baja al terreno junto al costado derecho. Mantener E no repite
la acción. La barra de título indica cuándo se puede subir y cómo bajar.
También aparece la imagen de la tecla E sobre la nube al poder subir; se
oculta mientras se conduce. `INTERACCIONES.md` explica cómo usar el mismo
indicador en los próximos objetos interactuables.

Al subir, Pablito queda de pie sobre la plataforma y la maneja con WASD o el
stick izquierdo, en relación con la orientación de la cámara. Se mueve a
28 unidades por segundo, frente a 8 caminando y 16 corriendo; la diagonal no
añade velocidad. La nube flota siguiendo el relieve y permanece dentro del
terreno. Al bajar queda estacionada para poder volver a subir.

La cámara pasa obligatoriamente a tercera persona, con más distancia para
encuadrar el vehículo. C y D-pad ↑ quedan bloqueados durante el manejo.
Al bajar se recupera la perspectiva que estaba activa antes de subir.
El salto se cancela al montar y se desactiva mientras se conduce.
La nube también se detiene ante los modelos del escenario. Al bajar se busca
un costado libre y se comprueba que Pablito pueda llegar sin atravesar paredes.

El tamaño provisional es 4 × 0.6 × 5 unidades, con 0.8 unidades de separación
del terreno bajo la plataforma. `cloudSize`, `cloudSpeed`, `cloudHoverHeight`
y `cloudPosition` se ajustan en el constructor de `DXRender`. La malla
`cloudVehicleMesh` usa un cubo procedural y un material gris creado en GPU,
sin depender de un modelo o una textura nuevos. Su transformación y sombra se
actualizan con la posición del vehículo; el modelo definitivo se puede integrar
en ese punto conservando el comportamiento.

## Verificación

- Compilación y enlace de `Debug | Win32` correctos con MSBuild.
- Ejecución real: tercera persona al iniciar, C a primera persona y C de vuelta.
- Prueba temporal de integración C++ con entrada XInput simulada: pulsación,
  mantenimiento y liberación de D-pad ↑; C y repetición; caminar/correr;
  orientación y posición del modelo; salto, cambio de vista en el aire y aterrizaje.
- 1,100 combinaciones de ubicación, rumbo e inclinación: matrices finitas,
  cielo alineado y cámara por encima del terreno. El brazo se acortó en 437 casos.

La entrada del gamepad se verificó mediante simulación; queda pendiente probar
la sensación de manejo con un mando físico.

Para el vehículo se verificó la compilación `Debug | Win32` y la ejecución
real: E desde primera persona, subida a tercera persona, bloqueo con C y
bajada recuperando la vista anterior. Una prueba temporal de integración C++
ejecutó 5,841 comprobaciones con entrada XInput simulada: distancia de acceso,
repetición de E, salto bloqueado, velocidades a 15/60/144 FPS, diagonal,
estacionamiento, límites del terreno y apoyo del vehículo sobre pendientes
en 968 combinaciones de ubicación y rumbo. También pasó el dibujo de la escena
y la liberación del recurso provisional.
