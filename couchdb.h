#ifndef COUCHDB_H
#define COUCHDB_H

#include <QObject>
#include <QNetworkReply>

#include "couchdbenums.h"
#include "couchdbresponse.h"

class QQmlEngine;
class QJSEngine;
class CouchDBListener;
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

signals:
    void installationChecked(const CouchDBResponse& response);
    void sessionStarted(const CouchDBResponse& response);
    void sessionEnded(const CouchDBResponse& response);
    void databasesListed(const CouchDBResponse& response);
    void databaseCreated(const CouchDBResponse& response);
    void databaseDeleted(const CouchDBResponse& response);
    void documentsListed(const CouchDBResponse& response);
    void revisionRetrieved(const CouchDBResponse& response);
    void documentRetrieved(const CouchDBResponse& response);
    void documentUpdated(const CouchDBResponse& response);
    void documentDeleted(const CouchDBResponse& response);
    void attachmentUploaded(const CouchDBResponse& response);
    void attachmentDeleted(const CouchDBResponse& response);
    void databaseReplicated(const CouchDBResponse& response);

public slots:
    Q_INVOKABLE void checkInstallation();

    Q_INVOKABLE void startSession(const QString& username, const QString& password);
    Q_INVOKABLE void endSession();

    Q_INVOKABLE void listDatabases();
    Q_INVOKABLE void createDatabase(const QString& database);
    Q_INVOKABLE void deleteDatabase(const QString& database);

    Q_INVOKABLE void listDocuments(const QString& database);
    Q_INVOKABLE void retrieveRevision(const QString& database, const QString& documentID);
    Q_INVOKABLE void retrieveDocument(const QString& database, const QString& documentID);
    Q_INVOKABLE void updateDocument(const QString& database, const QString& documentID, QByteArray document);
    Q_INVOKABLE void deleteDocument(const QString& database, const QString& documentID, const QString& revision);

    Q_INVOKABLE void uploadAttachment(const QString& database, const QString& documentID, const QString &attachmentName, QByteArray attachment,
                                      QString mimeType, const QString &revision);
    Q_INVOKABLE void deleteAttachment(const QString& database, const QString& documentID, const QString &attachmentName, const QString &revision);

    Q_INVOKABLE void replicateDatabaseFrom(CouchDBServer *sourceServer, const QString& sourceDatabase, const QString& targetDatabase,
                                           const bool& createTarget, const bool& continuous, const bool& cancel = false);
    Q_INVOKABLE void replicateDatabaseTo(CouchDBServer *targetServer, const QString& sourceDatabase, const QString& targetDatabase,
                                         const bool& createTarget, const bool& continuous, const bool& cancel = false);

    Q_INVOKABLE CouchDBListener* createListener(const QString& database, const QString& documentID);

private slots:
    void queryFinished();
    void queryTimeout();

protected:
    void executeQuery(CouchDBQuery *query);

private:
    Q_DECLARE_PRIVATE(CouchDB)
    CouchDBPrivate * const d_ptr;

};

#endif // COUCHDB_H
