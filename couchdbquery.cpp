#include "couchdbquery.h"

#include <QNetworkRequest>
#include <QTimer>

class CouchDBQueryPrivate
{
public:
    CouchDBQueryPrivate(CouchDBServer *s) :
        request(0),
        server(s),
        timer(0)
    {}

    virtual ~CouchDBQueryPrivate()
    {
        if(request) delete request;
        if(timer) delete timer;
    }

    CouchDBServer *server; //Query doesn't own server
    QNetworkRequest *request;
    CouchDBOperation operation;
    QString database;
    QString documentID;
    QByteArray body;
    QTimer *timer;
};

CouchDBQuery::CouchDBQuery(CouchDBServer *server, QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBQueryPrivate(server))
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

CouchDBServer *CouchDBQuery::server() const
{
    Q_D(const CouchDBQuery);
    return d->server;
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

QString CouchDBQuery::database() const
{
    Q_D(const CouchDBQuery);
    return d->database;
}

void CouchDBQuery::setDatabase(const QString &database)
{
    Q_D(CouchDBQuery);
    d->database = database;
}

QString CouchDBQuery::documentID() const
{
    Q_D(const CouchDBQuery);
    return d->documentID;
}

void CouchDBQuery::setDocumentID(const QString &documentID)
{
    Q_D(CouchDBQuery);
    d->documentID = documentID;
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
