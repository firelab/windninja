#include "mainWindow.h"
#include "windninja.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStyleFactory>
#include <QTimer>
#include <QEventLoop>
#include <QMouseEvent>

int main(int argc, char *argv[])
{
    setbuf(stdout, nullptr);
    QApplication a(argc, argv);

    // Initialize as a GUI run
    char ** options = NULL;
    NinjaErr ninjaErr = NinjaInit("gui", options);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaInit: ninjaErr =" << ninjaErr;
    }
    
    // Set icon and application settings
    QIcon icon(":/wn-icon.png");
    QString ver = NINJA_VERSION_STRING;
    a.setWindowIcon(icon);
    a.setApplicationName(QString("WindNinja"));
    a.setApplicationVersion(ver);
    a.setStyle(QStyleFactory::create("Fusion"));

    // Initialize mainwindow (for the version message)
    MainWindow* w = nullptr;
    try {
        w = new MainWindow;
    } catch (...) {
        return 1;
    }

    // Set the Splash Screen
    QPixmap pix(":wn-splash.png");
    QSplashScreen *splash = new QSplashScreen(pix.scaled(1200, 320, Qt::KeepAspectRatioByExpanding));
    
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(splash);
    splash->setGraphicsEffect(effect);
    splash->show();

    // Fade In Animation
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(800);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    // Messages and Timer
    QStringList messages;
    messages << "Loading WindNinja " + ver + "..."
             << "Loading mesh generator..."
             << "Loading conjugate gradient solver..."
             << "Loading preconditioner..."
             << "WindNinja " + ver + " loaded.";
    int messageIndex = 0;
    QTimer messageTimer;
    
    QObject::connect(&messageTimer, &QTimer::timeout, [&]() {
        if (messageIndex < messages.size()) {
            splash->showMessage(messages[messageIndex], 
                                Qt::AlignBottom | Qt::AlignLeft, 
                                Qt::gray);
            messageIndex++;
        } else {
            messageTimer.stop();
        }
    });
    messageTimer.start(600); 

    // Event Loop for splash screen
    QEventLoop loop;
    QTimer::singleShot(3500, &loop, &QEventLoop::quit);

    // Detect user click and skip splash screen
    QObject::connect(splash, &QSplashScreen::messageChanged, [&](){
        if (!splash->isVisible()) loop.quit();
    });

    loop.exec();

    // Transition from fade in to fade out
    messageTimer.stop();
    if (splash->isVisible()) {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
        fadeOut->setDuration(400); // Shorter fade for transition
        fadeOut->setStartValue(effect->opacity());
        fadeOut->setEndValue(0.0);
        
        QObject::connect(fadeOut, &QPropertyAnimation::finished, [w, splash]() {
            w->show();
            splash->finish(w);
            splash->deleteLater();
        });
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        w->show();
        splash->deleteLater();
    }

    return a.exec();
}