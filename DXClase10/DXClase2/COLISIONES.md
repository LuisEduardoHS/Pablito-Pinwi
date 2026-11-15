# Colisiones automáticas

Los modelos cargados por `MPrimitives` generan su colisión con los mismos
triángulos que se dibujan. El collider se registra automáticamente en el mundo:
no hay que añadir el modelo a una lista de obstáculos ni dibujar cajas a mano.
Los cubos, esferas, toros, cilindros y conos procedurales también la generan.

```cpp
MPrimitives* objeto = new MPrimitives(device, context,
    "Models/objeto.txt", L"Texturas/objeto.png", L"", ShaderDiffuse, true);
// UpdateScene aplica esta misma transformación al dibujo y a la colisión.
objeto->UpdateScene(scaling * rotation * translation);
```

La colisión conserva los huecos que existan en la geometría, por ejemplo el
espacio entre patas de una mesa. Una puerta cerrada o pintada sigue siendo
sólida; los píxeles transparentes de una textura no recortan los triángulos.
La colisión corresponde a la malla rígida: futuras animaciones que deformen
vértices necesitarán actualizarla o usar volúmenes para los huesos.

## Funcionamiento

- `SceneCollision.h` guarda los triángulos locales y crea un árbol BVH por
  modelo para descartar regiones lejanas antes de comprobar las caras.
- El registro del mundo se gestiona al crear y liberar el collider. Al cambiar
  la posición, giro o escala se actualizan sus límites e inversa, sin reconstruir
  el árbol. Se admiten escalas no uniformes y reflejadas; una escala singular
  desactiva la consulta hasta que vuelva a ser válida.
- Las comprobaciones usan los triángulos transformados al mundo y son de doble
  cara, sin depender del orden de los vértices exportados. Se omiten los
  triángulos degenerados.
- Los pasos de movimiento se subdividen para que correr o conducir no atraviese
  paredes delgadas. La respuesta separa el volumen del obstáculo y conserva
  el movimiento tangencial para deslizarse por las paredes.

Pablito utiliza una cápsula vertical cuya altura sigue su modelo y su escala.
El radio se ajusta entre 0.30 y 0.65 unidades. Su movimiento combina gravedad,
salto, apoyo sobre el terreno y superficies de objetos. Puede aterrizar sobre
mesas o plataformas, cae al salir de sus bordes y se detiene al golpear un
techo. Sigue conservando la velocidad de caminar/correr y el impulso de salto.
Los desniveles pequeños de hasta 0.35 unidades admiten un paso si cabe la cápsula.

La nube usa una caja orientada que incluye la plataforma y la altura de Pablito.
Se resuelve contra la geometría del escenario, incluyendo los giros; se detiene
o desliza ante obstáculos en lugar de atravesarlos. La nube continúa flotando
con la altura del terreno. Al bajar se busca un lugar libre junto a ella y se
comprueba el trayecto; si un costado está ocupado se intenta por los demás.
Si no hay una salida segura se permanece montado.

La tercera persona acorta su brazo cuando una pared queda entre el personaje
y la cámara. La acción de subir a la nube también comprueba que no haya una
pared entre Pablito y el vehículo, utilizando la misma condición que el icono E.

## Exclusiones

```cpp
objeto->SetCollisionEnabled(false); // Adorno, efecto o actor con otro volumen.
```

Pablito excluye su propia malla porque ya tiene la cápsula controlable. La nube
ignora su propio collider al conducir, pero su plataforma es sólida para
Pablito cuando está a pie. El cielo y la interfaz quedan fuera automáticamente;
los billboards y esferas de depuración tampoco actúan como obstáculos.
El terreno mantiene su consulta de altura, sin duplicar todos sus triángulos
en otro collider. Las superficies verticales y techos de modelos sí se
resuelven mediante la malla.

Las colisiones corresponden a objetos rígidos del escenario; esto no agrega
masa, rebotes o simulación de caída a todos los objetos decorativos. Los objetos
que se muevan actualizan su geometría de colisión al llamar `UpdateScene`.

## Liberación y verificación

`ReleaseModelResources` libera y desregistra el collider; destruir el modelo
también lo desregistra. `DXRender::ReleaseObjects` recorre todas las mallas de
la escena para evitar colliders sobrantes entre sesiones. Los modelos que no
se pueden cargar correctamente no dejan obstáculos invisibles registrados.

Se corrigieron los contadores de índices de las primitivas procedurales para
usar únicamente los triángulos inicializados al construir dibujo y colisión.

Prueba de CPU reproducible: `Tools/check_scene_collision.cpp`. Compilar en una
terminal de desarrollo x86 de Visual Studio con el SDK DirectX de junio de 2010
en Include/Lib y enlazar `d3dx10.lib`. Comprueba paredes delgadas a varias
velocidades/FPS, huecos de puerta, esquinas, techos, apoyo, deslizamiento,
transformaciones, caja orientada, descarte BVH y vida de los colliders.

También se verificó la compilación `Debug | Win32` y una prueba temporal de
integración contra la escena real: modelo nuevo sin registro manual, mesa,
nube, salida segura, techos, cámara contra la casa, velocidades de caminar,
correr y conducir a 15/60/144 FPS, dibujo y liberación completa.
