#include "MainWindow.h"
#include "DyslexiaLogic.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMenuBar>
#include <QTextCharFormat>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Configuración de la Ventana
    setWindowTitle("Dyslexia Focus Pro - GUI Edition");
    resize(800, 600);

    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // 2. Controles Superiores
    QHBoxLayout *topLayout = new QHBoxLayout();
    modeCombo = new QComboBox();
    modeCombo->addItem("Modo Espejo (b/d/p/q)");
    modeCombo->addItem("Modo Fonético (g/j)");
    
    processBtn = new QPushButton("Analizar Texto");
    connect(processBtn, &QPushButton::clicked, this, &MainWindow::processText);

    topLayout->addWidget(modeCombo);
    topLayout->addWidget(processBtn);
    layout->addLayout(topLayout);

    // 3. Área de Texto
    textEdit = new QTextEdit();
    QFont font("Arial", 14); // Letra grande por defecto
    textEdit->setFont(font);
    layout->addWidget(textEdit);

    // 4. Menú Archivo
    QMenu *fileMenu = menuBar()->addMenu("Archivo");
    QAction *openAction = fileMenu->addAction("Abrir .txt");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Archivo", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
        }
    }
}

void MainWindow::processText() {
    // 1. Obtener texto crudo de la interfaz
    QString qText = textEdit->toPlainText();
    std::string stdText = qText.toStdString();
    
    if(stdText.empty()) return;

    // 2. Limpiar formatos previos (resetear a negro)
    QTextCursor cursor(textEdit->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat fmt;
    fmt.setForeground(Qt::black);
    fmt.setBackground(Qt::transparent);
    cursor.setCharFormat(fmt);

    // 3. Llamar a la Lógica (El Cerebro)
    // Pasamos el índice del combo box como modo
    std::vector<TextStyle> styles = DyslexiaLogic::analyzeText(stdText, modeCombo->currentIndex());

    // 4. Aplicar los estilos devueltos
    for(const auto &style : styles) {
        cursor.setPosition(style.start);
        cursor.setPosition(style.start + style.length, QTextCursor::KeepAnchor);
        
        QTextCharFormat newFmt;
        QColor color(style.colorHex); // Convierte 0xFF0000 a color Qt

        if(style.isBackground) {
            newFmt.setBackground(color); // Para el Heatmap
        } else {
            newFmt.setForeground(color); // Para las letras
            newFmt.setFontWeight(QFont::Bold);
        }
        
        // Merge permite mezclar fondo amarillo con texto rojo
        cursor.mergeCharFormat(newFmt); 
    }
}