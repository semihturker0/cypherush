#include "ui/SplashScreen.h"

#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QKeySequence>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QScreen>
#include <QShortcut>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

namespace cypherush {

namespace {

constexpr int kWidth = 1400;
constexpr int kHeight = 360;
constexpr int kTickMs = 60;
constexpr int kTicksPerLetter = 4;   // ~240ms per locked letter
constexpr int kCipherPhaseMs = 2200;
constexpr int kHushMs = 700;
constexpr int kRushMs = 500;

QString titleQss(const char* color) {
    return QStringLiteral(
               "QLabel#splashTitle {"
               "  background: transparent;"
               "  font-family: \"Consolas\", \"Courier New\", monospace;"
               "  font-size: 144px;"
               "  font-weight: 800;"
               "  letter-spacing: 6px;"
               "  color: %1;"
               "  border: none;"
               "  padding: 0px;"
               "  margin: 0px;"
               "}")
        .arg(QString::fromLatin1(color));
}

} // namespace

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    setObjectName(QStringLiteral("CypherushSplash"));
    setStyleSheet(QStringLiteral(
        "QWidget#CypherushSplash { background: transparent; }"
        "QLabel { background: transparent; border: none; }"));
    setFixedSize(kWidth, kHeight);

    m_content = new QWidget(this);
    m_content->setObjectName(QStringLiteral("CypherushSplashContent"));
    m_content->setGeometry(0, 0, kWidth, kHeight);
    m_content->setAttribute(Qt::WA_TranslucentBackground);
    m_content->setStyleSheet(QStringLiteral(
        "QWidget#CypherushSplashContent { background: transparent; }"));

    auto* layout = new QVBoxLayout(m_content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);
    layout->addStretch();

    m_titleLabel = new QLabel(QStringLiteral("         "), m_content);
    m_titleLabel->setObjectName(QStringLiteral("splashTitle"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Preferred);
    m_titleLabel->setStyleSheet(titleQss("#B388FF"));
    layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    m_taglineLabel =
        new QLabel(QStringLiteral("Speak in cipher, hear in hush"),
                   m_content);
    m_taglineLabel->setObjectName(QStringLiteral("splashTagline"));
    m_taglineLabel->setAlignment(Qt::AlignCenter);
    m_taglineLabel->setStyleSheet(QStringLiteral(
        "QLabel#splashTagline {"
        "  background: transparent;"
        "  font-family: \"Segoe UI\", sans-serif;"
        "  font-size: 22px;"
        "  font-weight: 500;"
        "  color: #FFFFFF;"
        "  border: none;"
        "}"));
    layout->addWidget(m_taglineLabel, 0, Qt::AlignCenter);

    layout->addStretch();

    m_taglineOpacity = new QGraphicsOpacityEffect(m_taglineLabel);
    m_taglineOpacity->setOpacity(0.0);
    m_taglineLabel->setGraphicsEffect(m_taglineOpacity);

    m_scrambleTimer = new QTimer(this);
    m_scrambleTimer->setInterval(kTickMs);
    connect(m_scrambleTimer, &QTimer::timeout, this,
            &SplashScreen::onScrambleTick);

    auto* escSkip =
        new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escSkip, &QShortcut::activated, this, [this]() {
        if (m_scrambleTimer) {
            m_scrambleTimer->stop();
        }
        if (m_rushAnimation) {
            m_rushAnimation->stop();
        }
        emit animationFinished();
        close();
    });
}

void SplashScreen::paintEvent(QPaintEvent* /*event*/) {
    // Explicitly draw nothing — fully transparent background.
    // Intentionally NOT calling QWidget::paintEvent(), which would
    // paint the default widget background.
}

SplashScreen::~SplashScreen() = default;

void SplashScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect screenGeom = screen->availableGeometry();
        move(screenGeom.center() - rect().center());
    }
}

void SplashScreen::startAnimation() {
    m_scrambleTimer->start();
    QTimer::singleShot(kCipherPhaseMs, this,
                       &SplashScreen::onCipherPhaseEnd);
}

QChar SplashScreen::randomCipherChar() const {
    const int idx =
        QRandomGenerator::global()->bounded(m_cipherChars.length());
    return m_cipherChars.at(idx);
}

void SplashScreen::onScrambleTick() {
    ++m_tickCount;
    m_lockedLetters = m_tickCount / kTicksPerLetter;
    if (m_lockedLetters > m_finalText.length()) {
        m_lockedLetters = m_finalText.length();
    }

    QString display;
    for (int i = 0; i < m_finalText.length(); ++i) {
        if (i < m_lockedLetters) {
            display += m_finalText.at(i);
        } else {
            display += randomCipherChar();
        }
    }
    m_titleLabel->setText(display);
}

void SplashScreen::onCipherPhaseEnd() {
    m_scrambleTimer->stop();

    m_titleLabel->setText(m_finalText);
    m_titleLabel->setStyleSheet(titleQss("#5FFBC7"));

    m_taglineFadeAnim =
        new QPropertyAnimation(m_taglineOpacity, "opacity", this);
    m_taglineFadeAnim->setDuration(400);
    m_taglineFadeAnim->setStartValue(0.0);
    m_taglineFadeAnim->setEndValue(1.0);
    m_taglineFadeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(kHushMs, this, &SplashScreen::onHushEnd);
}

void SplashScreen::onHushEnd() {
    m_rushAnimation = new QPropertyAnimation(m_content, "pos", this);
    m_rushAnimation->setDuration(kRushMs);
    m_rushAnimation->setStartValue(QPoint(0, 0));
    m_rushAnimation->setEndValue(QPoint(1400, 0));
    m_rushAnimation->setEasingCurve(QEasingCurve::InCubic);
    connect(m_rushAnimation, &QPropertyAnimation::finished, this,
            [this]() {
                emit animationFinished();
                close();
            });
    m_rushAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace cypherush
