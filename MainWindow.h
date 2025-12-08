#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel> // <-- NUEVO: Para la leyenda

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile();
    void processText();
    void updateLegend(int index); // <-- NUEVO: Slot para cambiar texto leyenda

private:
    QTextEdit *textEdit;
    QComboBox *modeCombo;
    QPushButton *processBtn;
    QLabel *legendLabel; // <-- NUEVO: El widget de texto
};

#endif // MAINWINDOW_H