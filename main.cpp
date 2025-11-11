#include "bmp_reader.h"

#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <cstdint>

const int DEFAULT_WIDTH = 100; // largeur par défaut (en nombre de caractères)
const char* DEFAULT_CHARSET = ".:-=+*#%@"; // caractères utilisés pour la sortie, du plus clair au plus foncé
const double DEFAULT_RATIO = 0.5; // ratio largeur/hauteur d'un caractère
const std::string RESET_COLOR = "\x1b[0m"; // séquence d'échappement ANSI pour réinitialiser la couleur


std::string set_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // Retourne la séquence d'échappement ANSI pour définir la couleur spécifiée en RVB 24 bits
    std::stringstream s;
    s << "\x1b[38;2;" << (int)r << ";" << (int)g << ";" << (int)b << "m";
    return s.str();
}

int main(int argc, char* argv[]) {

    // Affichage de l'aide si pas assez d'arguments
    if (argc < 2) {
        std::cout << "Usage: ascii_art <image.bmp> [-w width] [-h height] [-c] [-i] [-s chars]\n";
        std::cout << "  Only uncompressed 24-bit or 32-bit BMP supported (portable, no external libs).\n";
        std::cout << "  -w width    target output width (characters) (default: " << DEFAULT_WIDTH << ")\n";
        std::cout << "  -h height   target output height (characters)\n";
        std::cout << "  -r ratio    height/width ratio of a character (default: 0.5)\n";
        std::cout << "  -c          enable color output (ANSI 24-bit)\n";
        std::cout << "  -i          invert brightness mapping\n";
        std::cout << "  -s chars    custom charset from lighter to darker (wrap in quotes)\n";
        std::cout << "Example: ascii_art image.bmp -w 120 -c\n";
        return 1;
    }

    // Paramètres par défaut
    int out_w = -1;
    int out_h = -1;
    double ratio = DEFAULT_RATIO;
    bool color = false;
    bool invert = false;
    std::string charset = DEFAULT_CHARSET;


    // Parse des arguments
    std::string filename = argv[1];
    for (int i = 2; i < argc; i++) {

        std::string a = argv[i];

        if (a == "-w" && i+1 < argc) {
            std::string val = argv[++i];
            int v = std::stoi(val);
            if (v > 0) out_w = v;
            else std::cerr << "Warning: width must be a positive integer. Using automatic calculation if height specified else default (" << DEFAULT_WIDTH << ").\n";

        }
        else if (a == "-h" && i+1 < argc) {
            std::string val = argv[++i];
            int v = std::stoi(val);
            if (v > 0) out_h = v;
            else std::cerr << "Warning: height must be a positive integer. Using automatic calculation.\n";
        }
        else if (a == "-r" && i+1 < argc) {
            std::string val = argv[++i];
            double v = std::stod(val);
            if (v > 0.0) ratio = v;
            else std::cerr << "Warning: ratio must be positive. Using default (" << DEFAULT_RATIO << ").\n";
        }
        else if (a == "-c") color = true;
        else if (a == "-i") invert = true;
        else if (a == "-s" && i+1 < argc) charset = argv[++i];
        else std::cerr << "Option inconnue : " << a << "\n";
    }


    // Chargement de l'image
    #ifdef _DEBUG
    std::cout << "Loading image: " << filename << "\n";
    #endif

    Image img;
    std::string loadErr;

    if (!load_bmp(filename, img, loadErr)) {
        std::cerr << "Failed to load BMP image: " << filename << "\n";
        std::cerr << "Reason: " << loadErr << "\n";
        std::cerr << "Note: this tool supports only uncompressed 24-bit or 32-bit BMP files.\n";
        return 2;
    }

    #ifdef _DEBUG
    std::cout << "Image loaded: " << img.width << "x" << img.height << "\n";
    #endif

    // Calcul de la taille de sortie
    if (out_h == -1) { //Si out_h n'a pas été spécifié, on le calcule automatiquement
        if (out_w == -1) { // si out_w n'a pas été spécifié non plus, on utilise la valeur par défaut
            out_w = DEFAULT_WIDTH;
        }
        out_h = (int)std::round(img.height * (out_w / (double)img.width) * ratio);
    }
    else if (out_w == -1) { //si la hauteur a été spécifiée mais pas la largeur, on la calcule automatiquement
        out_w = (int)std::round(img.width * (out_h / (double)img.height) / ratio);
    }

    #ifdef _DEBUG
    std::cout << "Output size: " << out_w << "x" << out_h << "\n";
    #endif

    // Vérification du charset
    int map_len = (int)charset.size();

    if (map_len == 0) {
        std::cerr << "Charset empty\n";
        return 3;
    }

    //Affichage
    for (int y = 0; y < out_h; y++) {

        std::ostringstream line; //construction de la ligne avant affichage pour éviter de faire un appel par caractère à std::cout

        for (int x = 0; x < out_w; x++) {

            // Calcul de la zone de pixels sur l'image source correspondant à cette position
            int sx0 = (int)std::floor(x * (double)img.width / out_w);
            int sx1 = (int)std::floor((x + 1.0) * (double)img.width / out_w);
            int sy0 = (int)std::floor(y * (double)img.height / out_h);
            int sy1 = (int)std::floor((y + 1.0) * (double)img.height / out_h);

            // On force la zone à couvrir au moins un pixel si ce n'est pas le cas
            // Evite les problèmes en cas d'upscaling
            if (sx1 <= sx0) {
                if (sx0 < img.width) sx1 = sx0 + 1;
                else { //Si sx0 est hors image, on le ramène au dernier pixel valide
                    sx0 = img.width - 1;
                    sx1 = img.width;
                }
            }

            if (sy1 <= sy0) {
                if (sy0 < img.height) sy1 = sy0 + 1;
                else { //Si sy0 est hors image, on le ramène au dernier pixel valide
                    sy0 = img.height - 1;
                    sy1 = img.height;
                }
            }

            // Calcul de la couleur moyenne dans cette zone
            uint32_t r_acc = 0, g_acc = 0, b_acc = 0; // uint32_t pour éviter un overflow
            int count = 0;

            for (int yy = sy0; yy < sy1; yy++) {
                for (int xx = sx0; xx < sx1; xx++) {
                    size_t off = calc_offset((size_t)xx, (size_t)yy, (size_t)img.width);
                    r_acc += img.data[off + 0];
                    g_acc += img.data[off + 1];
                    b_acc += img.data[off + 2];
                    count++;
                }
            }

            uint8_t    r = r_acc / (uint32_t)count;
            uint8_t    g = g_acc / (uint32_t)count;
            uint8_t    b = b_acc / (uint32_t)count;
            

            // Calcul de la luminance perçue avec la norme Rec 709 (pondération)
            double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;
            lum /= 255.0; //normalisation [0,1]
            if (invert) lum = 1.0 - lum;

            // Sélection du caractère correspondant à cette luminance dans la table de caractères
            int idx = (int)std::floor(lum * (map_len - 1));
            char ch = charset[idx];

            //Affichage du caractère
            if (color) {
                line << set_color_rgb(r, g, b) << ch << RESET_COLOR;
            } else {
                line << ch;
            }
        }

        std::cout << line.str() << '\n'; //on affiche la ligne complète
    }

    return 0;
}
