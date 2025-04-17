#include "TiFF.hpp"
#include <iostream>
#include <random>
#include <ctime>
#include <algorithm>
#include <cmath>

void smartGrow(const TiFF& inputMask, TiFF& outputMask, int voisinageMin = 2) {
    uint32_t width = inputMask.getWidth();
    uint32_t height = inputMask.getHeight();

    outputMask.create(width, height, 0.0);  // assure que la sortie est propre

    for (uint32_t y = 1; y < height - 1; ++y) {
        for (uint32_t x = 1; x < width - 1; ++x) {
            if (inputMask.getPixel(x, y) == 1.0) {
                int voisins_detectes = 0;

                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        if (inputMask.getPixel(x + dx, y + dy) == 1.0)
                            ++voisins_detectes;
                    }
                }

                if (voisins_detectes >= voisinageMin) {
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx)
                            outputMask.setPixel(x + dx, y + dy, 1.0);
                } else {
                    outputMask.setPixel(x, y, 1.0);
                }
            }
        }
    }
}

double autoTuneK_IQR(const TiFF& img) {
    std::vector<double> values;
    for (uint32_t y = 0; y < img.getHeight(); ++y)
        for (uint32_t x = 0; x < img.getWidth(); ++x)
            values.push_back(img.getPixel(x, y));

    std::sort(values.begin(), values.end());
    size_t n = values.size();

    // Percentiles plus conservateurs
    double q1 = values[n / 20];        // 5%
    double q9 = values[9 * n / 10];    // 90%
    double iqr = q9 - q1;

    // Ne garder que les pixels sous q90 pour calculer fond
    double sum = 0.0, sumSq = 0.0;
    size_t count = 0;
    for (double val : values) {
        if (val <= q9) {
            sum += val;
            sumSq += val * val;
            ++count;
        }
    }

    double mean = sum / count;
    double stddev = std::sqrt(std::max((sumSq / count) - (mean * mean), 1e-8));

    double threshold = q9 + 1.5 * iqr;  // Plus haut, plus tolérant
    double k = (threshold - mean) / stddev;

    return std::clamp(k, 1.0, 8.0);
}


int main() {
    TiFF image;
    const uint32_t width = 2048;
    const uint32_t height = 2048;

    if (!image.create(width, height)) {
        std::cerr << "Erreur lors de la création de l'image." << std::endl;
        return 1;
    }

    std::random_device rd;
    std::mt19937 generator(rd());

    // --- Étape 1 : Générer fond bruité + gradient ---
    std::uniform_real_distribution<double> uniform_fond(5000.0, 5000.0);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            double fx = std::sin(2.0 * M_PI * x / width);
            double fy = std::cos(2.0 * M_PI * y / height);
            double micro_var = 20.0 * (fx + fy);  // amplitude très faible
    
            double val = 5500.0 + micro_var;
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

    // --- Étape 4 : Bruit blanc additif + spikes aléatoires ---
    std::normal_distribution<double> bruit_blanc(0.0, 2.0);  // σ = 2.0
    std::uniform_int_distribution<uint32_t> spike_x(1, width - 2);
    std::uniform_int_distribution<uint32_t> spike_y(1, height - 2);
    std::uniform_real_distribution<double> spike_val(100.0, 300.0);  // intensité des spikes

    const int nb_spikes = 150;  // nombre de points blancs aléatoires

    // 1. Ajouter le bruit blanc
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            double val = image.getPixel(x, y);
            val += bruit_blanc(generator);
            image.setPixel(x, y, std::max(0.0, val));  // clamp si bruit rend négatif
        }
    }

    // 2. Injecter des spikes lumineux isolés
    for (int i = 0; i < nb_spikes; ++i) {
        uint32_t x = spike_x(generator);
        uint32_t y = spike_y(generator);
        double spike = spike_val(generator);
        double val = image.getPixel(x, y);
        image.setPixel(x, y, val + spike);
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

    // --- Étape 6 : Masque spikes - Z-score local sur image normalisée (P5-P95) ---
    TiFF image_normalisee = image;  // copie de l'image après bruit Poisson
    image_normalisee.normalizePercentile(0.05, 0.95);  // compresse les extrêmes

    TiFF mask_zscore;
    mask_zscore.create(width, height, 0.0);

    // On calcule k sur l’image normalisée (plus stable visuellement)
    double k = autoTuneK_IQR(image_normalisee);
    std::cout << "k (IQR sur image normalisée P5-P95) = " << k << std::endl;

    for (uint32_t y = 1; y < height - 1; ++y) {
        for (uint32_t x = 1; x < width - 1; ++x) {
            double val = image_normalisee.getPixel(x, y);

            // Moyenne et variance locale sur l'image normalisée
            double sum = 0.0, sumSq = 0.0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    double neighbor = image_normalisee.getPixel(x + dx, y + dy);
                    sum += neighbor;
                    sumSq += neighbor * neighbor;
                }
            }

            double mean = sum / 8.0;
            double variance = (sumSq / 8.0) - (mean * mean);
            double stddev = std::sqrt(std::max(variance, 1e-8));

            // Détection de spike sur image normalisée
            if (val > mean + k * stddev) {
                mask_zscore.setPixel(x, y, 1.0);
            }
        }
    }

    // --- Étape 7 : Dilatation intelligente (smart grow) ---
    TiFF mask_grow;
    mask_grow.create(width, height, 0.0);

    smartGrow(mask_zscore, mask_grow, 2); 

    // Sauvegarde du masque étendu
    mask_grow.save("masque_spikes_grow.tif");

    // --- Étape 8 : Sauvegardes finales ---
    image.save("image_gamma_poisson.tif");
    mask_zscore.save("masque_spikes_zscore.tif");

    std::cout << "Image bruitée + 3 masques sauvegardés (bruit, z-score, hybride)." << std::endl;
    return 0;
}

// Faire la soustraction de noir avant correction des artéfacts blancs