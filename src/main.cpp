#include <tiffio.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

// --- Classe TiffFloat64Image à insérer ici (déjà fournie précédemment) ---

class TiffFloat64Image {
public:
    TiffFloat64Image() : width(0), height(0) {}

    bool load(const std::string& filename) {
        TIFF* tif = TIFFOpen(filename.c_str(), "r");
        if (!tif) {
            std::cerr << "Erreur : impossible d'ouvrir le fichier : " << filename << std::endl;
            return false;
        }

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        uint16_t bitsPerSample = 0, sampleFormat = 0, samplesPerPixel = 1;
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
        TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

        if (bitsPerSample != 64 || sampleFormat != SAMPLEFORMAT_IEEEFP || samplesPerPixel != 1) {
            std::cerr << "Erreur : image non conforme (float64 grayscale attendu)." << std::endl;
            TIFFClose(tif);
            return false;
        }

        data.resize(width * height);

        for (uint32_t row = 0; row < height; ++row) {
            double* buf = (double*)_TIFFmalloc(width * sizeof(double));
            if (!buf || TIFFReadScanline(tif, buf, row) < 0) {
                std::cerr << "Erreur lors de la lecture de la ligne " << row << std::endl;
                _TIFFfree(buf);
                TIFFClose(tif);
                return false;
            }
            std::copy(buf, buf + width, &data[row * width]);
            _TIFFfree(buf);
        }

        TIFFClose(tif);
        return true;
    }

    bool save(const std::string& filename) const {
        TIFF* tif = TIFFOpen(filename.c_str(), "w");
        if (!tif) {
            std::cerr << "Erreur : impossible de créer le fichier : " << filename << std::endl;
            return false;
        }

        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 64);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB); // Little endian

        for (uint32_t row = 0; row < height; ++row) {
            const double* buf = &data[row * width];
            if (TIFFWriteScanline(tif, (void*)buf, row, 0) < 0) {
                std::cerr << "Erreur lors de l'écriture de la ligne " << row << std::endl;
                TIFFClose(tif);
                return false;
            }
        }

        TIFFClose(tif);
        return true;
    }

    bool create(uint32_t w, uint32_t h, double initial_value = 0.0) {
        if (w == 0 || h == 0) return false;
        width = w;
        height = h;
        data.resize(width * height, initial_value);
        return true;
    }

    double getPixel(uint32_t x, uint32_t y) const {
        if (x >= width || y >= height) return 0.0;
        return data[y * width + x];
    }

    void setPixel(uint32_t x, uint32_t y, double value) {
        if (x < width && y < height) {
            data[y * width + x] = value;
        }
    }

    void normalize() {
        if (data.empty()) return;
        double min_val = *std::min_element(data.begin(), data.end());
        double max_val = *std::max_element(data.begin(), data.end());
        if (max_val > min_val) {
            for (auto& px : data) {
                px = (px - min_val) / (max_val - min_val);
            }
        }
    }

    void fill(double value) {
        std::fill(data.begin(), data.end(), value);
    }

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

private:
    uint32_t width, height;
    std::vector<double> data;
};

// --------- MAIN : Création + seuillage ---------

int main() {
    // Étape 1 : Créer une image float64 avec une rampe simple
    TiffFloat64Image img;
    img.create(256, 256); // 256x256 pixels

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            img.setPixel(x, y, static_cast<double>(x + y) / (img.getWidth() + img.getHeight()));
        }
    }

    if (!img.save("image_64bit_grayscale.tiff")) {
        std::cerr << "Erreur lors de la création de l'image de base." << std::endl;
        return 1;
    }

    std::cout << "Image de base sauvegardée sous image_64bit_grayscale.tiff" << std::endl;

    // Étape 2 : Charger et seuiller
    if (!img.load("image_64bit_grayscale.tiff")) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        return 1;
    }

    double threshold = 0.5;

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            double val = img.getPixel(x, y);
            double bin = (val >= threshold) ? 1.0 : 0.0;
            img.setPixel(x, y, bin);
        }
    }

    if (!img.save("output_thresholded_64bit.tiff")) {
        std::cerr << "Erreur lors de l'enregistrement de l'image seuillée." << std::endl;
        return 1;
    }

    std::cout << "Seuillage terminé. Image sauvegardée dans output_thresholded_64bit.tiff" << std::endl;
    return 0;
}

img.flipud();   // retourne l'image verticalement
img.fliplr();   // retourne l'image horizontalement

void flipud() {
    for (uint32_t y = 0; y < height / 2; ++y) {
        uint32_t opposite = height - 1 - y;
        for (uint32_t x = 0; x < width; ++x) {
            std::swap(data[y * width + x], data[opposite * width + x]);
        }
    }
}

void fliplr() {
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width / 2; ++x) {
            uint32_t opposite = width - 1 - x;
            std::swap(data[y * width + x], data[y * width + opposite]);
        }
    }
}

Champs TIFF                    | Obligatoire / Facultatif          | Description
------------------------------|-----------------------------------|---------------------------------------------------------------
TIFFTAG_IMAGEWIDTH            | Obligatoire                       | Largeur de l'image (en pixels).
TIFFTAG_IMAGELENGTH           | Obligatoire                       | Hauteur de l'image (en pixels).
TIFFTAG_SAMPLESPERPIXEL       | Obligatoire                       | Nombre de canaux par pixel (1 = grayscale).
TIFFTAG_BITSPERSAMPLE         | Obligatoire                       | Nombre de bits par échantillon (64 pour float64).
TIFFTAG_SAMPLEFORMAT          | Obligatoire                       | Format des pixels (SAMPLEFORMAT_IEEEFP pour float64).
TIFFTAG_COMPRESSION           | Facultatif                        | Type de compression. COMPRESSION_NONE est la valeur par défaut.
TIFFTAG_PHOTOMETRIC           | Facultatif (fortement recommandé) | Indique 0 = noir, max = blanc. Évite les erreurs d'interprétation.
TIFFTAG_PLANARCONFIG          | Facultatif si 1 canal             | Obligatoire si >1 canal (ex : RGB). CONTIG = données regroupées.
TIFFTAG_ORIENTATION           | Facultatif                        | Indique le coin (0,0). TOPLEFT est la valeur par défaut.
TIFFTAG_FILLORDER             | Facultatif (inutile ici)          | Ordre des bits dans un octet. Inutile pour float64 (aligné).

