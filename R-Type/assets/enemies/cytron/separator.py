from PIL import Image

sheet = Image.open("./r-typesheet19.gif").convert("RGBA")
w, h = sheet.size
px = sheet.load()
bg = px[0, 0]  # top-left pixel is assumed to be background

def empty(x, y):
    p = px[x, y]
    return p[3] == 0 or p == bg

def spans(filled):
    out, start = [], None
    for i, f in enumerate(filled + [False]):
        if f and start is None:
            start = i
        elif not f and start is not None:
            out.append((start, i))
            start = None
    return out

FRAME_SIZE = 32  # body frames are centered on a fixed canvas so they don't jitter
BODY_FRAMES = 16  # the last sprite (the trail dot) keeps its own size

i = 0
for y0, y1 in spans([any(not empty(x, y) for x in range(w)) for y in range(h)]):
    cols = [any(not empty(x, y) for y in range(y0, y1)) for x in range(w)]
    for x0, x1 in spans(cols):
        sprite = sheet.crop((x0, y0, x1, y1))
        # make the background colour transparent
        sprite.putdata([(0, 0, 0, 0) if p == bg else p for p in sprite.getdata()])
        if i < BODY_FRAMES:
            canvas = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
            canvas.paste(sprite, ((FRAME_SIZE - sprite.width) // 2, (FRAME_SIZE - sprite.height) // 2))
            sprite = canvas
        sprite.save(f"cytron{i:03}.png")
        i += 1
