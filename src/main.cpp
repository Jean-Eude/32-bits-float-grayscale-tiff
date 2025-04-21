#include "TiFF.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

void dilateBinaryMask(const TiFF& inputMask, TiFF& outputMask, int radius = 1) {
    uint32_t width = inputMask.getWidth();
    uint32_t height = inputMask.getHeight();
    outputMask.create(width, height, 0.0);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            bool found = false;
            for (int dy = -radius; dy <= radius && !found; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                        if (inputMask.getPixel(nx, ny) == 1.0) {
                            found = true;
                            break;
                        }
                    }
                }
            }
            if (found)
                outputMask.setPixel(x, y, 1.0);
        }
    }
}


void erodeBinaryMask(const TiFF& inputMask, TiFF& outputMask, int radius = 1) {
    uint32_t width = inputMask.getWidth();
    uint32_t height = inputMask.getHeight();
    outputMask.create(width, height, 0.0);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            bool all_ones = true;
            for (int dy = -radius; dy <= radius && all_ones; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= width || ny >= height || inputMask.getPixel(nx, ny) < 1.0) {
                        all_ones = false;
                        break;
                    }
                }
            }
            if (all_ones)
                outputMask.setPixel(x, y, 1.0);
        }
    }
}

void invertMask(const TiFF& input, TiFF& output) {
    uint32_t width = input.getWidth();
    uint32_t height = input.getHeight();
    output.create(width, height, 0.0);
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
            output.setPixel(x, y, 1.0 - input.getPixel(x, y));
}

void detectWhitePoints(const TiFF& input, TiFF& outputMask, double seuil = 0.98) {
    uint32_t width = input.getWidth();
    uint32_t height = input.getHeight();
    outputMask.create(width, height, 0.0);

    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
            if (input.getPixel(x, y) >= seuil)
                outputMask.setPixel(x, y, 1.0);
}

void detectZScoreSpikes(const TiFF& input, TiFF& outputMask, double k, uint32_t window = 5) {
    uint32_t width = input.getWidth();
    uint32_t height = input.getHeight();
    outputMask.create(width, height, 0.0);

    const int offset = window / 2;
    const int windowArea = window * window - 1;

    for (uint32_t y = offset; y < height - offset; ++y) {
        for (uint32_t x = offset; x < width - offset; ++x) {
            double center = input.getPixel(x, y);
            double sum = 0.0, sumSq = 0.0;
            for (int dy = -offset; dy <= offset; ++dy) {
                for (int dx = -offset; dx <= offset; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    double val = input.getPixel(x + dx, y + dy);
                    sum += val;
                    sumSq += val * val;
                }
            }
            double mean = sum / windowArea;
            double stddev = std::sqrt(std::max((sumSq / windowArea) - (mean * mean), 1e-8));
            if (center > mean + k * stddev)
                outputMask.setPixel(x, y, 1.0);
        }
    }
}

double estimateK(const TiFF& img) {
    std::vector<double> pixels;
    for (uint32_t y = 0; y < img.getHeight(); ++y)
        for (uint32_t x = 0; x < img.getWidth(); ++x)
            pixels.push_back(img.getPixel(x, y));

    std::sort(pixels.begin(), pixels.end());

    size_t cutoff = pixels.size() * 0.9; // Évite les saturations
    double sum = 0.0, sumSq = 0.0;
    for (size_t i = 0; i < cutoff; ++i) {
        sum += pixels[i];
        sumSq += pixels[i] * pixels[i];
    }

    double mean = sum / cutoff;
    double stddev = std::sqrt((sumSq / cutoff) - (mean * mean));

    // Forcé à être agressif (max = 1.3)
    return std::clamp(1.0 + 0.2 * stddev, 0.7, 1.25);
}

