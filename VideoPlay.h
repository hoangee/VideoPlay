#pragma once

#include <QtWidgets/QWidget>
#include "ui_VideoPlay.h"

class VideoPlay : public QWidget
{
    Q_OBJECT

public:
    VideoPlay(QWidget *parent = Q_NULLPTR);

private:
    Ui::VideoPlayClass ui;
};
