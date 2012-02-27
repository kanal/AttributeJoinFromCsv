#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <QObject>

class QFile;

class ErrorLog : public QObject
{
    Q_OBJECT
public:
    explicit ErrorLog(QObject *parent = 0);
    virtual ~ErrorLog();
    bool open(const QString& filename);
    void close();
    void outLog(const QString& loginfo);

signals:

public slots:

private:

    QFile *m_file;
};

#endif // ERRORLOG_H
