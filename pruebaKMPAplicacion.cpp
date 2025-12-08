#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <clocale>
#include <fstream>  // Para leer archivos
#include <sstream>  // Para buffers de string
#include <filesystem> // Para chequear extensiones (C++17)

using namespace std;

// ==========================================
// 1. SISTEMA VISUAL (ANSI)
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
    const string WHITE_BG = "\033[47m";
    const string BLACK_TXT = "\033[30m";
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
// 2. GESTOR DE ARCHIVOS (File I/O)
// ==========================================

class FileManager {
public:
    // Lee archivo TXT usando streams estándar
    static string readTxt(const string& path) {
        ifstream file(path);
        if (!file.is_open()) {
            cerr << Color::RED_TXT << "Error: No se pudo abrir el archivo .txt" << Color::RESET << endl;
            return "";
        }
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Simulación de lectura PDF (Requiere herramientas externas en un entorno real)
    // NOTA: Para leer PDF real en C++ puro se necesita compilar con 'libpoppler' o 'podofo'.
    // Aquí implementamos una detección de cabecera para mostrar complejidad.
    static string readPdf(const string& path) {
        cout << Color::CYAN_TXT << "[INFO] Detectado archivo PDF..." << Color::RESET << endl;
        
        // Verificación básica de "Magic Number" de PDF
        ifstream file(path, ios::binary);
        if (!file.is_open()) return "";
        char header[5];
        file.read(header, 4);
        header[4] = '\0';
        string magic(header);
        
        if (magic != "%PDF") {
            cerr << "Error: El archivo no parece ser un PDF válido." << endl;
            return "";
        }

        // EXPLICACIÓN TÉCNICA:
        // C++ no puede decodificar el stream binario comprimido de un PDF sin librerías.
        // En un proyecto real, aquí llamaríamos a:
        // Poppler::Document::load_from_file(path)->create_page(0)->text()...
        
        cout << Color::YELLOW_BG << Color::BLACK_TXT 
             << " [NOTA TÉCNICA] " << Color::RESET 
             << " Para leer el contenido del PDF se requiere la librería 'libpoppler'.\n"
             << " Como fallback, intente guardar su PDF como .txt y cargarlo de nuevo.\n"
             << " (Simulando carga de texto para demostración...)\n";
        
        // Retornamos un texto dummy para probar el algoritmo KMP
        return "Esto es una simulacion de contenido extraido de un PDF. "
               "El sistema detecto la cabecera %PDF correctamente pero requiere "
               "librerias externas vinculadas para descomprimir los objetos binarios. "
               "Por favor use un archivo .txt para la prueba real.";
    }

    static string loadFile(const string& path) {
        // Detección simple de extensión (Búsqueda inversa del punto)
        size_t dotPos = path.rfind('.');
        if (dotPos == string::npos) return readTxt(path); // Asumir txt si no hay extensión

        string ext = path.substr(dotPos);
        // Convertir a minúsculas
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".pdf") {
            return readPdf(path);
        } else {
            return readTxt(path);
        }
    }
};

// ==========================================
// 3. CORE ALGORÍTMICO (KMP + Heatmap)
// ==========================================
// (Sin cambios en la lógica KMP pura, como solicitaste)

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

vector<Interval> detectConfusionZones(const string &text, const set<char> &triggers) {
    vector<Interval> zones;
    string lowerText = text;
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    vector<int> idxs;
    for(int i=0; i<lowerText.length(); i++) if(triggers.count(lowerText[i])) idxs.push_back(i);

    for(size_t i = 0; i + 1 < idxs.size(); i++) {
        if((idxs[i+1] - idxs[i]) < 5) {
            zones.push_back({idxs[i], idxs[i+1] + 1, "confusion", 100, Color::BOLD, Color::YELLOW_BG});
        }
    }
    return zones;
}

vector<Interval> heuristicsSyllables(const string &text) {
    vector<Interval> seps;
    int currentWordLen = 0;
    for(int i = 0; i < text.length(); i++) {
        unsigned char c = text[i];
        if(c == ' ' || c == '\n' || c == '\t') { currentWordLen = 0; continue; }
        
        bool isContinuation = (c >= 0x80 && c < 0xC0);
        if (!isContinuation) currentWordLen++;

        bool nextSep = (i + 1 < text.length() && (text[i+1] == ' ' || text[i+1] == '\n'));
        if (currentWordLen > 3 && currentWordLen % 3 == 0 && !nextSep && !isContinuation) {
             unsigned char nextC = (i + 1 < text.length()) ? text[i+1] : 0;
             if(nextC < 0xC0) seps.push_back({i, i+1, "syllable", 10, Color::GRAY_TXT, ""});
        }
    }
    return seps;
}

void applyIntervalsToCanvas(vector<CharStyle> &canvas, const vector<Interval> &intervals) {
    for (const auto &inter : intervals) {
        for (int i = inter.start; i < inter.end; i++) {
            if (i >= canvas.size()) break;
            if (inter.type == "confusion") canvas[i].bgCode = inter.bg;
            else if (inter.type == "syllable") canvas[i].isSeparator = true; 
            else if (inter.priority >= canvas[i].priority) {
                canvas[i].colorCode = inter.color;
                canvas[i].priority = inter.priority;
            }
        }
    }
}

