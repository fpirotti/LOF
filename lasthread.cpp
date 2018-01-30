#include "lasthread.h"

lasthread::lasthread(QObject *parent) : QObject(parent)
{

}

void lasthread::sqlite2pcd(){
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int npoints=0;
    int rc;

    QString vrtfilename = fileout.absolutePath() +"/"+ fileout.baseName() + ".vrt";

    int copied=true;
    if( !this->getlof  ) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question((QWidget*)(this->parent()), "Overwrite?", QString("File %1 will be overwritten is that ok?").arg(fileout.fileName()),  QMessageBox::Yes|QMessageBox::No );
        if (reply == QMessageBox::Yes) {
            copied=QFile::copy(filei.absoluteFilePath(), this->fileout.absoluteFilePath());
        } else {
            emit message(1,QString("You decided to NOT overwrite file %1, process was terminated.").arg(fileout.fileName()) );
            copied=false;
            return;
        }
    }
    else {
        if( !QFile::exists(filei.absoluteFilePath()) ) {
            copied=QFile::copy(filei.absoluteFilePath(), this->fileout.absoluteFilePath());
        }
    }
    if(!copied) {
        emit message(1,QString("Could NOT overwrite file %1, process was terminated.").arg(fileout.fileName()) );
        return;
    }


    rc = sqlite3_open_v2(filei.absoluteFilePath().toLocal8Bit().constData(), &db, SQLITE_OPEN_READONLY, NULL );
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        emit message(2,QString(""+filei.fileName() +" cannot open with sqlite3 driver! Error: %1").arg( sqlite3_errmsg(db)));
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_prepare_v2(db, QString("SELECT count(1) FROM %1;").arg(table).toLocal8Bit().constData(), -1, &stmt, NULL);
    if (rc == SQLITE_ERROR ) {
        emit message(2,QString("Error reading data in "+table +". Error: %1").arg(  sqlite3_errmsg(db) ));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR ) {
        emit message(2,QString("Error stepping in data in "+table +". Error: %1").arg(  sqlite3_errmsg(db) ));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }
    npoints = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);


    float every = float( npoints)/1000.0;
    int cevery = (int) ceil(every);

    pcl::PointCloud<pcl::InterestPoint >::Ptr cloud (new pcl::PointCloud<pcl::InterestPoint >);
    pcl::PointCloud<pcl::InterestPoint >::Ptr cloud_filtered (new pcl::PointCloud<pcl::InterestPoint >);

    pcl::InterestPoint  a;
    tx=="--scegli colonna"?tx="0.0":tx=tx;
    ty=="--scegli colonna"?ty="0.0":ty=ty;
    tz=="--scegli colonna"?tz="0.0":tz=tz;

    rc = sqlite3_prepare_v2(db, QString("SELECT %1,%2,%3 FROM %4;").arg(tx).arg(ty).arg(tz).arg(table).toLocal8Bit().constData(), -1, &stmt, NULL);
    int i =0;
    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        i++;
        if(i%cevery == 0)
            emit progress( (int)(i/every));

        a.z = sqlite3_column_double(stmt, 2);
        a.x = sqlite3_column_double(stmt, 0);
        a.y = sqlite3_column_double(stmt, 1);
        cloud->push_back(a);
    }

    sqlite3_finalize(stmt);

    emit message(0,QString("Finished reading %1 rows from table '"+table +"'").arg(  i ));



    FILE *fp = fopen("F:\\out.txt", "a");
    emit progress(0);

    QTime myTimer;
    myTimer.start();
    emit message(0, QString("LOF Filter starting"));



    pcl::LOFFilter<pcl::InterestPoint> loffilter (true); // Initializing with true will allow us to extract the removed indices
    loffilter.setInputCloud (cloud);
    loffilter.setK (k);
    loffilter.setThresh (threshold);

    if(this->getlof) emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving LOF in intensity value (x100)").arg(k).arg(threshold));
    else if(negative) emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving outliers in new SQLITE DB").arg(k).arg(threshold));
    else  emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving new SQLITE DB without outliers").arg(k).arg(threshold));


    loffilter.filter (*cloud_filtered);
    int ela = myTimer.elapsed();
    fprintf(fp, "%d\t%d\t%d\n",npoints, k, ela);


    emit message(0, QString( "Filter finished, took %1 ms.").arg(myTimer.elapsed()));

    if( (*cloud_filtered).size() > 0 ) {
        if(this->getlof) emit message(0, QString("%1 out of %2 points outliers found.").arg( (*cloud_filtered).size()  ).arg( npoints ));
    } else{
        emit message(1, QString("No outliers found - NOT writing output!"));
        return;
    }
    fclose(fp);
    sqlite3_close(db);



    rc = sqlite3_open_v2(fileout.absoluteFilePath().toLocal8Bit().constData(), &db, SQLITE_OPEN_READWRITE, NULL );
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        emit message(2,QString(""+fileout.fileName() +" cannot open with sqlite3 driver! Error: %1").arg( sqlite3_errmsg(db)));
        sqlite3_close(db);
        return;
    }



    pcl::IndicesConstPtr cp = loffilter.getRemovedIndices();
    // int index=0;
    int iter=0;


    boost::shared_ptr <std::vector<float> > sp;
    if(this->getlof) sp = loffilter.getLOFs();

    char *errmsg;
    QStringList vals;
    i=0;
    QString sql;
    if(this->getlof)
    {
        sql = QString("ALTER TABLE %1 ADD COLUMN lof_value_%2 real; ").arg(table).arg(k, 3,10, QChar('0'));
        //sql = QString("ALTER TABLE %1 ADD COLUMN outlier integer; ").arg(table);
        rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), NULL, NULL, &errmsg);
        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("Error alter table "+table +" with new column . Error: %1").arg(  errmsg ));
            sqlite3_close(db);
            return;
        }
        sql = QString("CREATE TABLE tmp(lof_value real); ");
        rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), NULL, NULL, &errmsg);
        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("Error creating temporary table. Error: %1").arg(  errmsg ));
            sqlite3_close(db);
            return;
        }

        sql=QString("INSERT INTO tmp (lof_value) VALUES (?);");
        rc = sqlite3_prepare_v2(db, sql.toLocal8Bit().data(), -1, &stmt, NULL);
        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("Error PREPARING temporary table. Error: %1").arg(   sqlite3_errmsg(db) ));
            sqlite3_close(db);
            return;
        }

        bool has_transaction_begun=false;
        emit message(0,QString("A new column 'lof_value' is being populated to table %1 with %2 lof values ").arg(table).arg((*sp).size()));


        for(iter; iter != (*sp).size(); iter++){

            if(iter%cevery == 0){
                emit progress( (int)(iter/every));

            }

            if(iter%10000 == 0){
                // call begin transaction every 1000 rows
                if(has_transaction_begun == true){
                    sqlite3_exec(db, "END TRANSACTION;",NULL,NULL,NULL);
                }
                sqlite3_exec(db, "BEGIN TRANSACTION;",NULL,NULL,NULL);
                has_transaction_begun = true;
            }
            double v = (double)((*sp)[iter]);
            if (sqlite3_bind_double(
                        stmt,
                        1,  // Index of wildcard
                        v
                        )
                    != SQLITE_OK) {
                emit message(2,QString("Iter=%3 - could not bind LOF values. SQL=%2 <br>Error: %1").arg(    sqlite3_errmsg(db)  ).arg(sql.toLocal8Bit().data()).arg(iter) );
                return;
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                emit message(2,QString("could not step execute. Error: %1").arg(    sqlite3_errmsg(db)  ));
                return ;
            }
            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);

        }
        sqlite3_exec(db, "END TRANSACTION;",NULL,NULL,NULL);

        sql = QString("UPDATE %1 SET lof_value_%2 =  (SELECT lof_value FROM tmp  WHERE tmp.rowid = %1.rowid); ").arg(table).arg(k, 3,10, QChar('0'));
        rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), NULL, NULL, &errmsg);
        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("Error updating table %1: table %2.").arg(table).arg(  errmsg ));
            sqlite3_close(db);
            return;
        }
        sql = QString("DROP TABLE tmp; ");
        //rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), NULL, NULL, &errmsg);

        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("Error dropping temporary table. Error: %1").arg(  errmsg ));
            sqlite3_close(db);
            return;
        }


        emit message(0,QString("A new column 'lof_value' was appended to table %1 with %2 lof values. <br>"
                               " You can open in GIS using the VRT file created in %3").arg(  table).arg(iter).arg(vrtfilename));

    }
    else
    {
        iter=0;
        for(iter; iter != (*cp).size(); iter++){
            vals.append( QString::number((*cp)[iter])  );
        }

        if(negative) sql = QString("DELETE FROM  %2 WHERE rowid in (%1);").arg(vals.join(",")).arg(table);
        else  sql = QString("DELETE FROM  %2 WHERE  rowid NOT in (%1);").arg(vals.join(",")).arg(table);

        rc = sqlite3_exec(db, sql.toLocal8Bit().constData(), NULL, NULL, &errmsg);

        if (rc == SQLITE_ERROR ) {
            emit message(2,QString("2Error stepping in data in "+table +". Error: %1<br>SQL=<br>").arg(  errmsg));
            sqlite3_close(db);
            return;
        }
        emit message(1,QString("%1 %2 outliers from a total of %3. %4 rows affected.").arg( negative?"Removed":"Kept" ).arg( (*cp).size()).arg(npoints).arg(sqlite3_changes(db)) );
    }


    sqlite3_close(db);

    QString vrt = QString("                      <OGRVRTDataSource> "
                          "                <OGRVRTLayer name=\"%3\">"
                          "                    <SrcDataSource  relativeToVRT=\"0\">%2</SrcDataSource>"
                          "                    <SrcSQL>SELECT * FROM %1</SrcSQL>"
                          "                    <GeometryType>wkbPoint</GeometryType>"
                          "                    <LayerSRS>WGS84</LayerSRS>"
                          "                <GeometryField encoding=\"PointFromColumns\" x=\"lon_media\" y=\"lat_media\"/>"
                          "                </OGRVRTLayer>"
                          "            </OGRVRTDataSource>").arg(table).arg(fileout.absoluteFilePath()).arg(fileout.baseName());

    QFile file(vrtfilename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << vrt << endl;
    }
    file.close();

}

