#include <TiFF.hpp>

void generateRandomImage(TiFF& img, uint32_t w, uint32_t h) {
    img.create(w, h, 0.f);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            float val = static_cast<float>(rand()) / RAND_MAX; // entre 0 et 1
            val = val * 2.0f - 1.0f; // entre -1 et 1
            img.setPixel(x, y, val);
        }
    }
}

TiFF divideImages(const TiFF& numerator, const TiFF& denominator) {
    uint32_t w = numerator.getWidth();
    uint32_t h = numerator.getHeight();

    if (w != denominator.getWidth() || h != denominator.getHeight()) {
        throw std::runtime_error("Dimensions des images incompatibles pour la division.");
    }

    TiFF result;
    result.create(w, h, 0.f);

    for (uint32_t y = 0; y < result.getHeight(); ++y) {
        for (uint32_t x = 0; x < result.getWidth(); ++x) {
            float a = numerator.getPixel(x, y);
            float b = denominator.getPixel(x, y);
            float div = a / b;
            float r = result.getPixel(x, y);
    
            if (std::abs(div - r) > 1e-6f) {
                std::cout << "Différence ! (" << x << ", " << y << "): a=" << a << " b=" << b
                          << " => div=" << div << " mais résultat enregistré=" << r << std::endl;
            }
        }
    }

    return result;
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    TiFF img1, img2;
    generateRandomImage(img1, 256, 256);
    generateRandomImage(img2, 256, 256);

    TiFF result = divideImages(img1, img2);

    result.normalize(); // optionnel, pour mieux visualiser l’image finale
    result.save("result_div.tiff");

    return 0;
}