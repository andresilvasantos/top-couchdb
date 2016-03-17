#include "couchdb.h"
#include "couchdbserver.h"
#include "couchdbquery.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QtQml>
#include <QDebug>

#define TIMEOUT_INTERVAL 20000
#define LISTEN_CHANGES_RESTART_TIMEOUT 1000
#define REPLICATION_TIMEOUT_INTERVAL 120000

class CouchDBPrivate
{
public:
    CouchDBPrivate() :
        server(0),
        cleanServerOnQuit(true),
        networkManager(0)
    {}
    
    virtual ~CouchDBPrivate()
    {
        if(server && cleanServerOnQuit) delete server;
        
        if(networkManager) delete networkManager;
    }
    
    CouchDBServer *server;
    bool cleanServerOnQuit;

    QNetworkAccessManager *networkManager;
    QHash<QNetworkReply*, CouchDBQuery*> currentQueries;
};

CouchDB::CouchDB(QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBPrivate)
{
    Q_D(CouchDB);

    d->server = new CouchDBServer(this);
    d->networkManager = new QNetworkAccessManager(this);
}

CouchDB::~CouchDB()
{
    delete d_ptr;
}

void CouchDB::declareQML()
{
    qmlRegisterType<CouchDB>("TOP.CouchDB", 1, 0, "CouchDB");
}

CouchDBServer *CouchDB::server() const
{
    Q_D(const CouchDB);
    return d->server;
}

void CouchDB::setServer(CouchDBServer *server)
{
    Q_D(CouchDB);
    if(d->server) delete d->server;

    d->server = server;
    d->cleanServerOnQuit = true;
}

void CouchDB::setServerConfiguration(const QString &url, const int &port, const QString &username, const QString &password)
{
    Q_D(CouchDB);
    d->server->setUrl(url);
    d->server->setPort(port);
    if(!username.isEmpty() && !password.isEmpty()) d->server->setCredential(username, password);
}

void CouchDB::executeQuery(CouchDBQuery *query)
{
    Q_D(CouchDB);

    if(d->server->hasCredential()) query->request()->setRawHeader("Authorization", "Basic " + d->server->credential());

    qDebug() << "Invoked url:" << query->request()->url().toString();

    QNetworkReply * reply;
    switch(query->operation()) {
    case COUCHDB_CHECKINSTALLATION:
    default:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_STARTSESSION:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_ENDSESSION:
        reply = d->networkManager->deleteResource(*query->request());
        break;
    case COUCHDB_LISTDATABASES:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_CREATEDATABASE:
        reply = d->networkManager->put(*query->request(), query->body());
        break;
    case COUCHDB_DELETEDATABASE:
        reply = d->networkManager->deleteResource(*query->request());
        break;
    case COUCHDB_LISTDOCUMENTS:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_RETRIEVEREVISION:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_RETRIEVEDOCUMENT:
        reply = d->networkManager->get(*query->request());
        break;
    case COUCHDB_UPDATEDOCUMENT:
        reply = d->networkManager->put(*query->request(), query->body());
        break;
    case COUCHDB_DELETEDOCUMENT:
        reply = d->networkManager->deleteResource(*query->request());
        break;
    case COUCHDB_UPLOADATTACHMENT:
        reply = d->networkManager->put(*query->request(), query->body());
        break;
    case COUCHDB_DELETEATTACHMENT:
        reply = d->networkManager->deleteResource(*query->request());
        break;
    case COUCHDB_REPLICATEDATABASE:
        reply = d->networkManager->get(*query->request());
        break;
    }

    connect(query, SIGNAL(timeout()), SLOT(queryTimeout()));
    connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));

    query->startTimeoutTimer();
    d->currentQueries[reply] = query;
}

