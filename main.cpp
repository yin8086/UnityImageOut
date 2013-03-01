/**
 * Copyright (c) 2013-2013 Stardrad Yin
 * email:yin8086+support@gmail.com
 *
 * Report bugs and download new versions at https://github.com/yin8086/UnityImageOut
 *
 * This software is distributed under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QtCore>
#include <QStringList>
#include <QDir>
#include <QRunnable>

#include "pvrtc_dll.h"

//#include "PVRTDecompress.h"

QString convert(const char* src, uchar* dest,
             quint32 dataSize, int pSize,
             int type = 0) {
    QString returnStr;
    if(pSize == 1) {
        //Alpha8
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[j]     = 0;
            dest[j+1]   = 0;
            dest[j+2]   = 0;
            dest[j+3]   = src[i];
        }
        returnStr="Alpha8";
    }
    else if(pSize == 2) {
        //2bpp
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            quint16 tmpData=*(quint16*)(src+i);
            if(type == 0) {
                //rgba4444
                if(i == 0) {
                    returnStr="rgba4444";
                }
                dest[j]     = (((tmpData&0xf0)>>4)*255+7)/15; //b
                dest[j+1]   = (((tmpData&0xf00)>>8)*255+7)/15; //g
                dest[j+2]   = (((tmpData&0xf000)>>12)*255+7)/15; //r
                dest[j+3]   = ((tmpData&0xf)*255+7)/15; //a
            }
            else if(type == 1) {
                //argb1555
                if(i == 0) {
                    returnStr="argb1555";
                }
                dest[j]     = ((tmpData&0x1f)*255+15)/31; //b
                dest[j+1]   = (((tmpData&0x3e0)>>5)*255+15)/31; //g
                dest[j+2]   = (((tmpData&0x7c00)>>10)*255+15)/31; //r
                dest[j+3]   = (tmpData>>15)*0xff; //a
            }
            else if(type == 2) {
                //rgb565
                if(i == 0) {
                    returnStr="rgb565";
                }
                dest[j]     = ((tmpData&0x1f)*255+15)/31; //b
                dest[j+1]   = (((tmpData&0x7e0)>>5)*255+31)/63; //g
                dest[j+2]   = (((tmpData&0xf800)>>11)*255+15)/31; //r
                dest[j+3]   = 0xff; //a

            }
        }
    }
    else if(pSize == 3) {
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[j]     = src[i+2]; //b
            dest[j+1]   = src[i+1]; //g
            dest[j+2]   = src[i]; //r
            dest[j+3]   = 0xff; //a
        }
        returnStr="rgb888";

    }
    else if(pSize == 4) {
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[j]     = src[i+2]; //b
            dest[j+1]   = src[i+1]; //g
            dest[j+2]   = src[i]; //r
            dest[j+3]   = src[i+3]; //a
        }
        returnStr="rgba8888";

    }
    else if (pSize == 5) {
        quint32 i, j;
        pSize = 4;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[j]     = src[i+3]; //b
            dest[j+1]   = src[i+2]; //g
            dest[j+2]   = src[i+1]; //r
            dest[j+3]   = src[i]; //a
        }
        returnStr="argb8888";
    }
    return returnStr;
}

/*
void changeEndian(uchar* pixelTable, int len) {
    quint32 tmpData;
    for(int i=0; i < len; i+=4) {
        tmpData = qFromLittleEndian<quint32>(pixelTable + i);
        *(quint32 *)(pixelTable + i) = tmpData;
    }
}
*/
void fileParse(const QString& fname,int myType = 0) {
    QFile srcf(fname);
    if(srcf.open(QIODevice::ReadOnly))
    {
        QDataStream br(&srcf);
        br.setByteOrder(QDataStream::LittleEndian);
        quint32 len;
        br>>len;
        srcf.seek(srcf.pos()+len);
        if(srcf.pos()%4 != 0) {
            srcf.seek((srcf.pos()/4+1)*4);
        }
        quint32 width,height;
        quint32 imageDataSize;
        quint32 pixelSize;
        br>>width>>height>>imageDataSize>>pixelSize;

        //quint32 testSize=srcf.pos()+0x28+imageDataSize;
        //if(srcf.size()-4 <= testSize && testSize <= srcf.size()){
        if(srcf.size() > imageDataSize + 20 && imageDataSize > 0) {
            if ( (1 <= pixelSize && pixelSize <=7 && pixelSize != 6) ||
                    ( pixelSize == 0x20 ||pixelSize == 0x21) ){
                if (pixelSize == 7) {
                    myType = 2;
                    pixelSize = 2;
                }

                quint32 imageSize = width*height*pixelSize;
                if (pixelSize ==0x20 || pixelSize == 0x21) {
                    imageSize = width*height/2;
                }
                else if(pixelSize == 5) {
                    imageSize = width*height*4;
                }
                //srcf.seek(srcf.pos()+0x28);
                srcf.seek(srcf.size() - imageDataSize);

                char* originTable=new char[imageSize];
                br.readRawData(originTable,imageSize);
                uchar* pixelTable=new uchar[width*height*4];
                QString typeStr;
                if (pixelSize ==0x20 || pixelSize == 0x21) {
                    //PVRTDecompressPVRTC(originTable,0,width,height,pixelTable);
                    pvrtc_decompress(pixelTable,originTable,width,height,0,1,0);
                    //changeEndian(pixelTable,width*height*4);
                    typeStr = "PVRTC4";
                }
                else {
                    typeStr=convert(originTable, pixelTable, imageSize, pixelSize, myType);
                }

                QImage im(pixelTable,width,height,QImage::Format_ARGB32);
                im=im.mirrored(false,true)/*.rgbSwapped()*/;
                //QFileInfo forName(srcf);
                QString baseName=srcf.fileName().left(srcf.fileName().indexOf("."));
                im.save(QObject::tr("%1_%2.png").arg(baseName).arg(typeStr));
                printf(QObject::tr("%1 in %2 Completed!\n").arg(srcf.fileName())
                       .arg(typeStr).toLatin1().data());
                delete [] pixelTable;
                delete [] originTable;
            }
            else {
                printf("%s Unknown format!\n",
                       srcf.fileName().toLatin1().data());
            }
        }
        else {
            printf("%s Not an image!\n",
                   srcf.fileName().toLatin1().data());
        }
        srcf.close();
    }
    else {
        printf("File not exist!\n");
    }
}

