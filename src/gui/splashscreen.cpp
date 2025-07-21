#include "splashscreen.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

SplashScreen::SplashScreen(const QPixmap &pixmap, QStringList list, int time)
    : QSplashScreen(pixmap), messages(list), messageTime(time), i(0), j(0), SplashScreenDone(false)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen);

    numMessages = messages.size();
    messageFadeInterval = time;
    alignment = Qt::AlignLeft | Qt::AlignBottom;
    map = pixmap;

    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0.0);

    fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeInAnimation->setDuration(1000);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    connect(fadeInAnimation, &QPropertyAnimation::finished, this, &SplashScreen::onFadeInFinished);

    fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnimation->setDuration(1000);
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);
    connect(fadeOutAnimation, &QPropertyAnimation::finished, this, &SplashScreen::onFadeOutFinished);

    messageTimer = new QTimer(this);
    connect(messageTimer, &QTimer::timeout, this, &SplashScreen::update);
}

void SplashScreen::display()
{
    show();
    fadeInAnimation->start();
}

void SplashScreen::update()
{
    if (SplashScreenDone || ++i >= numMessages) {
        messageTimer->stop();
        fadeOut();
        return;
    }

    clearMessage();
    showMessage(messages.value(i), alignment, Qt::gray);
    QApplication::processEvents();
}

void SplashScreen::fadeOut()
{
    fadeOutAnimation->start();
}

void SplashScreen::onFadeInFinished()
{
    showMessage(messages.value(0), alignment, Qt::gray);
    messageTimer->start(messageTime);
}

void SplashScreen::onFadeOutFinished()
{
    if (!SplashScreenDone) {
        emit done();
    }
    close();
}

void SplashScreen::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        SplashScreenDone = true;

        messageTimer->stop();

        fadeInAnimation->stop();
        fadeOutAnimation->stop();
        fadeOutAnimation->disconnect(this);

        emit done();
        close();
    }
}
