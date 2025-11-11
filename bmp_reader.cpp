#include "bmp_reader.h"

#include <fstream>
#include <cstring>
#include <vector>

bool load_bmp(const std::string &path, Image &out, std::string &err) {

    // Ouverture du fichier en mode binaire
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        err = "Le fichier n'a pas pu être ouvert";
        return false;
    }

    // Lecture des octets d'en-tête
    uint8_t fileHeader[14];
    f.read((char*)fileHeader, 14); // L'en-tête BMP fait 14 octets

    if (f.gcount() != 14) {
        err = "fichier trop petit pour l'en-tête BMP";
        return false;
    }
    if (fileHeader[0] != 'B' || fileHeader[1] != 'M'){ // Signature d'un fichier BMP est "BM"
        err = "mauvaise signature (pas 'BM')";
        return false;
    }

    // Les entiers dans les fichiers BMP sont stockés en little-endian (octet de poids faible en premier)
    // On créé des fonctions utilitaires pour lire des entiers little-endian
    // Retourne false en cas d'erreur de lecture
    auto read_u32 = [&](uint32_t &v)->bool {
        uint8_t b[4];
        f.read((char*)b, 4);
        if (f.gcount() != 4) return false;
        v = b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24); // on "inverse" l'ordre des octects
        return true;
    };
    auto read_u16 = [&](uint16_t &v)->bool {
        uint8_t b[2];
        f.read((char*)b, 2);
        if (f.gcount() != 2) return false;
        v = b[0] | (b[1]<<8); // on "inverse" l'ordre des octects
        return true;
    };

    // Lecture des informations du header BMP
    uint32_t fileSize = 0;
    memcpy(&fileSize, &fileHeader[2], 4);
    uint32_t pixelOffset = 0;
    memcpy(&pixelOffset, &fileHeader[10], 4);

    if (fileSize != 0 && pixelOffset > fileSize) {
        err = "pixel offset larger than file size";
        return false;
    }

    // Lecture de la taille du DIB header
    uint32_t dibSize = 0;
    if (!read_u32(dibSize)) {
        err = "failed to read DIB header size";
        return false;
    }
    if (dibSize < 40) {
        err = "unsupported DIB header (too small)";
        return false;
    }

    // On récupère les informations du header

    // Largeur
    uint32_t width = 0;
    if (!read_u32(width)) {
        err = "failed to read width";
        return false;
    }

    // Hauteur
    uint32_t tmpHeight = 0;
    if (!read_u32(tmpHeight)) {
        err = "failed to read height";
        return false;
    }
    int32_t signedHeight = (int32_t)tmpHeight; // On a besoin de la valeur signée
    // le signe de la hauteur indique si l'image est stockée de haut en bas (valeur négative) ou de bas en haut (valeur positive)
    uint32_t height = (signedHeight < 0) ? (uint32_t)(-signedHeight) : (uint32_t)signedHeight; // On prend la valeur absolue de la hauteur pour la suite
    
    // Plans (doit être 1, on ne l'utilise pas ici)
    uint16_t planes = 0;
    if (!read_u16(planes)) {
        err = "failed to read planes";
        return false;
    }

    // Bits par pixel (profondeur de couleur)
    uint16_t bpp = 0;
    if (!read_u16(bpp)) {
        err = "failed to read bpp";
        return false;
    }

    // Compression (doit être 0 pour BI_RGB, non compressé)
    uint32_t compression = 0;
    if (!read_u32(compression)) {
        err = "failed to read compression";
        return false;
    }
    uint32_t imageSize = 0;
    read_u32(imageSize);


    // Vérifications
    if (compression != 0) {
        err = "compression non prise en charge (seul BI_RGB non compressé est supporté)";
        return false;
    }
    if (bpp != 24 && bpp != 32) {
        err = "profondeur de couleur non prise en charge (seuls 24 ou 32 bits sont supportés)";
        return false;
    }

    // On déplace le curseur de lecture au début des données des pixels
    f.seekg(pixelOffset, std::ios::beg);
    if (!f) {
        err = "échec du déplacement vers les données des pixels";
        return false;
    }

    // Préparation de l'image de sortie
    out.width = (int)width;
    out.height = (int)height;
    if (out.width <= 0 || out.height <= 0) {
        err = "dimensions d'image invalides";
        return false;
    }
    // Allocation de l'espace pour le vecteur de stockage des pixels
    out.data.assign(out.width * out.height * 3, 0);
    
    // Calcul du nombre d'octets par ligne
    // Format BMP : chaque ligne est alignée sur un multiple de 4 octets
    size_t rowBytesNumber = ((bpp * out.width + 31) / 32) * 4;
    // (... +31)/32) pour arrondir au supérieur

    // Buffer pour la lecture d'une ligne de pixels
    std::vector<uint8_t> row(rowBytesNumber);

    bool topDown = (signedHeight < 0); // Si topDown alors l'image est stockée de haut en bas, sinon de bas en haut

    // Lecture des données des pixels ligne par ligne
    for (int rowIndex = 0; rowIndex < out.height; rowIndex++) {

        // Lecture de la ligne de pixels
        f.read((char*)row.data(), rowBytesNumber);
        if (!f) {
            err = "unexpected EOF while reading pixel data";
            return false;
        }

        // Calcul de l'index de la ligne dans l'image de sortie en "inversant" l'ordre si besoin
        int y = topDown ? rowIndex : (out.height - 1 - rowIndex);

        // Parcours des pixels de la ligne
        for (int x = 0; x < out.width; x++) {

            // Calcul de l'offset du pixel dans la ligne lue (en octets)
            size_t offset;
            if (bpp == 24) {
                offset = (size_t)x * 3u; // 3 octets par pixel (bpp = 24)
            }
            else {
                offset = (size_t)x * 4u; // 4 octets par pixel (bpp = 32)
            }

            // On vérifie qu'on peut lire 3 octects (R,G,B)
            if (offset + 2 >= rowBytesNumber) {
                err = "row data too small";
                return false;
            }

            // Lecture des composantes de couleurs
            // /!\ Dans les fichiers BMP, l'ordre des composantes est B G R
            uint8_t b = row[offset + 0];
            uint8_t g = row[offset + 1];
            uint8_t r = row[offset + 2];


            // Stockage dans l'image de sortie (en RGB)
            size_t dstOff = calc_offset((size_t)x, (size_t)y, (size_t)out.width); // Offset dans out.data
            out.data[dstOff + 0] = r;
            out.data[dstOff + 1] = g;
            out.data[dstOff + 2] = b;
        }
    }

    return true;
}
