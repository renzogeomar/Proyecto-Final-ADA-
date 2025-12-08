#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile();
    void processText();

private:
    QTextEdit *textEdit; // El área de texto
    QComboBox *modeCombo;
    QPushButton *processBtn;
    
    // Función para pintar el texto
    void applyStyles(const std::string &originalText);
};

#endif // MAINWINDOW_H