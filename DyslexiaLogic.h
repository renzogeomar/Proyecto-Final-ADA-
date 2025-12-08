#ifndef DYSLEXIALOGIC_H
#define DYSLEXIALOGIC_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>

// Estructura para comunicar el estilo a la GUI
struct TextStyle {
    int start;
    int length;
    bool isBackground; // true = pintar fondo, false = pintar texto
    unsigned int colorHex; // Ej: 0xFF0000 para rojo
};

class DyslexiaLogic {
public:
    // Configuración de patrones
    struct PatternConfig {
        std::string pattern;
        unsigned int color;
        int priority;
    };

    // Métodos principales
    static std::vector<TextStyle> analyzeText(const std::string &text, int mode);

private:
    // El algoritmo sagrado: KMP
    static std::vector<int> buildLPS(const std::string &pattern);
    static std::vector<int> KMPsearch(const std::string &text, const std::string &pattern);
    
    // Heurísticas auxiliares
    static std::vector<TextStyle> detectHeatmap(const std::string &text, const std::set<char> &triggers);
    static std::vector<TextStyle> detectSyllables(const std::string &text);
};

#endif // DYSLEXIALOGIC_H