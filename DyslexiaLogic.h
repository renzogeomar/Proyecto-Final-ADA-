#ifndef DYSLEXIALOGIC_H
#define DYSLEXIALOGIC_H

#include <QString> // Usamos QString para soportar tildes correctamente
#include <vector>

struct TextStyle {
    int start;
    int length;
    bool isBackground;
    unsigned int colorHex;
};

class DyslexiaLogic {
public:
    struct PatternConfig {
        QString pattern; // Ahora el patrón es QString
        unsigned int color;
        int priority;
    };

    // Recibe QString y devuelve posiciones exactas
    static std::vector<TextStyle> analyzeText(const QString &text, int mode);

private:
    static std::vector<int> buildLPS(const QString &pattern);
    static std::vector<int> KMPsearch(const QString &text, const QString &pattern);
};

#endif // DYSLEXIALOGIC_H
