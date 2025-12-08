#include "DyslexiaLogic.h"

using namespace std;

// --- Implementación KMP (Intacta) ---
vector<int> DyslexiaLogic::buildLPS(const string &pattern) {
    int m = pattern.length();
    vector<int> lps(m, 0);
    int len = 0, i = 1;
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

vector<int> DyslexiaLogic::KMPsearch(const string &text, const string &pattern) {
    vector<int> matches;
    string lowerText = text;
    // Conversión simple a minusculas
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    
    vector<int> lps = buildLPS(pattern);
    int n = text.length(), m = pattern.length();
    int i = 0, j = 0;

    while (i < n) {
        if (lowerText[i] == pattern[j]) { i++; j++; }
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

// --- Lógica de Análisis y Fusión ---
vector<TextStyle> DyslexiaLogic::analyzeText(const string &text, int mode) {
    vector<TextStyle> results;
    vector<PatternConfig> configs;
    set<char> triggers;

    // Configuración según modo (Igual que en consola)
    if (mode == 0) { // Dislexia Visual
        configs = { {"b", 0xFF0000, 20}, {"d", 0x0000FF, 20}, {"p", 0x008000, 20}, {"q", 0xFF00FF, 20} };
        triggers = {'b', 'd', 'p', 'q'};
    } else if (mode == 1) { // Fonética
        configs = { {"g", 0xFF0000, 20}, {"j", 0x0000FF, 20} };
        triggers = {'g', 'j'};
    }

    // 1. Ejecutar KMP para cada patrón
    for (const auto &cfg : configs) {
        vector<int> found = KMPsearch(text, cfg.pattern);
        for (int pos : found) {
            // Guardamos el estilo para enviar a la GUI
            results.push_back({pos, (int)cfg.pattern.length(), false, cfg.color});
        }
    }

    // 2. Heatmap (Zonas confusas)
    string lowerText = text;
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    vector<int> tIdx;
    for(int i=0; i<lowerText.length(); i++) if(triggers.count(lowerText[i])) tIdx.push_back(i);

    for(size_t i = 0; i + 1 < tIdx.size(); i++) {
        if((tIdx[i+1] - tIdx[i]) < 6) {
            // Fondo amarillo suave (0xFFFFE0)
            results.push_back({tIdx[i], tIdx[i+1] - tIdx[i] + 1, true, 0xFFFF00});
        }
    }

    return results;
}