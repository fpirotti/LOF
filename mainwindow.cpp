#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this->ui->tspin, SIGNAL(valueChanged(double)) , this, SLOT(setSlideValue(double)) );
    connect(this->ui->threshold, SIGNAL(valueChanged(int)) , this, SLOT(setSpinnerValue(int)) );
    connect(this->ui->buttkeep, SIGNAL(released()) , this, SLOT(launch()) );
    connect(this->ui->buttremove , SIGNAL(released()) , this, SLOT(launch()) );
    connect(this->ui->buttassign , SIGNAL(released()) , this, SLOT(launch()) );
}

void MainWindow::setSpinnerValue(int v)
{
    this->ui->tspin->setValue((double)v/10.0f);
}

void MainWindow::setSlideValue(double v)
{
    this->ui->threshold->setValue(v*10.0);
}


void MainWindow::launch()
{
    if(!kFilei.isFile() || (
                !kFilei.fileName().endsWith(".db", Qt::CaseInsensitive) &&
                !kFilei.fileName().endsWith(".sqlite", Qt::CaseInsensitive) &&
                !kFilei.fileName().endsWith(".las", Qt::CaseInsensitive) && !kFilei.fileName().endsWith(".laz", Qt::CaseInsensitive) ) )
    {
        logit(2,"No LAS/LAZ or SQLITE file dropped in program. Remember extension of file must be las/laz/db/sqlite. Choose a file be dragging it over the program");
        return;
    }
    bool removed = true;
   if((!kFilei.fileName().endsWith(".db", Qt::CaseInsensitive) &&
               !kFilei.fileName().endsWith(".sqlite", Qt::CaseInsensitive)))
   {
        if(kFileout.exists())
            removed = QFile(kFileout.absoluteFilePath()).remove();
        if(!removed){
            logit(2,QString("Could not overwrite file %1").arg(kFileout.absoluteFilePath()));
            return;
        }
   }
    QString whato = QObject::sender()->objectName();
    QThread *localthread = new QThread;

    lasthread *mThread = new lasthread(this);
    mThread->filei = kFilei;
    mThread->fileout = kFileout;

    if(whato=="buttkeep") mThread->negative=true;
    else  if(whato=="buttremove") mThread->negative=false;
    else mThread->getlof=true;
    mThread->k = this->ui->kspin->value();
   mThread->table= sqlitetable;
   mThread->tx= this->ui->xcomboBox->currentText();
   mThread->ty= this->ui->ycomboBox_2->currentText();
   mThread->tz= this->ui->zcomboBox_3->currentText();
    mThread->threshold = this->ui->tspin->value();
    mThread->pbar = this->ui->progressBar;
    connect(mThread, SIGNAL(progress(int)), this->ui->progressBar, SLOT(setValue(int)));

    connect(mThread, SIGNAL(message(int, QString)), this, SLOT(logit(int, QString)));

    mThread->moveToThread(localthread);
    connect(localthread, SIGNAL(started()), mThread, SLOT(process()));
    localthread->start();
}



void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}


