#ifndef BMP_READER_H
#define BMP_READER_H

#include <vector>
#include <string>
#include <cstdint>

struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data;
};

// Fonction inline pour calculer l'offset dans le vecteur de données d'image à partir des coordonnées (x, y)
inline constexpr size_t calc_offset(size_t x, size_t y, size_t width) noexcept {
    return (y * width + x) * 3u;
}

// Permet de charger un fichier BMP non compressé 24 bits ou 32 bits dans out
// Retourne true en cas de succès, false en cas d'échec et écrit un message lisible dans err
bool load_bmp(const std::string &path, Image &out, std::string &err);

#endif // BMP_READER_H
