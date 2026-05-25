#include "LayeredSvgDisplay.hpp"
#include <QPainter>
#include <QImage>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QEasingCurve>

LayeredSvgDisplay::LayeredSvgDisplay(const QString &svgPath, QWidget *parent)
    : QWidget(parent)
{
    m_svgRenderer = new QSvgRenderer(svgPath, this);
    setAttribute(Qt::WA_TranslucentBackground);
}

void LayeredSvgDisplay::setLayers(const std::vector<Layer> &layers)
{
    m_layers = layers;
    // Resize the animation tracking vector to match the number of layers
    m_activeAnimations.resize(layers.size(), nullptr);
    update();
}

void LayeredSvgDisplay::setLayerState(size_t index, QPointF offset, double scale)
{
    if (index >= m_layers.size()) return;
    
    // If we are forcing a snap, kill any running animations for this layer
    if (m_activeAnimations[index]) {
        m_activeAnimations[index]->stop();
    }

    m_layers[index].offset = offset;
    m_layers[index].scale = scale;
    update();
}

void LayeredSvgDisplay::animateLayerTo(size_t index, QPointF targetOffset, double targetScale, int durationMs)
{
    if (index >= m_layers.size()) return;

    // 1. Interrupt existing animation to prevent jitter
    if (m_activeAnimations[index]) {
        m_activeAnimations[index]->stop();
        m_activeAnimations[index]->deleteLater();
    }

    // 2. Create a new animation group for this specific layer
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    m_activeAnimations[index] = group;

    // Animate the Offset
    QVariantAnimation *offsetAnim = new QVariantAnimation(group);
    offsetAnim->setStartValue(m_layers[index].offset);
    offsetAnim->setEndValue(targetOffset);
    offsetAnim->setDuration(durationMs);
    offsetAnim->setEasingCurve(QEasingCurve::OutBack);
    
    connect(offsetAnim, &QVariantAnimation::valueChanged, this, [this, index](const QVariant &value){
        m_layers[index].offset = value.toPointF();
        update();
    });

    // Animate the Scale
    QVariantAnimation *scaleAnim = new QVariantAnimation(group);
    scaleAnim->setStartValue(m_layers[index].scale);
    scaleAnim->setEndValue(targetScale);
    scaleAnim->setDuration(durationMs);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    
    connect(scaleAnim, &QVariantAnimation::valueChanged, this, [this, index](const QVariant &value){
        m_layers[index].scale = value.toDouble();
        // Note: No need to call update() twice, the offset animation triggers it.
    });

    group->addAnimation(offsetAnim);
    group->addAnimation(scaleAnim);

    // Clean up pointer when done
    connect(group, &QAbstractAnimation::finished, this, [this, index, group]() {
        if (m_activeAnimations[index] == group) m_activeAnimations[index] = nullptr;
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void LayeredSvgDisplay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_svgRenderer || !m_svgRenderer->isValid() || m_layers.empty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QSize targetSize = this->size();
    
    // --- Aspect Ratio & Centering Calculation ---
    QSize svgOriginalSize = m_svgRenderer->defaultSize();
    QSize scaledSize = svgOriginalSize.scaled(targetSize, Qt::KeepAspectRatio);
    
    QRectF renderRect(
        (targetSize.width() - scaledSize.width()) / 2.0,
        (targetSize.height() - scaledSize.height()) / 2.0,
        scaledSize.width(),
        scaledSize.height()
    );

    QImage svgMask(targetSize, QImage::Format_ARGB32_Premultiplied);
    svgMask.fill(Qt::transparent);
    
    QPainter maskPainter(&svgMask);
    maskPainter.setRenderHint(QPainter::Antialiasing);
    
    // Render into the aspect-corrected rectangle instead of the full targetSize
    m_svgRenderer->render(&maskPainter, renderRect);
    maskPainter.end();

    for (const auto& layer : m_layers) {
        painter.save();

        painter.translate(layer.offset);
        painter.translate(targetSize.width() / 2.0, targetSize.height() / 2.0);
        painter.scale(layer.scale, layer.scale);
        painter.translate(-targetSize.width() / 2.0, -targetSize.height() / 2.0);

        QImage coloredImage(targetSize, QImage::Format_ARGB32_Premultiplied);
        coloredImage.fill(Qt::transparent);
        
        QPainter coloredPainter(&coloredImage);
        coloredPainter.fillRect(coloredImage.rect(), layer.color);
        coloredPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        coloredPainter.drawImage(0, 0, svgMask);
        coloredPainter.end();

        painter.drawImage(0, 0, coloredImage);
        painter.restore(); 
    }
}
