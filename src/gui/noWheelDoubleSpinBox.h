#ifndef NoWheelDoubleSpinBox_H
#define NoWheelDoubleSpinBox_H

#pragma once

#include <QSpinBox>
#include <QWheelEvent>

class NoWheelDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT
public:
    explicit NoWheelDoubleSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent) {}

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (!hasFocus()) {
            event->ignore();
            return;
        }
        QDoubleSpinBox::wheelEvent(event);
    }
};

#endif // NoWheelDoubleSpinBox_H
