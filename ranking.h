#ifndef RANKING_H
#define RANKING_H

#include <QObject>

class QTableWidget;
class QSqlDatabase;
class QLabel;

class Ranking : public QObject
{
    Q_OBJECT

public:
    explicit Ranking(int widgetWidth, QSqlDatabase* db, QLabel* label, QTableWidget* tableRanking, QTableWidget* tableScores, QObject *parent = 0);

    void init();
    void computeRanking();
    void reportResults();

public slots:
    void changeRoundUp();
    void changeRoundDown();

private:
    void computeColumnWidth();

    int           m_widgetWidth;
    QSqlDatabase* m_db;
    QLabel*       m_label;
    QTableWidget* m_tableRanking;
    QTableWidget* m_tableScores;
    bool          m_rankingComputed;
    int           m_round;
    int           m_maxRounds;
};

#endif // RANKING_H
