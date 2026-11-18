"""Build the playable valley and its aligned path/material control map.

Run: python -m pip install numpy pillow
     python Tools/generate_valley.py

The BMP is BI_RGB32: RGB holds the high height byte, alpha the low byte.
Both assets come from the same routes, so painted paths follow graded ground.
"""
from pathlib import Path
import argparse
import json
import struct

import numpy as np
from PIL import Image
import world_layout as layout

SIZE = 513
MASK_SIZE = 1025
EXTENT = 1152.0
MAX_HEIGHT = 360.0
PROJECT = Path(__file__).resolve().parent.parent
ROUTES = [
    # Main valley route skirts the east side of the house before the pass.
    (18.0, [(375, 100), (365, 240), (384, 380), (406, 414), (430, 478),
            (465, 545), (555, 625), (650, 700), (735, 825), (850, 950), (930, 1050)]),
    (16.0, [(465, 545), (365, 600), *layout.bridge_controls()[0], (200, 805), (170, 970)]),
    (16.0, [(555, 625), (690, 565), (815, 535), (940, 600), (1030, 745)]),
    (14.0, [(365, 240), (500, 230), (650, 300), (755, 405), (815, 535)]),
    (12.0, [(384, 380), (370, 403), (384, 416)]),
]
# Authored centerline elevations create real mountain passes instead of paths
# that simply climb whatever steep peak happens to be underneath them.
ROUTE_LEVELS = [
    [28, 30, 30, 30, 34, 42, 52, 68, 91, 112, 125],
    [42, 52, *layout.bridge_controls()[1], 85, 110],
    [52, 58, 70, 86, 110],
    [30, 34, 42, 57, 70],
    [30, 30, 30],
]


def smooth(a, b, x):
    t = np.clip((x - a) / (b - a), 0, 1)
    return t * t * (3 - 2 * t)


def noise(x, z, cell, seed):
    rng = np.random.default_rng(seed)
    lattice = rng.uniform(-1, 1, (int(EXTENT / cell) + 4,) * 2)
    gx, gz = x / cell, z / cell
    ix, iz = gx.astype(int), gz.astype(int)
    u, v = smooth(0, 1, gx - ix), smooth(0, 1, gz - iz)
    return ((1-u)*(1-v)*lattice[iz, ix] + u*(1-v)*lattice[iz, ix+1]
            + (1-u)*v*lattice[iz+1, ix] + u*v*lattice[iz+1, ix+1])


def base_height(x, z):
    broad = noise(x, z, 155, 14)
    detail = noise(x, z, 58, 21)
    ground = 29 + 8*broad + 3*detail
    # Continuous, asymmetrical massifs rather than isolated radial cones.
    # The broad lobes overlap into ridgelines, shoulders and subsidiary summits.
    warp_x = 34*noise(x,z,150,101) + 13*noise(x,z,65,103)
    warp_z = 30*noise(x,z,170,107) + 14*noise(x,z,72,109)
    wx, wz = x+warp_x, z+warp_z
    mountains = np.zeros_like(x)
    # Center, major/minor radius, height, rotation; heights stay independent of width.
    massifs = [(110, 820, 155, 300, 210, -0.22),
               (275, 1015, 230, 175, 244, 0.18),
               (525, 1050, 320, 170, 282, -0.10),
               (715, 1030, 230, 155, 248, 0.30),
               (990, 970, 220, 250, 268, -0.28),
               (1060, 730, 150, 230, 183, 0.12),
               (1100, 390, 170, 280, 231, -0.10),
               (925, 105, 250, 160, 194, 0.25),
               (610, 45, 190, 135, 138, -0.20),
               (80, 100, 185, 150, 151, 0.30),
               (55, 400, 125, 210, 119, -0.30),
               (555, 835, 160, 125, 105, -0.45)]
    for cx, cz, rx, rz, height, angle in massifs:
        dx, dz = wx-cx, wz-cz
        u = (dx*np.cos(angle)+dz*np.sin(angle))/rx
        v = (-dx*np.sin(angle)+dz*np.cos(angle))/rz
        radius = np.sqrt(u*u+v*v)
        rounded = np.exp(-1.55*radius*radius)
        bench = 1-smooth(0.40,1.12,radius)
        lobe = height*(0.36*rounded+0.64*bench)
        # Polynomial smooth union joins adjacent ridges without a hard crease.
        overlap = np.maximum(26-np.abs(mountains-lobe),0)/26
        mountains = np.maximum(mountains,lobe)+overlap*overlap*6.5
    mountain_zone = smooth(25,110,mountains)
    # Broad erosion gullies and smaller rock shoulders break up both skyline
    # and slopes; their amplitude fades to zero in the village/meadows.
    relief = (15*noise(x,z,95,131) + 7*noise(x,z,42,137)
              + 2.5*noise(x,z,20,139))
    ground += mountains + mountain_zone*relief
    # A few rolling meadows between the large silhouettes.
    ground += 18*np.exp(-((x-650)/180)**2 - ((z-420)/145)**2)
    ground += 23*np.exp(-((x-280)/130)**2 - ((z-720)/140)**2)
    # One level village terrace with a gradual, 45-unit transition into nature.
    village = np.sqrt(((x-384)/65)**2 + ((z-414)/78)**2)
    ground = 30 + (ground-30)*smooth(0.8, 1.5, village)
    return ground


