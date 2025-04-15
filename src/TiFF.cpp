#include "TiFF.hpp"
#include <tiffio.h>
#include <iostream>
#include <algorithm>

TiFF::TiFF() : width(0), height(0) {}

bool TiFF::load(const std::string& filename) {
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

bool TiFF::save(const std::string& filename) const {
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
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);

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

bool TiFF::create(uint32_t w, uint32_t h, double initial_value) {
    if (w == 0 || h == 0) return false;
    width = w;
    height = h;
    data.resize(width * height, initial_value);
    return true;
}

double TiFF::getPixel(uint32_t x, uint32_t y) const {
    if (x >= width || y >= height) return 0.0;
    return data[y * width + x];
}

void TiFF::setPixel(uint32_t x, uint32_t y, double value) {
    if (x < width && y < height) {
        data[y * width + x] = value;
    }
}

void TiFF::normalize() {
    if (data.empty()) return;
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    if (max_val > min_val) {
        for (auto& px : data) {
            px = (px - min_val) / (max_val - min_val);
        }
    }
}

void TiFF::fill(double value) {
    std::fill(data.begin(), data.end(), value);
}

uint32_t TiFF::getWidth() const {
    return width;
}

uint32_t TiFF::getHeight() const {
    return height;
}

