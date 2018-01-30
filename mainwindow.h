#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDebug>
#include <QTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStringList>
#include "lasthread.h"
#include <sqlite3.h>


//inline static int callback(void *, int, char **, char **);


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QFileInfo kFilei, kFileout;
    QString sqlitetable="";
    int *callback(void *NotUsed, int argc, char **argv,char **azColName);
    ~MainWindow();

public Q_SLOTS:
    void launch();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void logit(int type, QString value);

private slots:
    void setSlideValue(double v);
    void setSpinnerValue(int v);
private:
    Ui::MainWindow *ui;



};




#endif // MAINWINDOW_H