void MainWindow::dropEvent(QDropEvent *e)
{

    this->ui->sqlitepanel->setEnabled(false);
    if(e->mimeData()->urls().count()>1 )
    {
     this->ui->plainTextEdit->appendHtml("<font color=orange>More than one file dropped, only first file will be processed.</font></br>");
    }
      Q_FOREACH (const QUrl &url, e->mimeData()->urls())
    {
        const QString &fileName = url.toLocalFile();
        kFilei= QFileInfo(fileName);

    }

      if( kFilei.fileName().endsWith(".las", Qt::CaseInsensitive) ||
          kFilei.fileName().endsWith(".laz", Qt::CaseInsensitive)  )
      {
         logit(0,"<b>Will process file "+kFilei.fileName() +".</b>");
         kFileout =   QFileInfo(kFilei.absolutePath() + "\\"  + kFilei.baseName() + "_output." + kFilei.completeSuffix());
         if(kFileout.exists() && kFileout.isDir())
         {
             logit(1,"<b>"+kFileout.fileName() +" is a directory! Cannot write a new LAS/LAZ file with this name!</b>");
             kFilei = QFileInfo();
             kFileout = QFileInfo();
         }
         else if(kFileout.exists() && kFileout.isFile())
         {
             logit(1,"<b>File <u>"+kFileout.fileName() +"</u> exists and will be overwritten!</b>");
         }

      }
     else if( kFilei.fileName().endsWith(".db", Qt::CaseInsensitive) ||
          kFilei.fileName().endsWith(".sqlite", Qt::CaseInsensitive)  )
      {
         logit(0,"<b>Will try to open as SQLITE file "+kFilei.fileName() +".</b>");
         kFileout =   QFileInfo(kFilei.absolutePath() + "\\"  + kFilei.baseName() + "_output." + kFilei.completeSuffix());
         if(kFileout.exists() && kFileout.isDir())
         {
             logit(1,"<b>"+kFileout.fileName() +" is a directory! Cannot write a new SQLITE file with this name!</b>");
             kFilei = QFileInfo();
             kFileout = QFileInfo();
         }
         else if(kFileout.exists() && kFileout.isFile())
         {
             logit(1,"<b>File <u>"+kFileout.fileName() +"</u> exists and will be overwritten!</b>");
         }

         sqlite3 *db;
         char *zErrMsg = 0;
         int rc;

         rc = sqlite3_open_v2(kFilei.absoluteFilePath().toLocal8Bit().constData(), &db, SQLITE_OPEN_READONLY, NULL );
         if( rc ){
           fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
           logit(2,QString("<b>"+kFilei.fileName() +" cannot open with sqlite3 driver! Error: %1</b>").arg( sqlite3_errmsg(db)));
           sqlite3_close(db);
           return;
         }


         sqlite3_stmt *stmt;


         rc = sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master;", -1, &stmt, NULL);

         //rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), my_special_callback, 0, &zErrMsg);

         const unsigned char * table;

         char tb[1000];
         char tb2[256];
         while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
             table = sqlite3_column_text(stmt, 0);
             printf("%s\n", table);  /* 3 */
             sprintf(tb, "PRAGMA table_info('%s');\n", table);  /* 3 */
             sprintf(tb2, "%s", table);  /* 3 */
             sqlitetable=QString(tb2);
         }
        QStringList  stl;
         stl.append("--scegli colonna");
         rc = sqlite3_prepare_v2(db, tb, -1, &stmt, NULL);
         while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
             table = sqlite3_column_text(stmt, 1);
             sprintf(tb, "%s", table);  /* 3 */
             printf("%s\n", table);  /* 3 */
             stl.append(tb);
         }
         if (rc == SQLITE_ERROR ) {
             logit(2,QString("<b>Error reading data in "+kFilei.fileName() +". Error: %1</b>").arg(  sqlite3_errmsg(db) ));
             sqlite3_finalize(stmt);
             sqlite3_close(db);
             return;
         }
         this->ui->sqlitepanel->setEnabled(true);

        this->ui->xcomboBox->addItems(stl);
         this->ui->ycomboBox_2->addItems(stl);
         this->ui->zcomboBox_3->addItems(stl);
         //QString sql("PRAGMA table_info('"+table+"');");

         sqlite3_finalize(stmt);

         sqlite3_close(db);

      } else {
          logit(2,"<b>"+kFilei.fileName() +" is not LAS or LAZ format.</b>");
          return;
      }
}


void MainWindow::logit(int type, QString value)
{
    switch(type){
    case 1:
        value.prepend( QTime::currentTime().toString("hh:mm:ss.zzz").append( "&nbsp;<font color=orange>" ) ).append("</font>");
        break;
    case 2:
        value.prepend( QTime::currentTime().toString("hh:mm:ss.zzz").append( "&nbsp;<font color=red>" ) ).append("</font>");
        break;
    default:
        value.prepend( QTime::currentTime().toString("hh:mm:ss.zzz").append( "&nbsp;<font color=black>" ) ).append("</font>");
        break;
    }

    this->ui->plainTextEdit->appendHtml( value );
}

MainWindow::~MainWindow()
{
    delete ui;
}
