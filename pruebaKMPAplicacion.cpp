#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <clocale> // Para setlocale

using namespace std;

// ==========================================
// 1. SISTEMA DE COLORES Y CONFIGURACIÓN
// ==========================================
namespace Color {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string RED_TXT = "\033[31m";
    const string BLUE_TXT = "\033[34m";
    const string GREEN_TXT = "\033[32m";
    const string MAGENTA_TXT = "\033[35m";
    const string CYAN_TXT = "\033[36m";
    const string GRAY_TXT = "\033[90m";
    const string YELLOW_BG = "\033[43m";
}

struct CharStyle {
    string colorCode = "";
    string bgCode = "";
    bool isSeparator = false;
    int priority = 0;
};

struct Interval {
    int start;
    int end;
    string type;
    int priority;
    string color;
    string bg;
};

struct PatConfig {
    string pat;
    string color;
    int prio;
};

// ==========================================
// 2. LÓGICA CORE (KMP & Heurísticas)
// ==========================================

vector<int> buildLPS(const string &pattern) {
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

vector<Interval> findPatternsKMP(const string &text, const string &pattern, const string &color, int priority) {
    vector<Interval> matches;
    string lowerText = text;
    // Conversión básica a minúsculas (para caracteres ASCII)
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    
    vector<int> lps = buildLPS(pattern);
    int n = text.length(), m = pattern.length();
    int i = 0, j = 0;

    while (i < n) {
        if (lowerText[i] == pattern[j]) { i++; j++; }
        if (j == m) {
            matches.push_back({i - j, i, "pattern", priority, color, ""});
            j = lps[j - 1];
        } else if (i < n && lowerText[i] != pattern[j]) {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    return matches;
}

// HEATMAP DINÁMICO: Recibe el set de caracteres peligrosos según el modo elegido
vector<Interval> detectConfusionZones(const string &text, const set<char> &triggers) {
    vector<Interval> zones;
    string lowerText = text;
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    
    vector<int> triggerIndices;
    for(int i=0; i<lowerText.length(); i++) {
        // Verifica si el caracter actual está en la lista de peligrosos del modo seleccionado
        if(triggers.count(lowerText[i])) {
            triggerIndices.push_back(i);
        }
    }

    // Ventana deslizante: Si hay mucha densidad de triggers, marcar zona
    for(size_t i = 0; i + 1 < triggerIndices.size(); i++) {
        int dist = triggerIndices[i+1] - triggerIndices[i];
        if(dist < 5) { // Si están a menos de 5 espacios
            zones.push_back({
                triggerIndices[i], 
                triggerIndices[i+1] + 1, 
                "confusion", 
                100, 
                Color::BOLD, 
                Color::YELLOW_BG
            });
        }
    }
    return zones;
}

// HEURÍSTICA DE SÍLABAS (Con corrección UTF-8)
vector<Interval> heuristicsSyllables(const string &text) {
    vector<Interval> seps;
    int currentWordLen = 0;
    
    for(int i = 0; i < text.length(); i++) {
        unsigned char c = text[i];
        
        if(c == ' ' || c == ',' || c == '.' || c == ';' || c == '\n') {
            currentWordLen = 0;
            continue;
        }

        // Si NO es un byte de continuación UTF-8 (10xxxxxx), cuenta como letra
        bool isContinuation = (c >= 0x80 && c < 0xC0);
        if (!isContinuation) currentWordLen++;

        bool nextIsPunctuation = (i + 1 < text.length() && (text[i+1] == ' ' || text[i+1] == ',' || text[i+1] == '.'));
        
        // Separar cada 3 letras visuales, si no es final de palabra
        if (currentWordLen > 3 && currentWordLen % 3 == 0 && !nextIsPunctuation && !isContinuation) {
             // Chequeo extra: no romper justo antes de una tilde
             unsigned char nextC = (i + 1 < text.length()) ? text[i+1] : 0;
             bool nextIsMultiByteStart = (nextC >= 0xC0);
             
             if(!nextIsMultiByteStart) {
                 seps.push_back({i, i+1, "syllable", 10, Color::GRAY_TXT, ""});
             }
        }
    }
    return seps;
}

// ==========================================
// 3. FUSIÓN Y RENDERIZADO
// ==========================================

void applyIntervalsToCanvas(vector<CharStyle> &canvas, const vector<Interval> &intervals) {
    for (const auto &inter : intervals) {
        for (int i = inter.start; i < inter.end; i++) {
            if (i >= canvas.size()) break;

            if (inter.type == "confusion") {
                canvas[i].bgCode = inter.bg;
            } else if (inter.type == "syllable") {
                canvas[i].isSeparator = true; 
            } else {
                if (inter.priority >= canvas[i].priority) {
                    canvas[i].colorCode = inter.color;
                    canvas[i].priority = inter.priority;
                }
            }
        }
    }
}

// ==========================================
// 4. MENÚ Y MAIN
// ==========================================

int main() {
    setlocale(LC_ALL, ""); // Habilitar caracteres locales (ñ, tildes)

    cout << Color::BOLD << "===== Dyslexia-Focus Ultimate (v3.0) =====" << Color::RESET << "\n";
    
    // MENÚ DE SELECCIÓN
    cout << "Seleccione el perfil de dificultad:\n";
    cout << "1. Dislexia Visual (Espejo): b / d / p / q\n";
    cout << "2. Confusión Fonética (Sonido): g / j / ll / y\n";
    cout << "3. Confusión de Arcos (Forma): m / n / u / h\n";
    cout << "4. Confusión Vertical (Palitos): l / i / t / f\n";
    cout << "Opcion: ";
    
    int option;
    cin >> option;
    cin.ignore(); // Limpiar buffer

    vector<PatConfig> configs;
    set<char> triggerChars; // Para el heatmap

    switch(option) {
        case 1: // Espejo
            configs = { 
                {"b", Color::RED_TXT, 20}, {"d", Color::BLUE_TXT, 20},
                {"p", Color::GREEN_TXT, 20}, {"q", Color::MAGENTA_TXT, 20},
                // Sílabas trabadas comunes
                {"bra", Color::RED_TXT, 50}, {"dra", Color::BLUE_TXT, 50}
            };
            triggerChars = {'b', 'd', 'p', 'q'};
            break;
        case 2: // Fonética
            configs = { 
                {"ge", Color::RED_TXT, 30}, {"gi", Color::RED_TXT, 30},
                {"je", Color::BLUE_TXT, 30}, {"ji", Color::BLUE_TXT, 30},
                {"ll", Color::GREEN_TXT, 30}, {"y", Color::MAGENTA_TXT, 20}
            };
            triggerChars = {'g', 'j', 'l', 'y'}; // simplificado para triggers
            break;
        case 3: // Arcos
            configs = { 
                {"m", Color::RED_TXT, 20}, {"n", Color::BLUE_TXT, 20},
                {"u", Color::GREEN_TXT, 20}, {"h", Color::MAGENTA_TXT, 20},
                {"rn", Color::CYAN_TXT, 50} // "rn" a veces parece "m"
            };
            triggerChars = {'m', 'n', 'u', 'h'};
            break;
        case 4: // Vertical
            configs = { 
                {"l", Color::RED_TXT, 20}, {"i", Color::BLUE_TXT, 20},
                {"t", Color::GREEN_TXT, 20}, {"f", Color::MAGENTA_TXT, 20}
            };
            triggerChars = {'l', 'i', 't', 'f'};
            break;
        default:
            cout << "Opción inválida, usando modo estándar.\n";
            triggerChars = {'b', 'd'};
            configs = {{"b", Color::RED_TXT, 20}};
    }

    string originalText;
    cout << "\nIngrese el texto a procesar: ";
    getline(cin, originalText);

    if (originalText.empty()) return 0;

    // --- PROCESAMIENTO ---
    vector<CharStyle> canvas(originalText.length());
    vector<Interval> allIntervals;

    // 1. Buscar patrones configurados (KMP)
    for(auto &cfg : configs) {
        vector<Interval> found = findPatternsKMP(originalText, cfg.pat, cfg.color, cfg.prio);
        allIntervals.insert(allIntervals.end(), found.begin(), found.end());
    }

    // 2. Detectar zonas de confusión (Heatmap) usando los triggers seleccionados
    vector<Interval> confusionZones = detectConfusionZones(originalText, triggerChars);
    allIntervals.insert(allIntervals.end(), confusionZones.begin(), confusionZones.end());

    // 3. Separación silábica visual
    vector<Interval> syllables = heuristicsSyllables(originalText);
    allIntervals.insert(allIntervals.end(), syllables.begin(), syllables.end());

    // 4. Aplicar y Renderizar
    applyIntervalsToCanvas(canvas, allIntervals);

    cout << "\n" << Color::BOLD << "Lectura Enfocada:" << Color::RESET << "\n\n";
    
    for (int i = 0; i < originalText.length(); i++) {
        string color = (canvas[i].colorCode.empty()) ? Color::RESET : canvas[i].colorCode;
        string bg = (canvas[i].bgCode.empty()) ? "" : canvas[i].bgCode;
        
        cout << bg << color << originalText[i] << Color::RESET;
        
        if (canvas[i].isSeparator) {
            cout << Color::GRAY_TXT << "·" << Color::RESET;
        }
    }
    cout << "\n\n";

    return 0;
}