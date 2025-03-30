import sys
import matplotlib
matplotlib.use('Qt5Agg')

import matplotlib.pyplot as plt
from PIL import Image
import numpy as np

if len(sys.argv) < 2:
    print("Usage : python visualize_32_bits_grayscale_img.py <image.tiff>")
    sys.exit(1)

image_path = sys.argv[1]

try:
    image = Image.open(image_path)
except Exception as e:
    print(f"Erreur lors de l'ouverture de l'image : {e}")
    sys.exit(1)

image_data = np.array(image, dtype=np.float32)

if image_data.max() > 0:
    image_data = (image_data - image_data.min()) / (image_data.max() - image_data.min())

if image.mode not in ('I', 'F', 'L', 'I;16', 'I;32'):
    raise ValueError("L'image n'est pas en niveaux de gris. Mode détecté : " + image.mode)

plt.figure(figsize=(8, 6))
plt.imshow(image_data, cmap='gray', aspect='auto')
plt.colorbar(label="Intensité")
plt.title(f"Image 32-bit Grayscale : {image_path}")
plt.axis("off")
plt.show()

