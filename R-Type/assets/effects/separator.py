from PIL import Image

sheet = Image.open("./enemy-explosion-1.png")
count = sheet.width // 32

for i in range(count):
    sprite = sheet.crop((i * 32, 0, (i + 1) * 32, 32))
    sprite.save(f"enemy-explosion-1{i:03}.png")

