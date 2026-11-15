# Sombras del sol y la luna

La escena usa un mapa de sombras de una luz direccional: el sol durante el día
y la luna durante la noche. Los modelos proyectan sus siluetas sobre el terreno
y sobre otros modelos. Pablito proyecta sombra también en primera persona.

La configuración del mapa está en [SOL.md](SOL.md). El reloj, las transiciones,
los controles y las texturas se explican en [DIA_NOCHE.md](DIA_NOCHE.md).

`TerrainShadows.h` conserva las elipses de contacto anteriores como referencia.
El renderizador no las instancia. Los materiales usan la iluminación y las
sombras comunes de `SunLighting.hlsli`.