def erode_massifs(heights):
    """Carve drainage into the broad forms, leaving divides and rock buttresses.

    Downhill accumulation follows the actual surface, so gullies branch and
    converge naturally instead of repeating a radial or sinusoidal pattern.
    """
    size = heights.shape[0]
    spacing = EXTENT/(size-1)
    index = np.arange(heights.size).reshape(heights.shape)
    receiver = index.copy()
    steepest = np.zeros_like(heights)
    padded_h = np.pad(heights,1,mode='edge')
    padded_i = np.pad(index,1,mode='edge')
    for dz,dx in [(-1,-1),(-1,0),(-1,1),(0,-1),(0,1),(1,-1),(1,0),(1,1)]:
        neighbor = padded_h[1+dz:1+dz+size,1+dx:1+dx+size]
        grade = (heights-neighbor)/(spacing*np.hypot(dx,dz))
        down = grade>steepest
        receiver = np.where(down,padded_i[1+dz:1+dz+size,1+dx:1+dx+size],receiver)
        steepest = np.maximum(steepest,grade)
    drainage = np.ones(heights.size)
    receiver = receiver.ravel()
    for point in np.argsort(heights.ravel())[::-1]:
        if receiver[point] != point:
            drainage[receiver[point]] += drainage[point]
    depth = np.minimum(18,12*np.log1p(drainage.reshape(heights.shape))/np.log(400))
    depth *= smooth(0.12,0.8,steepest)*smooth(55,115,heights)
    # Rounded channel walls keep the style clean at the mesh's 2.25-unit spacing.
    for _ in range(3):
        p = np.pad(depth,1,mode='edge')
        depth = (4*p[1:-1,1:-1]+p[:-2,1:-1]+p[2:,1:-1]+p[1:-1,:-2]+p[1:-1,2:])/8
    return heights-depth


def curve(points):
    p = np.array([points[0], *points, points[-1]], dtype=float)
    sections = []
    for i in range(1, len(p)-2):
        a, b, c, d = p[i-1:i+3]
        t = np.linspace(0, 1, max(4, int(np.linalg.norm(c-b)/6)), endpoint=False)[:, None]
        sections.append(0.5*((2*b)+(-a+c)*t+(2*a-5*b+4*c-d)*t*t+(-a+3*b-3*c+d)*t*t*t))
    return np.vstack([*sections, p[-2: -1]])


def sample_routes(x, z):
    distance = np.full_like(x, 1e9)
    half_width = np.zeros_like(x)
    weighted_height = np.zeros_like(x)
    total_weight = np.zeros_like(x)
    # Short segments let the roads follow gentle spline bends without corners.
    for (width, controls), levels in zip(ROUTES, ROUTE_LEVELS):
        spline = curve([(*p, y) for p, y in zip(controls, levels)])
        route, elevations = spline[:, :2], spline[:, 2]
        route_distance = np.full_like(x, 1e9)
        route_height = np.zeros_like(x)
        for i, (a, b) in enumerate(zip(route[:-1], route[1:])):
            delta = b-a
            t = np.clip(((x-a[0])*delta[0]+(z-a[1])*delta[1])/np.dot(delta,delta), 0, 1)
            dist = np.hypot(x-a[0]-t*delta[0], z-a[1]-t*delta[1])
            nearer = dist < route_distance
            route_distance = np.where(nearer, dist, route_distance)
            route_height = np.where(nearer, elevations[i]*(1-t)+elevations[i+1]*t, route_height)
        clearance = route_distance-width/2
        nearer = clearance < distance-half_width
        distance = np.where(nearer, route_distance, distance)
        half_width = np.where(nearer, width/2, half_width)
        # Blend overlapping cut/fill zones. A nearest-route-only height creates
        # artificial cliffs halfway between roads that sit at different levels.
        weight = np.exp(-np.maximum(clearance,-4)/6)*(1-smooth(35,50,clearance))
        weighted_height += weight*route_height
        total_weight += weight
    return distance, half_width, weighted_height/np.maximum(total_weight,1e-12)


def grid(size):
    return np.meshgrid(np.linspace(0, EXTENT, size), np.linspace(0, EXTENT, size))


