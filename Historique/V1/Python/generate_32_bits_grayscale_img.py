from PIL import Image
import numpy as np

width, height = 256, 256
data = np.random.uniform(0, 255, (height, width)).astype(np.float32)
image = Image.fromarray(data)
image = image.convert("F")
image.save("image_32bit_grayscale.tiff", format="TIFF")

print("Image TIFF 32 bits en niveaux de gris sauvegardée.")
