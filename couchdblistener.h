#ifndef COUCHDBLISTENER_H
#define COUCHDBLISTENER_H

#include <QObject>
#include <QNetworkReply>

class CouchDBServer;
class CouchDBListenerPrivate;
class CouchDBListener : public QObject
{
    Q_OBJECT
public:
    CouchDBListener(CouchDBServer *server);
    virtual ~CouchDBListener();
    
    CouchDBServer* server() const;
    
    QString database() const;
    void setDatabase(const QString& database);
    
    QString documentID() const;
    void setDocumentID(const QString& documentID);

    QString revision(const QString& documentID = "") const;

    void setCookieJar(QNetworkCookieJar *cookieJar);

    void setParam(const QString &name, const QString &value);
    
    void launch();

signals:
    void changesMade(const QString& revision);

private slots:
    void start();
    void readChanges();
    void listenFinished(QNetworkReply *reply);

private:
    Q_DECLARE_PRIVATE(CouchDBListener)
    CouchDBListenerPrivate * const d_ptr;
};

#endif // COUCHDBLISTENER_H
