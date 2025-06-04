import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import csv

# --- Chargement des données ---
k_vals, psnr_vals, ssim_vals = [], [], []
with open("resultats.csv", 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        k_vals.append(float(row['k']))
        psnr_vals.append(float(row['PSNR']))
        ssim_vals.append(float(row['SSIM']))

# --- Création de la figure ---
fig, ax1 = plt.subplots(figsize=(10, 6))

# Couleurs
color1 = 'tab:blue'
color2 = 'tab:green'

# Axe X en log
ax1.set_xscale('log')
ax1.set_xlabel("k (échelle logarithmique)", fontsize=12)

# Axe Y gauche : PSNR
ax1.set_ylabel("PSNR (dB)", color=color1, fontsize=12)
psnr_plot, = ax1.plot(k_vals, psnr_vals, marker='o', color=color1, label="PSNR", linewidth=2)
ax1.tick_params(axis='y', labelcolor=color1)

# Ticks X manuels et format décimal (pas de notation scientifique)
ax1.set_xticks([0.5, 1, 2, 3, 4])
ax1.get_xaxis().set_major_formatter(ScalarFormatter())
ax1.ticklabel_format(style='plain', axis='x')
ax1.grid(True, which='both', linestyle='--', alpha=0.4)

# Axe Y droit : SSIM
ax2 = ax1.twinx()
ax2.set_ylabel("SSIM", color=color2, fontsize=12)
ssim_plot, = ax2.plot(k_vals, ssim_vals, marker='s', color=color2, label="SSIM", linewidth=2)
ax2.tick_params(axis='y', labelcolor=color2)

# Légende combinée
lines = [psnr_plot, ssim_plot]
labels = [line.get_label() for line in lines]
ax1.legend(lines, labels, loc='lower right', fontsize=11)

# Titre
plt.title("PSNR et SSIM en fonction de k (log sans notation scientifique)", fontsize=14)

# Sauvegarde et affichage
fig.tight_layout()
plt.savefig("psnr_ssim_logk_clean.png", dpi=300)
plt.show()
