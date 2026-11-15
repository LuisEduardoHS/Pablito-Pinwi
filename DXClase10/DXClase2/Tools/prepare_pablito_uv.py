"""Separate Pablito's overlapping UVs into the eight-material painted atlas.

Run from DXClase2: python Tools/prepare_pablito_uv.py
Geometry, triangle order and normals are preserved; only texture coordinates change.
The original model remains untouched.
"""

from collections import defaultdict
from math import atan2, pi
from pathlib import Path


def prepare():
    project = Path(__file__).resolve().parent.parent
    source = project / 'Models/pablitopinwi.txt'
    target = project / 'Models/pablitopinwi_textured.txt'
    header, data = source.read_text(encoding='utf-8-sig').split('Data:', 1)
    rows = [line.split() for line in data.splitlines() if line.strip()]
    assert len(rows) == 11424 and all(len(row) == 8 for row in rows), 'Unexpected model topology'
    triangles = [rows[i:i + 3] for i in range(0, len(rows), 3)]
    parent = list(range(len(triangles)))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    seen = {}
    for i, triangle in enumerate(triangles):
        for row in triangle:
            position = tuple(round(float(v), 5) for v in row[:3])
            if position in seen:
                parent[find(i)] = find(seen[position])
            else:
                seen[position] = i
    groups = defaultdict(list)
    for i in range(len(triangles)):
        groups[find(i)].append(i)
    components = sorted(groups.values(), key=len, reverse=True)
    expected = [512, 512, 304, 304, 224, 224, 192, 192, 192, 192, 192, 192, 128, 128, 128, 80, 80, 32]
    assert [len(group) for group in components] == expected, 'Unexpected mesh components'

    # Top row: head chart, belly chart, beak, eyes.
    # Bottom row: feet, navy feathers, mouth, tongue.
    materials = [5, 1, 2, 2, 5, 5, 3, 3, 0, 5, 0, 0, 4, 4, 7, 4, 4, 6]
    labels = ['belly', 'body', 'upper beak', 'lower beak', 'left flipper',
              'right flipper', 'left pupil', 'right pupil', 'head feathers',
              'face', 'left eye white', 'right eye white', 'left ankle',
              'right ankle', 'tongue', 'left foot', 'right foot', 'mouth']
    for component, (ids, material, label) in enumerate(zip(components, materials, labels)):
        positions = [tuple(map(float, row[:3])) for i in ids for row in triangles[i]]
        low = [min(p[k] for p in positions) for k in range(3)]
        high = [max(p[k] for p in positions) for k in range(3)]
        center_x, center_z = (low[0] + high[0]) / 2, (low[2] + high[2]) / 2
        for i in ids:
            back = component in (1, 8) and sum(float(row[0]) for row in triangles[i]) / 3 < center_x
            tile = 5 if back else material
            column, atlas_row = tile % 4, tile // 4
            projected = []
            for row in triangles[i]:
                u, v = map(float, row[3:5])
                local_u = (u - 0.125) / 0.75
                assert -1e-5 <= local_u <= 1.00001 and 0 <= v <= 1
                if component in (1, 8) and not back:
                    # Cylindrical charts put the painted face/belly on +X, and navy on the back.
                    x, y, z = map(float, row[:3])
                    local_u = max(0.0, min(1.0, atan2(z - center_z, x - center_x) / pi + 0.5))
                    v = (high[1] - y) / (high[1] - low[1])
                elif component in (10, 11):
                    # The smaller eye shells sample only the cream center of the face chart.
                    local_u, v = 0.5, 0.55
                projected.append((local_u, v))
            if component in (1, 8) and not back and max(uv[0] for uv in projected) - min(uv[0] for uv in projected) > 0.5:
                # Back seam triangles stay on the navy edge rather than interpolating over the face.
                projected = [(1.0 if u < 0.5 else u, v) for u, v in projected]
            for row, (local_u, v) in zip(triangles[i], projected):
                # Keep a generous inset to prevent adjacent palette colors bleeding in mipmaps.
                row[3] = f'{(column + 0.08 + 0.84 * local_u) / 4:.8f}'
                row[4] = f'{(atlas_row + 0.08 + 0.84 * v) / 2:.8f}'
        print(f'{label}: material {material}, {len(ids)} triangles')

    target.write_text(header + 'Data:\n\n' + '\n'.join(' '.join(row) for row in rows) + '\n', encoding='ascii')
    print(target)


if __name__ == '__main__':
    prepare()
