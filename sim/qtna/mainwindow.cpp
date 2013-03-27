#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /* set some default values */
    initialised = false;
    window_updated = 0xFF;
    firedown = 0;
    firecontrol = 0;
    current_display = WND_MAP;
    next_display = -1;
    current_filename="";
    clear_graph = 1;

    for (int i = 0; i < NUM_WINDOWS; i++)
    {
        image_item[i] = NULL;
        image_scene[i] = NULL;
        image[i] = NULL;
    }

    img_graph = NULL;

    ui->setupUi(this);

    this->show();
    init();

    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(closeApp()));
    connect(ui->actionViewMap,SIGNAL(triggered()),this,SLOT(menuViewMap()));
    connect(ui->actionViewTerrain,SIGNAL(triggered()),this,SLOT(menuViewTerrain()));
    connect(ui->actionViewIdeosphere,SIGNAL(triggered()),this,SLOT(menuViewIdeosphere()));
    connect(ui->actionViewBraincode,SIGNAL(triggered()),this,SLOT(menuViewBraincode()));
    connect(ui->actionViewGenepool,SIGNAL(triggered()),this,SLOT(menuViewGenepool()));
    connect(ui->actionViewHonor,SIGNAL(triggered()),this,SLOT(menuViewHonor()));
    connect(ui->actionViewPathogens,SIGNAL(triggered()),this,SLOT(menuViewPathogens()));
    connect(ui->actionViewRelationships,SIGNAL(triggered()),this,SLOT(menuViewRelationships()));
    connect(ui->actionViewPreferences,SIGNAL(triggered()),this,SLOT(menuViewPreferences()));
    connect(ui->actionViewPhasespace,SIGNAL(triggered()),this,SLOT(menuViewPhasespace()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(menuSave()));
    connect(ui->actionSaveAs,SIGNAL(triggered()),this,SLOT(menuSaveAs()));
    connect(ui->actionNew,SIGNAL(triggered()),this,SLOT(menuNew()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(menuOpen()));
    connect(ui->actionOpenScript,SIGNAL(triggered()),this,SLOT(menuOpenScript()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(menuAbout()));
    connect(ui->actionPause,SIGNAL(triggered()),this,SLOT(menuControlPause()));
    connect(ui->actionNext,SIGNAL(triggered()),this,SLOT(menuControlNext()));
    connect(ui->actionPrevious,SIGNAL(triggered()),this,SLOT(menuControlPrevious()));
    connect(ui->actionTerritory,SIGNAL(triggered()),this,SLOT(menuControlTerritory()));
    connect(ui->actionWeather,SIGNAL(triggered()),this,SLOT(menuControlWeather()));
    connect(ui->actionShowBrain,SIGNAL(triggered()),this,SLOT(menuControlShowBrain()));
    connect(ui->actionShowBraincode,SIGNAL(triggered()),this,SLOT(menuControlShowBraincode()));
    connect(ui->actionBrainNormal,SIGNAL(triggered()),this,SLOT(menuControlShowBrainNormal()));
    connect(ui->actionBrainFear,SIGNAL(triggered()),this,SLOT(menuControlShowBrainFear()));
    connect(ui->actionBrainDesire,SIGNAL(triggered()),this,SLOT(menuControlShowBrainDesire()));
    connect(ui->actionFlood,SIGNAL(triggered()),this,SLOT(menuControlFlood()));
    connect(ui->actionHealthyCarrier,SIGNAL(triggered()),this,SLOT(menuControlHealthyCarrier()));

    ui->graphicsView->installEventFilter(this);
    ui->graphicsView->setMouseTracking(true);

    initialised = true;
}

MainWindow::~MainWindow()
{
    if (io_disk_check("NobleApeAutoload.txt") == 1)
    {
        unsigned long		buff_len;
        unsigned char * buff = sim_fileout(&buff_len);
        FILE          * outputfile = fopen("NobleApeAutoload.txt","w");
        fwrite(buff, buff_len, 1, outputfile);
        fclose(outputfile);
        io_free(buff);
    }

    sim_close();
    for (int i = 0; i < NUM_WINDOWS; i++)
    {
        if (image_item[i] != NULL) delete image_item[i];
        if (image_scene[i] != NULL) delete image_scene[i];
        if (image[i] != NULL) delete image[i];
    }
    free(img_graph);

    delete ui;
}

/* closes the application */
void MainWindow::closeApp()
{
    shared_close();
    close();
}

void MainWindow::init()
{
    /* Now, get the location of the graphics buffers */
    local_buffer = (unsigned char *) control_init(KIND_START_UP, time(NULL));

    /* RGB image used for additional graphs */
    img_graph = (unsigned char *)malloc(WND_WIDTH_MAP*WND_HEIGHT_MAP*3);

    refresh();

    connect(&simTimer, SIGNAL(timeout()), SLOT(slotTimeout()));
    simTimer.start(TIMER_RATE_MSEC);
}

void MainWindow::menuAbout()
{
    shared_about("PC INTEL Qt");
}

/* toggle pause */
void MainWindow::menuControlPause()
{
    shared_notPause();
    ui->actionPause->setChecked(ui->actionPause->isChecked());
}

void MainWindow::menuControlPrevious()
{
    shared_previousApe();
}

void MainWindow::menuControlNext()
{
    shared_nextApe();
}

void MainWindow::menuControlTerritory()
{
    shared_notTerritory();
    ui->actionTerritory->setChecked(ui->actionTerritory->isChecked());
}

void MainWindow::menuControlWeather()
{
    shared_notWeather();
    ui->actionWeather->setChecked(ui->actionWeather->isChecked());
}

void MainWindow::menuControlShowBrain()
{
    shared_notBrain();
    ui->actionShowBrain->setChecked(ui->actionShowBrain->isChecked());
}

void MainWindow::menuControlShowBraincode()
{
    shared_notBrainCode();
    ui->actionShowBraincode->setChecked(ui->actionShowBraincode->isChecked());
}

void MainWindow::menuControlShowBrainNormal()
{
    shared_brainDisplay(3);
    ui->actionBrainDesire->setChecked(false);
    ui->actionBrainNormal->setChecked(true);
    ui->actionBrainFear->setChecked(false);
}

void MainWindow::menuControlShowBrainFear()
{
    shared_brainDisplay(1);
    ui->actionBrainDesire->setChecked(false);
    ui->actionBrainFear->setChecked(true);
    ui->actionBrainNormal->setChecked(false);
}

void MainWindow::menuControlShowBrainDesire()
{
    shared_brainDisplay(2);
    ui->actionBrainDesire->setChecked(true);
    ui->actionBrainFear->setChecked(false);
    ui->actionBrainNormal->setChecked(false);
}

void MainWindow::menuControlFlood()
{
    QMessageBox::StandardButton reply;
    reply =
        QMessageBox::question(this,
                              tr("Great Flood"),
                                 "Many apes will drown.  Do you wish to continue?",
                                 QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        shared_flood();
    }
}

void MainWindow::menuControlHealthyCarrier()
{
    shared_healthy_carrier();
}

/* reset the simulation */
void MainWindow::menuNew()
{
    QMessageBox::StandardButton reply;
    reply =
        QMessageBox::question(this,
                              tr("New Simulation"),
                                 "Do you really want create a new simulation?",
                                 QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        shared_new(clock());
    }
}

/* open an existing simulation file */
int MainWindow::menuOpen()
{
    QString filename =
        QFileDialog::getOpenFileName(this,
            tr("Open Simulation File"),
            QDir::homePath(),
            tr("Noble Ape Files (*.txt)"));

    if (filename.length() == 0) return -1;

    if (!shared_openFileName((n_string)filename.toStdString().c_str(),0))
    {
        QMessageBox::information(this, "Open simulation file","Unable to read from file!");
    }

    return 1;
}

/* open an existing apescript file */
int MainWindow::menuOpenScript()
{
    QString filename =
        QFileDialog::getOpenFileName(this,
            tr("Open Apescript File"),
            QDir::homePath(),
            tr("Apescript Files (*.ape)"));

    if (filename.length() == 0) return -1;

    if (!shared_openFileName((n_string)filename.toStdString().c_str(),1))
    {
        QMessageBox::information(this, "Open apescript file","Unable to read from file!");
    }
    shared_notPause();

    return 1;
}

/* save the simulation */
unsigned char MainWindow::menuSave()
{
    if (current_filename == "") return menuSaveAs();
    shared_saveFileName((n_string)current_filename.toStdString().c_str());
    return 1;
}

/* save the simulation with a filename */
unsigned char MainWindow::menuSaveAs()
{
    QString filename =
        QFileDialog::getSaveFileName(this,
            tr("Enter a filename to save as"),
            QDir::homePath(),
            tr("Noble Ape files (*.txt)"));

    /* if no filename was given */
    if (filename.length()==0) return 0;

    current_filename = filename;
    shared_saveFileName((n_string)filename.toStdString().c_str());
    return 1;
}

/* creates a palette of 256 colours */
void MainWindow::createPalette()
{
    draw_fit(land_points, fit);

    palette.clear();
    for (int i = 0; i < 256; i++)
    {
        palette.push_back(qRgb(((fit[i * 3 + 0] >> 8)), ((fit[i * 3 + 1] >> 8)), (fit[i * 3 + 2] >> 8)));
    }
}

/* a single simulation step */
bool MainWindow::refresh()
{
    QGraphicsView * g = ui->graphicsView;
    unsigned char * img = NULL;
    QImage::Format format = QImage::Format_Indexed8;
    int img_width=0,img_height=0;
    bool create_palette = false;

    if (next_display > -1)
    {
        current_display = next_display;
        next_display = -1;
    }

    sim_thread_console();

    if (firedown != 0)
    {
        control_mouse((n_byte)(firedown - 1), fire_x, fire_y, firecontrol);
    }
    control_simulate((60*clock())/CLOCKS_PER_SEC);
    window_updated = 0;

    if (sim_thread_console_quit())
    {
        exit(0);
    }

    switch(current_display)
    {
    case WND_MAP:
    {
        img = (unsigned char*)VIEWWINDOW(local_buffer);
        format = QImage::Format_Indexed8;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        create_palette = true;
        break;
    }
    case WND_TERRAIN:
    {
        img = (unsigned char*)TERRAINWINDOW(local_buffer);
        format = QImage::Format_Indexed8;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        create_palette = true;
        break;
    }
    case WND_IDEOSPHERE:
    {
        graph_ideosphere(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_BRAINCODE:
    {
        graph_braincode(sim_sim(), &(sim_sim()->beings[sim_sim()->select]), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP,clear_graph);
        clear_graph = 0;
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_GENEPOOL:
    {
        graph_genepool(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_HONOR:
    {
        graph_honor_distribution(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_PATHOGENS:
    {
        graph_pathogens(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_RELATIONSHIPS:
    {
        graph_relationship_matrix(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_PREFERENCES:
    {
        graph_preferences(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    case WND_PHASESPACE:
    {
        graph_phasespace(sim_sim(), img_graph, WND_WIDTH_MAP, WND_HEIGHT_MAP,0,0);
        img = img_graph;
        format = QImage::Format_RGB888;
        img_width = WND_WIDTH_MAP;
        img_height = WND_HEIGHT_MAP;
        break;
    }
    }

    if (img == NULL)
    {
        qDebug("%s", "WARNING: No image was specified");
        return false;
    }

    if (image[current_display]==NULL)
    {
        /* allocate a new image */
        image[current_display] =
            new QImage(img, img_width, img_height, format);
    }
    else
    {
        /* copy data into the existing image */
        for (int y = 0; y < image[current_display]->height(); y++)
        {
            memcpy((void*)image[current_display]->scanLine(y),
                   (void*)&img[y*image[current_display]->bytesPerLine()],
                   image[current_display]->bytesPerLine());
        }
    }

    if (create_palette)
    {
        createPalette();
        image[current_display]->setColorTable(palette);
        image[current_display]->setColorCount(256);
    }

    if (image_scene[current_display] != NULL)
    {
        /* free previous allocations */
        delete image_item[current_display];
        delete image_scene[current_display];
        image_scene[current_display] = NULL;
        image_item[current_display] = NULL;
    }

    /* Allocating each time seems to be needed to refresh the image.
       Maybe there is a better way */
    image_scene[current_display] = new QGraphicsScene();
    image_item[current_display] = new QGraphicsPixmapItem(QPixmap::fromImage(*image[current_display]));
    image_scene[current_display]->setSceneRect(image[current_display]->rect());
    g->setScene(image_scene[current_display]);
    g->scene()->addItem(image_item[current_display]);

    g->fitInView(image[current_display]->rect(), Qt::KeepAspectRatio);
    g->show();

    return true;
}

/* Mouse click events */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->graphicsView)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {            
            int x = ((QMouseEvent*)event)->pos().x();
            int y = ((QMouseEvent*)event)->pos().y();
            QString str = "Pos " + QString::number(x) + " " + QString::number(y);
            qDebug("%s", str.toStdString().c_str());
        }

    }
    return false;
}

/* Form resized */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!initialised) return;
    refresh();
}

/* Timer event */
void MainWindow::slotTimeout()
{
    if (!initialised) return;
    refresh();
}

void MainWindow::menuViewMap()
{
    menuView(WND_MAP,0);
}

void MainWindow::menuViewTerrain()
{
    menuView(WND_TERRAIN,0);
}

void MainWindow::menuViewIdeosphere()
{
    menuView(WND_IDEOSPHERE,0);
}

void MainWindow::menuViewBraincode()
{
    menuView(WND_BRAINCODE,1);
}

void MainWindow::menuViewGenepool()
{
    menuView(WND_GENEPOOL,0);
}

void MainWindow::menuViewHonor()
{
    menuView(WND_HONOR,0);
}

void MainWindow::menuViewPathogens()
{
    menuView(WND_PATHOGENS,0);
}

void MainWindow::menuViewRelationships()
{
    menuView(WND_RELATIONSHIPS,0);
}

void MainWindow::menuViewPreferences()
{
    menuView(WND_PREFERENCES,0);
}

void MainWindow::menuViewPhasespace()
{
    menuView(WND_PHASESPACE,0);
}


void MainWindow::menuView(int display, int clear)
{
    next_display = display;
    clear_graph = clear;
}
