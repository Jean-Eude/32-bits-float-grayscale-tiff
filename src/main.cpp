#include "TiFF.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

// Soft thresholding function
double softThreshold(double value, double threshold) {
    if (std::abs(value) <= threshold) return 0.0;
    return (value > 0) ? value - threshold : value + threshold;
}

// Simple 1-level Haar DWT on 2D image
void haarDWT(std::vector<double>& data, uint32_t width, uint32_t height) {
    std::vector<double> temp(data.size());

    // Horizontal pass
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width / 2; ++x) {
            double a = data[y * width + 2 * x];
            double b = data[y * width + 2 * x + 1];
            temp[y * width + x] = (a + b) / std::sqrt(2.0);           // Approx
            temp[y * width + x + width / 2] = (a - b) / std::sqrt(2.0); // Detail
        }
    }

    // Vertical pass
    for (uint32_t x = 0; x < width; ++x) {
        for (uint32_t y = 0; y < height / 2; ++y) {
            double a = temp[(2 * y) * width + x];
            double b = temp[(2 * y + 1) * width + x];
            data[y * width + x] = (a + b) / std::sqrt(2.0);
            data[(y + height / 2) * width + x] = (a - b) / std::sqrt(2.0);
        }
    }
}

// Inverse DWT
void haarIDWT(std::vector<double>& data, uint32_t width, uint32_t height) {
    std::vector<double> temp(data.size());

    // Vertical
    for (uint32_t x = 0; x < width; ++x) {
        for (uint32_t y = 0; y < height / 2; ++y) {
            double a = data[y * width + x];
            double d = data[(y + height / 2) * width + x];
            temp[2 * y * width + x] = (a + d) / std::sqrt(2.0);
            temp[(2 * y + 1) * width + x] = (a - d) / std::sqrt(2.0);
        }
    }

    // Horizontal
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width / 2; ++x) {
            double a = temp[y * width + x];
            double d = temp[y * width + x + width / 2];
            data[y * width + 2 * x] = (a + d) / std::sqrt(2.0);
            data[y * width + 2 * x + 1] = (a - d) / std::sqrt(2.0);
        }
    }
}

// Denoise detail coefficients with soft threshold
void thresholdWavelet(std::vector<double>& data, uint32_t width, uint32_t height, double threshold) {
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            // Skip top-left (LL band, approx)
            if (x < width / 2 && y < height / 2) continue;
            data[y * width + x] = softThreshold(data[y * width + x], threshold);
        }
    }
}

// Simulate white noise
void addWhiteNoise(std::vector<double>& data, double stddev) {
    std::default_random_engine rng(42);
    std::normal_distribution<double> dist(0.0, stddev);
    for (auto& px : data) px += dist(rng);
}

int main() {
    TiFF image;
    const uint32_t W = 128, H = 128;

    // Step 1: Create clean image (radial gradient)
    image.create(W, H, 0.0);
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            double dx = x - W / 2;
            double dy = y - H / 2;
            double r = std::sqrt(dx * dx + dy * dy) / (W / 2);
            image.setPixel(x, y, std::exp(-r * r));
        }
    }

    // Step 2: Add white noise
    auto noisy = image; // Copy
addWhiteNoise(noisy.getData(), 0.1);

    noisy.normalize();
    noisy.save("noisy_wavelet.tif");

    // Step 3: Wavelet Denoising
    auto denoised = noisy;
haarDWT(denoised.getData(), W, H);
thresholdWavelet(denoised.getData(), W, H, 0.05);
haarIDWT(denoised.getData(), W, H);
    denoised.normalize();
    denoised.save("denoised_wavelet.tif");

    std::cout << "Done! Saved noisy and denoised images." << std::endl;
    return 0;
}

