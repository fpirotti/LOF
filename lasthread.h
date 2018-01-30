#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QDebug>
#include <QProgressBar>
#include <QMessageBox>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/filters/lof.h>
#include <pcl/exceptions.h>
#include "sqlite3.h"

class lasthread: public QObject
{
    Q_OBJECT
    void las2pcd();
    void sqlite2pcd();
    LASreader *lasreader;

public:
    explicit lasthread(QObject *parent = nullptr);
    QFileInfo filei, fileout;
    QProgressBar *pbar;
    bool negative = true;
    QString table, tx, ty,tz;
    bool getlof = false;
    int k;
    float threshold;

public slots:
    void process();

Q_SIGNALS:
    void progress(int value);
    void message(int type, QString str);
};

#endif // THREAD_H
