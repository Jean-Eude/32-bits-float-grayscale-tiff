#include <tiffio.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include <tiffio.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

class TiffFloat32Image {
    public:
        TiffFloat32Image() : width(0), height(0) {}

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

            if (bitsPerSample != 32 || sampleFormat != SAMPLEFORMAT_IEEEFP || samplesPerPixel != 1) {
                std::cerr << "Erreur : image non conforme (float32 grayscale attendu)." << std::endl;
                TIFFClose(tif);
                return false;
            }

            data.resize(width * height);

            for (uint32_t row = 0; row < height; ++row) {
                float* buf = (float*)_TIFFmalloc(width * sizeof(float));
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
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
            TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
            TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

            for (uint32_t row = 0; row < height; ++row) {
                const float* buf = &data[row * width];
                if (TIFFWriteScanline(tif, (void*)buf, row, 0) < 0) {
                    std::cerr << "Erreur lors de l'écriture de la ligne " << row << std::endl;
                    TIFFClose(tif);
                    return false;
                }
            }

            TIFFClose(tif);
            return true;
        }

        bool create(uint32_t w, uint32_t h, float initial_value = 0.0f) {
            if (w == 0 || h == 0) return false;
            width = w;
            height = h;
            data.resize(width * height, initial_value);
            return true;
        }

        float getPixel(uint32_t x, uint32_t y) const {
            if (x >= width || y >= height) return 0.0f;
            return data[y * width + x];
        }

        void setPixel(uint32_t x, uint32_t y, float value) {
            if (x < width && y < height) {
                data[y * width + x] = value;
            }
        }

        void normalize() {
            if (data.empty()) return;
            float min_val = *std::min_element(data.begin(), data.end());
            float max_val = *std::max_element(data.begin(), data.end());
            if (max_val > min_val) {
                for (auto& px : data) {
                    px = (px - min_val) / (max_val - min_val);
                }
            }
        }

        void fill(float value) {
            std::fill(data.begin(), data.end(), value);
        }

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

    private:
        uint32_t width, height;
        std::vector<float> data;
};


int main() {
    TiffFloat32Image img;
    if (!img.load("image_32bit_grayscale.tiff")) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        return 1;
    }

    float threshold = 0.5f;  // Seuil de binarisation

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            float val = img.getPixel(x, y);
            float bin = (val >= threshold) ? 1.0f : 0.0f;
            img.setPixel(x, y, bin);
        }
    }

    if (!img.save("output_thresholded.tiff")) {
        std::cerr << "Erreur lors de l'enregistrement de l'image." << std::endl;
        return 1;
    }

    std::cout << "Seuillage terminé. Image sauvegardée dans output_thresholded.tiff" << std::endl;
    return 0;
}
