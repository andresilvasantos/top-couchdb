#ifndef COUCHDBCHANGES_H
#define COUCHDBCHANGES_H

#include <QObject>

class CouchDBChangesPrivate;
class CouchDBChanges : public QObject
{
    /*Q_OBJECT
public:
    CouchDBChanges(const QString& db, CouchDBServer * server = nullptr );
    ~CouchDBChanges();

    void launch();
    void setParam(QString, QString);

signals:
    void notification( const QString& db, const QList<QString>& v );

private slots:
    void slotNotification( QNetworkReply* reply);
    void readChange();
    void start();

private:
    Q_DECLARE_PRIVATE(CouchDBChanges)
    CouchDBChangesPrivate * const d_ptr;*/
};

#endif // COUCHDBCHANGES_H
