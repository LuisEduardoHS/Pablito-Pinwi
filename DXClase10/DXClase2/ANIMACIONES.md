# Animaciones de Pablito

## Modelo y articulaciones

Se conserva `Models/pablitopinwi_textured.txt` y su textura original. El archivo
no contenía esqueleto ni clips: son 18 superficies independientes. Se asignaron
sus 11,424 vértices a nueve articulaciones rígidas: cuerpo, cabeza, dos aletas,
dos pies, dos ojos y pico inferior. Las partes completas se transforman juntas;
no se reemplazan por otro personaje ni se deforman triángulos entre piezas.

`Tools/rig_pablito.py` encuentra componentes conectados, identifica las piezas
por su posición y genera `Gameplay/PablitoRigData.h`. El encabezado registra el
SHA-256 del modelo de origen. Si se modifica la malla, regenerar y verificar:

```powershell
python Tools/rig_pablito.py
python Tools/rig_pablito.py --check
```

El generador solo necesita la biblioteca estándar de Python. El ejecutable no
necesita Python ni herramientas de animación externas.

## Estados y acciones

`Gameplay/PablitoAnimator.h` mezcla los ángulos antes de construir las matrices.
Incluye 23 estados:

| Estado | Activación y comportamiento |
| --- | --- |
| Idle | Respiración, mirada y parpadeo al estar quieto |
| Walk / Run | Pasos alternos, balanceo y aletas según el desplazamiento real |
| Carry | Aletas hacia delante sujetando el objeto; compatible con caminar/correr |
| Jump / Fall / Land | Despegue, caída y compresión suave al tocar el suelo |
| Mount / Ride / Dismount | Salto visual a la nube, equilibrio al girar y bajada |
| Pickup | Alcance de las aletas y traslado suave del objeto a su agarre |
| Place / Drop | Colocación en estaciones y gesto al soltar con Q |
| Grind | Movimiento repetido con utensilio al triturar en el mortero |
| Stir | Movimiento circular con utensilio al mezclar en el caldero |
| Heat | Atención al fogón y movimiento de las aletas al cocinar |
| Bottle | Manipulación del frasco al embotellar |
| Deliver | Gesto de entrega de la poción |
| Read | Mirada hacia abajo al abrir la receta con R |
| Nope | Negación con la cabeza cuando la interacción no cambia el juego |
| Victory / Defeat | Poses de celebración y tristeza disponibles en el controlador |
| SixSeven | Emote secreto de aletas alternadas, balanceo y gesto del pico; dura 5.4 s |

Durante la partida, pulsar **6 seis veces y luego 7 siete veces**, sin otra tecla
entre ellas, activa SixSeven cuando Pablito está en el suelo y fuera de la nube.
Se admiten las teclas superiores y el teclado numérico. Mantener una tecla no
cuenta como varias pulsaciones. Una tecla incorrecta, perder el foco, entrar en
un menú o reiniciar borra la secuencia parcial. El emote no aparece en la UI.
La cámara pasa temporalmente a tercera persona y luego recupera la perspectiva
previa; moverse, saltar o interactuar interrumpe el gesto y permite seguir jugando.
La física, los ingredientes y el reloj de las recetas continúan sin cambios.

`Gameplay/SecretEmoteCode.h` reconoce las pulsaciones; `WM_KEYDOWN` descarta la
repetición del teclado. `Tools/check_secret_emote.cpp` verifica la secuencia,
reinicios, teclado numérico, alternancia de aletas, duración e interrupciones.

Las estaciones siguen sus tiempos originales. El movimiento de trabajo se
mantiene mientras el proceso está activo y Pablito está cerca y quieto. Puede
alejarse sin quedar atrapado en una animación; el aparato continúa procesando.
La animación se decide después de evaluar la interacción, por lo que un intento
inválido no reproduce una recogida o un embotellado que no ocurrió.

Los pies usan la velocidad efectivamente recorrida: empujar una pared no
reproduce una carrera. Los gestos se superponen a la locomoción; los resultados
tienen prioridad. Reiniciar borra eventos, transiciones y objetos temporales.

## Renderizado y transporte

`MPrimitives` admite animación solo mediante el argumento opcional `animate`.
Los demás modelos conservan sus buffers estáticos. Pablito conserva una copia de
reposo y actualiza un buffer dinámico; las normales usan la inversa transpuesta
de cada articulación, incluida la compresión de los ojos al parpadear.

El pase de color y el de sombras leen los mismos vértices animados. La cápsula
de física conserva sus dimensiones y posición: una aleta o un salto visual de
subida no alteran las colisiones. Las transiciones a la nube suavizan el cuerpo
y el punto seguido por la cámara; conducir sigue exigiendo tercera persona.

En tercera persona el objeto sigue el agarre frente al pecho. Tras recogerlo,
su posición visual interpola desde el origen hasta ese agarre. En primera
persona se conserva la ocultación del cuerpo, con el objeto frente a la cámara
y un balanceo leve. Los utensilios de trabajo son un cilindro compartido sin
colisiones, visible únicamente durante Grind/Stir.

Las pantallas de resultado muestran únicamente los letreros Ganaste/Has perdido
y los botones Reiniciar/Salir. El personaje no se dibuja en esas pantallas;
las poses de resultado siguen disponibles en el controlador. La partida y el
reloj permanecen detenidos.

## Ajustes y validación

- `PablitoAnimator::Duration`: duración de los gestos.
- `ActionPose` y `Update`: amplitud, ritmo y mezcla de los movimientos.
- `BuildMatrices`: pivotes y jerarquía de las partes del modelo actual.
- `DXRender::AnimateInteraction`: enlace entre interacciones aceptadas y gestos.
- `AnimatePablito`: velocidad real, saltos, conducción y proximidad al trabajo.
- `UpdateAnimationTool` / `UpdateCarriedObject`: utensilio y agarre del objeto.

`Tools/check_pablito_animation.cpp` verifica el rig, estados, poses finitas,
locomoción con objetos, saltos/aterrizaje, retorno de gestos, ciclos de trabajo,
nube, resultados, reinicio y comparación entre 15 y 144 FPS. Se compila en el
entorno x86 de Visual Studio con los Include/Lib del SDK DirectX June 2010:

```powershell
cl /EHsc /std:c++14 /utf-8 Tools/check_pablito_animation.cpp /Fe:check_pablito_animation.exe
./check_pablito_animation.exe
```

También se verificó la integración de menús y las 104 comprobaciones de la escena
real: recetas, selección E, transporte, estaciones, entregas, nube y reinicio.
