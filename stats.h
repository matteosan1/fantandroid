#ifndef STATS_H
#define STATS_H

#include <QObject>

class QSqlDatabase;
class QTableWidget;

class Stats : public QObject
{
    Q_OBJECT
public:
    explicit Stats(QSqlDatabase* db,
                   QTableWidget* tableTopScorer,
                   QTableWidget* tableTopWeek,
                   QTableWidget* tableFlopWeek,
                   QTableWidget* tableTopOverall,
                   QObject *parent = 0);

    void init();
    void fillTopScorerRanking();
    void fillTopFlop();

signals:

public slots:

private:
    QSqlDatabase* m_db;
    QTableWidget* m_tableTopScorer;
    QTableWidget* m_tableTopWeek;
    QTableWidget* m_tableFlopWeek;
    QTableWidget* m_tableTopOverall;

    bool          m_topScorerRanking;
    bool          m_topFlop;
};

#endif // STATS_H
