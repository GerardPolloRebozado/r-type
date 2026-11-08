from PIL import Image

# Splits r-typesheet30.gif (Dobkeratops) into the frames used by
# scripts/dobkeratops.lua and scripts/dobkeratopsBullet.lua.
#
# Sheet layout (all coordinates measured on the sheet):
#   - Body: 4 columns x 9 rows of 155x204 frames. Columns only differ in the
#     head (mouth animation), rows only differ in the 32x32 core slot. We keep
#     the first row (no core) and draw the core as its own entity.
#   - Core row A (y=1928): full core -> shrinking -> empty hole -> closed skin.
#   - Core row B (y=1964): core fully out, chomping.
#   - Tail balls: one large, one medium, one small (copies are identical).
#   - Tail tip: 9 rotation frames. Bullets: 4 rotation frames.

sheet = Image.open("./r-typesheet30.gif").convert("RGBA")
px = sheet.load()
bg = px[0, 0]  # top-left pixel is assumed to be background


def crop(x, y, w, h):
    sprite = sheet.crop((x, y, x + w, y + h))
    sprite.putdata([(0, 0, 0, 0) if p == bg else p for p in sprite.getdata()])
    return sprite


def place(sprite, size, align="center"):
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    if align == "right":
        x = size - sprite.width
    else:
        x = (size - sprite.width) // 2
    canvas.paste(sprite, (x, (size - sprite.height) // 2))
    return canvas


BODY_W, BODY_H = 155, 204
for i, x in enumerate([27, 189, 351, 513]):
    crop(x, 2, BODY_W, BODY_H).save(f"dobkeratops{i:03}.png")

# Core frames are right-aligned: their right edge sticks to the body.
cores = [(471, 1928, 32), (510, 1928, 26), (549, 1928, 20), (582, 1928, 20),
         (615, 1928, 20), (643, 1928, 25),
         (570, 1964, 32), (603, 1964, 32), (636, 1964, 32)]
for i, (x, y, w) in enumerate(cores):
    place(crop(x, y, w, 32), 32, "right").save(f"dobkeratops-core{i:03}.png")

tails = [(544, 2000, 16, 15), (574, 2017, 14, 13), (600, 2032, 12, 11)]
for i, (x, y, w, h) in enumerate(tails):
    place(crop(x, y, w, h), 16).save(f"dobkeratops-tail{i:03}.png")

for i, x in enumerate([508, 526, 544, 562, 580, 598, 616, 634, 652]):
    place(crop(x, 2045, 16, 16), 16).save(f"dobkeratops-tip{i:03}.png")

for i, (x, w) in enumerate([(576, 21), (599, 22), (623, 21), (646, 22)]):
    place(crop(x, 2063, w, 20), 22).save(f"dobkeratops-bullet{i:03}.png")
