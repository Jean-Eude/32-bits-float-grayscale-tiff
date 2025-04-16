#include "TiFF.hpp"
#include <iostream>
#include <random>
#include <ctime>
#include <algorithm>
#include <cmath>

int main() {
    TiFF image;
    const uint32_t width = 256;
    const uint32_t height = 256;

    if (!image.create(width, height)) {
        std::cerr << "Erreur lors de la création de l'image." << std::endl;
        return 1;
    }

    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> gauss(2.0, 1.0);

    // --- Étape 1 : Générer fond bruité + gradient ---
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            double val = gauss(generator);
            val += 0.005 * (x + y);  // gradient
            val = std::clamp(val, 0.0, 10.0);
            image.setPixel(x, y, val);
        }
    }

    // --- Étape 2 : Ajouter des sources diffuses (zones circulaires) ---
    std::vector<std::pair<int, int>> sources = {
        {60, 60}, {100, 120}, {180, 90}, {220, 200}, {140, 180}
    };
    std::uniform_real_distribution<double> source_intensity(10.0, 20.0);
    const int radius = 10;

    for (const auto& [cx, cy] : sources) {
        double intensity = source_intensity(generator);
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int x = cx + dx;
                int y = cy + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    double distance = std::sqrt(dx * dx + dy * dy);
                    if (distance <= radius) {
                        // Intensité décroissante avec la distance
                        double weight = 1.0 - (distance / (radius + 1));
                        double val = image.getPixel(x, y);
                        image.setPixel(x, y, val + intensity * weight);
                    }
                }
            }
        }
    }

    // --- Étape 3 : Copier l’image avant le bruit ---
    TiFF imageAvantBruit = image;

    // --- Étape 4 : Bruit de Poisson ---
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            double val = image.getPixel(x, y);
            std::poisson_distribution<int> poisson(std::max(val, 0.0));
            image.setPixel(x, y, static_cast<double>(poisson(generator)));
        }
    }

    // --- Étape 5 : Masque des pixels modifiés par le bruit de Poisson ---
    TiFF mask_bruit_poisson;
    mask_bruit_poisson.create(width, height, 0.0);
    const double seuil_delta = 2.0;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            double avant = imageAvantBruit.getPixel(x, y);
            double apres = image.getPixel(x, y);
            if (std::abs(apres - avant) >= seuil_delta) {
                mask_bruit_poisson.setPixel(x, y, 1.0);
            }
        }
    }

    if (!mask_bruit_poisson.save("masque_bruit_poisson.tif")) {
        std::cerr << "Erreur lors de la sauvegarde du masque de bruit de Poisson." << std::endl;
        return 1;
    }

    // --- Étape 6 : Masque spikes - Méthode 1 : Z-score local ---
    TiFF mask_zscore;
    mask_zscore.create(width, height, 0.0);
    const double z_threshold = 3.5;

    for (uint32_t y = 1; y < height - 1; ++y) {
        for (uint32_t x = 1; x < width - 1; ++x) {
            double val = image.getPixel(x, y);

            // Moyenne + variance locale
            double sum = 0.0, sumSq = 0.0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    double neighbor = image.getPixel(x + dx, y + dy);
                    sum += neighbor;
                    sumSq += neighbor * neighbor;
                }
            }

            double mean = sum / 8.0;
            double variance = (sumSq / 8.0) - (mean * mean);
            double stddev = std::sqrt(std::max(variance, 0.0));

            if (val > mean + 3 * stddev) {
                mask_zscore.setPixel(x, y, 1.0);
            }
        }
    }

    // --- Étape 7 : Masque spikes - Méthode 2 : Quantile + z-score ---
    TiFF mask_hybride;
    mask_hybride.create(width, height, 0.0);

    // Calcul du seuil global (99.9e percentile)
    std::vector<double> allPixels;
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
            allPixels.push_back(image.getPixel(x, y));
    std::sort(allPixels.begin(), allPixels.end());
    size_t index = static_cast<size_t>(0.999 * allPixels.size());
    index = std::min(index, allPixels.size() - 1);
    double globalThreshold = allPixels[index];

    for (uint32_t y = 1; y < height - 1; ++y) {
        for (uint32_t x = 1; x < width - 1; ++x) {
            double val = image.getPixel(x, y);
            if (val < globalThreshold) continue;

            double sum = 0.0, sumSq = 0.0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    double neighbor = image.getPixel(x + dx, y + dy);
                    sum += neighbor;
                    sumSq += neighbor * neighbor;
                }
            }

            double mean = sum / 8.0;
            double var = (sumSq / 8.0) - (mean * mean);
            double stddev = std::sqrt(std::max(var, 0.0));
            double z = (val - mean) / (stddev + 1e-6);

            if (z > z_threshold)
                mask_hybride.setPixel(x, y, 1.0);
        }
    }

    // --- Étape 8 : Sauvegardes finales ---
    image.normalize();  // pour affichage plus lisible
    image.save("image_gamma_poisson.tif");
    mask_zscore.save("masque_spikes_zscore.tif");
    mask_hybride.save("masque_spikes_hybride.tif");

    std::cout << "Image bruitée + 3 masques sauvegardés (bruit, z-score, hybride)." << std::endl;
    return 0;
}

// Faire la soustraction de noir avant correction des artéfacts blancs