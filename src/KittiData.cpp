#include <iostream>

#include <QDir>
#include <QTextStream>
#include <QDataStream>

#include <QDebug>
#include "KittiData.h"

KittiData::KittiData()
{

}

KittiData::KittiData(QString path)
{
    _leftImgPath = path+"image_0";
    _rightImgPath = path+"image_1";
    _velodynePath = path+"velodyne";

    _flistLeftImg = get_filelist(_leftImgPath, "*.png");
    _flistRightImg = get_filelist(_rightImgPath,"*.png");
    _flistVelodyne = get_filelist(_velodynePath,"*.bin");

    QFile _ftimes(path+"times.txt");
    if (!_ftimes.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&_ftimes);
    while (!in.atEnd()) {
        QString line = in.readLine();
        _times.push_back(line.toDouble());
    }
}

KittiData::~KittiData()
{

}

QFileInfoList KittiData::get_filelist(const QString path, const QString name_filter)
{
    QDir dir(path);
    QStringList name_filters;
    name_filters << name_filter;

    QFileInfoList filelist = dir.entryInfoList(name_filters, QDir::NoDotAndDotDot|QDir::AllDirs|QDir::Files);

    return filelist;
}

void KittiData::print_filelist(const QFileInfoList flist)
{
//    std::cout << "[KittiData]\t" << flist.size() << std::endl;
    for (int i = 0; i < flist.size(); i++)
    {
        QFileInfo fi = flist.at(i);
        if (fi.isFile()) {
//            std::cout << fi.fileName().toStdString() << std::endl;
        }
    }
}

void KittiData::read_velodyne(QString fname)
{
    QFile velodyne_file(fname);
    if (!velodyne_file.open(QIODevice::ReadOnly))
        return;

    QDataStream in(&velodyne_file);
    _velodyneData.clear();
    _velodyneReflectance.clear();

    _velodyneLayerData.clear();
    _velodyneLayerReflectance.clear();

    int cnt=0;

    bool init_azimuth = false;
    double prev_azimuth;
    QVector<GLfloat> singleLayerData;
    QVector<GLfloat> singleLayerReflectance;

    while(!in.atEnd()) {
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);
        double x, y, z, r;

        in >> x >> y >> z >> r;

//        double angle = atan2(static_cast<double> (z), static_cast<double> (sqrt(x*x+y*y)))*180.0/M_PI;
        double azimuth = atan2(static_cast<double> (y), static_cast<double>(x));
        azimuth = (azimuth > 0 ? azimuth : (2*M_PI + azimuth));

        if (!init_azimuth) {
            prev_azimuth = azimuth;
            init_azimuth = true;
        }
        else {
            if((azimuth - prev_azimuth) < -0.2) {
//                std::cout << azimuth << " - " << prev_azimuth << " = " << azimuth - prev_azimuth << std::endl;
                _velodyneLayerData.push_back(singleLayerData);
                _velodyneLayerReflectance.push_back(singleLayerReflectance);
                singleLayerData.clear();
                singleLayerReflectance.clear();
            }
            prev_azimuth = azimuth;
        }

        singleLayerData.push_back(x);
        singleLayerData.push_back(y);
        singleLayerData.push_back(z);
        singleLayerReflectance.push_back(r);
        singleLayerReflectance.push_back(r);
        singleLayerReflectance.push_back(r);

        _velodyneData.push_back(x);
        _velodyneData.push_back(y);
        _velodyneData.push_back(z);
        _velodyneReflectance.push_back(r);
        _velodyneReflectance.push_back(r);
        _velodyneReflectance.push_back(r);

    }

    _velodyneLayerData.push_back(singleLayerData);
    _velodyneLayerReflectance.push_back(singleLayerReflectance);
    singleLayerData.clear();
    singleLayerReflectance.clear();

}
