#ifndef COUCHDB_H
#define COUCHDB_H

#include <QObject>
#include <QNetworkReply>

#include "couchdbenums.h"
#include "couchdbresponse.h"

class QQmlEngine;
class QJSEngine;
class CouchDBQuery;
class CouchDBServer;
class CouchDBPrivate;
class CouchDB : public QObject
{
    Q_OBJECT

public:
    explicit CouchDB(QObject *parent = 0);
    virtual ~CouchDB();

    static void declareQML();

    CouchDBServer *server() const;
    void setServer(CouchDBServer *server);
    void setServerConfiguration(const QString& url, const int& port, const QString& username = "", const QString& password = "");

//    void setBaseUrl(const QString &url);
//    void setCredentials(const QString &username, const QString &password);
//    void clearCredentials();

signals:
    void installationChecked(const CouchDBResponse& response);
    void sessionStarted(const CouchDBResponse& response);
    void sessionEnded(const CouchDBResponse& response);
    void databasesListed(const CouchDBResponse& response);
    void databaseCreated(const CouchDBResponse& response);
    void databaseDeleted(const CouchDBResponse& response);
    void documentsListed(const CouchDBResponse& response);
    void revisionRetrieved(const CouchDBResponse& response);
    void documentUpdated(const CouchDBResponse& response);
    void documentDeleted(const CouchDBResponse& response);
    void documentRetrieved(const CouchDBResponse& response);
    void attachmentUploaded(const CouchDBResponse& response);
    void attachmentDeleted(const CouchDBResponse& response);
    void databaseReplicated(const CouchDBResponse& response);

    //void listenToChangesFailed(const QString&);
    //void changesMade(const QString& id, const QString& revision);

public slots:
    Q_INVOKABLE void checkInstallation();

    Q_INVOKABLE void startSession(const QString& username, const QString& password);
    /*Q_INVOKABLE void endSession();

    Q_INVOKABLE void createDatabase(const QString& database);
    Q_INVOKABLE void deleteDatabase(const QString& database);
    Q_INVOKABLE void listDatabases();

    Q_INVOKABLE void retrieveRevision(const QString& database, const QString& documentID);

    Q_INVOKABLE void updateDocument(const QString& database, const QString& documentID, QByteArray document);
    Q_INVOKABLE void deleteDocument(const QString& database, const QString& documentID, const QString& revision);
    Q_INVOKABLE void listDocuments(const QString& database);
    Q_INVOKABLE void retrieveDocument(const QString& database, const QString& documentID);

    Q_INVOKABLE void uploadAttachment(const QString& database, const QString& documentID, const QString &attachmentName, QByteArray attachment,
                                      QString mimeType, const QString &revision);
    Q_INVOKABLE void deleteAttachment(const QString& database, const QString& documentID, const QString &attachmentName, const QString &revision);

    Q_INVOKABLE void replicateDatabaseFrom(const QString& sourceDatabase, const QString& targetDatabase, CouchDBServer *sourceServer,
                                       const bool& createTarget, const bool& continuous, const bool& cancel = false);

    Q_INVOKABLE void replicateDatabaseTo(const QString& sourceDatabase, const QString& targetDatabase, CouchDBServer *targetServer,
                                         const bool& createTarget, const bool& continuous, const bool& cancel = false);

    Q_INVOKABLE void listenToChanges(const QString& database, const QString& documentID);
    Q_INVOKABLE bool stopListenToChanges(const QString& database, const QString& documentID);*/

private slots:
    void queryFinished();
    void queryTimeout();
    /*void installationCheckFinished();
    void installationCheckError(QNetworkReply::NetworkError error);
    void installationCheckTimeout();

    void startSessionFinished();
    void startSessionError(QNetworkReply::NetworkError error);
    void startSessionTimeout();

    void endSessionFinished();
    void endSessionError(QNetworkReply::NetworkError error);
    void endSessionTimeout();

    void databaseCreationFinished();
    void databaseCreationError(QNetworkReply::NetworkError error);
    void databaseCreationTimeout();

    void databaseDeletionFinished();
    void databaseDeletionError(QNetworkReply::NetworkError error);
    void databaseDeletionTimeout();

    void databasesListingFinished();
    void databasesListingError(QNetworkReply::NetworkError error);
    void databasesListingTimeout();

    void retrieveRevisionFinished();
    void retrieveRevisionError(QNetworkReply::NetworkError error);
    void retrieveRevisionTimeout();

    void documentUpdateFinished();
    void documentUpdateError(QNetworkReply::NetworkError error);
    void documentUpdateTimeout();

    void documentDeletionFinished();
    void documentDeletionError(QNetworkReply::NetworkError error);
    void documentDeletionTimeout();

    void documentsListingFinished();
    void documentsListingError(QNetworkReply::NetworkError error);
    void documentsListingTimeout();

    void retrieveDocumentFinished();
    void retrieveDocumentError(QNetworkReply::NetworkError error);
    void retrieveDocumentTimeout();

    void uploadAttachmentFinished();
    void uploadAttachmentError(QNetworkReply::NetworkError error);
    void uploadAttachmentTimeout();

    void deleteAttachmentFinished();
    void deleteAttachmentError(QNetworkReply::NetworkError error);
    void deleteAttachmentTimeout();

    void replicateDatabaseFinished();
    void replicateDatabaseError(QNetworkReply::NetworkError error);
    void replicateDatabaseTimeout();

    void listenToChangesFinished();
    void listenToChangesReadyRead();
    void listenToChangesError(QNetworkReply::NetworkError error);

    void removeTimer(QNetworkReply *reply);*/

protected:
    void executeQuery(CouchDBQuery *query);
    //QUrl generateUrl(const QString &url, const CouchDBAccessType &accessType);

private:
    Q_DECLARE_PRIVATE(CouchDB)
    CouchDBPrivate * const d_ptr;

};

#endif // COUCHDB_H
