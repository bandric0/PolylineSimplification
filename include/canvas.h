#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QPointF>
#include <vector>
#include "core_types.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);

    // Methods to load data and control animation
    void setPolyline(const std::vector<QPointF>& points);
    void setHistory(const std::vector<SimplificationStep>& history);
    void setStep(int stepIndex);
    void setMountainMode(bool enabled);

protected:
    // Main Qt rendering function
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<QPointF> m_originalPoints;
    std::vector<SimplificationStep> m_history;

    int m_currentStep;
    bool m_mountainMode;

    // Sub-routines for different visual styles
    void drawAlgorithmicView(QPainter& painter);
    void drawMountainView(QPainter& painter);
};

#endif // CANVAS_H
