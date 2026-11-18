# Prototipo de alquimia — Pablito Pinwi

## Rama y diagnóstico previo

Se ejecutó `git switch main`, `git pull origin main` y se creó
`feat/alchemy-gameplay`. El árbol estaba limpio y main ya estaba actualizado.
No se realizaron commits ni pushes.

Se revisó el repositorio (146 archivos antes de estos cambios). La solución
activa es `../DXClase2.sln` y su proyecto `DXClase2.vcxproj`; la carpeta
`../Objetos/plantilla` corresponde a otra plantilla y no se modifica.

La plantilla ya incluía teclado por WinAPI/DirectInput, XInput, Pablito con
movimiento y salto, primera/tercera persona, la nube con E, transformaciones
de `MPrimitives`, colisiones de malla y cápsula, terreno, SkyDome, shaders,
sombras, día/noche e indicador E. Se conservaron esos sistemas. No había una
simulación de alquimia, pedidos, transporte ni texto HUD general.

Los riesgos principales eran activar dos acciones con E, duplicar el modelo
transportado, bloquearlo con su propia colisión y contar fallos varias veces.
La implementación usa un candidato por pulsación, una identidad/estado por
objeto, colisión desactivada para los pequeños objetos transportables y un
cierre único por pedido. La lógica se comprobó primero con Energía completa
y después se amplió a las demás recetas y casos de error.

## Ejecutar

1. Abrir `../DXClase2.sln` en Visual Studio con el toolset v143 y el SDK DirectX
   de junio de 2010 instalado.
2. Elegir `DXClase2` como proyecto de inicio y `Debug | Win32` o `Release | Win32`.
3. Compilar y ejecutar con F5/Ctrl+F5. El directorio de trabajo debe ser esta
   carpeta, donde están los shaders, `Texturas` y `Models`.
4. Al comenzar aparece el pedido de Energía y su receta. R libera espacio de
   pantalla al ocultar la ficha; vuelve a pulsarla para consultar.

El EXE de Debug se genera en `../Debug/DXClase2.exe`. Para iniciarlo desde una
terminal, situada en la carpeta `DXClase2`, usar:

```powershell
& ../Debug/DXClase2.exe
```

No se añadieron bibliotecas externas. El HUD provisional dibuja texto con GDI
en una textura RGBA y la presenta con DirectX 11 usando el mismo dibujante de
cuadriláteros del icono E. No usa ventanas MessageBox para los resultados.

## Controles

| Control | Acción |
| --- | --- |
| WASD / stick izquierdo | Mover a Pablito o conducir |
| Mouse / stick derecho | Mirar |
| E | Una interacción contextual: recoger, colocar, operar, entregar o nube |
| Q | Soltar el objeto en una superficie libre cercana |
| R | Mostrar/ocultar la receta |
| Clic / Enter / Espacio | Activar el botón del menú |
| Tab / flechas | Cambiar de botón en los menús |
| C / D-pad arriba | Primera/tercera persona; bloqueada al conducir |
| Shift / L3 | Correr |
| Espacio / A | Saltar a pie |
| F6 / F7 | Mantener los controles anteriores de día/noche |
| Esc | Cerrar |

Las acciones E/Q/R y las teclas de menú descartan la repetición de `WM_KEYDOWN`. Una pulsación
de E elige un solo candidato por alcance, distancia, altura, orientación y
visibilidad. Al conducir, E se reserva para bajar. Se puede llevar un objeto
en la nube. El tiempo del pedido y las operaciones se pausan cuando la ventana
pierde el foco; F7 no acelera la preparación ni los pedidos.

## Taller y modelos

Se reutilizan el modelo de mesa en los mostradores, el modelo de poción como
frasco/poción transportable, el horno existente como fogón, la casa, el
personaje y la nube. Los ingredientes, mortero, caldero y mecanismo de
embotellado usan primitivas coloreadas. No se modificaron assets artísticos.

| Elemento | Ubicación X/Z | Identificación |
| --- | --- | --- |
| Hongo, flor, cristal | Mesa 375 / 399 | Rojo, azul, violeta |
| Agua, hierba, raíz | Mesa 375 / 407 | Cian, verde, amarillo |
| Frasco vacío | Mesa original 384 / 403 | Modelo de poción existente |
| Mortero | 390 / 395 | Recipiente sobre mesa |
| Caldero | 401 / 392 | Olla violeta; líquido cambia con el estado |
| Fogón | 394 / 412 | Horno existente; indicador naranja al calentar |
| Embotellado | 390 / 407 | Mesa con soporte metálico |
| Vecina | 367 / 419 | Señal dorada junto a la casa |
| Mensajero | 417 / 424 | Señal azul |
| Guardabosques | 360 / 445 | Señal verde |

La altura se obtiene del terreno y de la mesa. Las estaciones sólidas usan las
colisiones automáticas existentes. Un objeto recogido desaparece de su lugar
original y su única instancia lógica sigue una transformación de transporte
relativa al jugador (o a la cámara en primera persona). Al soltarlo conserva
identidad, tipo, triturado y metadatos de poción. La escala no cambia al cargarlo.
Las existencias se reponen al comenzar el siguiente pedido, nunca al recoger
el mismo objeto. Las cantidades del catálogo determinan las existencias.

