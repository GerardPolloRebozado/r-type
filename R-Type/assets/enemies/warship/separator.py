from PIL import Image

# Splits r-typesheet37.gif (Warship, stage 3 boss) into separate pngs.
# Run from this folder: python3 separator.py
#
# All coordinates are measured on the sheet. Direction sets (turrets,
# fireballs) are numbered by angle: frame k points at k * 22.5 degrees,
# counter-clockwise from the right. So 000 points right, 004 points up (or
# down for "-bottom" / "-down" sets) and 008 points left.
#
# Turrets are pasted on a fixed canvas so their pivot stays at the same spot
# in every frame: swapping frames never makes them jump.

sheet = Image.open("./r-typesheet37.gif").convert("RGBA")
px = sheet.load()
bg = px[0, 0]  # top-left pixel is assumed to be background


def crop(x0, y0, x1, y1):
    """Crops [x0, x1) x [y0, y1) and makes the background transparent."""
    sprite = sheet.crop((x0, y0, x1, y1))
    sprite.putdata([(0, 0, 0, 0) if p == bg else p for p in sprite.getdata()])
    return sprite


def trim(sprite):
    box = sprite.getbbox()
    return sprite.crop(box) if box else sprite


def centered(sprite, w, h):
    canvas = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    canvas.paste(sprite, ((w - sprite.width) // 2, (h - sprite.height) // 2))
    return canvas


def save(sprite, name):
    sprite.save(f"warship-{name}.png")


def components(x0, y0, x1, y1, merge=3):
    """Groups the opaque pixels of a region into boxes, merging boxes closer
    than `merge` pixels (bullet trails are made of loose dots). Sorted by x."""
    seen = set()
    boxes = []
    for y in range(y0, y1):
        for x in range(x0, x1):
            if (x, y) in seen or px[x, y] == bg:
                continue
            stack = [(x, y)]
            seen.add((x, y))
            bx0, by0, bx1, by1 = x, y, x, y
            while stack:
                cx, cy = stack.pop()
                bx0, by0 = min(bx0, cx), min(by0, cy)
                bx1, by1 = max(bx1, cx), max(by1, cy)
                for nx in (cx - 1, cx, cx + 1):
                    for ny in (cy - 1, cy, cy + 1):
                        if (x0 <= nx < x1 and y0 <= ny < y1
                                and (nx, ny) not in seen and px[nx, ny] != bg):
                            seen.add((nx, ny))
                            stack.append((nx, ny))
            boxes.append([bx0, by0, bx1 + 1, by1 + 1])
    merged = True
    while merged:
        merged = False
        for i in range(len(boxes)):
            for j in range(i + 1, len(boxes)):
                a, b = boxes[i], boxes[j]
                if (a[0] - merge < b[2] and b[0] - merge < a[2]
                        and a[1] - merge < b[3] and b[1] - merge < a[3]):
                    boxes[i] = [min(a[0], b[0]), min(a[1], b[1]),
                                max(a[2], b[2]), max(a[3], b[3])]
                    del boxes[j]
                    merged = True
                    break
            if merged:
                break
    return sorted(boxes)


def by_angle(row_left_to_right):
    """Sheet rows go from pointing left to pointing right: reverse them so
    index k points at k * 22.5 degrees."""
    return list(reversed(row_left_to_right))


# --- Hull -------------------------------------------------------------------
# Whole ship (front section, rod and rear section) in one 592x176 piece.
save(crop(1, 1, 593, 177), "hull")
save(crop(1, 1247, 593, 1423), "hull-destroyed")  # same size, swap on death

# --- Front cannon and its beam -----------------------------------------------
save(crop(459, 695, 555, 727), "cannon")  # nozzle on the left
save(crop(455, 654, 487, 686), "cannon-flash000")  # vents, above the nozzle
save(crop(455, 736, 487, 768), "cannon-flash001")  # vents, below the nozzle
save(crop(390, 681, 454, 741), "glow")  # big white ball
for i, (x, y, w, h) in enumerate([(39, 701, 20, 20), (63, 697, 24, 28),
                                  (91, 695, 30, 32)]):
    save(centered(crop(x, y, x + w, y + h), 32, 32), f"beam-ring{i:03}")
save(crop(126, 695, 256, 727), "beam000")  # solid
save(crop(256, 695, 388, 727), "beam001")  # dithered start

# --- Small shots and puffs ----------------------------------------------------
for i, x in enumerate([251, 270, 289, 308]):
    save(centered(crop(x, 744, x + 16, 758), 16, 16), f"orb{i:03}")
save(crop(329, 739, 355, 763), "orb-flash")
for i, x in enumerate([270, 284, 298, 312]):
    save(centered(crop(x, 1224, x + 12, 1234), 12, 12), f"orb-small{i:03}")
for i, (y0, y1) in enumerate([(550, 582), (586, 607), (611, 636), (643, 669)]):
    save(centered(trim(crop(280, y0, 312, y1)), 32, 32), f"puff{i:03}")

# --- Hull add-ons -------------------------------------------------------------
save(crop(233, 770, 288, 802), "hatch000")  # slanted grille
save(crop(299, 770, 363, 802), "hatch001")  # yellow box with two windows
save(crop(217, 805, 377, 837), "undercarriage")  # bar under the thrusters

# Pink engine between the two sections, 66x32 each. Left column: idle
# animation. Right column: the cap opens and shows the red core.
for i, y in enumerate([1087, 1121, 1155, 1189]):
    save(crop(230, y, 296, y + 32), f"engine{i:03}")
    save(crop(296, y, 362, y + 32), f"engine-core{i:03}")

# --- Turrets ------------------------------------------------------------------
# Small turret on a flat base. "-top" sits on top of the hull, "-bottom" hangs
# under it. Pivot = middle of the base.
TURRET_X = [194, 224, 249, 271, 289, 307, 327, 347, 372]
TURRET_W = [28, 23, 18, 16, 16, 16, 18, 23, 28]
TURRET_CANVAS = 40


def turret_frames(y0, y1, base_y):
    frames = []
    for x, w in zip(TURRET_X, TURRET_W):
        sprite = crop(x, y0, x + w, y1)
        base = [bx for bx in range(sprite.width)
                if sprite.getpixel((bx, base_y - y0))[3]]
        pivot = (base[0] + base[-1] + 1) // 2
        canvas = Image.new("RGBA", (TURRET_CANVAS, y1 - y0), (0, 0, 0, 0))
        canvas.paste(sprite, (TURRET_CANVAS // 2 - pivot, 0))
        frames.append(canvas)
    return by_angle(frames)


for i, f in enumerate(turret_frames(202, 226, 224)):
    save(f, f"turret-top{i:03}")
for i, f in enumerate(turret_frames(228, 252, 228)):
    save(f, f"turret-bottom{i:03}")

# Dome turret with a striped base, 32x27 cells every 34px.
for i, f in enumerate(by_angle([crop(x, 841, x + 32, 868)
                                for x in range(146, 147 + 8 * 34, 34)])):
    save(f, f"dome{i:03}")

# Ball turret with a barrel. Pivot = middle of the ball, found by matching a
# patch of the ball (it is shaded the same in every frame).
BULB_X = [153, 189, 227, 266, 289, 308, 342, 376, 410]
BULB_W = [32, 30, 26, 21, 17, 21, 26, 30, 32]
BULB_CANVAS = 57
PATCH = crop(293, 893, 303, 903)  # 10x10 inside the ball of the "up" frame


def ball_center(sprite):
    best, best_pos = None, (0, 0)
    pw, ph = PATCH.size
    patch = list(PATCH.getdata())
    for oy in range(sprite.height - ph + 1):
        for ox in range(sprite.width - pw + 1):
            area = sprite.crop((ox, oy, ox + pw, oy + ph)).getdata()
            score = sum(sum((a - b) ** 2 for a, b in zip(p, q))
                        for p, q in zip(area, patch))
            if best is None or score < best:
                best, best_pos = score, (ox + pw // 2, oy + ph // 2)
    return best_pos


def bulb_frames(y0, y1):
    frames = []
    for x, w in zip(BULB_X, BULB_W):
        sprite = crop(x, y0, x + w, y1)
        cx, cy = ball_center(sprite)
        canvas = Image.new("RGBA", (BULB_CANVAS, BULB_CANVAS), (0, 0, 0, 0))
        canvas.paste(sprite, (BULB_CANVAS // 2 - cx, BULB_CANVAS // 2 - cy))
        frames.append(canvas)
    return by_angle(frames)


# The bottom row of the sheet is an exact mirror of the top one. The canvas
# size is odd so flipping keeps the pivot on the same pixel.
for i, f in enumerate(bulb_frames(878, 907)):
    save(f, f"bulb-top{i:03}")
    save(f.transpose(Image.FLIP_TOP_BOTTOM), f"bulb-bottom{i:03}")

# Spinning spiked mine, 16 frames in reading order.
MINE_X = [(169, 194), (202, 229), (232, 262), (265, 293),
          (300, 328), (331, 361), (364, 391), (399, 424)]
mines = [crop(x0, y0, x1, y1) for (y0, y1) in [(255, 285), (286, 318)]
         for (x0, x1) in MINE_X]
for i, m in enumerate(mines):
    save(centered(trim(m), 32, 32), f"mine{i:03}")

# --- Thrusters ----------------------------------------------------------------
# Nozzle: 000 points down, 001-004 tilted, 005 seen from below.
nozzles = [crop(285, 326, 309, 350), crop(230, 421, 260, 448),
           crop(268, 421, 294, 447), crop(299, 421, 325, 447),
           crop(333, 421, 363, 448), crop(282, 518, 309, 543)]


def fill_rows(sprite, color=(24, 24, 24, 255)):
    """The nozzle stripes use the sheet's background color, so they come out
    see-through. Fill the gaps between the outermost pixels of
    each column, then of each row (the nozzle is convex)."""
    sprite = sprite.copy()
    p = sprite.load()
    w, h = sprite.size
    for x in range(w):
        ys = [y for y in range(h) if p[x, y][3]]
        for y in range(ys[0], ys[-1] + 1) if ys else []:
            if not p[x, y][3]:
                p[x, y] = color
    for y in range(h):
        xs = [x for x in range(w) if p[x, y][3]]
        for x in range(xs[0], xs[-1] + 1) if xs else []:
            if not p[x, y][3]:
                p[x, y] = color
    return sprite


for i, n in enumerate(nozzles):
    save(centered(fill_rows(n), 32, 28), f"thruster{i:03}")

# Flame going down, 36x64, top aligned (it starts at the nozzle).
for i, (x0, x1) in enumerate([(229, 261), (263, 295), (300, 330), (331, 365)]):
    flame = crop(x0, 353, x1, 417)
    canvas = Image.new("RGBA", (36, 64), (0, 0, 0, 0))
    canvas.paste(flame, ((36 - flame.width) // 2, 0))
    save(canvas, f"flame{i:03}")

# Big exhaust clouds for tilted thrusters, left to right as on the sheet.
EXHAUST_X = [30, 98, 164, 232, 297, 361, 429, 495, 563]
for i in range(8):
    save(crop(EXHAUST_X[i], 453, EXHAUST_X[i + 1], 518), f"exhaust{i:03}")

# --- Fireballs ----------------------------------------------------------------
# Two sizes, 16 directions each, centered on 32x32.
def fireballs(top_band, bottom_band, name):
    up = components(150, top_band[0], 445, top_band[1])
    down = components(150, bottom_band[0], 445, bottom_band[1])
    assert len(up) == 9 and len(down) == 9, (name, len(up), len(down))
    # up row: left .. up .. right (180 -> 0 deg)
    # down row: left .. down .. right (180 -> 360 deg)
    ordered = by_angle(up) + down[1:8]
    for i, (x0, y0, x1, y1) in enumerate(ordered):
        save(centered(crop(x0, y0, x1, y1), 32, 32), f"{name}{i:03}")


fireballs((939, 971), (1045, 1077), "fireball")
fireballs((972, 1004), (1012, 1044), "fireball-small")

# --- Explosions ---------------------------------------------------------------
EXPLOSION_X = [(142, 163), (166, 190), (193, 225), (228, 261), (263, 296),
               (300, 333), (335, 368), (370, 403), (405, 430), (432, 454)]
for row, (y0, y1) in [("top", (1450, 1484)), ("bottom", (1486, 1520))]:
    for i, (x0, x1) in enumerate(EXPLOSION_X):
        save(centered(crop(x0, y0, x1, y1), 34, 34), f"explosion-{row}{i:03}")
