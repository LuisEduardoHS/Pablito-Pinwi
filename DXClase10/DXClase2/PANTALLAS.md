# Pantallas ilustradas

- Inicio: `Texturas/Pantallas/letrero-pablitopinwi.png` y `boton-jugar.png`,
  copiados de los archivos del usuario sin modificar píxeles ni transparencia.
- Resultado: se mantienen `ganaste.png` y `has-perdido.png`. Los botones
  `boton-reiniciar.png` y `boton-salir.png` son ediciones del botón Jugar con
  lettering blanco, relleno morado y borde blanco redondeado.
- Fondo azul discreto, sin paneles ni texto adicional. Solo se muestra el
  letrero y los botones de cada pantalla; no hay modelos ni HUD de partida.

`Gameplay/GameScreens.h` usa `Ui::HudImage` para ajustar las zonas visibles,
ignorando el amplio margen transparente de las imágenes. Conserva su relación
de aspecto y compone todo en una textura RGBA que se actualiza cuando cambia
la selección, la pulsación o el tamaño del viewport. Ya no necesita fuentes ni
recursos GDI para dibujar los menús.

La escala se calcula respecto de 800×600 y tiene un máximo de 1.5 para evitar
botones excesivos en monitores grandes. `Layout` define tanto la colocación
como el área pulsable; se mantienen los clics escalados al tamaño de cliente,
la cancelación al soltar fuera, Tab/flechas y Enter/Espacio. Un halo azul claro
marca la selección y el botón se comprime ligeramente mientras está pulsado.
Las acciones Start/Restart/Exit, la pausa en menús y la jugabilidad no cambian.

Verificación: Debug y Release Win32, capturas reales de DirectX a 800×600 y
1200×800, y 32 comprobaciones de flujo de menús por resolución: inicio,
temporizadores detenidos, mouse/teclado, victoria/derrota, reiniciar, salir y
clics después de estirar la ventana.

## Edición de los botones adicionales

Se utilizó la herramienta integrada `image_gen` en modo edición, con
`boton-jugar.png` como objetivo y salida con transparencia. Los originales del
usuario no se editaron. Estas fueron las indicaciones enviadas:

**Reiniciar**

> Use case: text-localization. EDIT TARGET: supplied purple JUGAR game button PNG. Change only the lettering to exact Spanish word "REINICIAR". Keep the identical hand-drawn friendly chunky rounded white lettering style, original lavender purple fill, rounded rectangular shape, thin slightly handmade white border and small white bottom/right offset. Preserve button aspect ratio approximately 3.4:1, original silhouette and colors, no extra ornaments or icons, no gradient, no 3D effects, no black background. Center new word inside button with comfortable padding; scale lettering down only enough to fit the longer word clearly. Genuine transparent background around button. This is the same button edited to a new action, not a redesign. Output a single button.

**Salir**

> Use case: text-localization. EDIT TARGET: supplied purple JUGAR game button PNG. Change only the lettering to exact Spanish word "SALIR". Keep the identical hand-drawn friendly chunky rounded white lettering style, original flat lavender purple fill, rounded rectangular shape, thin slightly handmade white border and small white bottom/right offset. Preserve button aspect ratio approximately 3.4:1, original silhouette and colors, no extra ornaments or icons, no gradient, no 3D effects, no black background. Center new word inside button with the same padding and visual lettering size as JUGAR. Genuine transparent background around button, clean edges without speckles. This is the same button edited to a new action, not a redesign. Output a single button.
