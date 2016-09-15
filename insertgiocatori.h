#ifndef GIOCATORIINSERT_H
#define GIOCATORIINSERT_H

#include "giocatore.h"

#include <QSqlTableModel>
#include <QStringList>
#include <QList>

class GiocatoriInsert : public QAbstractTableModel
{
    Q_OBJECT

public:
    GiocatoriInsert(QObject *parent = 0);//, QSqlDatabase db);
    QList<Giocatore*> GetGiocatori() { return m_giocatori; }
    void setPlayers(const QList<Giocatore*>& players);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

private:

    QList<Giocatore*> m_giocatori;
};
#endif
