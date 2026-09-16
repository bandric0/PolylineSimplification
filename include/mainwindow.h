#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <vector>
#include <QPointF>
#include "canvas.h"
#include "core_types.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Data generation and loading
    void generateRandomMountain();
    void loadFromFile();

    // Algorithm execution
    void runAlgorithm();

    // Animation controls
    void togglePlayPause();
    void nextStep();
    void prevStep();
    void onTimerTick();
    void toggleViewMode(bool checked);

private:
    Ui::MainWindow *ui;

    // UI Components
    Canvas *m_canvas;
    QPushButton *m_btnPlay;
    QTimer *m_timer;
    QSpinBox *m_spinPointsToRemove;
    QSlider *m_sliderSpeed;

    // Data state
    std::vector<QPointF> m_points;
    std::vector<SimplificationStep> m_history;
    int m_currentStep;

    // Helper method to limit spinbox based on current point count
    void updateLimits();
};

#endif // MAINWINDOW_H
