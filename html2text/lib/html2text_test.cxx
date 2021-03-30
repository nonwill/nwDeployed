#include "html2text.h"
#include <QTextDocumentFragment>

#include <QFile>
#include <QByteArray>
#include <QtEndian>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QTime>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QRegularExpression>
#include <QGuiApplication>
#else
#include <QRegExp>
#include <QApplication>
#define QGuiApplication QApplication
#endif
#include <iostream>
#include <vector>

//const char *filename = "Z:\\test.html";
const char *filename = "Z:\\test.html";
QSemaphore sem;

class MyThread : public QThread
{
public:
    ~MyThread();
    using QThread::msleep;
    using QThread::sleep;
};

class semPost : public QRunnable
{
public:
    virtual ~semPost(){}
    void run()
    {
        std::cout << "Do initial jobs, semPost after 10s ...... " << std::endl;
        MyThread::sleep(2);
        sem.release(4);
    }
};

class test_utf8buffer : public QRunnable
{
public:
    virtual ~test_utf8buffer(){}
public:
    QString outfile;
    test_utf8buffer(QString outfile_) : outfile(outfile_)
    {}

    void run()
    {
        int ret = 0;
        FILE *in_file = ::fopen(filename/*"Z:\\test11.html"*/, "r");
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
            ret = html_to_text(utf8buffer, 0);
            int msec = begin.msecsTo(QTime::currentTime());
            std::cout << "h2t_utf8buffer msec: " << msec << std::endl;

            if ( ret )
            {
                out_buffer.resize(outbuffer_len);
                out_buffer = out_buffer.trimmed();

                //std::cout << "h2t_utf8buffer out_buffer.size = " << out_buffer.size() << std::endl;
                FILE *out = fopen(outfile.toLocal8Bit().data(), "w");
                if( out )
                {
                    ::fwrite(out_buffer.data(), out_buffer.length(), 1, out);
                    ::fflush(out);
                    ::fclose(out);
                }

            }
        }
        sem.release();
    }
};

class test_utf8file : public QRunnable
{
public:
    virtual ~test_utf8file(){}
public:
    QString outfile;
    test_utf8file(QString outfile_) : outfile(outfile_)
    {}

    void run()
    {
        int ret = 0;
        std::string out_string;
        h2t_utf8file utf8file(filename, out_string);

        sem.acquire();
        QTime begin = QTime::currentTime();
        ret = html_to_text(utf8file, 30);
        int msec = begin.msecsTo(QTime::currentTime());
        std::cout << "h2t_utf8file msec: " << msec << std::endl;

        if ( ret )
        {
            //std::cout << "h2t_utf8file out_buffer.size = " << out_string.size() << std::endl;
            FILE *out = fopen(outfile.toLocal8Bit().data(), "w");
            if( out )
            {
                ::fwrite(out_string.c_str(), out_string.size(), 1, out);
                ::fflush(out);
                ::fclose(out);
            }

        }

        sem.release();
    }
};

class test_utf8string : public QRunnable
{
public:
    virtual ~test_utf8string(){}
public:
    QString outfile;
    test_utf8string(QString outfile_) : outfile(outfile_)
    {}

    void run()
    {
        int ret = 0;
        QFile qfile(filename);
        if( qfile.open(QIODevice::ReadOnly) )
        {
            std::string instring = QString::fromUtf8(qfile.readAll()).toStdString();
            qfile.close();
            std::string out_string;
            h2t_utf8string utf8string(instring, out_string);

            sem.acquire();
            QTime begin = QTime::currentTime();
            ret = html_to_text(utf8string, 35);
            int msec = begin.msecsTo(QTime::currentTime());
            std::cout << "h2t_utf8string msec: " << msec << std::endl;

            if ( ret )
            {
                //std::cout << "h2t_utf8string out_buffer.size = " << out_string.size() << std::endl;
                FILE *out = fopen(outfile.toLocal8Bit().data(), "w");
                if( out )
                {
                    ::fwrite(out_string.c_str(), out_string.size(), 1, out);
                    ::fflush(out);
                    ::fclose(out);
                }

            }
        }

        sem.release();
    }
};

class test_QTextDocumentFragment : public QRunnable
{
public:
    virtual ~test_QTextDocumentFragment(){}
public:
    QString outfile;
    test_QTextDocumentFragment(QString outfile_) : outfile(outfile_)
    {}

