from PIL import Image
import numpy as np
import tifffile

# Charger le PNG et convertir en niveaux de gris
img = Image.open("bin/ChatGPT Image 18 avr. 2025, 22_23_29.png").convert("L")  # "L" = grayscale

# Convertir en tableau float64
arr = np.array(img).astype(np.float64)

# Sauvegarder en TIFF float64
tifffile.imwrite("image_float64.tiff", arr)
