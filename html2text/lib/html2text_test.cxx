#include "html2text.h"
#include <QCoreApplication>
#include <QFile>
#include <QByteArray>
#include <QtEndian>
#include <QThread>
#include <QSemaphore>
#include <QTime>
#include <iostream>
#include <vector>

//const char *filename = "Z:\\test.html";
const char *filename = "Z:\\goldendict-installation-and-operation.html";
QSemaphore sem;

class semPost : public QThread
{
public:
    void operator()() { QThread::start(); }
    void run()
    {
        QThread::sleep(30);
        sem.release(99);
    }
}sem_ost;

class test1 : public QThread
{
public:
    void operator()() { QThread::start(); }
    void run()
    {
        QThread::sleep(10);
        int ret = 0;
        FILE *in_file = ::fopen(filename, "r");
        if( in_file )
        {
            std::string in_buffer;
            while ( !::feof( in_file ) )
                in_buffer.push_back( ::fgetc( in_file ) & 0xFF );

            fclose(in_file);

            QByteArray out_buffer;
            size_t outbuffer_len = in_buffer.size();
            out_buffer.resize(outbuffer_len);

            h2t_utf8buffer utf8buffer(in_buffer.c_str(), in_buffer.size(),
                                      out_buffer.data(), outbuffer_len);

            sem.acquire();
            QTime begin = QTime::currentTime();
            ret = html_to_text(utf8buffer, 80);
            int msec = begin.msecsTo(QTime::currentTime());
            std::cout << "h2t_utf8buffer msec: " << msec << std::endl;

            if ( ret )
            {
                out_buffer.resize(outbuffer_len);
                out_buffer = out_buffer.trimmed();

                std::cout << "h2t_utf8buffer out_buffer.size = " << out_buffer.size() << std::endl;
                FILE *out = fopen("Z:\\out.txt", "w");
                if( out )
                {
                    ::fwrite(out_buffer.data(), out_buffer.length(), 1, out);
                    ::fflush(out);
                    ::fclose(out);
                }

            }
        }
    }
} utf8buffer;

class test2 : public QThread
{
public:
    void operator()() { QThread::start(); }
    void run()
    {
        QThread::sleep(10);
        int ret = 0;
        std::string out_string;
        h2t_utf8file utf8file(filename, out_string);

        sem.acquire();
        QTime begin = QTime::currentTime();
        ret = html_to_text(utf8file, 80);
        int msec = begin.msecsTo(QTime::currentTime());
        std::cout << "h2t_utf8file msec: " << msec << std::endl;

        if ( ret )
        {
            std::cout << "h2t_utf8file out_buffer.size = " << out_string.size() << std::endl;
            FILE *out = fopen("Z:\\out1.txt", "w");
            if( out )
            {
                ::fwrite(out_string.c_str(), out_string.size(), 1, out);
                ::fflush(out);
                ::fclose(out);
            }

        }
    }
} utf8file;

class test3 : public QThread
{
public:
    void operator()() { QThread::start(); }
    void run()
    {
        QThread::sleep(10);
        int ret = 0;
        QFile qfile(filename);
        if( qfile.open(QIODevice::ReadOnly) )
        {
            std::string instring = QString::fromUtf8(qfile.readAll()).toStdString();
            std::string out_string;
            h2t_utf8string utf8string(instring, out_string);

            sem.acquire();
            QTime begin = QTime::currentTime();
            ret = html_to_text(utf8string, 80);
            int msec = begin.msecsTo(QTime::currentTime());
            std::cout << "h2t_utf8string msec: " << msec << std::endl;

            if ( ret )
            {
                std::cout << "h2t_utf8string out_buffer.size = " << out_string.size() << std::endl;
                FILE *out = fopen("Z:\\out2.txt", "w");
                if( out )
                {
                    ::fwrite(out_string.c_str(), out_string.size(), 1, out);
                    ::fflush(out);
                    ::fclose(out);
                }

            }
        }

    }
}utf8string;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    utf8buffer();
    utf8file();
    utf8string();

    sem_ost();

    return app.exec();
}
