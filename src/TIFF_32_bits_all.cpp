#include <tiffio.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstring>

class TiffFloat32Image {
public:
    TiffFloat32Image() : width(0), height(0) {}

    bool load(const std::string& filename) {
        TIFF* tif = TIFFOpen(filename.c_str(), "r");
        if (!tif) {
            std::cerr << "Erreur : impossible d'ouvrir " << filename << std::endl;
            return false;
        }

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        uint16_t bitsPerSample = 0, sampleFormat = SAMPLEFORMAT_UINT, samplesPerPixel = 1;
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

        if (samplesPerPixel != 1) {
            std::cerr << "Erreur : image couleur non supportée (samplesPerPixel = " << samplesPerPixel << ")" << std::endl;
            TIFFClose(tif);
            return false;
        }

        data.resize(width * height);

        for (uint32_t row = 0; row < height; ++row) {
            // Alloue un buffer temporaire brut selon la profondeur
            void* buf = _TIFFmalloc(TIFFScanlineSize(tif));
            if (!buf || TIFFReadScanline(tif, buf, row) < 0) {
                std::cerr << "Erreur lecture ligne " << row << std::endl;
                _TIFFfree(buf);
                TIFFClose(tif);
                return false;
            }

            for (uint32_t col = 0; col < width; ++col) {
                float val = 0.0f;
                switch (bitsPerSample) {
                    case 8:
                        val = static_cast<float>(reinterpret_cast<uint8_t*>(buf)[col]) / 255.0f;
                        break;
                    case 16:
                        val = static_cast<float>(reinterpret_cast<uint16_t*>(buf)[col]) / 65535.0f;
                        break;
                    case 32:
                        if (sampleFormat == SAMPLEFORMAT_IEEEFP) {
                            val = reinterpret_cast<float*>(buf)[col];
                        } else {
                            val = static_cast<float>(reinterpret_cast<uint32_t*>(buf)[col]) / static_cast<float>(UINT32_MAX);
                        }
                        break;
                    default:
                        std::cerr << "Format non supporté : bitsPerSample=" << bitsPerSample << std::endl;
                        _TIFFfree(buf);
                        TIFFClose(tif);
                        return false;
                }
                data[row * width + col] = val;
            }

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
                std::cerr << "Erreur écriture ligne " << row << std::endl;
                TIFFClose(tif);
                return false;
            }
        }

        TIFFClose(tif);
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

    bool create(uint32_t w, uint32_t h, float initial_value = 0.0f) {
        if (w == 0 || h == 0) return false;
        width = w;
        height = h;
        data.resize(width * height, initial_value);
        return true;
    }

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

private:
    uint32_t width = 0, height = 0;
    std::vector<float> data;
};

int main() {
    TiffFloat32Image img;

    if (!img.load("image_various_types.tiff")) {
        return 1;
    }

    img.normalize();  // optionnel

    img.setPixel(0, 0, 1.0f);  // exemple d'édition

    if (!img.save("converted_float32.tiff")) {
        return 1;
    }

    std::cout << "Image convertie en float32 avec succès !" << std::endl;
    return 0;
}
