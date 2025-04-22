import numpy as np
from skimage.metrics import structural_similarity as ssim
from skimage.metrics import peak_signal_noise_ratio as psnr
import tifffile as tiff

# Charger les images TIFF (float64)
ref_image = tiff.imread('image_reference.tiff')  # image originale
test_image = tiff.imread('image_test.tiff')      # image à comparer

# Assurer que les deux images ont les mêmes dimensions
assert ref_image.shape == test_image.shape, "Les images doivent avoir les mêmes dimensions"

# Calcul du PSNR
psnr_value = psnr(ref_image, test_image, data_range=np.max(ref_image) - np.min(ref_image))

# Calcul du SSIM
ssim_value = ssim(ref_image, test_image, data_range=np.max(ref_image) - np.min(ref_image))

# Affichage des résultats
print(f"PSNR : {psnr_value:.2f} dB")
print(f"SSIM : {ssim_value:.4f}")
