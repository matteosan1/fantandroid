#ifndef STATS_H
#define STATS_H

#include <QObject>

class QSqlDatabase;
class QTextEdit;

class Stats : public QObject
{
    Q_OBJECT
public:
    explicit Stats(QSqlDatabase* db,
                   QTextEdit* tableTopScorer,
                   QTextEdit* tableTopWeek,
                   QTextEdit* tableFlopWeek,
                   QTextEdit* tableTopOverall,
                   QObject *parent = 0);

    void init();
    void fillTopScorerRanking();
    void fillTopFlop();

signals:

public slots:

private:
    QSqlDatabase* m_db;
    QTextEdit* m_tableTopScorer;
    QTextEdit* m_tableTopWeek;
    QTextEdit* m_tableFlopWeek;
    QTextEdit* m_tableTopOverall;

    bool          m_topScorerRanking;
    bool          m_topFlop;
};

#endif // STATS_H
