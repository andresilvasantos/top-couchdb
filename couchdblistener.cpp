#include "couchdblistener.h"
#include "couchdb.h"
#include "couchdbserver.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>


class CouchDBListenerPrivate
{
public:
    CouchDBListenerPrivate(CouchDBServer *s) :
        server(s),
        networkManager(0),
        reply(0),
        retryTimer(0)
    {}

    virtual ~CouchDBListenerPrivate()
    {
        if(retryTimer) retryTimer->stop();
        if(reply) reply->abort();

        if(networkManager) delete networkManager;
        if(reply) delete reply;
        if(retryTimer) delete retryTimer;
    }

    CouchDBServer *server;
    QNetworkAccessManager *networkManager;
    QString database;
    QString documentID;
    QNetworkReply *reply;
    QTimer* retryTimer;
    QMap<QString,QString> parameters;
};


CouchDBListener::CouchDBListener(CouchDBServer * server) :
    QObject(server),
    d_ptr(new CouchDBListenerPrivate(server))
{
    Q_D(CouchDBListener);

    d->networkManager = new QNetworkAccessManager(this);
    connect(d->networkManager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(listenFinished(QNetworkReply*)));

    d->retryTimer = new QTimer(this);
    d->retryTimer->setInterval(500);
    d->retryTimer->setSingleShot(true);
    connect(d->retryTimer, SIGNAL(timeout()), this, SLOT(start()));

    d->parameters.insert("feed", "continuous");
    d->parameters.insert("heartbeat", "10000");
    d->parameters.insert("timeout", "60000");
}

CouchDBListener::~CouchDBListener()
{
    delete d_ptr;
}

CouchDBServer *CouchDBListener::server() const
{
    Q_D(const CouchDBListener);
    return d->server;
}

QString CouchDBListener::database() const
{
    Q_D(const CouchDBListener);
    return d->database;
}

void CouchDBListener::setDatabase(const QString &database)
{
    Q_D(CouchDBListener);
    d->database = database;
}

QString CouchDBListener::documentID() const
{
    Q_D(const CouchDBListener);
    return d->documentID;
}

void CouchDBListener::setDocumentID(const QString &documentID)
{
    Q_D(CouchDBListener);
    d->documentID = documentID;
}

void CouchDBListener::setParam(const QString& name, const QString& value)
{
    Q_D(CouchDBListener);
    d->parameters.insert(name, value);
}

void CouchDBListener::launch()
{
    Q_D(CouchDBListener);
    d->retryTimer->start();
}

void CouchDBListener::start()
{
    Q_D(CouchDBListener);

    QUrlQuery urlQuery;
    QMapIterator<QString, QString> i(d->parameters);
    while (i.hasNext())
    {
        i.next();
        urlQuery.addQueryItem(i.key(), i.value());
    }

    QUrl url;
    if(d->documentID.isEmpty()) url = QUrl(QString("%1/%2/_changes").arg(d->server->baseURL(), d->database));
    else url = QUrl(QString("%1/%2/_changes?name=%3&filter=app/docFilter").arg(d->server->baseURL(), d->database, d->documentID));
    url.setQuery(urlQuery);

    QNetworkRequest request;
    request.setUrl(url);
    if(d->server->hasCredential()) request.setRawHeader("Authorization", "Basic " + d->server->credential());

    d->reply = d->networkManager->get(request);
    connect(d->reply, SIGNAL(readyRead()), this, SLOT(readChanges()));
}

void CouchDBListener::readChanges()
{
    Q_D(CouchDBListener);

    QList<QString> changes_list;
    QTextStream in(d->reply->readAll());
    while (!in.atEnd()) {
        QJsonDocument json_line = QJsonDocument::fromJson(in.readLine().toUtf8());
        if(json_line.object().contains("id"))
        {
            changes_list.append(json_line.object().find("id").value().toString());
        }
        else
        {
            if(json_line.object().contains("last_seq"))
            {
                qDebug() << json_line.object().find("last_seq").value().toInt();
            }
        }
    }

    //emit notification(d->database, changes_list);
    //emit changesMade(document.object().value("id").toString(), document.object().value("changes").toArray().first().toObject().value("rev").toString());
}

void CouchDBListener::listenFinished(QNetworkReply *reply)
{
    Q_D(CouchDBListener);

    // Check the network reply for errors.
    QNetworkReply::NetworkError netError = reply->error();
    if(netError != QNetworkReply::NoError)
    {
        qWarning() << "ERROR";
        switch(netError)
        {
        case QNetworkReply::ContentNotFoundError:
            qWarning() << "The content was not found on the server";
            break;

        case QNetworkReply::HostNotFoundError:
            qWarning() << "The server was not found";
            break;

        default:
            qWarning() << reply->errorString();
            break;
        }

    }
    reply->deleteLater();
    d->retryTimer->start();
}