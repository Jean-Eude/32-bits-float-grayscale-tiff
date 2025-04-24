#include "TiFF.hpp"
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <fstream>

void saveHistogram(const TiFF& img, const std::string& filename, int numBins = 256) {
    const std::vector<double>& data = img.getData();
    if (data.empty()) return;

    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());
    double range = maxVal - minVal;

    if (range == 0.0) {
        std::cerr << "Image uniforme, histogramme inutile." << std::endl;
        return;
    }

    std::vector<int> histogram(numBins, 0);

    for (double value : data) {
        int bin = static_cast<int>((value - minVal) / range * (numBins - 1));
        bin = std::clamp(bin, 0, numBins - 1);
        histogram[bin]++;
    }

    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Erreur : impossible d’écrire le fichier " << filename << std::endl;
        return;
    }

    for (int i = 0; i < numBins; ++i) {
        double binValue = minVal + (i / static_cast<double>(numBins - 1)) * range;
        out << binValue << "," << histogram[i] << "\n";
    }

    out.close();
    std::cout << "📊 Histogramme sauvegardé dans : " << filename << std::endl;
}

bool isInPlusPattern(uint32_t x, uint32_t y, uint32_t w, uint32_t h, int thickness) {
    return (std::abs(int(x) - int(w / 2)) < thickness) || (std::abs(int(y) - int(h / 2)) < thickness);
}

void createBaseImage(TiFF& img, uint32_t w, uint32_t h) {
    img.create(w, h, 10000.0);
    double cx = w / 2.0, cy = h / 2.0;
    double radius = std::min(w, h) * 0.4;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            double v = 10000.0;
            if (dist < radius) {
                v += 55000.0 * std::cos((dist / radius) * M_PI);
            }
            img.setPixel(x, y, std::clamp(v, 0.0, 65535.0));
        }
    }
}

void applySinusoidalPattern(TiFF& img, double amplitude, double periodX, double periodY) {
    uint32_t w = img.getWidth(), h = img.getHeight();

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            double fx = std::sin(2.0 * M_PI * x / periodX);
            double fy = std::sin(2.0 * M_PI * y / periodY);
            double factor = 1.0 + amplitude * fx * fy;

            double val = img.getPixel(x, y);
            img.setPixel(x, y, std::min(65535.0, val * factor));
        }
    }
}

void divideImages(const TiFF& numerator, const TiFF& denominator, TiFF& result) {
    uint32_t w = numerator.getWidth();
    uint32_t h = numerator.getHeight();
    result.create(w, h, 0.0);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            double denom = denominator.getPixel(x, y);
            double val = numerator.getPixel(x, y);
            result.setPixel(x, y, denom > 1e-8 ? val / denom : 0.0);
        }
    }
}

int main() {
    uint32_t width = 512, height = 512;
    double multiplicative_factor = 1.2;

    TiFF imgBase, imgRaw, flatField, result;

    // Étape 1 : image de base
    createBaseImage(imgBase, width, height);

    // Étape 2 : copie sur laquelle on applique le motif multiplicatif
    imgRaw = imgBase;
    applySinusoidalPattern(imgRaw, 0.3, 64.0, 64.0);  // Motif sinusoïdal

    // Étape 3 : créer flatfield = imgRaw / imgBase (=> contient * uniquement dans le +)
    divideImages(imgRaw, imgBase, flatField);  // flatField = multiplicative pattern

    // Étape 4 : imgRaw / flatField → devrait revenir à imgBase
    divideImages(imgRaw, flatField, result);

    // corrected(x, y) = imgRaw(x, y) / flatField(x, y) = (imgBase(x, y) * f(x, y)) / f(x, y) = imgBase(x, y)

    // Sauvegardes
    imgBase.save("img_base.tiff");
    imgRaw.save("img_with_plus.tiff");
    flatField.save("flat_field.tiff");
    result.save("corrected.tiff");

    saveHistogram(imgBase, "hist_raw.dat");
    saveHistogram(flatField, "hist_flat.dat");
    saveHistogram(result, "hist_corrected.dat");

    std::cout << "✅ Images générées avec succès." << std::endl;
    return 0;
}
