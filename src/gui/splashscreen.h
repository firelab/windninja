#include <QSplashScreen>
#include <QTimer>
#include <QStringList>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class SplashScreen : public QSplashScreen {
    Q_OBJECT

public:
    SplashScreen(const QPixmap &pixmap, QStringList list, int time);
    void display();

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void done();

private slots:
    void update();
    void fadeOut();
    void onFadeOutFinished();
    void onFadeInFinished();

private:
    QStringList messages;
    int messageTime, numMessages, i, j, messageFadeInterval;
    Qt::Alignment alignment;
    QPixmap map;
    QTimer *messageTimer;
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation *fadeInAnimation;
    QPropertyAnimation *fadeOutAnimation;
    bool SplashScreenDone;
};
