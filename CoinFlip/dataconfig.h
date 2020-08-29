#ifndef DATACONFIG_H
#define DATACONFIG_H

#include <QObject>
#include <QMap>
#include <QVector>

class dataConfig : public QObject
{
    Q_OBJECT
public:
    explicit dataConfig(QObject *parent = nullptr);
    QMap<int, QVector< QVector<int> > >mData;

signals:

};

#endif // DATACONFIG_H
