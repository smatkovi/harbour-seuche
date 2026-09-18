#!/usr/bin/env python3
"""Draw the land silhouette that lies behind the board.

There is no coastline data on the device, but harbour-fgview ships the
FlightGear airport list for every country — 27 000 points spread over the
inhabited world. Rasterised and smoothed, that gives a coarse land mask which is
plenty as a backdrop: it orients the eye, it is never read as a border, and no
rule depends on it.

The image is cropped to exactly the lat/lon rectangle the cities span, so it
lines up with the same projection SeucheEngine::cities() uses.

Usage:  python3 tools/make_world.py [path-to-airports-dir]
"""
import glob
import json
import os
import sys

from PIL import Image, ImageFilter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
AIRPORTS = sys.argv[1] if len(sys.argv) > 1 else "/usr/share/harbour-fgview/airports"

# The bounds of the 48 cities, from src/core/Map.cpp — keep in step with it.
MIN_LAT, MAX_LAT = -34.6, 59.9
MIN_LON, MAX_LON = -122.4, 151.2

CELLS_PER_DEGREE = 4
LAND = (54, 72, 66)          # a muted green grey, calm under the coloured cities
SEA = (0, 0, 0, 0)           # the page's own background shows through


def airports(directory):
    points = []

    def walk(node):
        if isinstance(node, dict):
            for value in node.values():
                walk(value)
        elif isinstance(node, list):
            if (len(node) >= 5 and isinstance(node[0], str) and isinstance(node[1], str)
                    and isinstance(node[3], (int, float)) and isinstance(node[4], (int, float))):
                points.append((float(node[3]), float(node[4])))
            else:
                for value in node:
                    walk(value)

    for path in sorted(glob.glob(os.path.join(directory, "*.json"))):
        try:
            walk(json.load(open(path)))
        except (ValueError, OSError):
            continue
    return [(lat, lon) for lat, lon in points if -90 <= lat <= 90 and -180 <= lon <= 180]


def fill_small_holes(mask, limit):
    width, height = mask.size
    pixels = mask.load()
    seen = bytearray(width * height)

    # everything the border reaches is sea and stays sea
    stack = []
    for x in range(width):
        for y in (0, height - 1):
            if not pixels[x, y] and not seen[y * width + x]:
                seen[y * width + x] = 1
                stack.append((x, y))
    for y in range(height):
        for x in (0, width - 1):
            if not pixels[x, y] and not seen[y * width + x]:
                seen[y * width + x] = 1
                stack.append((x, y))
    while stack:
        x, y = stack.pop()
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= nx < width and 0 <= ny < height and not pixels[nx, ny] \
                    and not seen[ny * width + nx]:
                seen[ny * width + nx] = 1
                stack.append((nx, ny))

    filled = 0
    for start_y in range(height):
        for start_x in range(width):
            if pixels[start_x, start_y] or seen[start_y * width + start_x]:
                continue
            # an enclosed gap: collect it, fill it only if it is small
            gap = []
            stack = [(start_x, start_y)]
            seen[start_y * width + start_x] = 1
            while stack:
                x, y = stack.pop()
                gap.append((x, y))
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if 0 <= nx < width and 0 <= ny < height and not pixels[nx, ny] \
                            and not seen[ny * width + nx]:
                        seen[ny * width + nx] = 1
                        stack.append((nx, ny))
            if len(gap) <= limit:
                filled += 1
                for x, y in gap:
                    pixels[x, y] = 255
    print("filled %d enclosed gaps" % filled)
    return mask


def main():
    points = airports(AIRPORTS)
    if len(points) < 1000:
        raise SystemExit("too few airports found in %s" % AIRPORTS)

    width = int((MAX_LON - MIN_LON) * CELLS_PER_DEGREE)
    height = int((MAX_LAT - MIN_LAT) * CELLS_PER_DEGREE)
    mask = Image.new("L", (width, height), 0)
    pixels = mask.load()

    for lat, lon in points:
        x = int((lon - MIN_LON) * CELLS_PER_DEGREE)
        y = int((MAX_LAT - lat) * CELLS_PER_DEGREE)
        if 0 <= x < width and 0 <= y < height:
            pixels[x, y] = 255

    # Points to areas: dilate, then close (dilate/erode with a wide kernel) so
    # thinly served regions like the Sahara do not stay full of holes, then open
    # (erode/dilate) so single remote airfields do not become islands.
    mask = mask.filter(ImageFilter.MaxFilter(7))
    mask = mask.filter(ImageFilter.MaxFilter(13))
    mask = mask.filter(ImageFilter.MinFilter(13))
    mask = mask.filter(ImageFilter.MinFilter(9))
    mask = mask.filter(ImageFilter.MaxFilter(7))

    # Close the remaining pinholes — regions nobody flies to — but leave the
    # real seas alone: only fill a gap that does not touch the border and stays
    # small.
    mask = fill_small_holes(mask, limit=(width * height) // 400)

    # Soften the coast: upscale, blur, cut at half, blur once more for an edge
    # that survives being scaled up on the board.
    mask = mask.resize((width * 3, height * 3), Image.LANCZOS)
    mask = mask.filter(ImageFilter.GaussianBlur(9))
    mask = mask.point(lambda v: 255 if v > 128 else 0)
    mask = mask.filter(ImageFilter.GaussianBlur(2.5))

    land = Image.new("RGBA", mask.size, LAND + (255,))
    land.putalpha(mask)
    out = os.path.join(ROOT, "sailfish/world.png")
    land.save(out, optimize=True)
    print("wrote %s (%d×%d, %d airports)" % (out, land.size[0], land.size[1], len(points)))


if __name__ == "__main__":
    main()
