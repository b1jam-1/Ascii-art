# 🖼️ ASCII Art Generator (C++)

Transformez vos images BMP en œuvres d'art ASCII directement dans votre terminal.

<img width="4244" height="1236" alt="illustration" src="https://github.com/user-attachments/assets/eee8080a-bd8e-4c1a-af23-7ea8c9db4137" />

---

## Sommaire
- [Présentation](#présentation)
- [Fonctionnalités](#fonctionnalités)
- [Installation & Compilation](#installation--compilation)
- [Utilisation](#utilisation)
- [Conversion d'images](#conversion-dimages)
- [Compatibilité Windows](#compatibilité-windows)
- [Structure du projet](#structure-du-projet)

---

## Présentation

**ASCII Art** est un utilitaire C++ portable qui convertit des images BMP non compressées (24/32 bits) en texte ASCII, avec prise en charge de la couleur ANSI 24 bits. Il ne dépend d'aucune bibliothèque externe pour la lecture d'image.

---

## Fonctionnalités

- Lecture native des fichiers BMP non compressés (24-bit BGR, 32-bit BGRA)
- Sortie en niveaux de gris **ou** en couleur ANSI 24-bit
- Redimensionnement flexible (largeur, hauteur, ratio caractère)
- Jeu de caractères personnalisable
- **Aucune dépendance externe** pour la lecture BMP


---

## Installation & Compilation

### Prérequis
- Compilateur C++ (g++ recommandé)
- GNU Make (optionnel, pour la compilation simplifiée)

### Compilation rapide (Unix, WSL, MSYS2/MinGW)

```sh
make        # Compile l'exécutable ascii_art(.exe)
make clean  # Nettoie les fichiers générés
```

### Compilation manuelle (Windows PowerShell ou bash)


```powershell
g++ -o ascii_art.exe main.cpp bmp_reader.cpp
```


---

## Utilisation

### Syntaxe générale

```sh
./ascii_art <image.bmp> [options]
```

Sous Windows :

```powershell
.\ascii_art.exe <image.bmp> [options]
```

### Options principales

- `-w <width>`    : largeur cible en caractères (défaut : 100)
- `-h <height>`   : hauteur cible en caractères
- `-r <ratio>`    : ratio hauteur/largeur d'un caractère (défaut : 0.5)
- `-c`            : active la sortie couleur (ANSI 24-bit)
- `-i`            : inverse la correspondance de luminosité
- `-s "chars"`   : jeu de caractères personnalisé (du plus clair au plus foncé)

### Exemples

```powershell
.\ascii_art.exe image.bmp -w 100 -c -s "@#S%?*+;:, ."
```

```powershell
.\ascii_art.exe image.bmp -w 80 > output.txt
```

---

## Conversion d'images

Si votre image est au format PNG/JPG/etc., convertissez-la en BMP avec [ImageMagick](https://imagemagick.org) :

```sh
magick input.png -resize 800x800\> -background white -flatten -type TrueColor output.bmp
```

Ou, sous Windows, ouvrez l'image dans Paint puis : **Fichier → Enregistrer sous → Bitmap (*.bmp)**

---

## Compatibilité Windows

- L'affichage couleur (`-c`) nécessite un terminal compatible ANSI (PowerShell 7+, Windows Terminal, etc.).
- Si la couleur ne s'affiche pas, essayez un autre terminal ou activez le support ANSI.

---

## Structure du projet

- `main.cpp`         : point d'entrée de l'application
- `bmp_reader.cpp`   : lecteur BMP natif
- `bmp_reader.h`     : déclarations du lecteur BMP
- `Makefile`         : compilation simplifiée
- `README.md`        : ce fichier

---
