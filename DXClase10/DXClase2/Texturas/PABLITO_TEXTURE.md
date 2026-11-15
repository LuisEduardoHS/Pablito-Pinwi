# Textura de Pablito Pinwi

`pablito_diffuse.png` es un atlas pintado creado para el personaje: plumas azul
oscuro, cara y vientre crema, pico y patas naranja, ojos oscuros y boca coral.
El atlas es opaco y usa el shader difuso existente de DirectX 11.

`Models/pablitopinwi_textured.txt` conserva los vértices, triángulos y normales
del modelo original, modificando únicamente sus UV. Sus 18 componentes tienen
materiales separados para resolver las UV superpuestas del original.
El original `Models/pablitopinwi.txt` se conserva sin cambios.

Para regenerar las UV desde la carpeta DXClase2:

```powershell
python Tools/prepare_pablito_uv.py
```

La carga se configura en `DXRender::InitDevice()`. Pablito es ahora el personaje
controlable y conserva su escala original; su ubicación y orientación siguen
el movimiento del jugador. Consulta `../CAMARAS.md` para controles y cámaras.
El PNG se copia sin modificar desde el resultado de ImageGen. Python solo es
necesario para regenerar las UV, no para ejecutar el juego.
