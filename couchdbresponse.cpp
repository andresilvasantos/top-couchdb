#include "couchdbresponse.h"

class CouchDBResponsePrivate
{
public:
    CouchDBResponsePrivate() :
        status(COUCHDB_ERROR)
    {}

    QString database;
    QString documentID;
    QByteArray data;
    CouchDBReplyStatus status;
};

CouchDBResponse::CouchDBResponse(QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBResponsePrivate)
{
}

CouchDBResponse::~CouchDBResponse()
{
    delete d_ptr;
}

QString CouchDBResponse::database() const
{
    Q_D(const CouchDBResponse);
    return d->database;
}

void CouchDBResponse::setDatabase(const QString &database)
{
    Q_D(CouchDBResponse);
    d->database = database;
}

QString CouchDBResponse::documentID() const
{
    Q_D(const CouchDBResponse);
    return d->documentID;
}

void CouchDBResponse::setDocumentID(const QString &documentID)
{
    Q_D(CouchDBResponse);
    d->documentID = documentID;
}

CouchDBReplyStatus CouchDBResponse::status() const
{
    Q_D(const CouchDBResponse);
    return d->status;
}

void CouchDBResponse::setStatus(const CouchDBReplyStatus &status)
{
    Q_D(CouchDBResponse);
    d->status = status;
}

QByteArray CouchDBResponse::data() const
{
    Q_D(const CouchDBResponse);
    return d->data;
}

void CouchDBResponse::setData(const QByteArray &data)
{
    Q_D(CouchDBResponse);
    d->data = data;
}