## Primera receta completa

1. Acercarse a la mesa izquierda y recoger el **hongo rojo** con E.
2. Ir al caldero violeta y pulsar E para depositarlo.
3. Repetir con la **flor azul** y el **agua**. Solo se carga una unidad cada vez.
4. Con la mano libre, E en el caldero inicia tres segundos de mezcla.
5. Recoger el **frasco vacío** de la mesa original.
6. E en embotellado coloca el frasco; otra E embotella cuando la mezcla está lista.
7. Llevar la poción a la **señal dorada** y pulsar E para entregar.
8. Tras tres segundos se inicia el siguiente pedido y se reponen los ingredientes.

No hace falta calentar Energía. Para Velocidad y Vitalidad hay que controlar
el fogón: E enciende, E apaga; el panel muestra segundos y progreso. Puede
apagarse incluso mientras se lleva un frasco. Un apagado temprano permite
continuar calentando antes de sellar; embotellar con calor insuficiente crea
una poción incorrecta. Superar el máximo quema definitivamente la mezcla.

El mortero requiere E para colocar, E para iniciar y, tras tres segundos, E
para recuperar el ingrediente triturado. En Vitalidad se deposita primero el
agua y después la raíz ya triturada, respetando el orden de la ficha.

## Recetas y operaciones

| Receta | Ingredientes (una unidad por entrada) | Mezcla | Calor | Destino |
| --- | --- | --- | --- | --- |
| Energía | Hongo rojo, flor azul, agua | 3 s | Sin calor | Vecina |
| Velocidad | Hierba verde, cristal mágico, agua | 3 s | 3–6 s | Mensajero |
| Vitalidad | Agua → raíz triturada → hierba verde → hongo rojo | 4 s | 5–8 s | Guardabosques |

El catálogo contiene IDs, cantidades, procesamiento obligatorio, orden opcional,
duración de mezcla, intervalo de calor, destino y límite de tiempo. No se
valida solo el número de objetos: se comprueban dosis, tipos, triturado, orden,
mezcla y calor. La poción conserva receta, número del pedido y calidad.

El caldero acepta hasta cuatro unidades y cierra la incorporación al mezclar.
El fogón controla lógicamente el contenido del caldero; no se traslada la olla
entre estaciones en este prototipo. Embotellado necesita un frasco colocado,
una mezcla terminada y el fogón apagado. No puede generar pociones vacías ni
duplicar una mezcla ya embotellada.

## Pedidos, fallos y final

Solo hay un pedido activo. Se alternan las recetas del catálogo; Energía y
Velocidad conceden 180 segundos y Vitalidad 210. El tiempo y las reglas son
configurables en `Recipe` y `Rules`.

- Cinco entregas correctas producen victoria.
- Tres fallos producen derrota.
- Fallan la mezcla inválida, el sobrecalentamiento, el tiempo agotado y la
  entrega con calidad, receta, pedido o destino incorrectos.
- El mismo pedido solo puede cerrarse una vez. La espera entre pedidos no
  vuelve a procesar ni contabilizar el fallo anterior.
- La entrega retira la poción. El botón Reiniciar del resultado restablece
  contadores, pedido, objetos, estaciones, posiciones de Pablito/nube,
  cámara en tercera persona y reloj a las 09:00.

## Inicio y pantallas de resultado

Cada ejecución abre una pantalla con el letrero ilustrado Pablito Pinwi y
un único botón Jugar. La escena se carga previamente; los pedidos, el reloj,
el movimiento y las interacciones permanecen detenidos hasta pulsar el botón.
Jugar entra en la escena existente, en su punto inicial y con el tiempo completo.

Victoria y derrota abren pantallas independientes con los letreros originales
`Texturas/Pantallas/ganaste.png` y `has-perdido.png` y dos botones funcionales:
Reiniciar y Salir. Solo se muestran el letrero y los botones, sin el personaje
ni contadores adicionales. El primero comienza otra partida
directamente; el segundo cierra el juego por la ruta normal de la ventana.
El cursor queda libre en los menús y el mouse vuelve a controlar la cámara al
jugar. Un clic solo se activa si se suelta sobre el mismo botón donde comenzó.
También se puede navegar con Tab/flechas y activar con Enter/Espacio.

`Gameplay/GameScreens.h` concentra las imágenes, la distribución de los botones
y su selección. Las imágenes originales de inicio se conservan en
`Texturas/Pantallas`; Reiniciar y Salir son variantes del botón Jugar. El fondo
azul, las proporciones y la indicación de selección se describen en `PANTALLAS.md`.
Se conservan las acciones Start/Restart/Exit y el flujo de `DXRender`.

El HUD muestra receta, destino, requisitos, tiempo, transporte, éxitos/fallos,
acción contextual, estados/progreso de estaciones y avisos durante la partida.
Su diseño utiliza texto monoespaciado pequeño, colores suaves y contorno oscuro,
sin fondos ni tarjetas. La receta y los avisos aparecen letra por letra; el
reloj, las acciones y las lecturas de estaciones se muestran inmediatamente.
La ficha se abre/cierra con R y las estaciones solo muestran texto cuando están
en uso o son el objetivo cercano. El icono E del mundo mide 42 píxeles.
Las pantallas inicial y de resultado reemplazan completamente el HUD.

