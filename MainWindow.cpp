#include "mainwindow.h"
#include "dyslexialogic.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTextCharFormat>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Configuración Ventana
    setWindowTitle("Dyslexia Focus - Lectura Limpia");
    resize(1000, 750);

    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Barra Superior
    QHBoxLayout *topLayout = new QHBoxLayout();

    modeCombo = new QComboBox();
    modeCombo->addItem("1. Modo Espejo (b / d / p / q)");
    modeCombo->addItem("2. Modo Fonético (g / j / ll / y)");
    modeCombo->addItem("3. Modo Formas (m / n / u / h)");
    modeCombo->addItem("4. Modo Vertical (l / i / t / f)");
    //modeCombo->setStyleSheet("padding: 5px; font-size: 14px;");

    QFont comboFont("Segoe UI", 11);
    modeCombo->setFont(comboFont);

    processBtn = new QPushButton("Aplicar Filtro");
    // Al botón SÍ le puedes dejar el estilo, no suele dar problemas
    processBtn->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");

    topLayout->addWidget(modeCombo, 1);
    topLayout->addWidget(processBtn);
    layout->addLayout(topLayout);


    //processBtn = new QPushButton("Aplicar Filtro");
    processBtn->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");

    topLayout->addWidget(modeCombo, 1);
    topLayout->addWidget(processBtn);
    layout->addLayout(topLayout);

    // --- ÁREA DE TEXTO (CAMBIO IMPORTANTE) ---
    textEdit = new QTextEdit();

    // Fuente: Verdana o Arial son mejores para dislexia que Times New Roman
    QFont font("Verdana", 18);
    textEdit->setFont(font);

    // ESTILO PAPEL: Fondo Crema suave (#FAF9F6) y texto Gris Oscuro (#333)
    // Esto reduce el deslumbramiento del blanco puro o el contraste duro del negro.
    textEdit->setStyleSheet("background-color: #FAF9F6; color: #333333; border: 1px solid #CCC; padding: 10px;");

    layout->addWidget(textEdit);

    // Leyenda
    legendLabel = new QLabel("Seleccione un modo para resaltar patrones.");
    legendLabel->setStyleSheet("background-color: #EEE; color: #333; padding: 10px; border-top: 1px solid #CCC; font-size: 14px;");
    legendLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(legendLabel);

    // Menú
    QMenu *fileMenu = menuBar()->addMenu("Archivo");
    QAction *openAction = fileMenu->addAction("Abrir Texto (.txt)");

    // Conexiones
    connect(processBtn, &QPushButton::clicked, this, &MainWindow::processText);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateLegend);

    updateLegend(0);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Archivo", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
            // Limpiar estilos al cargar nuevo
            processText();
        }
    }
}

void MainWindow::updateLegend(int index) {
    QString text;
    // Texto común para las sílabas naranjas
    QString syllInfo = " | <b><span style='color:#E65100'>Sílabas complejas</span></b>";

    switch(index) {
    case 0: // Espejo
        text = "<b>b</b> (<span style='color:#D32F2F'>Rojo</span>) vs <b>d</b> (<span style='color:#1976D2'>Azul</span>) | "
               "<b>p</b> (<span style='color:#388E3C'>Verde</span>) vs <b>q</b> (<span style='color:#7B1FA2'>Morado</span>)" + syllInfo;
        break;
    case 1: // Fonética
        text = "<b>g</b> (<span style='color:#D32F2F'>Rojo</span>) vs <b>j</b> (<span style='color:#1976D2'>Azul</span>) | "
               "<b>gui/gue</b> (<span style='color:#E65100'>Naranja</span>)";
        break;
    case 2: // Formas
        text = "<b>m</b> (<span style='color:#D32F2F'>Rojo</span>) vs <b>n</b> (<span style='color:#1976D2'>Azul</span>) | "
               "<b>u</b> (<span style='color:#388E3C'>Verde</span>) vs <b>h</b> (<span style='color:#7B1FA2'>Morado</span>)" + syllInfo;
        break;
    case 3: // Vertical (NUEVO CASE AGREGADO)
        text = "<b>l</b> (<span style='color:#D32F2F'>Rojo</span>) vs <b>i</b> (<span style='color:#1976D2'>Azul</span>) | "
               "<b>t</b> (<span style='color:#388E3C'>Verde</span>) vs <b>f</b> (<span style='color:#7B1FA2'>Morado</span>) | "
               "<b>tra/tre/li...</b> (<span style='color:#E65100'>Naranja</span>)";
        break;
    default:
        text = "Seleccione un modo para comenzar.";
    }
    legendLabel->setText(text);
}

void MainWindow::processText() {
    // 1. Obtenemos el texto directamente como QString
    QString qText = textEdit->toPlainText();

    if(qText.isEmpty()) return;

    // 2. Limpieza de formato
    QTextCursor cursor(textEdit->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat fmt;
    fmt.setForeground(QColor("#333333"));
    fmt.setBackground(Qt::transparent);
    fmt.setFontWeight(QFont::Normal);
    cursor.setCharFormat(fmt);

    // 3. Llamamos a la lógica pasando el QString DIRECTAMENTE
    // (Ya no usamos stdText)
    std::vector<TextStyle> styles = DyslexiaLogic::analyzeText(qText, modeCombo->currentIndex());

    // 4. Aplicar estilos
    for(const auto &style : styles) {
        cursor.setPosition(style.start);
        cursor.setPosition(style.start + style.length, QTextCursor::KeepAnchor);

        QTextCharFormat newFmt;
        QColor color(style.colorHex);

        newFmt.setForeground(color);
        newFmt.setFontWeight(QFont::ExtraBold);
        newFmt.setFontPointSize(20);

        cursor.mergeCharFormat(newFmt);
    }
}
