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
#include <QMouseEvent>

extern "C" {
#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../gui/gui.h"
#include "../gui/shared.h"
#include "../command/command.h"
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

enum {
    WND_TERRAIN = 0,
    WND_MAP,
    WND_IDEOSPHERE,
    WND_BRAINCODE,
    WND_GENEPOOL,
    WND_HONOR,
    WND_PATHOGENS,
    WND_RELATIONSHIPS,
    WND_PREFERENCES,
    WND_PHASESPACE,
    NUM_WINDOWS
};

#define TIMER_RATE_MSEC        (1000/120)

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
    unsigned char * img_graph;
    QVector<QRgb> palette;
    unsigned char window_updated;
    int clear_graph;

    QString current_filename;

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

    void resizeEvent(QResizeEvent *event);

protected slots:
    unsigned char menuSaveAs();
    unsigned char menuSave();
    bool eventFilter(QObject *obj, QEvent *event);
    void closeApp();
    void menuNew();
    int menuOpen();
    int menuOpenScript();
    void menuAbout();
    void menuControlPause();
    void menuControlPrevious();
    void menuControlNext();
    void menuControlTerritory();
    void menuControlWeather();
    void menuControlShowBrain();
    void menuControlShowBraincode();
    void menuControlShowBrainNormal();
    void menuControlShowBrainFear();
    void menuControlShowBrainDesire();
    void menuControlFlood();
    void menuControlHealthyCarrier();

    /* Why aren't these all just parameterised in a single function?
       It's a quirk of Qt which doesn't allow more slot parameters
       than signal parameters */
    void menuView(int display, int clear);
    void menuViewMap();
    void menuViewTerrain();
    void menuViewIdeosphere();
    void menuViewBraincode();
    void menuViewGenepool();
    void menuViewHonor();
    void menuViewPathogens();
    void menuViewRelationships();
    void menuViewPreferences();
    void menuViewPhasespace();

    void slotTimeout();
    void createPalette();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