Cada ingrediente de la receta lleva su imagen de `Texturas/Ingredientes/`
en una casilla transparente de 20 píxeles. El ingrediente que lleva Pablito
aparece también junto a MANO en una casilla de 24 píxeles. Los iconos de la
receta aparecen con cada línea del texto progresivo y se ocultan al cerrar R.
Se conservan los nombres, cantidades, orden y requisito de triturado.
Las imágenes mantienen proporciones y transparencia; los PNG originales no
se modifican. Frascos y pociones conservan su texto, al no tener icono asignado.

## Archivos y responsabilidades

Creado:

- `Gameplay/InteractionSystem.h`: posiciones, objetivos y selección reutilizable.
- `Gameplay/AlchemyGame.h`: ingredientes y estados, transporte, cuatro estaciones,
  catálogo/validación, pedidos, entregas, temporizadores, victoria/derrota/reinicio.
  Solo usa C++ estándar y no depende de DirectX ni de nombres de mallas.
- `Gameplay/AlchemyScene.h`: reutilización de modelos, primitivas, poses, dibujo
  y sombras del taller, objetos y señales; gestión de sus recursos.
- `Gameplay/AlchemyHud.h`: presentación provisional mediante texto y barras.
- `Gameplay/IngredientIcons.h` / `HudImage.h`: asignación de ingredientes y composición de PNG con transparencia y proporciones conservadas.
- `Gameplay/GameScreens.h`: inicio, victoria/derrota y botones provisionales.
- `Gameplay/PablitoAnimator.h` / `PablitoRigData.h`: poses, transiciones y rig de Pablito; ver `ANIMACIONES.md`.
- `Tools/check_alchemy.cpp`: regresiones de lógica sin ventana ni dispositivo GPU.
- `JUGABILIDAD.md`: esta guía y el diagnóstico de integración.

Modificado:

- `main.cpp`: entrada E/Q/R/Enter sin repetición.
- `DXRender.h`: enlace entre simulación, selección, transporte, escena y HUD.
  Se conserva el bucle, física, cámaras y sistemas gráficos existentes.
- `InteractionPrompt.h`: comparte su dibujante transparente con una textura HUD.
- `DXClase2.vcxproj` / `.filters`: archivos nuevos, compilación UTF-8 y rutas/
  bibliotecas del SDK también en Release.

Para ampliar, editar `DefaultRecipes`, `Rules` y `Layout`. Un nuevo destino
añade una posición a `layout.deliveries` y su nombre a `destinationNames`; la
escena crea su señal y el selector lo incorpora. Para sustituir las primitivas,
cambiar las mallas en `AlchemyScene`, sin tocar la validación. El compañero que
desarrolle la interfaz final puede reemplazar `AlchemyHud` leyendo los mismos
estados de `Game` y el objetivo seleccionado.

## Pruebas

`Tools/check_alchemy.cpp` se compila con C++14/UTF-8, sin DirectX:

```powershell
cl /EHsc /std:c++14 /utf-8 Tools/check_alchemy.cpp /Fo:Debug/check_alchemy.obj /Fe:Debug/check_alchemy.exe
./Debug/check_alchemy.exe
```

Se verificó realmente:

- Compilación y enlace de Debug y Release, Win32, mediante MSBuild.
- 79 comprobaciones de lógica: selección, un objeto por mano, soltar/recuperar,
  recetas completas, cantidades, orden, triturado, mezcla, calor insuficiente/
  excesivo, destinos, temporizador, cierre único, victoria/derrota y reinicio.
- 104 comprobaciones en una prueba temporal de integración con dispositivo DirectX real y entrada
  WinAPI/XInput simulada: las tres recetas mediante la selección y la misma
  ruta de E del juego, cinco entregas, pantallas de resultado, Q/R/repetición,
  pausa sin foco, nube, transporte y liberación de todos los colliders.
- Comprobación espacial de 14 objetivos: posiciones libres y conectadas con el
  inicio en una cuadrícula de navegación que consulta las colisiones reales.
- Exportaciones del framebuffer y revisión visual de inicio, transporte,
  mezcla, calor, poción, entrega, victoria y derrota.
- `git diff --check`.

La integración usa posiciones de interacción calculadas para recorrer los
casos y una comprobación de conectividad; no sustituye una partida completa
de una persona con teclado o un mando físico.

## Limitaciones del prototipo

Los ingredientes y mecanismos sin asset definitivo siguen siendo primitivas.
No hay animaciones de manos, partículas de líquido ni física de objetos
sueltos; Q los coloca en una superficie libre. No se añade IA de habitantes:
las entregas son señales junto a la casa. No hay guardado de partida ni una
cola de pedidos simultáneos. El HUD está preparado para ser sustituido por la
interfaz artística final. El stock se repone entre pedidos; una mezcla
incorrecta se resuelve como fallo y no tiene una operación de deshacer.
