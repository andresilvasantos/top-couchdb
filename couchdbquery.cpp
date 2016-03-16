#include "couchdbquery.h"

#include <QNetworkRequest>
#include <QTimer>

class CouchDBQueryPrivate
{
public:
    CouchDBQueryPrivate() :
        request(0),
        timer(0)
    {}

    virtual ~CouchDBQueryPrivate()
    {
        if(request) delete request;
        if(timer) delete timer;
    }

    QNetworkRequest *request;
    QByteArray body;
    CouchDBOperation operation;
    QTimer *timer;
};

CouchDBQuery::CouchDBQuery(QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBQueryPrivate)
{
    Q_D(CouchDBQuery);
    d->request = new QNetworkRequest;
    d->timer = new QTimer(this);
    d->timer->setInterval(20000);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), SIGNAL(timeout()));
}

CouchDBQuery::~CouchDBQuery()
{
    delete d_ptr;
}

QNetworkRequest* CouchDBQuery::request() const
{
    Q_D(const CouchDBQuery);
    return d->request;
}

QUrl CouchDBQuery::url() const
{
    Q_D(const CouchDBQuery);
    return d->request->url();
}

void CouchDBQuery::setUrl(const QUrl &url)
{
    Q_D(CouchDBQuery);
    d->request->setUrl(url);
}

CouchDBOperation CouchDBQuery::operation() const
{
    Q_D(const CouchDBQuery);
    return d->operation;
}

void CouchDBQuery::setOperation(const CouchDBOperation &operation)
{
    Q_D(CouchDBQuery);
    d->operation = operation;
}

QByteArray CouchDBQuery::body() const
{
    Q_D(const CouchDBQuery);
    return d->body;
}

void CouchDBQuery::setBody(const QByteArray &body)
{
    Q_D(CouchDBQuery);
    d->body = body;
}

void CouchDBQuery::startTimeoutTimer()
{
    Q_D(CouchDBQuery);
    d->timer->start();
}
