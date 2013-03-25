#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    window_updated = 0xFF;

    firedown = 0;
    firecontrol = 0;
    current_display = WND_MAP;

    this->show();
    init();

    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(close()));
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
    for (int i = 0; i < 2; i++) {
        delete image_item[i];
        delete image_scene[i];
        delete image[i];
    }

    delete ui;
}

void MainWindow::init()
{
    int i = 0;
    unsigned short fit[256*3];

    /* Let's create the palette for terrain drawing */

    draw_fit(land_points, fit);

    /* Now, get the location of the graphics buffers */
    local_buffer = (unsigned char *) control_init(KIND_START_UP, time(NULL));

    for (i = 0; i < 256; i++)
    {
        palette.push_back(qRgb(((fit[i * 3 + 0] >> 8)), ((fit[i * 3 + 1] >> 8)), (fit[i * 3 + 2] >> 8)));
    }

    QGraphicsView * g = ui->graphicsView;

    /* create an image */
    for (i = 0; i < NUM_WINDOWS; i++)
    {
        switch(i) {
        case WND_MAP: {
            image[WND_MAP] = new QImage((unsigned char*)VIEWWINDOW(local_buffer), WND_WIDTH_MAP, WND_HEIGHT_MAP, QImage::Format_Indexed8);
            break;
        }
        case WND_TERRAIN: {
            image[WND_TERRAIN] = new QImage((unsigned char*)TERRAINWINDOW(local_buffer), WND_WIDTH_MAP, WND_HEIGHT_MAP, QImage::Format_Indexed8);
            break;
        }
        }

        if (!image[i]->isNull()) {
            if ((i == WND_MAP) || (i == WND_TERRAIN)) {
                image[i]->setColorTable(palette);
                image[i]->setColorCount(256);
            }

            image_scene[i] = new QGraphicsScene();
            image_item[i] = new QGraphicsPixmapItem(QPixmap::fromImage(*image[i]));
            image_scene[i]->setSceneRect(image[i]->rect());

            if (i == current_display) {
                g->setScene(image_scene[i]);
                g->scene()->addItem(image_item[i]);
                g->fitInView(image[i]->rect(), Qt::KeepAspectRatio);
                g->show();
            }
        }
    }
}

// reset the simulation
void MainWindow::resetSim()
{
    control_init(KIND_NEW_SIMULATION,clock());
}

// save the simulation
unsigned char MainWindow::file_save()
{
    unsigned long buff_len;
    unsigned char* buff;
    FILE* file;

    buff = sim_fileout(&buff_len);

    file = fopen(current_file_name,"w");

    if (file == NULL)
    {
        QMessageBox::information(this, this->windowTitle(),"Unable to open file for writing!");
        return 0;
    }

    if (fwrite(buff,sizeof(unsigned char), buff_len, file) != buff_len)
    {
        QMessageBox::information(this, this->windowTitle(),"Unable to write!");
        return 0;
    }

    fclose(file);

    io_free(buff);

    return 1;
}

// save the simulation with a filename
unsigned char MainWindow::file_save_as()
{
    QString filename =
        QFileDialog::getSaveFileName(this,
            tr("Enter a filename to save as"),
            QDir::homePath(),
            tr("Noble Ape files (*.txt)"));

    // if no filename was given
    if (filename.length()==0) return 0;

    sprintf(current_file_name,"%s",filename.toStdString().c_str());

    file_save();

    return 1;
}

// a single simulation step
bool MainWindow::refresh()
{
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

    return true;
}

// Mouse click events
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->graphicsView && event->type() == QEvent::MouseButtonPress)
    {

    }

    return false;
}

// Form resized
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!initialised) return;
    QGraphicsView * g = ui->graphicsView;

    g->setScene(image_scene[current_display]);
    g->fitInView(image[current_display]->rect(), Qt::KeepAspectRatio);
    g->show();
}