def build(height_only=False):
    layout.write_header()
    textures = PROJECT / 'Texturas'
    x, z = grid(SIZE)
    heights = erode_massifs(base_height(x, z))
    distance, width, graded = sample_routes(x, z)
    influence = 1-smooth(width+5, width+50, distance)
    heights = heights*(1-influence) + graded*influence
    # Reassert the terrace after road blending: furniture has a common footing.
    plaza = np.hypot((x-384)/55, (z-414)/65)
    heights = 30 + (heights-30)*smooth(0.8, 1.4, plaza)
    # Level plots for the closed secondary houses at the three road termini.
    for house in layout.DATA['secondary_houses']:
        hx,hy,hz=house['center'];d=np.hypot(x-hx,z-hz)
        weight=1-smooth(18,40,d);heights=heights*(1-weight)+(hy-.08)*weight
    # Excavate a continuous channel from the north mountains to the south edge.
    river_distance=np.abs(x-layout.river_x(z));half=layout.river_half_width(z)
    bed=layout.river_height(z)-1.25
    weight=1-smooth(half-1,half+12,river_distance)
    heights=heights*(1-weight)+bed*weight
    # Flat approaches meet the sloping bridge deck, while its channel stays open.
    along,side=layout.bridge_coordinates(x,z);bridge=layout.DATA['bridge']
    approach=(1-smooth(bridge['width']/2,bridge['width']/2+4,np.abs(side)))
    approach*=1-smooth(bridge['length']/2,bridge['length']/2+7,np.abs(along))
    approach*=smooth(half+1,half+5,river_distance)
    deck=bridge['center'][1]+along*bridge['slope']+.05
    heights=heights*(1-approach)+deck*approach
    encoded = np.rint(np.clip(heights/MAX_HEIGHT, 0, 1)*65535).astype(np.uint16)
    pixels = np.empty((SIZE, SIZE, 4), dtype=np.uint8)
    pixels[:, :, :3] = (encoded >> 8)[:, :, None]
    pixels[:, :, 3] = encoded & 255
    # BMP stores bottom-up rows; our arrays already start at world z=0.
    data = pixels.tobytes()
    header = struct.pack('<2sIHHI', b'BM', 54+len(data), 0, 0, 54)
    info = struct.pack('<IiiHHIIiiII', 40, SIZE, SIZE, 1, 32, 0, len(data), 2835, 2835, 0, 0)
    (textures/'Pablito_Valley_513.bmp').write_bytes(header+info+data)

    print(json.dumps({'extent':EXTENT, 'height_range':[float(heights.min()),float(heights.max())],
                      'vertices':SIZE*SIZE,'triangles':(SIZE-1)**2*2,
                      'road_widths':[r[0] for r in ROUTES]},indent=2))
    if height_only:
        return

    x, z = grid(MASK_SIZE)
    distance, width, _ = sample_routes(x, z)
    edge = 0.7*noise(x,z,7,81) + 0.35*noise(x,z,2.5,16)
    road = 1-smooth(width-1.5, width+1.2, distance+edge)
    shoulder = 1-smooth(width+0.6, width+4.5, distance+edge)
    plaza = np.hypot((x-384)/24, (z-414)/27)
    road = np.maximum(road, 1-smooth(0.85,1.06,plaza))
    shoulder = np.maximum(shoulder, 1-smooth(1,1.25,plaza))
    for house in layout.DATA['secondary_houses']:
        hx,_,hz=house['center'];plot=np.hypot(x-hx,z-hz)
        road=np.maximum(road,1-smooth(15,20,plot))
        shoulder=np.maximum(shoulder,1-smooth(20,25,plot))
    # Short, broad approach to the actual east-side doorway of the main house.
    a,b=np.array([406.,414.]),np.array([395.8,411.3]);delta=b-a
    t=np.clip(((x-a[0])*delta[0]+(z-a[1])*delta[1])/np.dot(delta,delta),0,1)
    entrance=np.hypot(x-a[0]-t*delta[0],z-a[1]-t*delta[1])
    road=np.maximum(road,1-smooth(3,4,entrance));shoulder=np.maximum(shoulder,1-smooth(4,6,entrance))
    river_distance=np.abs(x-layout.river_x(z));half=layout.river_half_width(z)
    water=1-smooth(half-1,half+1,river_distance)
    road*=1-water
    shoulder=np.maximum(shoulder,(1-smooth(half+1,half+6,river_distance))*(1-water))
    macro = np.clip(0.5+0.32*noise(x,z,90,26)+0.18*noise(x,z,27,11),0,1)
    control = np.stack([road, shoulder, macro],axis=-1)
    Image.fromarray(np.rint(control[::-1]*255).astype(np.uint8)).save(textures/'Pablito_Valley_Paths.png')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--height-only', action='store_true', help='Keep the existing path/material map; update relief only.')
    build(parser.parse_args().height_only)
