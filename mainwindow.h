#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QHorizontalStackedBarSeries>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    QPushButton *exit = new QPushButton("Exit Program");
    QPushButton *drawGantt = new QPushButton("Draw Gantt");

    ~MainWindow();

private:

    Ui::MainWindow *ui;
    int itr1 = 0, itr2 = 0, itr3 = 0, algorithmNumber = 0;
    float arr[1000] = {0}, timeQuantum = 0;
    int pr[1000], o[1000];
    QGridLayout *layout = new QGridLayout;
    QStandardItemModel  *model = new QStandardItemModel;
    QHorizontalStackedBarSeries *gantt = new QHorizontalStackedBarSeries;
    QTableView *showTable = new QTableView();
    QBarSet *p[1000];
    QChart *chart = new QChart;
    QChartView *showGantt = new QChartView(chart);
    QLabel *avgTurnaround = new QLabel("");
    QLabel *avg = new QLabel("");

    void createMainMenu();

public slots:

    void getAlgorithm(const int algorithmNO);
    void getNumberOfProcesses(const int rows);
    void getBurst(const QModelIndex &index);
    void appendBurst();
    void getQuantum(const int q);
    void restart();
};

#endif // MAINWINDOW_H
