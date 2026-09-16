#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "efficientsimplifier.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRandomGenerator>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_currentStep(0) {
    ui->setupUi(this);
    resize(1000, 600);
    setWindowTitle("Polyline Simplification");

    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 1. Add Canvas (Left side, ratio 3:1)
    m_canvas = new Canvas(this);
    mainLayout->addWidget(m_canvas, 3);

    // 2. Control panel (Right side)
    QVBoxLayout *controlLayout = new QVBoxLayout();
    mainLayout->addLayout(controlLayout, 1);

    // Data Input Section
    QGroupBox *groupData = new QGroupBox("1. Data Input", this);
    QVBoxLayout *dataLayout = new QVBoxLayout(groupData);
    QPushButton *btnGenRandom = new QPushButton("Generate Random", this);
    QPushButton *btnLoadFile = new QPushButton("Load from File", this);
    dataLayout->addWidget(btnGenRandom);
    dataLayout->addWidget(btnLoadFile);
    controlLayout->addWidget(groupData);

    // Algorithm Section
    QGroupBox *groupAlgo = new QGroupBox("2. Algorithm", this);
    QVBoxLayout *algoLayout = new QVBoxLayout(groupAlgo);
    QHBoxLayout *mLayout = new QHBoxLayout();
    mLayout->addWidget(new QLabel("Points to remove:", this));
    m_spinPointsToRemove = new QSpinBox(this);
    mLayout->addWidget(m_spinPointsToRemove);
    algoLayout->addLayout(mLayout);
    QPushButton *btnRun = new QPushButton("Run Simplification", this);
    algoLayout->addWidget(btnRun);
    controlLayout->addWidget(groupAlgo);

    // Animation Section
    QGroupBox *groupAnim = new QGroupBox("3. Animation", this);
    QVBoxLayout *animLayout = new QVBoxLayout(groupAnim);
    QCheckBox *chkMountainMode = new QCheckBox("Mountain Mode (Zoom out effect)", this);
    animLayout->addWidget(chkMountainMode);

    QHBoxLayout *speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel("Delay (Fast <-> Slow):", this));
    m_sliderSpeed = new QSlider(Qt::Horizontal, this);
    m_sliderSpeed->setRange(10, 1000); // Od 10ms (jako brzo) do 1000ms (1 sekunda pauze)
    m_sliderSpeed->setValue(150);      // Početna brzina
    speedLayout->addWidget(m_sliderSpeed);
    animLayout->addLayout(speedLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnPrev = new QPushButton("⏮", this);
    m_btnPlay = new QPushButton("▶ Play", this);
    QPushButton *btnNext = new QPushButton("⏭", this);
    btnLayout->addWidget(btnPrev);
    btnLayout->addWidget(m_btnPlay);
    btnLayout->addWidget(btnNext);
    animLayout->addLayout(btnLayout);
    controlLayout->addWidget(groupAnim);

    // Push everything to the top
    controlLayout->addStretch();

    setCentralWidget(centralWidget);

    // 3. Initialize timer
    m_timer = new QTimer(this);
    m_timer->setInterval(150); // Animation speed

    connect(m_sliderSpeed, &QSlider::valueChanged, this, [this](int value) {
        m_timer->setInterval(value);
    });

    // 4. Connect signals and slots
    connect(btnGenRandom, &QPushButton::clicked, this, &MainWindow::generateRandomMountain);
    connect(btnLoadFile, &QPushButton::clicked, this, &MainWindow::loadFromFile);
    connect(btnRun, &QPushButton::clicked, this, &MainWindow::runAlgorithm);
    connect(chkMountainMode, &QCheckBox::toggled, this, &MainWindow::toggleViewMode);

    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    connect(btnPrev, &QPushButton::clicked, this, &MainWindow::prevStep);
    connect(btnNext, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // Generate initial view on startup
    generateRandomMountain();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateLimits() {
    if (m_points.size() > 2) {
        // Maximum points to remove is total points - 2
        m_spinPointsToRemove->setRange(1, m_points.size() - 2);
        m_spinPointsToRemove->setValue((m_points.size() - 2) / 2); // Default: remove half
    } else {
        m_spinPointsToRemove->setRange(0, 0);
    }
}

void MainWindow::generateRandomMountain() {
    m_points.clear();
    double startX = 50.0;
    double yBase = 450.0;

    int numPoints = QRandomGenerator::global()->bounded(50, 100);
    double stepX = 800.0 / numPoints; // Scale X to fit screen width

    m_points.push_back(QPointF(startX, yBase));
    for (int i = 1; i < numPoints; ++i) {
        double x = startX + i * stepX;
        double y = yBase - QRandomGenerator::global()->bounded(50, 350);
        m_points.push_back(QPointF(x, y));
    }
    m_points.push_back(QPointF(startX + numPoints * stepX, yBase));

    m_history.clear();
    updateLimits();
    m_canvas->setPolyline(m_points);
}

void MainWindow::loadFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Polyline File", QString(PROJECT_DIR) + "/examples", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open the file.");
        return;
    }

    QTextStream in(&file);
    int n;
    in >> n; // Read number of points

    if (n < 3) {
        QMessageBox::warning(this, "Error", "A polyline needs at least 3 points for simplification.");
        return;
    }

    m_points.clear();
    for (int i = 0; i < n; ++i) {
        double x, y;
        in >> x >> y;
        m_points.push_back(QPointF(x, y));
    }

    m_history.clear();
    updateLimits();
    m_canvas->setPolyline(m_points);
}

void MainWindow::runAlgorithm() {
    if (m_points.size() <= 2) return;

    // Calculate remaining points target (total - points to remove)
    int targetRemainingPoints = m_points.size() - m_spinPointsToRemove->value();

    // Execute O(n log n) simplification
    m_history = EfficientSimplifier::simplify(m_points, targetRemainingPoints);

    m_currentStep = 0;
    m_canvas->setHistory(m_history);
    m_canvas->setStep(m_currentStep);

    if (m_timer->isActive()) togglePlayPause();
}

void MainWindow::togglePlayPause() {
    if (m_history.empty()) return;

    if (m_timer->isActive()) {
        m_timer->stop();
        m_btnPlay->setText("▶ Play");
    } else {
        // Restart if we reached the end
        if (m_currentStep >= (int)m_history.size()) m_currentStep = 0;
        m_timer->start();
        m_btnPlay->setText("⏸ Pause");
    }
}

void MainWindow::nextStep() {
    if (m_currentStep < (int)m_history.size()) {
        m_currentStep++;
        m_canvas->setStep(m_currentStep);
    }
}

void MainWindow::prevStep() {
    if (m_currentStep > 0) {
        m_currentStep--;
        m_canvas->setStep(m_currentStep);
    }
}

void MainWindow::onTimerTick() {
    if (m_currentStep < (int)m_history.size()) {
        m_currentStep++;
        m_canvas->setStep(m_currentStep);
    } else {
        togglePlayPause(); // Finished, pause animation
    }
}

void MainWindow::toggleViewMode(bool checked) {
    m_canvas->setMountainMode(checked);
}
