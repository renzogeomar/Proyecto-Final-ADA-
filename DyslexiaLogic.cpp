#include "dyslexialogic.h"
#include <vector>
#include <QStringList>

// Estructura interna para el mapa de resolución de conflictos
struct StyleMapInfo {
    unsigned int color;
    int priority;
    bool active; // Si hay algo pintado aquí
};

// --- KMP Core (Intacto) ---
std::vector<int> DyslexiaLogic::buildLPS(const QString &pattern) {
    int m = pattern.length();
    std::vector<int> lps(m, 0);
    int len = 0;
    int i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++; lps[i] = len; i++;
        } else {
            if (len != 0) len = lps[len - 1];
            else { lps[i] = 0; i++; }
        }
    }
    return lps;
}

std::vector<int> DyslexiaLogic::KMPsearch(const QString &text, const QString &pattern) {
    std::vector<int> matches;
    QString lowerText = text.toLower(); // Normalización

    std::vector<int> lps = buildLPS(pattern);
    int n = text.length();
    int m = pattern.length();
    int i = 0, j = 0;

    while (i < n) {
        if (lowerText[i] == pattern[j]) {
            i++; j++;
        }
        if (j == m) {
            matches.push_back(i - j);
            j = lps[j - 1];
        } else if (i < n && lowerText[i] != pattern[j]) {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    return matches;
}

// --- Lógica con Resolución de Conflictos (Versión Completa) ---
std::vector<TextStyle> DyslexiaLogic::analyzeText(const QString &text, int mode) {
    std::vector<PatternConfig> configs;

    // COLORES
    unsigned int cRed = 0xD32F2F;    // Rojo
    unsigned int cBlue = 0x1976D2;   // Azul
    unsigned int cGreen = 0x388E3C;  // Verde
    unsigned int cPurple = 0x7B1FA2; // Morado
    unsigned int cSyllable = 0xE65100; // Naranja (Prioridad Alta)

    // --- CONFIGURACIÓN JERÁRQUICA ---
    if (mode == 0) { // Modo Espejo (b/d/p/q)
        configs.push_back({"b", cRed, 20});
        configs.push_back({"d", cBlue, 20});
        configs.push_back({"p", cGreen, 20});
        configs.push_back({"q", cPurple, 20});

        // Sílabas trabadas comunes
        QStringList trabadas = {
            "bra", "bre", "bri", "bro", "bru", "bla", "ble", "bli", "blo", "blu",
            "dra", "dre", "dri", "dro", "dru", "pla", "ple", "pli", "plo", "plu",
            "cla", "cle", "cli", "clo", "clu",
            "pra", "pre", "pri", "pro", "pru", "pla", "ple", "pli", "plo", "plu"
        };
        for(const QString &s : trabadas) configs.push_back({s, cSyllable, 50});
    }
    else if (mode == 1) { // Fonética (g/j)
        configs.push_back({"g", cRed, 20});
        configs.push_back({"j", cBlue, 20});
        configs.push_back({"ll", cGreen, 30});
        configs.push_back({"y", cPurple, 20});

        // Excepciones fonéticas
        configs.push_back({"gui", cSyllable, 50});
        configs.push_back({"gue", cSyllable, 50});
    }
    else if (mode == 2) { // Modo Formas (m/n/u/h) -> ¡AQUÍ ESTABA EL ERROR, FALTABA ESTO!
        configs.push_back({"m", cRed, 20});
        configs.push_back({"n", cBlue, 20});
        configs.push_back({"u", cGreen, 20});
        configs.push_back({"h", cPurple, 20});

        // Conflictos visuales de arcos juntos
        QStringList arcos = {"mn", "nm", "nn", "mm", "un", "nu"};
        for(const QString &s : arcos) configs.push_back({s, cSyllable, 50});
    }
    else if (mode == 3) { // Modo Vertical (l/i/t/f) -> ¡Y ESTO!
        configs.push_back({"l", cRed, 20});
        configs.push_back({"i", cBlue, 20});
        configs.push_back({"t", cGreen, 20});
        configs.push_back({"f", cPurple, 20});

        QStringList verticalComplex = {
            "il", "li", "ll", "it", "ti", "fl", "fi", "if",
            "tra", "tre", "tri", "tro", "tru", "fla", "fle",
            "fli", "flo", "flu"
        };
        for(const QString &s : verticalComplex) configs.push_back({s, cSyllable, 50});
    }

    // --- ALGORITMO DE FUSIÓN (MAPEO) ---
    int len = text.length();
    // (Nota: Si te da error aquí, asegúrate de tener definidos StyleMapInfo arriba)
    std::vector<StyleMapInfo> styleMap(len, {0, 0, false});

    for (const auto &cfg : configs) {
        std::vector<int> found = KMPsearch(text, cfg.pattern);
        for (int startPos : found) {
            int endPos = startPos + cfg.pattern.length();
            for (int k = startPos; k < endPos; k++) {
                if (k < len) {
                    if (!styleMap[k].active || cfg.priority > styleMap[k].priority) {
                        styleMap[k].color = cfg.color;
                        styleMap[k].priority = cfg.priority;
                        styleMap[k].active = true;
                    }
                }
            }
        }
    }

    // --- GENERAR RESULTADOS ---
    std::vector<TextStyle> finalResults;
    if (len == 0) return finalResults;

    int currentStart = -1;
    unsigned int currentColor = 0;
    bool tracking = false;

    for (int i = 0; i < len; i++) {
        if (styleMap[i].active) {
            if (!tracking) {
                tracking = true; currentStart = i; currentColor = styleMap[i].color;
            } else if (styleMap[i].color != currentColor) {
                finalResults.push_back({currentStart, i - currentStart, false, currentColor});
                currentStart = i; currentColor = styleMap[i].color;
            }
        } else {
            if (tracking) {
                finalResults.push_back({currentStart, i - currentStart, false, currentColor});
                tracking = false;
            }
        }
    }
    if (tracking) finalResults.push_back({currentStart, len - currentStart, false, currentColor});

    return finalResults;
}