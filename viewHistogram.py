import sys
import matplotlib.pyplot as plt
import numpy as np
import os

def load_histogram(filename):
    values, counts = [], []
    with open(filename, 'r') as f:
        for line in f:
            if ',' in line:
                v, c = line.strip().split(',')
                values.append(float(v))
                counts.append(int(c))
    return np.array(values), np.array(counts)

if len(sys.argv) != 2:
    print("Usage : python3 viewHistogram.py <fichier_histogramme.dat>")
    sys.exit(1)

filepath = sys.argv[1]

if not os.path.exists(filepath):
    print(f"Fichier introuvable : {filepath}")
    sys.exit(1)

x, y = load_histogram(filepath)

plt.figure(figsize=(10, 5))
plt.plot(x, y, label=os.path.basename(filepath), color='steelblue')
plt.title("Histogramme")
plt.xlabel("Valeur des pixels")
plt.ylabel("Nombre de pixels")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("histogramme_single.png")
plt.show()
