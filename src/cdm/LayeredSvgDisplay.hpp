#pragma once

#include <QWidget>
#include <QSvgRenderer>
#include <QColor>
#include <QPointF>
#include <vector>

class QParallelAnimationGroup;

class LayeredSvgDisplay : public QWidget
{
    Q_OBJECT
public:
    struct Layer {
        QColor color;
        QPointF offset; // The CURRENT offset
        double scale;   // The CURRENT scale
    };

    explicit LayeredSvgDisplay(const QString &svgPath, QWidget *parent = nullptr);

    // Initial setup
    void setLayers(const std::vector<Layer> &layers);

    // 1. Direct Control: Snaps the layer instantly (Useful for tying to sliders/mouse)
    void setLayerState(size_t index, QPointF offset, double scale);

    // 2. Animated Control: Smoothly animates the layer to a new position
    void animateLayerTo(size_t index, QPointF targetOffset, double targetScale, int durationMs = 500);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSvgRenderer *m_svgRenderer;
    std::vector<Layer> m_layers;
    
    // We need to keep track of active animations so we can interrupt them 
    // if you command the layer to move somewhere else mid-animation.
    std::vector<QParallelAnimationGroup*> m_activeAnimations;
};
