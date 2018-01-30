#-------------------------------------------------
#
# Project created by QtCreator 2017-07-27T19:18:22
#
#-------------------------------------------------

QT       += core gui
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LASoutlierLOF
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    lasthread.cpp \
    sqlite3.c

HEADERS += \
        mainwindow.h \
    lasthread.h \
    sqlite3.h \
    sqlite3ext.h

FORMS += \
        mainwindow.ui



win32:RC_ICONS += res/header.ico


INCLUDEPATH +=  $$quote(F:/dev/PCL/include/pcl-1.8)
#INCLUDEPATH +=  $$quote(F:/dev/sqlite)
INCLUDEPATH +=  $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/Boost/include/boost-1_64)
INCLUDEPATH +=  $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/Eigen/eigen3)
INCLUDEPATH +=  $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/VTK/include/vtk-8.0)
INCLUDEPATH +=  $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/FLANN/include)


INCLUDEPATH +=  $$quote(F:/dev/LAStools/LASlib/inc)
INCLUDEPATH +=  $$quote(F:/dev/LAStools/LASzip/src)


win32 {
QMAKE_LIBDIR +=   \
                "F:/dev/LAStools/LASlib/lib"   \
                 $$quote(F:/dev/PCL/lib) \
                 $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/Boost/lib)\
                 $$quote(F:/dev/PCL 1.8.1_msvc2015/3rdParty/FLANN/lib)




    CONFIG(debug, debug|release) {
        QMAKE_LIBS += \
                    # -lgdi32 -luser32 -ladvapi32  -lOpenGL32 \
                    #    -lvtkexpat-gd -lvtkzlib-gd -lvtkpng-gd -lvtklibxml2-gd -lvtkjpeg-gd -lvtktiff-gd -lvtkVolumeRendering-gd -lvtkFiltering-gd \
                    #    -lvtkGraphics-gd -lvtkCommon-gd -lvtkViews-gd -lvtkRendering-gd -lvtkIO-gd -lvtkImaging-gd  -lvtkalglib-gd -lvtksys-gd \
                    #    -lvtkHybrid-gd -lvtkWidgets-gd \
                    #    -lflann_cpp_s-gd  -lflann_cuda_s-gd
                        -lflann-gd   -lflann_s-gd -lflann_cpp-gd -lflann_cpp_s-gd \
                    #    -llibtiff  -llibnn \
                         -llibboost_thread-vc140-mt-gd-1_64 \
                         -llibboost_system-vc140-mt-gd-1_64 \
                         -llibboost_serialization-vc140-mt-gd-1_64 \
                         -llibboost_chrono-vc140-mt-gd-1_64 \
                        -lpcl_common_debug \
                        -lpcl_search_debug \
                        -lpcl_features_debug \
                        -lpcl_filters_debug \
                        -lpcl_io_ply_debug \
                        -lpcl_io_debug \
                        -lpcl_kdtree_debug \
                        -lpcl_keypoints_debug \
#                        -lpcl_ml_debug \
#                        -lpcl_octree_debug \
#                        -lpcl_outofcore_debug \
#                        -lpcl_people_debug \
#                        -lpcl_recognition_debug \
#                        -lpcl_registration_debug \
#                        -lpcl_sample_consensus_debug \
#                        -lpcl_segmentation_debug \
#                        -lpcl_stereo_debug \
#                        -lpcl_surface_debug \
                     #   -lpcl_tracking_debug
                     #   -lpcl_visualization_debug \
                        -lLASlibD


    } else {
        QMAKE_LIBS +=  \
                      #  -lgdi32 -luser32 -ladvapi32 -lOpenGL32 \
                      #  -lvtkexpat -lvtkzlib -lvtkpng -lvtklibxml2 -lvtkjpeg -lvtktiff -lvtkVolumeRendering -lvtkFiltering \
                      #  -lvtkGraphics -lvtkCommon -lvtkViews -lvtkRendering -lvtkIO -lvtkmetaio -lvtkImaging  -lvtkalglib -lvtksys \
                      #  -lvtkHybrid -lvtkWidgets  -llibboost_thread-vc120-mt-1_63  -lflann \
                      #  -llibtiff -llibnn  \
                        -lflann   -lflann_s -lflann_cpp -lflann_cpp_s \
                    #    -llibtiff  -llibnn \
                         -llibboost_thread-vc140-mt-1_64 \
                         -llibboost_system-vc140-mt-1_64 \
                         -llibboost_serialization-vc140-mt-1_64 \
                         -llibboost_chrono-vc140-mt-1_64 \
                        -lpcl_common_release \
                        -lpcl_search_release \
                        -lpcl_features_release \
                        -lpcl_filters_release \
                        -lpcl_io_ply_release \
                        -lpcl_io_release \
                        -lpcl_kdtree_release \
                        -lpcl_keypoints_release \
                     #   -lpcl_ml_release \
                     #   -lpcl_octree_release \
                     #   -lpcl_outofcore_release \
                     #   -lpcl_people_release \
                     #   -lpcl_recognition_release \
                     #   -lpcl_registration_release \
                      #  -lpcl_sample_consensus_release \
                      #  -lpcl_segmentation_release \
                      #  -lpcl_stereo_release \
                      #  -lpcl_surface_release \
                      #  -lpcl_tracking_release \
                      #  -lpcl_visualization_release  \
                        -lLASlib
        }
    }

QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter -Wmacro-redefinition

##LIBS += -lgdal_i  -llibtiff_i  -ldelaunaynn

#QMAKE_CXXFLAGS+= /ZW