void CouchDB::queryFinished()
{
    Q_D(CouchDB);

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;

    QByteArray data;
    CouchDBQuery *query = d->currentQueries[reply];
    bool hasError = false;
    if(reply->error() == QNetworkReply::NoError)
    {
        data = reply->readAll();
    }
    else
    {
        qWarning() << reply->errorString();
        hasError = true;
    }

    d->currentQueries.remove(reply);
    reply->deleteLater();

    CouchDBResponse response;
    response.setData(data);
    response.setStatus(hasError ? COUCHDB_ERROR : COUCHDB_SUCCESS);

    switch(query->operation())
    {
    case COUCHDB_CHECKINSTALLATION:
    default:
//        QJsonDocument document = QJsonDocument::fromJson(replyBA);
//        emit installationChecked(document.object().contains("couchdb"));
        emit installationChecked(response);
        break;
    case COUCHDB_STARTSESSION:
//        QString cookie(reply->rawHeader("Set-Cookie"));
//        cookie = cookie.right(cookie.count() - (cookie.indexOf("=") + 1));
//        cookie = cookie.left(cookie.indexOf(";"));
//        //d->cookieJar->insertCookie(QNetworkCookie("AuthSession", cookie.toStdString().c_str()));
//        emit sessionStarted(true, false);

        emit sessionStarted(response);
        break;
    case COUCHDB_ENDSESSION:
        emit sessionEnded(response);
        break;
    case COUCHDB_LISTDATABASES:
        emit databasesListed(response);
        break;
    case COUCHDB_CREATEDATABASE:
        emit databaseCreated(response);
        break;
    case COUCHDB_DELETEDATABASE:
        emit databaseDeleted(response);
        break;
    case COUCHDB_LISTDOCUMENTS:
        emit documentsListed(response);
        break;
    case COUCHDB_RETRIEVEREVISION:
        emit revisionRetrieved(response);
        break;
    case COUCHDB_RETRIEVEDOCUMENT:
        emit documentRetrieved(response);
        break;
    case COUCHDB_UPDATEDOCUMENT:
        emit documentUpdated(response);
        break;
    case COUCHDB_DELETEDOCUMENT:
        emit documentDeleted(response);
        break;
    case COUCHDB_UPLOADATTACHMENT:
        emit attachmentUploaded(response);
        break;
    case COUCHDB_DELETEATTACHMENT:
        emit attachmentDeleted(response);
        break;
    case COUCHDB_REPLICATEDATABASE:
        emit databaseReplicated(response);
        break;
    }

    delete query;
}

void CouchDB::queryTimeout()
{
    CouchDBQuery *query = qobject_cast<CouchDBQuery*>(sender());
    if(!query) return;

    qWarning() << query->url() << "timed out. Retrying...";

    executeQuery(query);
}

void CouchDB::checkInstallation()
{
    Q_D(CouchDB);

    CouchDBQuery *query = new CouchDBQuery(this);
    query->setUrl(QString("%1").arg(d->server->baseURL()));
    query->setOperation(COUCHDB_CHECKINSTALLATION);

    executeQuery(query);
}

