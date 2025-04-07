from PIL import Image
import sys

if len(sys.argv) < 2:
    print("Usage : python inspect_image.py <image_path>")
    sys.exit(1)

image_path = sys.argv[1]

try:
    image = Image.open(image_path)
except Exception as e:
    print(f"Erreur lors de l'ouverture de l'image : {e}")
    sys.exit(1)

format_image = image.format
size_image = image.size
mode_image = image.mode
info_image = image.info

bits_per_pixel = None
if mode_image == '1':
    bits_per_pixel = 1
elif mode_image == 'L':
    bits_per_pixel = 8
elif mode_image == 'I;16':
    bits_per_pixel = 16
elif mode_image in ('RGB', 'YCbCr'):
    bits_per_pixel = 24
elif mode_image == 'RGBA':
    bits_per_pixel = 32
elif mode_image == 'I;32' or mode_image == 'F':
    bits_per_pixel = 32
else:
    bits_per_pixel = "Inconnu"

if mode_image in ('L', 'I;16', 'I;32', 'F'):
    color_type = "Grayscale"
elif mode_image in ('RGB', 'RGBA', 'CMYK', 'YCbCr'):
    color_type = "Couleur"
else:
    color_type = "Inconnu"

print(f"Fichier       : {image_path}")
print(f"Format        : {format_image}")
print(f"Dimensions    : {size_image[0]}x{size_image[1]}")
print(f"Mode          : {mode_image}")
print(f"Type de couleur : {color_type}")
print(f"Bits par pixel : {bits_per_pixel}")

if info_image:
    print(f"Métadonnées   : {info_image}")
else:
    print("Aucune métadonnée disponible.")
