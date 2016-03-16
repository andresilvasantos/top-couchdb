#include "couchdbchanges.h"
#include "couchdb.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include <QTimer>
#include <QDebug>

/*
class CouchDBChangesPrivate
{
public:
    CouchDBChangesPrivate() :
        networkManager(0),
        newReply(0),
        retryTimer(0)
    {}

    virtual ~CouchDBChangesPrivate()
    {
        if(retryTimer) retryTimer->stop();
        if(newReply) newReply->abort();

        if(networkManager) delete networkManager;
        if(newReply) delete newReply;
        if(retryTimer) delete retryTimer;
    }

    QString db;
    QNetworkAccessManager *networkManager;
    QNetworkReply *newReply;
    QTimer* retryTimer;
    QMap<QString,QString> parameters;
};


CouchDBChanges::CouchDBChanges(const QString& db , CouchDBServer *_server ) :
    QObject(parent),
    d_ptr(new CouchDBChangesPrivate)
{
    Q_D(CouchDBChanges);

    this->db = db;

    d->networkManager = new QNetworkAccessManager(this);
    connect(this->nam, SIGNAL(finished(QNetworkReply*)),  this, SLOT(slotNotification(QNetworkReply *)));

    d->retryTimer = new QTimer(this);
    d->retryTimer->setInterval(500);
    d->retryTimer->setSingleShot(true);
    connect(d->retryTimer, SIGNAL(timeout()), this, SLOT(start()));
}

CouchDBChanges::~CouchDBChanges()
{
    delete d_ptr;
}

void CouchDBChanges::start()
{
    Q_D(CouchDBChanges);

    QUrlQuery urlq;
    QMapIterator<QString, QString> i(parameters);
    while (i.hasNext())
    {
        i.next();
        urlq.addQueryItem(i.key(), i.value());
    }

    QNetworkRequest request;
    QUrl url = QUrl(QString("%1/%2/_changes").arg(CouchDB::singleton()->serverBaseURL(), this->db));
    url.setQuery(urlq);
    request.setUrl(url);
    if(this->server->hasCredential())
    {
        request.setRawHeader("Authorization", "Basic "+this->server->getCredential());
    }
    d->newReply = d->networkManager->get(request);
    connect(d->newReply, SIGNAL(readyRead()), this, SLOT(readChange()));
}

void CouchDBChanges::readChange()
{
    QList<QString> changes_list;
    QTextStream in(this->new_reply->readAll());
    while (!in.atEnd()) {
        QJsonDocument json_line =  QJsonDocument::fromJson(in.readLine().toUtf8());
        if(json_line.object().contains("id"))
            changes_list.append(json_line.object().find("id").value().toString());
        else
            if(json_line.object().contains("last_seq"))
                qDebug() << json_line.object().find("last_seq").value().toInt();
    }

    emit notification( this->db, changes_list);

}

void CouchDBChanges::slotNotification( QNetworkReply *reply  )
{
    // Check the network reply for errors.
    QNetworkReply::NetworkError netError = reply->error();
    if (netError != QNetworkReply::NoError)
    {
        qWarning() << "ERROR";
        switch(netError)
        {
        case QNetworkReply::ContentNotFoundError:
            qWarning() << "The content was not found on the server";
            break;

        case QNetworkReply::HostNotFoundError:
            qWarning() <<"The server was not found";
            break;

        default:
            qWarning() <<reply->errorString();
            break;
        }

    }
    reply->deleteLater();
    this->m_retry->start();
}

void CouchDBChanges::launch() {
    this->m_retry->start();
}

void CouchDBChanges::setParam(QString key, QString val) {
    parameters.insert(key,val);
}*/