void lasthread::las2pcd(){

    LASreadOpener lasreadopener;
    LASwriteOpener laswriteopener;

    lasreadopener.set_file_name(this->filei.absoluteFilePath().toLocal8Bit().constData());
    lasreader = lasreadopener.open();
    if(lasreader == 0x0 )  {
        emit message(0, QString("Reading %1 points from LAS file.").arg(lasreader->npoints) );
        return;
    }
    int i=0;

    float every = float(lasreader->npoints)/1000.0;
    int cevery = (int) ceil(every);
    int npoints = lasreader->npoints;
    // if(lasreader->npoints < 1000)  emit message(0, QString("Reading %1 points from LAS file.").arg(lasreader->npoints) );
    // else if (lasreader->npoints < 1000000)    emit message(0, QString("Reading %1K points from LAS file.").arg((int)(lasreader->npoints/1000)) );
    // else  emit message(0, QString("Reading %1 Million points from LAS file.").arg((int)(lasreader->npoints/1000000)) );

    pcl::PointCloud<pcl::InterestPoint >::Ptr cloud (new pcl::PointCloud<pcl::InterestPoint >);
    pcl::PointCloud<pcl::InterestPoint >::Ptr cloud_filtered (new pcl::PointCloud<pcl::InterestPoint >);

    pcl::InterestPoint  a;
    while (lasreader->read_point()) {
        i++;
        if(i%cevery == 0)
            emit progress( (int)(i/every));

        a.z =  lasreader->point.get_z();
        a.x = lasreader->point.get_x();
        a.y = lasreader->point.get_y();
        cloud->push_back(a);

    }

    FILE *fp = fopen("F:\\out.txt", "a");
    emit progress(0);

    QTime myTimer;
    myTimer.start();
    emit message(0, QString("LOF Filter starting"));



    pcl::LOFFilter<pcl::InterestPoint> loffilter (true); // Initializing with true will allow us to extract the removed indices
    loffilter.setInputCloud (cloud);
    loffilter.setK (k);
    loffilter.setThresh (threshold);

    if(this->getlof) emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving LOF in intensity value (x100)").arg(k).arg(threshold));
    else if(negative) emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving outliers in new LAS").arg(k).arg(threshold));
    else  emit message(0, QString( "Launching filter with k=%1 threshold=%2 and saving new LAS without outliers").arg(k).arg(threshold));


    loffilter.filter (*cloud_filtered);
    int ela = myTimer.elapsed();
    fprintf(fp, "%d\t%d\t%d\n",npoints, k, ela);


    emit message(0, QString( "Filter finished, took %1 ms.").arg(myTimer.elapsed()));

    if( (*cloud_filtered).size() > 0 ) {
        emit message(0, QString("%1 out of %2 points outliers found.").arg( (*cloud_filtered).size()  ).arg( npoints ));
    } else{
        emit message(1, QString("No outliers found - NOT writing output!"));
        return;
    }

    emit message(0, QString("Writing output..."));
    myTimer.restart();
    LASheader lasheader = lasreader->header;

    strncpy(lasheader.system_identifier, "LASoutlierLOF by Francesco Pirotti with LASLib from rapidlasso", 62);
    laswriteopener.set_file_name(this->fileout.absoluteFilePath().toLocal8Bit().constData());
    LASwriter* laswriter = laswriteopener.open(&lasheader);
    if (laswriter == 0)
    {
        emit message(2, QString("Could not save new LAS/LAZ file in path %1!").arg(this->fileout.absoluteFilePath()));
        return;
    }
    LASpoint laspoint;
    laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);
    i=0;

    pcl::IndicesConstPtr cp = loffilter.getRemovedIndices();
    // int index=0;
    int iter=0;

    lasreader->seek(0);
    bool endreached = (iter==(*cp).size()) ;
    boost::shared_ptr <std::vector<float> > sp;
    if(this->getlof) sp = loffilter.getLOFs();
    U16 mini, maxi;
    mini = 99999999;
    maxi = 0;
    int vhigh=0;
    int vhigh2=0;

    while (lasreader->read_point()) {
        if(i%cevery == 0)
            emit progress( (int)(i/every));
        if(this->getlof)
        {

            U16 a=(U16) (((*sp)[i])*100.0);
            a>1000?a=1000:a=a;
            lasreader->point.intensity = a;
            laswriter->write_point( &lasreader->point);
            //  a=lasreader->point.intensity;
            laswriter->update_inventory( &lasreader->point);
            i++;
            continue;
        }

        if( !endreached && (((*cp).at(iter)==i)==negative ) ) {
            iter++;
            endreached = (iter==(*cp).size()) ;
            i++;
            continue;
        }

        i++;
        laswriter->write_point( &lasreader->point);
        laswriter->update_inventory( &lasreader->point);

    }
    fclose(fp);
    emit message(0, QString("Updating header in LAS....max_LOF=%1, min_LOF=%2 - n. of over 20:%3, over 100:%4").arg(maxi).arg(mini).arg(vhigh).arg(vhigh2));
    laswriter->update_header(&lasheader, TRUE);
    emit progress(0);

    emit message(0, QString("Writing %2 points in file <u>"+this->fileout.absoluteFilePath()+"</u> finished - skipped %3 outliers, in %1 ms.").arg(myTimer.elapsed()).arg(i).arg(iter));
    lasreader->close();
    laswriter->close();

}



void lasthread::process()
{
    if( this->table.length()>0 )
        sqlite2pcd();
    else
        las2pcd();

}