class MyRun : public QRunnable {
    QString fName;
    int type;
public:
    MyRun(const QString& fn, int t):QRunnable(),fName(fn),type(t) {}
    void run();
};

void MyRun::run() {
    fileParse(fName,type);
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    if (argc == 2) {
        fileParse(argv[1],0);

    }
    else if (argc ==3) {
        fileParse(argv[1],QString(argv[2]).toInt());
    }
    else {
        printf("Now running in batch mode\n");
        printf("automatically transform the *.bin file\n");
        printf("in the current folder\n");
        printf("Using Alpha8,rgba4444,rgb888,rgba8888\n");
        printf("========================================\n");

        printf("Also Available in Single File Mode\n");
        printf("UnityImageOut.exe *.* [type]\n");
        printf("[type] only take effect when the texture is 16bpp\n");
        printf("type will be 0, i.e. rgba4444 when not set\n");
        printf("1 = argb1555\n");
        printf("2 = rgb565\n");


        QDir curPath=QDir::current();
        QStringList filter;
        //QList<QFuture<void> > reList;
        filter<<"*.bin";
        curPath.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        curPath.setNameFilters(filter);
        curPath.setSorting(QDir::Name);

        foreach(const QString &fn, curPath.entryList()) {
            MyRun* tmpR=new MyRun(fn,0);
            tmpR->setAutoDelete(true);
            QThreadPool::globalInstance()->start(tmpR);
        }


    }
    return 0;
    //return a.exec();
}