    void run()
    {
        int ret = 0;
        QFile qfile(filename);
        if( qfile.open(QIODevice::ReadOnly) )
        {
            QByteArray inbytes = qfile.readAll();
            qfile.close();

            sem.acquire();
            QTime begin = QTime::currentTime();
            QString instring = QString::fromUtf8(inbytes);
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
            instring.replace( QRegularExpression( QStringLiteral("<(?:\\s*/?(?:div|h[1-6r]|q|p(?![alr])|br|li(?![ns])|td|blockquote|[uo]l|pre|d[dl]|nav|address))[^>]{0,}>"),
                                             QRegularExpression::CaseInsensitiveOption ), QString(' ') );
            instring.remove( QRegularExpression( QStringLiteral("<[^>]*>") ) );
#else
            instring.replace( QRegExp( "<(?:\\s*/?(?:div|h[1-6r]|q|p(?![alr])|br|li(?![ns])|td|blockquote|[uo]l|pre|d[dl]|nav|address))[^>]{0,}>",
                                  Qt::CaseInsensitive, QRegExp::RegExp2 ), " " );
            instring.remove( QRegExp( "<[^>]*>", Qt::CaseSensitive, QRegExp::RegExp2 ) );
#endif
            std::string out_string = QTextDocumentFragment::fromHtml(instring).toPlainText().toStdString();
            int msec = begin.msecsTo(QTime::currentTime());
            std::cout << "QTextDocumentFragment msec: " << msec << std::endl;

            //if ( ret )
            {
                //std::cout << "h2t_utf8string out_buffer.size = " << out_string.size() << std::endl;
                FILE *out = fopen(outfile.toLocal8Bit().data(), "w");
                if( out )
                {
                    ::fwrite(out_string.c_str(), out_string.size(), 1, out);
                    ::fflush(out);
                    ::fclose(out);
                }

            }
        }

        sem.release();
    }
};

void unscape()
{
    std::string str;
    str += "&#918;abcd&#917;hello&#169;<a>&#8658;&rArr;"
    "&fnof;&#8230;</a>"
    "asfas&#8225;dfe&#933;&Psi;&#937;<img>n&#8595;n&sdot;n&#969;mmm</img>1jh\n";

    str +=  "<td>&amp;#8736;</td>\n";

    str +=  "<td>&amp;#8736;</td>;;#8736;\n";

    str +=  "<td>&amp;#8736;</td>;&amp;\n";

    str +=  "<td>&amp;#8736;</td>;&amp\n";

    str +=  "<td>&amp;#8736;</td>;&\n";

    str +=  "ab\n";

    std::string out_string;
    h2t_utf8string utf8string(str, out_string);
    if(html_to_text(utf8string, 0))
    {
        std::cout << "raw: " << str << std::endl;
        std::cout << "out: " << QString::fromStdString(out_string).toLocal8Bit().data() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    unscape();
    //return app.exec();
    MyThread::sleep(10);
    int count = 900000;
    int i = 0;
    QThreadPool::globalInstance()->start(new test_utf8file(QString("Z:\\out%1.txt").arg(i++ % 20)));
    QThreadPool::globalInstance()->start(new test_utf8string(QString("Z:\\out%1.txt").arg(i++ % 20)));
    QThreadPool::globalInstance()->start(new test_utf8buffer(QString("Z:\\out%1.txt").arg(i++ % 20)));
    QThreadPool::globalInstance()->start(new test_QTextDocumentFragment(QString("Z:\\out%1.txt").arg(i++ % 20)));
    QThreadPool::globalInstance()->start(new semPost());

    for ( ; i < 10000; ) {
        std::cout << "l: " << i << std::endl;
      //  QThreadPool::globalInstance()->start(new test_utf8file(QString("Z:\\out%1.txt").arg(i++ % 20)));
        QThreadPool::globalInstance()->start(new test_utf8string(QString("Z:\\out%1.txt").arg(i++ % 20)));
      //  QThreadPool::globalInstance()->start(new test_utf8buffer(QString("Z:\\out%1.txt").arg(i++ % 20)));
        QThreadPool::globalInstance()->start(new test_QTextDocumentFragment(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8file(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8string(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8buffer(QString("Z:\\out%1.txt").arg(i++ % 20)));
        QThreadPool::globalInstance()->start(new test_QTextDocumentFragment(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8file(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8string(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8buffer(QString("Z:\\out%1.txt").arg(i++ % 20)));
        QThreadPool::globalInstance()->start(new test_QTextDocumentFragment(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8file(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8string(QString("Z:\\out%1.txt").arg(i++ % 20)));
//        QThreadPool::globalInstance()->start(new test_utf8buffer(QString("Z:\\out%1.txt").arg(i++ % 20)));
        QThreadPool::globalInstance()->start(new test_QTextDocumentFragment(QString("Z:\\out%1.txt").arg(i++ % 20)));

        //QThreadPool::globalInstance()->waitForDone();
        MyThread::msleep(2000);
    }
    //sem_ost();

    return app.exec();
}
