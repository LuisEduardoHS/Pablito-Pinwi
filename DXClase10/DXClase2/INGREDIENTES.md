# Modelos de ingredientes

Los seis OBJ entregados están en `Models/ingredientes/`. Son los mismos modelos
que ya contenía la actualización de main `7461e57`; se conservaron sus mallas,
normales y UV. No se requieren los MTL ausentes porque todos usan la paleta
existente `Texturas/paleta.png`, con los colores indicados por sus UV.

Se regeneraron los TXT con `Tools/convert_obj.py`, el conversor de la plantilla.
Centra X/Z, coloca la base en Y=0, adapta Z y el orden de los triángulos a DirectX
y conserva las proporciones. El juego carga esos TXT, igual que los demás
modelos convertidos; los OBJ permanecen como fuentes originales.

```powershell
Get-ChildItem Models/ingredientes/*.obj | ForEach-Object {
    python Tools/convert_obj.py $_.FullName
}
```

`Gameplay/AlchemyScene.h` configura una escala uniforme por modelo. Estas son
las longitudes máximas en unidades del escenario:

| Ingrediente | Archivo | Dimensión mayor |
| --- | --- | --- |
| Hongo rojo | hongorojo.txt | 0.82 |
| Flor azul | florazul.txt | 0.80 |
| Cristal mágico | cristal.txt | 0.86 |
| Agua | agua.txt | 0.78 |
| Hierba verde | hierba.txt | 0.76 |
| Raíz luminosa | raiz.txt | 0.84 |

Los ingredientes caben entre las posiciones de suministro separadas por una
unidad. La raíz gira 90° para mostrar su longitud y quedar atravesada entre
las manos. La base se apoya sobre la mesa, el suelo o la estación; al llevarlos,
se baja visualmente el origen un 45 % de su altura para centrar el cuerpo del
ingrediente en el agarre. Las posiciones lógicas, recetas, controles, iconos y
resaltado de proximidad permanecen iguales. Los ingredientes siguen sin ser
obstáculos físicos, como los objetos recogibles anteriores.

Al comprobar la liberación de la escena se restauró también la liberación del
utensilio de animación, omitida en la actualización anterior de main.

## Verificación

- Debug y Release Win32 compilan correctamente.
- 45 comprobaciones con DirectX real: seis modelos cargados, dimensiones,
  recogida con E, agarre, soltar con Q y liberación completa de la escena.
- 87 comprobaciones de alquimia en la escena: las tres recetas, las cinco
  entregas, triturado/mezcla/calor/embotellado, victoria, derrota y reinicio.
- Capturas revisadas de las dos mesas y de ingredientes en manos de Pablito.

La regresión general del mapa anterior no pasó tras sincronizar `7461e57`:
una posición candidata del fogón no quedó conectada a la cuadrícula de navegación
del ensayo y E desde el inicio ya no montó la nube nueva. Las pruebas específicas
de ingredientes y alquimia anteriores no incluyen esa comprobación de navegación
ni la de montar la nube; esos cambios del escenario se conservaron.
