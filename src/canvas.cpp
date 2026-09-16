#include "canvas.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPolygonF>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent), m_currentStep(0), m_mountainMode(false) {}

void Canvas::setPolyline(const std::vector<QPointF>& points) {
    m_originalPoints = points;
    m_history.clear();
    m_currentStep = 0;
    update(); // Trigger repaint
}

void Canvas::setHistory(const std::vector<SimplificationStep>& history) {
    m_history = history;
    m_currentStep = 0;
    update();
}

void Canvas::setStep(int stepIndex) {
    if (stepIndex >= 0 && stepIndex <= (int)m_history.size()) {
        m_currentStep = stepIndex;
        update();
    }
}

void Canvas::setMountainMode(bool enabled) {
    m_mountainMode = enabled;
    update();
}

void Canvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (m_originalPoints.empty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_mountainMode) {
        drawMountainView(painter);
    } else {
        drawAlgorithmicView(painter);
    }
}

void Canvas::drawAlgorithmicView(QPainter& painter) {
    // Background
    painter.fillRect(rect(), QColor(245, 245, 245));

    // 1. Draw original polyline (faint dashed gray line)
    QPen originalPen(QColor(180, 180, 180), 2, Qt::DashLine);
    painter.setPen(originalPen);
    for (size_t i = 0; i < m_originalPoints.size() - 1; ++i) {
        painter.drawLine(m_originalPoints[i], m_originalPoints[i+1]);
    }

    // Determine currently active indices
    std::vector<int> activeIndices;
    if (m_currentStep == 0 || m_history.empty()) {
        for (size_t i = 0; i < m_originalPoints.size(); ++i) activeIndices.push_back(i);
    } else {
        activeIndices = m_history[m_currentStep - 1].active_points;
    }

    // 2. Draw current simplified polyline (solid blue)
    QPen currentPen(QColor(41, 128, 185), 3, Qt::SolidLine);
    painter.setPen(currentPen);
    for (size_t i = 0; i < activeIndices.size() - 1; ++i) {
        painter.drawLine(m_originalPoints[activeIndices[i]], m_originalPoints[activeIndices[i+1]]);
    }

    // 3. Highlight the triangle being removed NEXT (if not at the end)
    if (m_currentStep < (int)m_history.size()) {
        int removedIdx = m_history[m_currentStep].removed_index;

        // Find neighbors in the CURRENT active list
        int leftIdx = -1, rightIdx = -1;
        for (size_t i = 0; i < activeIndices.size(); ++i) {
            if (activeIndices[i] < removedIdx) leftIdx = activeIndices[i];
            if (activeIndices[i] > removedIdx && rightIdx == -1) rightIdx = activeIndices[i];
        }

        if (leftIdx != -1 && rightIdx != -1) {
            QPolygonF triangle;
            triangle << m_originalPoints[leftIdx]
                     << m_originalPoints[removedIdx]
                     << m_originalPoints[rightIdx];

            // Translucent red fill, thick red border
            painter.setBrush(QColor(231, 76, 60, 100));
            painter.setPen(QPen(QColor(231, 76, 60), 2));
            painter.drawPolygon(triangle);
        }
    }

    // 4. Draw vertices (small circles)
    painter.setBrush(QColor(44, 62, 80));
    painter.setPen(Qt::NoPen);
    for (int idx : activeIndices) {
        painter.drawEllipse(m_originalPoints[idx], 4.0, 4.0);
    }
}

void Canvas::drawMountainView(QPainter& painter) {
    // Sky gradient
    QLinearGradient skyGradient(0, 0, 0, height());
    skyGradient.setColorAt(0, QColor(44, 62, 80));
    skyGradient.setColorAt(1, QColor(230, 126, 34));
    painter.fillRect(rect(), skyGradient);

    // Sun / Moon
    painter.setBrush(QColor(241, 196, 15, 200));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(width() * 0.7, height() * 0.2, 60, 60);

    // --- ZOOM OUT EFFECT ---
    double progress = 0.0;
    if (!m_history.empty()) {
        progress = (double)m_currentStep / m_history.size();
    }

    // Scale from 1.0 (close) to 0.4 (far) as animation progresses
    double currentScale = 1.0 - (progress * 0.6);

    painter.save(); // Save initial QPainter state

    // Translate center to bottom-middle, apply scale, and translate back
    painter.translate(width() / 2.0, height() * 0.85);
    painter.scale(currentScale, currentScale);
    painter.translate(-width() / 2.0, -height() * 0.85);
    // -----------------------

    // Determine active points
    std::vector<int> activeIndices;
    if (m_currentStep == 0 || m_history.empty()) {
        for (size_t i = 0; i < m_originalPoints.size(); ++i) activeIndices.push_back(i);
    } else {
        activeIndices = m_history[m_currentStep - 1].active_points;
    }

    // Build polygon for the mountain
    QPolygonF mountainPoly;
    for (int idx : activeIndices) {
        mountainPoly << m_originalPoints[idx];
    }

    // Close polygon at the bottom
    if (!activeIndices.empty()) {
        mountainPoly << QPointF(m_originalPoints[activeIndices.back()].x(), height());
        mountainPoly << QPointF(m_originalPoints[activeIndices.front()].x(), height());
    }

    QLinearGradient mountainGradient(0, height() * 0.3, 0, height());
    mountainGradient.setColorAt(0, QColor(52, 73, 94));
    mountainGradient.setColorAt(1, QColor(20, 25, 30));

    painter.setBrush(mountainGradient);
    painter.setPen(QPen(QColor(189, 195, 199, 100), 2));
    painter.drawPolygon(mountainPoly);

    painter.restore(); // Restore QPainter state (prevents fog from scaling)

    // Add depth/fog effect based on progress
    if (m_currentStep > 0 && !m_history.empty()) {
        QColor fogColor(230, 126, 34, int(progress * 130));
        painter.fillRect(rect(), fogColor);
    }
}
