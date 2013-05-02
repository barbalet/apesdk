#include <QApplication>
#include "mainwindow.h"
#include "../noble/noble.h"

int main(int argc, char *argv[])
{
    io_entry_execution(argc, argv);

    QApplication a(argc, argv);
    MainWindow w;


    w.show();
    
    return a.exec();
}