void corrigerImageRobuste(const TiFF& inputImage, const TiFF& masque, TiFF& imageCorrigee, uint32_t window = 5, TiFF* pixelsModifies = nullptr) {
    uint32_t width = inputImage.getWidth();
    uint32_t height = inputImage.getHeight();
    imageCorrigee.create(width, height, 0.0);
    if (pixelsModifies) pixelsModifies->create(width, height, 0.0);

    int offset = window / 2;
    double sigma = window / 2.0; // σ = moitié de la fenêtre
    double sigma2 = 2.0 * sigma * sigma;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            if (masque.getPixel(x, y) == 1.0) {
                double weightedSum = 0.0;
                double weightTotal = 0.0;

                for (int dy = -offset; dy <= offset; ++dy) {
                    for (int dx = -offset; dx <= offset; ++dx) {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && ny >= 0 && nx < width && ny < height && !(dx == 0 && dy == 0)) {
                            if (masque.getPixel(nx, ny) < 1.0) {
                                // Gaussienne
                                double distance2 = dx * dx + dy * dy;
                                double weight = std::exp(-distance2 / sigma2);
                                weightedSum += weight * inputImage.getPixel(nx, ny);
                                weightTotal += weight;
                            }
                        }
                    }
                }

                double replacement;
                if (weightTotal > 0.0) {
                    replacement = weightedSum / weightTotal;
                } else {
                    // Fallback : moyenne brute des voisins
                    double sum = 0.0;
                    int count = 0;
                    for (int dy = -offset; dy <= offset; ++dy)
                        for (int dx = -offset; dx <= offset; ++dx) {
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && ny >= 0 && nx < width && ny < height && !(dx == 0 && dy == 0)) {
                                sum += inputImage.getPixel(nx, ny);
                                ++count;
                            }
                        }
                    replacement = count > 0 ? sum / count : inputImage.getPixel(x, y);
                }

                imageCorrigee.setPixel(x, y, replacement);
                if (pixelsModifies)
                    pixelsModifies->setPixel(x, y, 1.0);
            } else {
                imageCorrigee.setPixel(x, y, inputImage.getPixel(x, y));
            }
        }
    }
}


// ... Inclure les headers comme avant
#include <cmath>
#include <algorithm>

void computeSobelMagnitude(const TiFF& input, TiFF& output) {
    uint32_t width = input.getWidth();
    uint32_t height = input.getHeight();
    output.create(width, height, 0.0);

    for (uint32_t y = 1; y < height - 1; ++y) {
        for (uint32_t x = 1; x < width - 1; ++x) {
            double gx =
                -1 * input.getPixel(x - 1, y - 1) + 1 * input.getPixel(x + 1, y - 1) +
                -2 * input.getPixel(x - 1, y    ) + 2 * input.getPixel(x + 1, y    ) +
                -1 * input.getPixel(x - 1, y + 1) + 1 * input.getPixel(x + 1, y + 1);

            double gy =
                -1 * input.getPixel(x - 1, y - 1) + -2 * input.getPixel(x, y - 1) + -1 * input.getPixel(x + 1, y - 1) +
                 1 * input.getPixel(x - 1, y + 1) +  2 * input.getPixel(x, y + 1) +  1 * input.getPixel(x + 1, y + 1);

            double magnitude = std::sqrt(gx * gx + gy * gy);

            // Inversion binaire directe
            output.setPixel(x, y, magnitude > 0.0 ? 0.0 : 1.0);
        }
    }
}

void thresholdMask(const TiFF& input, TiFF& output) {
    uint32_t width = input.getWidth();
    uint32_t height = input.getHeight();
    output.create(width, height, 0.0);
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
            if (input.getPixel(x, y) >= 1.)
                output.setPixel(x, y, 1.0);
}

void openBinaryMask(const TiFF& inputMask, TiFF& outputMask, int radius = 1) {
    TiFF eroded, e2, e3, e4;
    erodeBinaryMask(inputMask, eroded, radius);
    dilateBinaryMask(eroded, outputMask, radius);
}

void closeBinaryMask(const TiFF& inputMask, TiFF& outputMask, int radius = 1) {
    TiFF eroded, e2, e3, e4;
    dilateBinaryMask(eroded, outputMask, radius);
    erodeBinaryMask(inputMask, eroded, radius);
}

#include <numeric>

double computePSNR(const TiFF& ref, const TiFF& test) {
    double mse = 0.0;
    uint32_t width = ref.getWidth();
    uint32_t height = ref.getHeight();
    uint64_t N = width * height;

    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x) {
            double diff = ref.getPixel(x, y) - test.getPixel(x, y);
            mse += diff * diff;
        }

    mse /= N;
    if (mse == 0) return INFINITY;
    return 10.0 * std::log10(1.0 / mse);
}

