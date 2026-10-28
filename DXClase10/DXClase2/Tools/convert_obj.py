"""Convert textured OBJ meshes to the teaching template's Vertex Count/Data format.

Usage: python Tools/convert_obj.py Models/PablitoPinwi/casa_pablo.obj
Requires only Python's standard library; the game does not require Python.
"""

import argparse
import math
from pathlib import Path


def triangulate(corners, positions):
    """Ear clipping on the dominant plane, preserving source winding and UV seams."""
    points = [positions[c[0]] for c in corners]
    normal = [sum((a[(k + 1) % 3] - b[(k + 1) % 3])
                  * (a[(k + 2) % 3] + b[(k + 2) % 3])
                  for a, b in zip(points, points[1:] + points[:1])) for k in range(3)]
    drop = max(range(3), key=lambda k: abs(normal[k]))
    axes = [k for k in range(3) if k != drop]
    xy = [(p[axes[0]], p[axes[1]]) for p in points]

    def cross(a, b, c):
        return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])

    signed_area = sum(a[0] * b[1] - b[0] * a[1]
                      for a, b in zip(xy, xy[1:] + xy[:1]))
    if abs(signed_area) < 1e-14:
        raise ValueError("Degenerate polygon")
    sign = 1 if signed_area > 0 else -1
    span = max(max(p[k] for p in xy) - min(p[k] for p in xy) for k in range(2))
    epsilon = max(span * span * 1e-10, 1e-16)
    remaining = list(range(len(corners)))
    triangles = []
    while len(remaining) > 3:
        for j, current in enumerate(remaining):
            before, after = remaining[j - 1], remaining[(j + 1) % len(remaining)]
            a, b, c = xy[before], xy[current], xy[after]
            if sign * cross(a, b, c) <= epsilon:
                continue
            if any(sign * cross(a, b, xy[p]) >= -epsilon
                   and sign * cross(b, c, xy[p]) >= -epsilon
                   and sign * cross(c, a, xy[p]) >= -epsilon
                   for p in remaining if p not in (before, current, after)):
                continue
            triangles.append((corners[before], corners[current], corners[after]))
            del remaining[j]
            break
        else:
            raise ValueError("Cannot triangulate polygon (degenerate or intersecting edges)")
    triangles.append(tuple(corners[i] for i in remaining))
    # Every polygon must retain its projected area after triangulation.
    lookup = {id(c): p for c, p in zip(corners, xy)}
    area = sum(cross(*(lookup[id(c)] for c in triangle)) for triangle in triangles)
    if not math.isclose(area, signed_area, rel_tol=1e-6, abs_tol=epsilon * 10):
        raise ValueError("Triangulation changed polygon area")
    return triangles


def convert(source, destination=None):
    source = Path(source)
    destination = Path(destination) if destination else source.with_suffix('.txt')
    positions, texcoords, normals, triangles = [], [], [], []

    def index(value, sequence):
        value = int(value)
        resolved = value - 1 if value > 0 else len(sequence) + value
        if value == 0 or not 0 <= resolved < len(sequence):
            raise ValueError("OBJ index out of range")
        return resolved

    for line_number, raw in enumerate(source.read_text(encoding='utf-8-sig').splitlines(), 1):
        fields = raw.split('#', 1)[0].split()
        if not fields:
            continue
        tag, *values = fields
        try:
            if tag in ('v', 'vt', 'vn'):
                size = 2 if tag == 'vt' else 3
                numbers = tuple(float(v) for v in values[:size])
                if len(numbers) != size or not all(math.isfinite(v) for v in numbers):
                    raise ValueError("Invalid vector")
                {'v': positions, 'vt': texcoords, 'vn': normals}[tag].append(numbers)
            elif tag == 'f':
                corners = []
                for token in values:
                    components = token.split('/')
                    if len(components) != 3 or not all(components):
                        raise ValueError("Each face corner must have position/UV/normal")
                    corners.append(tuple(index(v, seq) for v, seq in
                                         zip(components, (positions, texcoords, normals))))
                if len(corners) < 3:
                    raise ValueError("Face has fewer than three vertices")
                triangles.extend(triangulate(corners, positions))
        except ValueError as error:
            raise ValueError(f'{source.name}:{line_number}: {error}') from error

    if not triangles:
        raise ValueError("OBJ contains no triangles")
    # Recenter X/Z and put the base on Y=0, retaining dimensions and proportions.
    used = [positions[c[0]] for triangle in triangles for c in triangle]
    low = [min(p[k] for p in used) for k in range(3)]
    high = [max(p[k] for p in used) for k in range(3)]
    origin = ((low[0] + high[0]) / 2, low[1], (low[2] + high[2]) / 2)
    rows = []
    for triangle in triangles:
        # Reflect Z and reverse winding for DirectX's left-handed coordinates.
        for vi, ti, ni in reversed(triangle):
            p, uv, n = positions[vi], texcoords[ti], normals[ni]
            length = math.sqrt(sum(v * v for v in n))
            if length < 1e-12:
                raise ValueError("OBJ has a zero-length normal")
            values = (p[0] - origin[0], p[1] - origin[1], -(p[2] - origin[2]),
                      uv[0], 1 - uv[1], n[0] / length, n[1] / length, -n[2] / length)
            rows.append(' '.join(f'{v:.9g}' for v in values))
    destination.write_text(f'Vertex Count: {len(rows)}\n\nData:\n\n'
                           + '\n'.join(rows) + '\n', encoding='ascii')
    return len(triangles), tuple(high[k] - low[k] for k in range(3))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    count, extent = convert(args.source, args.output)
    print(f'{args.source.name}: {count} triangles; dimensions {extent}')
