#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Función para construir la tabla LPS (Longest Prefix Suffix) del algoritmo KMP
vector<int> buildLPS(const string &pattern) {
    int m = pattern.length();
    vector<int> lps(m, 0);
    int len = 0;
    int i = 1;

    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

// Función KMP para buscar patrón en el texto
vector<int> KMPsearch(const string &text, const string &pattern) {
    vector<int> result;
    int n = text.length();
    int m = pattern.length();

    vector<int> lps = buildLPS(pattern);

    int i = 0; // índice del texto
    int j = 0; // índice del patrón

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
        }

        if (j == m) {
            result.push_back(i - j); // coincidencia encontrada
            j = lps[j - 1];
        } else if (i < n && text[i] != pattern[j]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i++;
        }
    }
    return result;
}

// Función para resaltar patrones encontrados en el texto
string highlightText(const string &text, const vector<pair<string, vector<int>>> &matches) {
    string highlighted = text;
    // Para no romper índices al insertar, se recorre desde el final
    for (int p = matches.size() - 1; p >= 0; p--) {
        string pattern = matches[p].first;
        vector<int> positions = matches[p].second;
        for (int pos : positions) {
            // Insertamos corchetes alrededor del patrón detectado
            highlighted.insert(pos + pattern.length(), "]");
            highlighted.insert(pos, "[");
        }
    }
    return highlighted;
}

int main() {
    cout << "===== Dyslexia-Focus MVP =====\n";

    // Texto original (puede ser leído desde archivo o ingresado por usuario)
    string originalText;
    cout << "Ingrese el texto a leer: ";
    getline(cin, originalText);

    // Lista de patrones problemáticos para dislexia
    vector<string> patterns = {
        "b", "d", "p", "q", // letras que se confunden
        "ba", "da", "pa", "qa", "bre", "dra", "pla", "tra" // sílabas complejas
    };

    // Convertimos texto a minúsculas para comparación más simple
    string lowerText = originalText;
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);

    // Vector para guardar coincidencias de todos los patrones
    vector<pair<string, vector<int>>> allMatches;

    for (const string &pat : patterns) {
        vector<int> found = KMPsearch(lowerText, pat);
        if (!found.empty()) {
            allMatches.push_back({pat, found});
        }
    }

    // Generamos texto resaltado
    string highlighted = highlightText(originalText, allMatches);

    // Mostramos resultados
    cout << "\nTexto resaltado con patrones detectados:\n";
    cout << highlighted << "\n";

    // Tabla de errores (opcional)
    cout << "\nTabla de patrones detectados:\n";
    cout << "Patrón\tPosiciones\n";
    for (auto &match : allMatches) {
        cout << match.first << "\t";
        for (int pos : match.second) {
            cout << pos << " ";
        }
        cout << "\n";
    }

    cout << "\n===== Fin del análisis =====\n";
    return 0;
}