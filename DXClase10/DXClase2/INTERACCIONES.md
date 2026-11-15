# Indicadores de interacción

`Texturas/Tecla-E.png` es una copia exacta de la imagen proporcionada,
incluyendo su transparencia. Se carga una vez para todos los indicadores.

La nube muestra la tecla E sobre su posición cuando `CanMountCloudVehicle()`
permite subir: dentro del alcance horizontal y vertical, estando a pie.
Al subir o alejarse desaparece; vuelve a estar disponible al bajar junto a ella.
El indicador funciona en primera y tercera persona. Los objetivos fuera de
pantalla o detrás de la cámara no generan un icono.

## Agregar otro objeto interactuable

`InteractionPrompt.h` es un dibujante compartido, independiente de la nube.
Cada objeto proporciona un punto del mundo y la misma condición de
disponibilidad que usa su acción de E. En el pase final de `DXRender::DrawScene`,
después de la geometría y antes de `Present`, agregar:

```cpp
interactionPrompt.Draw(*camara,
    objetoPosition + D3DXVECTOR3(0, alturaDelIndicador, 0),
    PuedeInteractuarConObjeto());
```

Se puede llamar varias veces para distintos objetos sin volver a cargar la
imagen. El cuarto argumento opcional ajusta el tamaño del cuadro en píxeles
(64 por defecto). La nube usa un punto a 1.65 unidades sobre su cubierta.
El pequeño desplazamiento vertical de pantalla separa la tecla de ese punto.

El icono se proyecta desde el mundo y mantiene un tamaño legible al mover la
cámara. Es una capa de interfaz: no recibe luz ni sombras y conserva sus
colores de día y noche. Usa mezcla alfa para respetar los bordes transparentes,
no escribe profundidad y se dibuja sobre la escena. Restaura los estados de
mezcla, profundidad y rasterizado al terminar; los shaders y la entrada de
geometría se establecen de nuevo en el siguiente pase de la escena.

## Verificación

- Compilación y enlace de `Debug | Win32`.
- Ejecución de la escena con compilación real de ambos shaders del indicador.
- Prueba temporal de integración C++: alcance, subida/bajada, ambas cámaras,
  descarte de objetivos detrás de cámara o fuera de pantalla, anclas de otros
  objetos, restauración de estados y liberación de recursos.
- Comparación de imágenes dibujadas por la GPU con y sin indicador: el cambio
  queda limitado al icono, sus esquinas permanecen transparentes y la E blanca
  conserva su color en la escena nocturna.
