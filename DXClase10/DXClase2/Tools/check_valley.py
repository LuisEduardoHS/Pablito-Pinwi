"""Check the generated assets: precision, scale, route grades and connectivity.

Run with the same numpy/Pillow environment as generate_valley.py.
"""
from collections import deque
import struct

import numpy as np
from PIL import Image

import generate_valley as valley
import world_layout as layout


def check():
    raw = (valley.PROJECT/'Texturas/Pablito_Valley_513.bmp').read_bytes()
    magic, length, _, _, offset = struct.unpack_from('<2sIHHI', raw)
    _, width, height, planes, bits, compression = struct.unpack_from('<IiiHHI', raw, 14)
    assert magic == b'BM' and length == len(raw) and offset == 54
    assert (width, height, planes, bits, compression) == (513, 513, 1, 32, 0)
    pixels = np.frombuffer(raw, np.uint8, offset=offset).reshape(height, width, 4)
    assert np.array_equal(pixels[:,:,0], pixels[:,:,1]) and np.array_equal(pixels[:,:,1], pixels[:,:,2])
    heights = (pixels[:,:,2].astype(float)*256+pixels[:,:,3])*valley.MAX_HEIGHT/65535
    assert np.isfinite(heights).all() and heights.min() >= 0 and 300 < heights.max() < valley.MAX_HEIGHT
    # Low-byte precision matters on the level terrace and subtle meadow slopes.
    assert np.unique(pixels[:,:,3]).size == 256
    x, z = valley.grid(valley.SIZE)
    village = ((x-384)/45)**2 + ((z-414)/50)**2 < 1
    assert np.max(np.abs(heights[village]-30)) < 0.01

    paths = np.asarray(Image.open(valley.PROJECT/'Texturas/Pablito_Valley_Paths.png'))[::-1]
    assert paths.shape == (1025,1025,3)
    core = paths[::2,::2,0] > 240
    along,side=layout.bridge_coordinates(x,z);bridge=layout.DATA['bridge']
    bridge_zone=(np.abs(along)<bridge['length']/2+9)&(np.abs(side)<bridge['width']/2+5)
    core &= ~bridge_zone # Terrain banks under the deck are not its walk surface.
    dz, dx = np.gradient(heights, valley.EXTENT/(valley.SIZE-1))
    grade = np.hypot(dx,dz)[core]
    assert np.percentile(grade,99) < 0.20 and grade.max() < 0.5

    # The road system must be one continuous component from the spawn point.
    road = paths[:,:,0] > 160
    mx,mz=valley.grid(valley.MASK_SIZE);along,side=layout.bridge_coordinates(mx,mz)
    road |= (np.abs(along)<=bridge['length']/2+2)&(np.abs(side)<bridge['width']/2-1)
    start = (round(380/valley.EXTENT*1024), round(384/valley.EXTENT*1024))
    assert road[start]
    visited = np.zeros(road.shape, dtype=bool)
    visited[start] = True
    queue = deque([start])
    while queue:
        row, col = queue.popleft()
        for r,c in ((row-1,col),(row+1,col),(row,col-1),(row,col+1)):
            if 0 <= r < 1025 and 0 <= c < 1025 and road[r,c] and not visited[r,c]:
                visited[r,c] = True
                queue.append((r,c))
    assert np.array_equal(visited,road), 'Disconnected painted road'
    for house in layout.DATA['secondary_houses']:
        hx,hy,hz=house['center'];plot=(x-hx)**2+(z-hz)**2<14**2
        assert np.max(np.abs(heights[plot]-(hy-.08)))<.01,'Uneven secondary house plot'
    # The full-width river has a real bed below its surface, edge to edge.
    for rz in np.linspace(0,valley.EXTENT,257):
        col=round(float(layout.river_x(rz))/valley.EXTENT*512);row=round(rz/valley.EXTENT*512)
        assert heights[row,col]<layout.river_height(rz)-.75,'River bed obstructs water'
    for (width, controls), levels in zip(valley.ROUTES,valley.ROUTE_LEVELS):
        curve = valley.curve([(*p,y) for p,y in zip(controls,levels)])
        grade = np.abs(np.diff(curve[:,2]))/np.linalg.norm(np.diff(curve[:,:2],axis=0),axis=1)
        assert grade.max() < 0.18
        for world_x,world_z in controls:
            col, row = round(world_x/valley.EXTENT*1024), round(world_z/valley.EXTENT*1024)
            assert visited[row,col], 'A route endpoint is unreachable'
    print(f'PASS: 16-bit heights {heights.min():.2f}..{heights.max():.2f}; '
          f'{road.sum()} connected path pixels; level village; graded routes below 18%.')


if __name__ == '__main__':
    check()
