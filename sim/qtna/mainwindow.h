#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef _NOBLEAPE_PLATFORM_H_
#define _NOBLEAPE_PLATFORM_H_

#define NOBLE_APE_FILE_EXTN      ".txt"

#endif

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QVector>
#include <QRgb>
#include <QTimer>

extern "C" {
#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../gui/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <assert.h>

/* Operating system includes */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
}

#define WND_HEIGHT_MAP         512
#define WND_WIDTH_MAP          512

#define WND_TERRAIN  0
#define WND_MAP      1

#define NUM_WINDOWS  2

#define TIMER_RATE_MSEC 10

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    int current_display, next_display;

    unsigned char check;
    unsigned char* local_buffer;
    QVector<QRgb> palette;
    unsigned char window_updated;

    char current_file_name[256];

    unsigned char firedown;
    int firecontrol;
    int fire_x, fire_y;

    QImage * image[NUM_WINDOWS];
    QGraphicsScene * image_scene[NUM_WINDOWS];
    QGraphicsPixmapItem * image_item[NUM_WINDOWS];

    void init();
    bool refresh();

private:
    QTimer simTimer;
    bool initialised;
    unsigned short fit[256*3];

    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);

protected slots:
    unsigned char menuSaveAs();
    unsigned char menuSave();

    void resetSim();
    void slotTimeout();
    void createPalette();
    void menuViewMap();
    void menuViewTerrain();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