void CouchDB::startSession(const QString &username, const QString &password)
{
    Q_D(CouchDB);

    QUrlQuery postData;
    postData.addQueryItem("name", username);
    postData.addQueryItem("password", password);

    CouchDBQuery *query = new CouchDBQuery(this);
    query->setUrl(QString("%1/_session").arg(d->server->baseURL()));
    query->setOperation(COUCHDB_STARTSESSION);
    query->request()->setRawHeader("Accept", "application/json");
    query->request()->setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    query->setBody(postData.toString(QUrl::FullyEncoded).toUtf8());

    executeQuery(query);
}
/*
void CouchDB::startSessionFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    
    if(document.object().value("ok").toBool())
    {
        QString cookie(reply->rawHeader("Set-Cookie"));
        cookie = cookie.right(cookie.count() - (cookie.indexOf("=") + 1));
        cookie = cookie.left(cookie.indexOf(";"));
        //d->cookieJar->insertCookie(QNetworkCookie("AuthSession", cookie.toStdString().c_str()));
        emit sessionStarted(true, false);
    }
    else
    {
        emit sessionStarted(false, true);
    }
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::startSessionError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit sessionStarted(false, false);
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::startSessionTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit sessionStarted(false, false);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::endSession(const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl("_session", accessType);
    qDebug() << "Ending session" << url;
    QNetworkRequest request(url);
    
    QNetworkReply* reply = d->networkManagerListener->deleteResource(request);
    d->networkManager->deleteResource(request);
    connect(reply, SIGNAL(finished()), SLOT(endSessionFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(endSessionError(QNetworkReply::NetworkError)));
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(endSessionTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::endSessionFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit sessionEnded(!document.object().contains("error"));
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::endSessionError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit sessionEnded(false);
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::endSessionTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit sessionEnded(false);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::createDatabase(const QString &database, const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database, accessType);
    qDebug() << "Creating database" << url;
    QNetworkRequest request(url);
    //TODO: MAKE AUTHORIZED REQUEST
    QNetworkReply* reply = d->networkManager->put(request, "");
    connect(reply, SIGNAL(finished()), SLOT(databaseCreationFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(databaseCreationError(QNetworkReply::NetworkError)));
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(databaseCreationTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::databaseCreationFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit databaseCreated(!document.object().contains("error"));
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::databaseCreationError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit databaseCreated(false);
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::databaseCreationTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit databaseCreated(false);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::deleteDatabase(const QString &database, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database, accessType);
    qDebug() << "Deleting database" << url;
    QNetworkRequest request(url);
    //TODO: MAKE AUTHORIZED REQUEST
    QNetworkReply* reply = d->networkManager->deleteResource(request);
    connect(reply, SIGNAL(finished()), SLOT(databaseDeletionFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(databaseDeletionError(QNetworkReply::NetworkError)));
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(databaseDeletionTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::databaseDeletionFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    qDebug() << replyBA;
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit databaseDeleted(!document.object().contains("error"));
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::databaseDeletionError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit databaseDeleted(false);
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::databaseDeletionTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit databaseDeleted(false);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::listDatabases(const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl("_all_dbs", accessType);
    qDebug() << "Listing databases" << url;
    QNetworkRequest request(url);
    QNetworkReply* reply = d->networkManager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(databasesListingFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(databasesListingError(QNetworkReply::NetworkError)));
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(databasesListingTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::databasesListingFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    qDebug() << replyBA;
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::databasesListingError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit databasesListed(false, QStringList());
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::databasesListingTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit databasesListed(false, QStringList());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::retrieveRevision(const QString &database, const QString &id, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database + "/" + id, accessType);
    qDebug() << "Retrieving revision of document" << url;
    QNetworkRequest request(url);
    QNetworkReply* reply = d->networkManager->head(request);
    connect(reply, SIGNAL(finished()), SLOT(retrieveRevisionFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(retrieveRevisionError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(retrieveRevisionTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::retrieveRevisionFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    QString revision = reply->rawHeader("ETag");
    
    if(revision.isEmpty())
    {
        emit revisionRetrieved(false, reply->property("id").toString(), "");
    }
    else
    {
        revision.remove("\"");
        emit revisionRetrieved(true, reply->property("id").toString(), revision);
    }
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::retrieveRevisionError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit revisionRetrieved(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::retrieveRevisionTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit revisionRetrieved(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::updateDocument(const QString &database, const QString &id, QByteArray document, const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    QByteArray postDataSize = QByteArray::number(document.size());
    
    QUrl url = generateUrl(database + "/" + id, accessType);
    qDebug() << "Updating document" << url;
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Content-Length", postDataSize);
    QNetworkReply* reply = d->networkManager->put(request, document);
    connect(reply, SIGNAL(finished()), SLOT(documentUpdateFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(documentUpdateError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(documentUpdateTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::documentUpdateFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    
    if(document.object().value("ok").toBool())
    {
        emit documentUpdated(true, reply->property("id").toString(), document.object().value("rev").toString());
    }
    else
    {
        emit documentUpdated(false, reply->property("id").toString(), "");
    }
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::documentUpdateError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit documentUpdated(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::documentUpdateTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit documentUpdated(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::deleteDocument(const QString &database, const QString &id, const QString &revision, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database + "/" + id + "?rev=" + revision, accessType);
    qDebug() << "Deleting document" << url;
    QNetworkRequest request(url);
    //TODO: MAKE AUTHORIZED REQUEST
    QNetworkReply* reply = d->networkManager->deleteResource(request);
    connect(reply, SIGNAL(finished()), SLOT(documentDeletionFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(documentDeletionError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(documentDeletionTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::documentDeletionFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    qDebug() << replyBA;
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit documentDeleted(!document.object().contains("error"), reply->property("id").toString());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::documentDeletionError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit documentDeleted(false, reply->property("id").toString());
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::documentDeletionTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit documentDeleted(false, reply->property("id").toString());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::listDocuments(const QString& database, const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database + "/_all_docs", accessType);
    qDebug() << "Listing documents" << url;
    QNetworkRequest request(url);
    QNetworkReply* reply = d->networkManager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(documentsListingFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(documentsListingError(QNetworkReply::NetworkError)));
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(documentsListingTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::documentsListingFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    
    qDebug() << replyBA;
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::documentsListingError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit documentsListed(false, QStringList());
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::documentsListingTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit documentsListed(false, QStringList());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::retrieveDocument(const QString &database, const QString &id, const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database + "/" + id, accessType);
    qDebug() << "Retrieving document" << url;
    QNetworkRequest request(url);
    QNetworkReply* reply = d->networkManager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(retrieveDocumentFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(retrieveDocumentError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(retrieveDocumentTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::retrieveDocumentFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit documentRetrieved(true, reply->property("id").toString(), document);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::retrieveDocumentError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit documentRetrieved(false, reply->property("id").toString(), QJsonDocument());
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::retrieveDocumentTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit documentRetrieved(false, reply->property("id").toString(), QJsonDocument());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::uploadAttachment(const QString &database, const QString &id, const QString& attachmentName,
                               QByteArray attachment, QString mimeType, const QString& revision, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QByteArray postDataSize = QByteArray::number(attachment.size());
    
    QUrl url = generateUrl(database + "/" + id + "/" + attachmentName + "?rev=" + revision, accessType);
    qDebug() << "Uploading attachment" << url;
    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", mimeType.toLatin1());
    request.setRawHeader("Content-Length", postDataSize);
    QNetworkReply* reply = d->networkManager->put(request, attachment);
    connect(reply, SIGNAL(finished()), SLOT(uploadAttachmentFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(uploadAttachmentError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(uploadAttachmentTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::uploadAttachmentFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit attachmentUploaded(document.object().value("ok").toBool(), reply->property("id").toString(), document.object().value("rev").toString());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::uploadAttachmentError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    qDebug() << reply->readAll() << error;
    emit attachmentUploaded(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::uploadAttachmentTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit attachmentUploaded(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::deleteAttachment(const QString &database, const QString &id, const QString &attachmentName, const QString &revision, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl(database + "/" + id + "/" + attachmentName + "?rev=" + revision, accessType);
    qDebug() << "Deleting attachment" << url;
    QNetworkRequest request(url);
    QNetworkReply* reply = d->networkManager->deleteResource(request);
    connect(reply, SIGNAL(finished()), SLOT(deleteAttachmentFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(deleteAttachmentError(QNetworkReply::NetworkError)));
    reply->setProperty("id", id);
    
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(TIMEOUT_INTERVAL);
    timer->start();
    connect(timer, SIGNAL(timeout()), SLOT(deleteAttachmentTimeout()));
    
    d->repliesTimeoutMap.insert(timer, reply);
}

void CouchDB::deleteAttachmentFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    if(!d->repliesTimeoutMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit attachmentDeleted(document.object().value("ok").toBool(), reply->property("id").toString(), document.object().value("rev").toString());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::deleteAttachmentError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    qDebug() << reply->readAll() << error;
    emit attachmentDeleted(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::deleteAttachmentTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit attachmentDeleted(false, reply->property("id").toString(), "");
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::replicateDatabase(const QString &sourceDatabase, const QString &targetDatabase, const CouchDBAccessType& sourceAccessType,
                                const CouchDBAccessType& targetAccessType, const bool& createTarget, const bool& continuous, const bool& cancel)
{
    Q_D(CouchDB);
    
    QUrl url = generateUrl("_replicate", sourceAccessType == COUCHDB_LOCAL ? sourceAccessType : targetAccessType);
    qDebug() << "Replicating database" << url;
    QNetworkRequest request(url);
    
    QString sourceDatabaseStr , targetDatabaseStr;
    
    if(sourceAccessType == targetAccessType)
    {
        sourceDatabaseStr = sourceDatabase;
        targetDatabaseStr = targetDatabase;
    }
    else
    {
        if(sourceAccessType == COUCHDB_LOCAL)
        {
            sourceDatabaseStr = sourceDatabase;
            targetDatabaseStr = generateUrl(targetDatabase, targetAccessType).toString();
        }
        else
        {
            sourceDatabaseStr = generateUrl(sourceDatabase, sourceAccessType).toString();
            targetDatabaseStr = targetDatabase;
        }
    }
    
    if(!cancel) qDebug() << "Starting replication from" << sourceDatabaseStr << "to" << targetDatabaseStr;
    else qDebug() << "Cancelling replication from" << sourceDatabaseStr << "to" << targetDatabaseStr;
    
    QJsonObject object;
    object.insert("source", sourceDatabaseStr);
    object.insert("target", targetDatabaseStr);
    object.insert("create_target", createTarget);
    object.insert("continuous", continuous);
    object.insert("cancel", cancel);
    QJsonDocument document(object);
    
    QByteArray json = document.toJson();
    
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Content-Type", "application/json");
    
    QNetworkReply* reply = d->networkManager->post(request, json);
    connect(reply, SIGNAL(finished()), SLOT(replicateDatabaseFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replicateDatabaseError(QNetworkReply::NetworkError)));
    
    if(continuous)
    {
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(REPLICATION_TIMEOUT_INTERVAL);
        timer->start();
        connect(timer, SIGNAL(timeout()), SLOT(replicateDatabaseTimeout()));
        
        d->repliesTimeoutMap.insert(timer, reply);
    }
}

void CouchDB::replicateDatabaseFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    const QByteArray replyBA = reply->readAll();
    //qDebug() << replyBA;
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    emit databaseReplicated(document.object().value("ok").toBool());
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::replicateDatabaseError(QNetworkReply::NetworkError error)
{
    if(error >= 201 && error <= 299) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;
    
    emit databaseReplicated(false);
    
    removeTimer(reply);
    reply->deleteLater();
}

void CouchDB::replicateDatabaseTimeout()
{
    Q_D(CouchDB);
    
    QTimer *timer = qobject_cast<QTimer*>(sender());
    
    if(!timer) return;
    
    QNetworkReply *reply = d->repliesTimeoutMap.value(timer, 0);
    if(!reply) return;
    
    emit databaseReplicated(false);
    
    removeTimer(reply);
    delete reply;
}

void CouchDB::listenToChanges(const QString &database, const QString &id, const CouchDBAccessType& accessType)
{
    Q_D(CouchDB);
    
    //QUrl url = generateUrl(database + "/_changes?name=" + id + "&filter=app/docFilter&feed=continuous&heartbeat=10000&timeout=60000", accessType);
    QUrl url = generateUrl(database + "/_changes?name=" + id + "&filter=app/docFilter&feed=continuous&heartbeat=10000", accessType);
    qDebug() << "Listening to changes" << url << id;
    QNetworkRequest request(url);
    QJsonArray idsArray;
    idsArray.append(id);
    
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Content-Type", "application/json");
    
    QNetworkReply* reply = d->networkManagerListener->get(request);
    connect(reply, SIGNAL(finished()), SLOT(listenToChangesFinished()));
    connect(reply, SIGNAL(readyRead()), SLOT(listenToChangesReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(listenToChangesError(QNetworkReply::NetworkError)));
    reply->setProperty("doc_id", id);
    
    d->repliesChangesMap.insert(database + " " + id + " " + QString::number(accessType), reply);
}

bool CouchDB::stopListenToChanges(const QString &database, const QString &id, const CouchDBAccessType &accessType)
{
    Q_D(CouchDB);
    
    qDebug() << "Stop listening to changes " << id;
    
    QString key(database + "_" + id + QString::number(accessType));
    QNetworkReply *reply = d->repliesChangesMap.value(key, 0);
    if(!reply) return false;
    
    disconnect(reply, SIGNAL(finished()), this, SLOT(listenToChangesFinished()));
    disconnect(reply, SIGNAL(readyRead()), this, SLOT(listenToChangesReadyRead()));
    disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(listenToChangesError(QNetworkReply::NetworkError)));
    d->repliesChangesMap.remove(key);
    
    reply->abort();
    reply->deleteLater();
    //delete reply;
    
    return true;
}

void CouchDB::listenToChangesFinished()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    qDebug() << "LISTEN TO CHANGES FINISHED:" << reply->property("doc_id").toString();
    if(!reply) return;
    
    if(!d->repliesChangesMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    
    if(document.object().contains("error"))
    {
        qDebug() << "LISTEN TO CHANGES ERROR:" << reply->property("doc_id").toString();
        emit listenToChangesFailed(document.object().value("id").toString());
    }
    
    QString value = d->repliesChangesMap.key(reply);
    d->repliesChangesMap.remove(value);
    reply->deleteLater();
    
    //    QString database = value.left(value.indexOf(" "));
    //    QString id = reply->property("doc_id").toString();
    //    CouchDBAccessType accessType = CouchDBAccessType(QString(value.right(value.lastIndexOf(" ") + 1)).toInt());
}

void CouchDB::listenToChangesReadyRead()
{
    Q_D(CouchDB);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    qDebug() << "LISTEN TO CHANGES READY READ:" << reply->property("doc_id").toString();
    if(!reply) return;
    
    if(!d->repliesChangesMap.values().contains(reply)) return;
    
    const QByteArray replyBA = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);
    
    if(document.object().contains("error"))
    {
        qDebug() << "LISTEN TO CHANGES ERROR:" << reply->property("doc_id").toString();
        emit listenToChangesFailed(document.object().value("id").toString());
        return;
    }
    
    if(document.object().contains("id"))
    {
        emit changesMade(document.object().value("id").toString(), document.object().value("changes").toArray().first().toObject().value("rev").toString());
    }
}

void CouchDB::listenToChangesError(QNetworkReply::NetworkError error)
{
    Q_D(CouchDB);
    
    qDebug() << "LISTEN TO CHANGES ERROR!";
    
    if((error >= 201 && error <= 299) || error == 5) return;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    qDebug() << "ERROR FOR DOCUMENT:" << reply->property("doc_id").toString();
    if(!reply) return;
    
    if(!d->repliesChangesMap.values().contains(reply)) return;
    
    
    emit listenToChangesFailed(reply->property("id").toString());
    
    //    d->repliesChangesMap.remove(d->repliesChangesMap.key(reply));
    //    reply->deleteLater();
}

void CouchDB::removeTimer(QNetworkReply *reply)
{
    Q_D(CouchDB);
    
    QTimer *timer = d->repliesTimeoutMap.key(reply, 0);
    if(timer) d->repliesTimeoutMap.remove(timer);
    delete timer;
}*/
