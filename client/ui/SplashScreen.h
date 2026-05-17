#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QTimer;
class QWidget;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

namespace cypherush {

class SplashScreen : public QWidget {
    Q_OBJECT

public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen() override;

    void startAnimation();

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onScrambleTick();
    void onCipherPhaseEnd();
    void onHushEnd();

private:
    QChar randomCipherChar() const;

    QWidget* m_content = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_taglineLabel = nullptr;
    QTimer* m_scrambleTimer = nullptr;
    QPropertyAnimation* m_rushAnimation = nullptr;
    QGraphicsOpacityEffect* m_taglineOpacity = nullptr;
    QPropertyAnimation* m_taglineFadeAnim = nullptr;

    int m_tickCount = 0;
    int m_lockedLetters = 0;
    const QString m_finalText = QStringLiteral("CYPHERUSH");
    const QString m_cipherChars = QStringLiteral("@#$%&*?!<>+=~/\\|");
};

} // namespace cypherush
