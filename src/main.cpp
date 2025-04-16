#include <stdio.h>
#include <stdint.h>

uint32_t swap_endian(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}

int main() {
    FILE *file = fopen("mon_fichier.bin", "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return 1;
    }

    uint32_t magic;
    if (fread(&magic, sizeof(uint32_t), 1, file) != 1) {
        perror("Erreur lors de la lecture du magic number");
        fclose(file);
        return 1;
    }

    // Pour le debug : affichage brut
    printf("Magic brut : 0x%08X\n", magic);

    if (magic == 123) {
        printf("Fichier en little endian\n");
    } else if (swap_endian(magic) == 123) {
        printf("Fichier en big endian\n");
        magic = swap_endian(magic);  // On convertit pour l'utiliser ensuite
    } else {
        printf("Magic number inattendu : 0x%08X\n", magic);
        fclose(file);
        return 1;
    }

    printf("Magic validé : %u\n", magic);

    fclose(file);
    return 0;
}

#include <iostream>
#include <fstream>
#include <cstdint>

// Fonction pour swapper un uint32_t (big <-> little)
uint32_t swapEndian(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}

int main() {
    std::ifstream file("mon_fichier.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier." << std::endl;
        return 1;
    }

    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (!file) {
        std::cerr << "Erreur lors de la lecture du magic number." << std::endl;
        return 1;
    }

    std::cout << "Magic lu (brut) : 0x" << std::hex << magic << std::dec << std::endl;

    // Vérifie et convertit si besoin
    if (magic == 123) {
        std::cout << "Fichier en little endian." << std::endl;
    } else if (swapEndian(magic) == 123) {
        std::cout << "Fichier en big endian. Conversion vers little endian..." << std::endl;
        magic = swapEndian(magic);
    } else {
        std::cerr << "Magic number inattendu : " << magic << std::endl;
        return 1;
    }

    std::cout << "Magic confirmé : " << magic << std::endl;

    // Tu peux continuer à lire d'autres champs ici...
    
    file.close();
    return 0;
}