double computeSSIM(const TiFF& img1, const TiFF& img2) {
    uint32_t width = img1.getWidth();
    uint32_t height = img1.getHeight();
    uint64_t N = width * height;

    double mu1 = 0, mu2 = 0;
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x) {
            mu1 += img1.getPixel(x, y);
            mu2 += img2.getPixel(x, y);
        }

    mu1 /= N;
    mu2 /= N;

    double sigma1_sq = 0, sigma2_sq = 0, sigma12 = 0;
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x) {
            double v1 = img1.getPixel(x, y) - mu1;
            double v2 = img2.getPixel(x, y) - mu2;
            sigma1_sq += v1 * v1;
            sigma2_sq += v2 * v2;
            sigma12 += v1 * v2;
        }

    sigma1_sq /= N;
    sigma2_sq /= N;
    sigma12 /= N;

    const double C1 = 0.01 * 0.01;
    const double C2 = 0.03 * 0.03;

    return ((2 * mu1 * mu2 + C1) * (2 * sigma12 + C2)) /
           ((mu1 * mu1 + mu2 * mu2 + C1) * (sigma1_sq + sigma2_sq + C2));
}

int main() {
    TiFF image;
    if (!image.load("bestimage.tiff")) {
        std::cerr << "Erreur chargement image." << std::endl;
        return 1;
    }

    uint32_t width = image.getWidth();
    uint32_t height = image.getHeight();

    // 1. Normalisation (P5-P95)
    TiFF image_normalisee = image;
    image_normalisee.normalizePercentile(0.05, 0.95);
    image_normalisee.save("image_normalisee.tif");

    // 2. Détection Z-score sur image normalisée
    double k_norm = estimateK(image_normalisee);
    std::cout << k_norm << std::endl;
    uint32_t win_norm = 5;
    TiFF mask_norm;
    detectZScoreSpikes(image_normalisee, mask_norm, k_norm, win_norm);
    mask_norm.save("masque_zscore_normalisee.tif");

    // 3. Détection par gradient (lignes brillantes)
    TiFF grad, mask_grad;
    computeSobelMagnitude(image_normalisee, grad);
    grad.save("gradient_magnitude.tif");

    thresholdMask(grad, mask_grad);
    mask_grad.save("masque_gradient_brut.tif");

    // ✅ 4. Raffinage morphologique du masque gradient uniquement
    TiFF mask_grad_morpho;
    TiFF temp_open;
    //openBinaryMask(mask_grad, temp_open, 1);
    dilateBinaryMask(mask_grad, mask_grad_morpho, 3);

    mask_grad_morpho.save("masque_gradient_morpho.tif");
    

    // une passe sur l'image de base
    TiFF mask_spikes_raw;
    double k = estimateK(image);
    std::cout << k << std::endl;
    detectZScoreSpikes(image, mask_spikes_raw, k, win_norm);
    
    mask_spikes_raw.save("masque_spikes_brut.tif");

    // 5. Fusion des masques raffinés
    TiFF mask_fusion;
    mask_fusion.create(width, height, 0.0);
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x) {
            double m = std::max({mask_norm.getPixel(x, y), mask_spikes_raw.getPixel(x, y), mask_grad_morpho.getPixel(x, y)});
            mask_fusion.setPixel(x, y, m);
        }
    mask_fusion.save("masque_fusion_total.tif");

    // 6. Correction finale par interpolation robuste
    TiFF image_corrigee;
    TiFF carte_modifs;
    corrigerImageRobuste(image_normalisee, mask_fusion, image_corrigee, 11, &carte_modifs);
    image_corrigee.save("image_corrigee_finale.tif");
    carte_modifs.save("pixels_modifies_finale.tif");

    TiFF image_corrigee_norm = image_corrigee;
    TiFF image_ref_norm = image_normalisee;
    
    // Normalisation P5-P95 des deux images
    image_corrigee_norm.normalizePercentile(0.05, 0.95);
    image_ref_norm.normalizePercentile(0.05, 0.95);

    double psnr = computePSNR(image_ref_norm, image_corrigee_norm);
    double ssim = computeSSIM(image_ref_norm, image_corrigee_norm);
    
    std::cout << "PSNR : " << psnr << " dB" << std::endl;
    std::cout << "SSIM : " << ssim << std::endl;

    std::cout << "✅ Correction finale avec post-traitement morphologique du **masque gradient** terminée." << std::endl;
    return 0;
}