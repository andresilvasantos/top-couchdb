#ifndef COUCHDBQUERY_H
#define COUCHDBQUERY_H

#include <QObject>

#include "couchdbenums.h"

class QNetworkRequest;
class CouchDBQueryPrivate;
class CouchDBQuery : public QObject
{
    Q_OBJECT
public:
    explicit CouchDBQuery(QObject *parent = 0);
    virtual ~CouchDBQuery();

    QNetworkRequest* request() const;

    QUrl url() const;
    void setUrl(const QUrl& url);

    CouchDBOperation operation() const;
    void setOperation(const CouchDBOperation& operation);

    QByteArray body() const;
    void setBody(const QByteArray& body);

signals:
    void timeout();

public slots:
    void startTimeoutTimer();

private:
    Q_DECLARE_PRIVATE(CouchDBQuery)
    CouchDBQueryPrivate * const d_ptr;
};

#endif // COUCHDBQUERY_H
