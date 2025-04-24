import sys
import matplotlib
matplotlib.use('Qt5Agg')

import matplotlib.pyplot as plt
import numpy as np
import tifffile as tiff

if len(sys.argv) < 2:
    print("Usage : python visualize_64bit_grayscale_img.py <image.tif>")
    sys.exit(1)

image_path = sys.argv[1]

try:
    image_data = tiff.imread(image_path)
except Exception as e:
    print(f"Erreur lors de l'ouverture de l'image : {e}")
    sys.exit(1)

# Check dtype
print(f"Dtype de l'image : {image_data.dtype}, shape : {image_data.shape}")

# Display
plt.figure(figsize=(8, 6))
plt.imshow(image_data, cmap='gray', aspect='auto')
plt.colorbar(label="Intensité")
plt.title(f"Image float64 TIFF : {image_path}")
plt.axis("off")
plt.show()