// ==========================================
// 4. MOTOR DE PAGINACIÓN (NUEVO)
// ==========================================

void displayPaginated(const string &text, const vector<CharStyle> &canvas) {
    int pageSize = 500; // Caracteres por página
    int totalLen = text.length();
    int currentPage = 0;
    int totalPages = (totalLen / pageSize) + 1;

    while (true) {
        // Limpiar pantalla (Comando ANSI)
        cout << "\033[2J\033[1;1H"; 
        
        cout << Color::BOLD << "=== Visor Dyslexia-Focus (Página " << currentPage + 1 << "/" << totalPages << ") ===" << Color::RESET << "\n\n";

        int start = currentPage * pageSize;
        int end = min(start + pageSize, totalLen);

        // Renderizado del fragmento actual
        for (int i = start; i < end; i++) {
            string color = (canvas[i].colorCode.empty()) ? Color::RESET : canvas[i].colorCode;
            string bg = (canvas[i].bgCode.empty()) ? "" : canvas[i].bgCode;
            
            cout << bg << color << text[i] << Color::RESET;
            if (canvas[i].isSeparator) cout << Color::GRAY_TXT << "·" << Color::RESET;
        }

        cout << "\n\n" << string(50, '-') << "\n";
        cout << "[N] Siguiente  |  [P] Anterior  |  [Q] Salir\n";
        cout << "Opción: ";
        
        char cmd;
        cin >> cmd;
        cmd = tolower(cmd);

        if (cmd == 'n' && currentPage < totalPages - 1) currentPage++;
        else if (cmd == 'p' && currentPage > 0) currentPage--;
        else if (cmd == 'q') break;
    }
}

// ==========================================
// 5. MAIN
// ==========================================

int main() {
    setlocale(LC_ALL, ""); 

    cout << Color::BOLD << "===== Dyslexia-Focus Architect (v4.0) =====" << Color::RESET << "\n";
    
    // 1. SELECCIÓN DE ENTRADA
    cout << "Fuente del texto:\n";
    cout << "1. Escribir manualmente\n";
    cout << "2. Cargar archivo (.txt / .pdf)\n";
    cout << "Opción: ";
    int inputOpt;
    cin >> inputOpt;
    cin.ignore();

    string textToProcess = "";

    if (inputOpt == 1) {
        cout << "Ingrese el texto: ";
        getline(cin, textToProcess);
    } else {
        cout << "Ingrese ruta del archivo (ej: libro.txt): ";
        string path;
        getline(cin, path);
        textToProcess = FileManager::loadFile(path);
    }

    if (textToProcess.empty()) {
        cout << "No hay texto para procesar.\n";
        return 0;
    }

    // 2. SELECCIÓN DE PERFIL (Igual que antes)
    cout << "\nSeleccione perfil de dificultad:\n";
    cout << "1. Dislexia Visual (b/d/p/q)\n";
    cout << "2. Fonética (g/j/ll/y)\n";
    cout << "3. Formas (m/n/u)\nOpción: ";
    
    int mode;
    cin >> mode;
    
    vector<PatConfig> configs;
    set<char> triggerChars;

    // Configuración simplificada para el ejemplo
    if (mode == 2) {
        configs = { {"ge", Color::RED_TXT, 30}, {"je", Color::BLUE_TXT, 30} };
        triggerChars = {'g', 'j'};
    } else if (mode == 3) {
        configs = { {"m", Color::RED_TXT, 20}, {"n", Color::BLUE_TXT, 20} };
        triggerChars = {'m', 'n'};
    } else {
        configs = { 
            {"b", Color::RED_TXT, 20}, {"d", Color::BLUE_TXT, 20},
            {"p", Color::GREEN_TXT, 20}, {"q", Color::MAGENTA_TXT, 20},
            {"bra", Color::RED_TXT, 50}, {"cla", Color::BLUE_TXT, 50}
        };
        triggerChars = {'b', 'd', 'p', 'q'};
    }

    // 3. PROCESAMIENTO MASIVO
    cout << "\nProcesando " << textToProcess.length() << " caracteres...\n";
    
    vector<CharStyle> canvas(textToProcess.length());
    vector<Interval> allIntervals;

    // Ejecutamos KMP sobre todo el texto (Memoria: O(N))
    // Si el archivo fuera de 1GB, aquí deberíamos usar buffers, pero para <50MB esto es seguro.
    for(auto &cfg : configs) {
        vector<Interval> found = findPatternsKMP(textToProcess, cfg.pat, cfg.color, cfg.prio);
        allIntervals.insert(allIntervals.end(), found.begin(), found.end());
    }
    
    vector<Interval> zones = detectConfusionZones(textToProcess, triggerChars);
    allIntervals.insert(allIntervals.end(), zones.begin(), zones.end());
    
    vector<Interval> sylls = heuristicsSyllables(textToProcess);
    allIntervals.insert(allIntervals.end(), sylls.begin(), sylls.end());

    applyIntervalsToCanvas(canvas, allIntervals);

    // 4. VISUALIZACIÓN PAGINADA
    displayPaginated(textToProcess, canvas);

    return 0;
}