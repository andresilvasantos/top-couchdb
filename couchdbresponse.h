#ifndef COUCHDBRESPONSE_H
#define COUCHDBRESPONSE_H

#include <QObject>

#include "couchdbenums.h"

class CouchDBResponsePrivate;
class CouchDBResponse : public QObject
{
    Q_OBJECT
public:
    explicit CouchDBResponse(QObject *parent = 0);
    virtual ~CouchDBResponse();

    QString database() const;
    void setDatabase(const QString& database);

    QString documentID() const;
    void setDocumentID(const QString& documentID);

    CouchDBReplyStatus status() const;
    void setStatus(const CouchDBReplyStatus& status);

    QByteArray data() const;
    void setData(const QByteArray& data);

private:
    Q_DECLARE_PRIVATE(CouchDBResponse)
    CouchDBResponsePrivate * const d_ptr;
};

#endif // COUCHDBRESPONSE_H
