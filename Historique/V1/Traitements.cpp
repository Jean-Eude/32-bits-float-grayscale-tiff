#include <Traitements.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <cmath>

Traitements::Traitements() { }

void Traitements::OnReadConfigFile(const char* cfg) {
    readConfigFile(cfg);

    for (const auto& a : m_cVars) {
        std::visit([&](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                if (a.first == "dir_imgs_entree") this->folderEntrees = arg;
                if (a.first == "img_noir") this->imgNoirPath = arg;
                if (a.first == "img_trame") this->imgTramePath = arg;
                if (a.first == "img_homog") this->imgHomogPath = arg;
            }
        }, a.second);
    }
}

bool Traitements::Conversion(const std::string& fichier, TiFF& img) {
    std::string ext = Filesystem::getExtension(fichier);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    std::string inputPath = folderEntrees + "/" + fichier;

    if (ext == "tiff" || ext == "tif") {
        return img.load(inputPath);
    }

    else if (ext == "png" || ext == "jpg" || ext == "jpeg") {
        std::string tiffPath = inputPath + ".tiff";

        // 🧠 Appel d’un script shell de conversion externe
        std::string cmd = "./convert_to_tiff.sh \"" + inputPath + "\" \"" + tiffPath + "\"";
        std::cout << "Exécution : " << cmd << std::endl;

        int result = std::system(cmd.c_str());
        if (result != 0) {
            std::cerr << "Erreur : échec du script de conversion." << std::endl;
            return false;
        }

        // Charge l'image convertie
        bool ok = img.load(tiffPath);
        if (ok) {
            // Supprimer le fichier source (PNG/JPG)
            Filesystem::removeFile(inputPath);
        }

        return ok;
    }

    return false;
}

void Traitements::Process() {
    OnReadConfigFile("config.p");
    fichiersEntree = Filesystem::listFiles(folderEntrees);

    int index = 0;
    for (const auto& fichier : fichiersEntree) {
        std::cout << fichier << std::endl;
        TiFF img;
        if (Conversion(fichier, img)) {
            CorrectionPOintsBlancs(img);
            CorrectionNoirs(img);
            CorrectionTrameCCD(img);
            CorrectionHomog(img);
            Reduction(img, index++);
        }
    }
}

void Traitements::CorrectionPOintsBlancs(TiFF& img) {
    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            float val = img.getPixel(x, y);
            float noise = static_cast<float>(rand()) / RAND_MAX * 0.05f;
            img.setPixel(x, y, std::clamp(val + noise, 0.0f, 1.0f));
        }
    }
}

void Traitements::CorrectionNoirs(TiFF& img) {
    static TiFF ref;
    static bool loaded = false;

    if (!loaded) loaded = ref.load(imgNoirPath);
    if (!loaded) return;

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            float val = img.getPixel(x, y) - ref.getPixel(x, y);
            img.setPixel(x, y, std::max(val, 0.0f));
        }
    }
}

void Traitements::CorrectionTrameCCD(TiFF& img) {
    static TiFF ref;
    static bool loaded = false;

    if (!loaded) loaded = ref.load(imgTramePath);
    if (!loaded) return;

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            float refVal = ref.getPixel(x, y);
            if (refVal > 0.0f) {
                img.setPixel(x, y, img.getPixel(x, y) / refVal);
            }
        }
    }
}

void Traitements::CorrectionHomog(TiFF& img) {
    static TiFF ref;
    static bool loaded = false;

    if (!loaded) loaded = ref.load(imgHomogPath);
    if (!loaded) return;

    for (uint32_t y = 0; y < img.getHeight(); ++y) {
        for (uint32_t x = 0; x < img.getWidth(); ++x) {
            float refVal = ref.getPixel(x, y);
            if (refVal > 0.0f) {
                img.setPixel(x, y, img.getPixel(x, y) / refVal);
            }
        }
    }
}

void Traitements::Reduction(TiFF& img, int index) {
    uint32_t w = img.getWidth();
    uint32_t h = img.getHeight();
    /*
    if (w < 2 || h < 2) return; // rien à faire

    TiFF reduced;
    reduced.create(w / 2, h / 2);

    for (uint32_t y = 0; y < h - 1; y += 2) {
        for (uint32_t x = 0; x < w - 1; x += 2) {
            float sum =
                img.getPixel(x, y) +
                img.getPixel(x + 1, y) +
                img.getPixel(x, y + 1) +
                img.getPixel(x + 1, y + 1);
            reduced.setPixel(x / 2, y / 2, sum / 4.0f);
        }
    }

    std::string output = "res_" + std::to_string(index) + ".tiff";
    reduced.save(output);
    std::cout << "Image réduite sauvegardée : " << output << std::endl;*/
}
