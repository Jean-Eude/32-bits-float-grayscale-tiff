import sys
import matplotlib.pyplot as plt
import numpy as np
import os
import tifffile

def load_image(filepath):
    """Charge une image TIFF 64 bits (float64 compatible)."""
    try:
        image = tifffile.imread(filepath)
        return image
    except Exception as e:
        print(f"Erreur lors du chargement de l'image : {e}")
        sys.exit(1)

if len(sys.argv) != 2:
    print("Usage : python3 viewLineProfile.py <image.tiff>")
    sys.exit(1)

filepath = sys.argv[1]

if not os.path.exists(filepath):
    print(f"Fichier introuvable : {filepath}")
    sys.exit(1)

# Chargement de l'image (64 bits compatible)
img = load_image(filepath)

if img.ndim != 2:
    print("L'image doit être en niveaux de gris (2D).")
    sys.exit(1)

height, width = img.shape

# Extraire la ligne du milieu
middle_line = img[height // 2, :]

# Créer l'axe X (colonnes de l'image)
x = np.arange(width)

# Afficher le profil de ligne
plt.figure(figsize=(12, 6))
plt.plot(x, middle_line, linewidth=1., color='teal')  # couleur changée
plt.title("Profil d’intensité – ligne centrale", fontsize=16)
plt.xlabel("Position horizontale (pixels)", fontsize=12)
plt.ylabel("Intensité des pixels", fontsize=12)
plt.grid(True, linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig("line_profile_middle.png", dpi=300)