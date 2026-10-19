from PIL import Image

sheet = Image.open("./bink.png")
count = sheet.width // 32

for i in range(count):
    sprite = sheet.crop((i * 32, 0, (i + 1) * 32, 32))
    sprite.save(f"bink{i:03}.png")

