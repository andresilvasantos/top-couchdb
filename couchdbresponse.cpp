#include "couchdbresponse.h"

#include <QJsonDocument>
#include <QJsonObject>

class CouchDBResponsePrivate
{
public:
    CouchDBResponsePrivate() :
        query(0),
        status(COUCHDB_ERROR)
    {}

    CouchDBQuery *query; //Response do not own query
    CouchDBReplyStatus status;
    QString revisionData;
    QByteArray data;
    QJsonDocument document;
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

CouchDBQuery *CouchDBResponse::query() const
{
    Q_D(const CouchDBResponse);
    return d->query;
}

void CouchDBResponse::setQuery(CouchDBQuery *query)
{
    Q_D(CouchDBResponse);
    d->query = query;
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

QString CouchDBResponse::revisionData() const
{
    Q_D(const CouchDBResponse);
    return d->revisionData;
}

void CouchDBResponse::setRevisionData(const QString &revision)
{
    Q_D(CouchDBResponse);
    d->revisionData = revision;
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
    d->document = QJsonDocument::fromJson(data);

    if(d->document.isNull() || d->document.isEmpty())
    {
        d->document = QJsonDocument();
    }
    else
    {
        if(d->document.object().contains("revision"))
        {
            d->revisionData = d->document.object().value("revision").toString();
        }
    }
}

QJsonDocument CouchDBResponse::document() const
{
    Q_D(const CouchDBResponse);
    return d->document;
}

QJsonObject CouchDBResponse::documentObj() const
{
    Q_D(const CouchDBResponse);
    return d->document.object();
}
