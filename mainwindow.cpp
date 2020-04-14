#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QtCharts>
#include <QBarSet>
#include <QHorizontalStackedBarSeries>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QDebug>
#include <QSpinBox>
#include <QInputDialog>
#include <QValueAxis>
#include <QRandomGenerator>
#include <QLegend>
#include <QLegendMarker>
#include <QMessageBox>

QT_CHARTS_USE_NAMESPACE

using namespace std;

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createMainMenu();
}

void MainWindow::createMainMenu()
{
    setWindowTitle("CPU Scheduler");

    QLabel *m1 = new QLabel("Please choose the algorithm:");
    QComboBox *alg = new QComboBox;
    QLabel *m2 = new QLabel("Please enter the number of processes:");
    QInputDialog *processNo = new QInputDialog;
    QPushButton *reset = new QPushButton("Reset");

    processNo->setLabelText("");
    processNo->setInputMode(QInputDialog::IntInput);
    processNo->setCancelButtonText("");
    processNo->clearFocus();

    alg->addItem("First Come First Serve");
    alg->addItem("Shortest Job First (Non Preemptive)");
    alg->addItem("Shortest Remaining Time First (Preemptive)");
    alg->addItem("Round Robin");
    alg->addItem("Non Preemptive Priority");
    alg->addItem("Preemptive Priority");

    layout->setVerticalSpacing(10);
    layout->setHorizontalSpacing(10);

    layout->setColumnMinimumWidth(0, 450);
    layout->setColumnMinimumWidth(1, 450);

    layout->addWidget(m1, 0, 0);
    layout->addWidget(alg, 0, 1);
    layout->addWidget(m2, 2, 0);
    layout->addWidget(processNo, 2, 1);
    layout->addWidget(reset, 23, 0);
    layout->addWidget(exit, 23, 1);

    QObject::connect(alg, SIGNAL(activated(int)), this, SLOT(getAlgorithm(int)));
    QObject::connect(processNo, SIGNAL(intValueSelected(int)), this, SLOT(getNumberOfProcesses(int)));
    QObject::connect(drawGantt, SIGNAL(clicked()), this, SLOT(appendBurst()));
    QObject::connect(reset, SIGNAL(clicked()), this, SLOT(restart()));

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);
    resize(900,700);
}

void MainWindow::getNumberOfProcesses(const int rows)
{
    model->setRowCount(rows);

    for (int row = 0; row < model->rowCount(); ++row)
    {
        for (int column = 0; column < model->columnCount(); ++column)
        {
            QStandardItem *item = new QStandardItem();
            model->setItem(row, column, item);
        }
    }

    model->setHorizontalHeaderItem(0, new QStandardItem(tr("Burst Time")));
    model->setHorizontalHeaderItem(1, new QStandardItem(tr("Arrival Time")));

    showTable->setModel(model);
    showTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    layout->addWidget(showTable, 3, 0, 3, 1);
    layout->addWidget(drawGantt, 7, 0, 1, 1);

    QObject::connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(getBurst(QModelIndex)));
}

void MainWindow::getBurst(const QModelIndex &index)
{
    if (index.isValid())
    {
        QString number = index.data().toString();
        for(int i = 0, l = number.length(); i < l; i++)
        {
            if(!(number[i].isDigit() || number[i] == '.'))
            {
                QMessageBox enteredStirng;
                enteredStirng.setText("Please enter a number");
                enteredStirng.exec();
                restart();
                break;
            }
        }
        if(index.column() == 0)
        {
            o[itr1] = index.row();
            p[index.row()] = new QBarSet(QString("p%1").arg(o[itr1] + 1));
            p[index.row()]->append(index.data().toFloat());
            itr1++;
        }
        if(index.column() == 1)
        {
            arr[index.row()] = index.data().toFloat();
            itr2++;
        }
        if(index.column() == 2)
        {
            pr[index.row()] = index.data().toInt();
            itr3++;
        }
    }
}

void MainWindow::getAlgorithm(const int algorithmNO)
{
    algorithmNumber = algorithmNO;
    if(algorithmNO == 4 || algorithmNO == 5)
    {
        model->setColumnCount(3);
        model->setHorizontalHeaderItem(2, new QStandardItem(tr("Priority")));
    }
    else
    {
        model->setColumnCount(2);
    }

    if(algorithmNO == 2 || algorithmNO == 3 || algorithmNO == 5)
    {
        QInputDialog *Quantum = new QInputDialog;

        Quantum->setLabelText("Please enter the time Quantum:");
        Quantum->setInputMode(QInputDialog::IntInput);
        layout->addWidget(Quantum, 1, 0);
        QObject::connect(Quantum, SIGNAL(intValueSelected(int)), this, SLOT(getQuantum(int)));
    }
}

void MainWindow::getQuantum(const int q)
{
    timeQuantum = q;
}

void MainWindow::restart()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void MainWindow::appendBurst()
{
    drawGantt->hide();

    float lastArrived = 0, sumOfBurst = 0, avgWait = 0, idle = 0, avgTurn = 0;

    for(int i = 0; i < itr2; i++)
    {
        if(arr[i] > lastArrived)
        {
            lastArrived = arr[i];
        }
    }

    for(int i = 0; i < itr1; i++)
    {
        sumOfBurst += (*p[i])[0];

    }

    if(algorithmNumber == 0)
    {

        float currTime = 0, compTime = 0, turnaround = 0;
        for(int i = 0; i < itr2; i++)
        {
            for(int j = 0; j < itr2 - i - 1; j++)
            {
                if (arr[j] > arr[j + 1])
                {
                    swap(p[j], p[j + 1]);
                    swap(arr[j], arr[j + 1]);
                }
            }
        }

        for(int i = 0; i < itr1; i++)
        {
            if(currTime < arr[i])
            {
                QBarSet *temp = new QBarSet(QString("Idle"));
                temp->append(arr[i] - currTime);
                gantt->append(temp);
                temp->setColor(Qt::black);
                idle += arr[i] - currTime;
                currTime = arr[i];
            }
            gantt->append(p[i]);
            currTime += (*p[i])[0];
            compTime = currTime;
            turnaround = compTime - arr[i];
            avgTurn += turnaround;
            avgWait += turnaround - (*p[i])[0];
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    else if(algorithmNumber == 1)
    {
        int arrivedProcess = 0;
        float currTime = 0, compTime = 0, turnaround = 0, sameArr = 0, allArrivedAt = 0;

        for(int i = 0; i < itr1; i++)
        {
            for(int j = 0; j < itr1 - i - 1; j++)
            {
                if (arr[o[j]] > arr[o[j] + 1])
                {
                    swap(p[o[j]], p[o[j] + 1]);
                    swap(arr[o[j]], arr[o[j] + 1]);
                }
            }
        }

        for(int i = 0; i < itr1; i++)
        {
            sameArr += arr[o[i]];
            if(arr[o[i]] != 0 && arr[o[i + 1]] != 0)
            {
                allArrivedAt = arr[o[i]];
            }
        }

        if(sameArr == 0 || sameArr == allArrivedAt * itr1)
        {
            for(int i = 0; i < itr1; i++)
            {
                for(int j = 0; j < itr1 - i - 1; j++)
                {
                    if ((*p[o[j]])[0] > (*p[o[j + 1]])[0])
                    {
                            swap(p[o[j]], p[o[j + 1]]);
                    }
                }
            }
            for(int i = 0; i < itr1; i++)
            {
                if((*p[o[i]])[0] != 0)
                {
                    gantt->append(p[o[i]]);
                    currTime += (*p[o[i]])[0];
                    compTime = currTime;
                    turnaround = compTime - arr[o[i]];
                    avgWait += turnaround - (*p[o[i]])[0];
                }
            }
        }
        else
        {
            for(int k = 0; k < itr2; k++)
            {
                for(int j = k; j < itr2; j++)
                {
                    if(currTime <= arr[o[j]])
                    {
                        arrivedProcess = j;
                        break;
                    }
                    if(currTime > lastArrived)
                    {
                        arrivedProcess = itr2 - 1;
                        break;
                    }
                }
                for(int i = 0; i < arrivedProcess; i++)
                {
                    for(int j = k; j < arrivedProcess + k - i - 2; j++)
                    {
                        if(j < arrivedProcess)
                        {
                            if ((*p[o[j]])[0] > (*p[o[j + 1]])[0])
                            {
                                swap(p[o[j]], p[o[j + 1]]);
                            }
                        }
                    }
                }
                if((*p[o[k]])[0] != 0)
                {
                    gantt->append(p[o[k]]);
                    currTime += (*p[o[k]])[0];
                    compTime = currTime;
                    turnaround = compTime - arr[o[k]];
                    avgTurn += turnaround;
                    avgWait += turnaround - (*p[o[k]])[0];
                }
            }
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    else if(algorithmNumber == 2)
    {
        int arrivedProcess = 0, currMinBurstIndex = 0, colorIndex = 0;
        float currTime = 0, temp = 0, compTime = 0, turnaround = 0, bursts[1000];
        bool allArrived = 0;
        QBarSet *srtf[1000];

        for(int i = 0; i < itr2; i++)
        {
            for(int j = 0; j < itr2 - i - 1; j++)
            {
                if (arr[j] > arr[j + 1])
                {
                    swap(p[j], p[j + 1]);
                    swap(arr[j], arr[j + 1]);
                }
            }
        }

        for(int i = 0; i < itr1; i++)
        {
            p[i]->setLabel(QString("%1").arg(((p[i]->label())[1])));
            o[i] = p[i]->label().toInt();
            p[i]->setLabel(QString("p%1").arg(o[i]));
        }

        for(int i = 0; i < itr1; i++)
        {
            bursts[i] = (*p[i])[0];
        }

        float currMinBurst = sumOfBurst;

        while(sumOfBurst != currTime)
        {
            for(int i = 0; i < itr1; i++)
            {
                if(currTime < arr[i] && currTime != 0)
                {
                    arrivedProcess = i;
                    break;
                }
                if(currTime >= lastArrived)
                {
                    arrivedProcess = itr1;
                    allArrived = 1;
                }
            }
            for(int i = 0; i < arrivedProcess; i++)
            {
                if((*p[i])[0] < currMinBurst)
                {
                    currMinBurst = (*p[i])[0];
                    currMinBurstIndex = i;
                }
                if((*p[i])[0] == 0)
                {
                    currMinBurst = sumOfBurst;
                }
            }

            if(allArrived == 1)
            {
                for(int i = 0; i < itr1; i++)
                {
                    for(int j = 0; j < itr1 - i - 1; j++)
                    {
                        if ((*p[j])[0] > (*p[j + 1])[0])
                        {
                            swap(p[j], p[j + 1]);
                            swap(arr[j], arr[j + 1]);
                            swap(bursts[j],bursts[j + 1]);
                        }
                    }
                }
                for(int i = 0; i < itr1; i++)
                {
                    if((*p[i])[0] != 0)
                    {
                        gantt->append(p[i]);
                        currTime += (*p[i])[0];
                        compTime = currTime;
                        turnaround = compTime - arr[i];
                        avgTurn += turnaround;
                        avgWait += turnaround - bursts[i];
                    }
                }
            }
            else if ((*p[currMinBurstIndex])[0] != 0)
            {
                temp = (*p[currMinBurstIndex])[0];
                srtf[colorIndex] = new QBarSet(QString("p%1").arg(o[currMinBurstIndex]));
                srtf[colorIndex]->append(timeQuantum);
                gantt->append(srtf[colorIndex]);
                colorIndex++;
                currTime += timeQuantum;
                p[currMinBurstIndex]->remove(0,1);
                p[currMinBurstIndex]->append(temp - timeQuantum);
                if(temp <= timeQuantum)
                {
                    compTime = currTime;
                    turnaround = compTime - arr[currMinBurstIndex];
                    avgTurn += turnaround;
                    avgWait += turnaround - bursts[currMinBurstIndex];
                }
            }
        }

        int i = 0;
        while(i < itr1)
        {
            p[i]->setColor(QColor::fromRgb(QRandomGenerator::global()->generate()));
            for(int j = 0; j < colorIndex; j++)
            {
                if(srtf[j]->label() == p[i]->label())
                {
                    srtf[j]->setColor(p[i]->color());
                }
            }
            i++;
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    else if(algorithmNumber == 3)
    {
        int  arrivedProcess = 0, rq[1000] = {0}, q = 0, colorIndex = 0;
        float currTime = 0, compTime = 0, turnaround = 0, bursts[1000], temp = 0;
        QBarSet *rr[1000];

        for(int i = 0; i < itr1; i++)
        {
            for(int j = 0; j < itr1 - i - 1; j++)
            {
                if (arr[j] > arr[j + 1])
                {
                    swap(p[j], p[j + 1]);
                    swap(arr[j], arr[j + 1]);
                }
            }
        }

        for(int i = 0; i < itr1; i++)
        {
            p[i]->setLabel(QString("%1").arg(((p[i]->label())[1])));
            o[i] = p[i]->label().toInt();
            p[i]->setLabel(QString("p%1").arg(o[i]));
        }

        for(int i = 0; i < itr1; i++)
        {
            bursts[i] = (*p[i])[0];
        }

        for(int j = 0; j <= q; j++)
        {
            if(currTime < arr[rq[j]])
            {
                QBarSet *temp = new QBarSet(QString("Idle"));
                temp->append(arr[rq[j]] - currTime);
                gantt->append(temp);
                temp->setColor(Qt::black);
                idle += arr[rq[j]] - currTime;
                currTime = arr[rq[j]];
            }
            if((*p[rq[j]])[0] <= timeQuantum && (*p[rq[j]])[0] != 0)
            {
                temp = (*p[rq[j]])[0];
                rr[colorIndex] = new QBarSet(QString("p%1").arg(o[rq[j]]));
                rr[colorIndex]->append(temp);
                gantt->append(rr[colorIndex]);
                colorIndex++;
                currTime += temp;
                compTime = currTime;
                turnaround = compTime - arr[rq[j]];
                avgTurn += turnaround;
                avgWait += turnaround - bursts[rq[j]];
                p[rq[j]]->remove(0,1);
                p[rq[j]]->append(0);

                for(int i = 0; i < itr1; i++)
                {
                    if(currTime < arr[i])
                    {
                        arrivedProcess = i;
                        break;
                    }
                    if(currTime >= lastArrived)
                    {
                        arrivedProcess = itr1;
                    }
                }
                for(int i = j+1; i < arrivedProcess; i++)
                {
                    if(arrivedProcess < itr1)
                    {
                        q++;
                        rq[q] = i;
                    }
                }
            }
            else if((*p[rq[j]])[0] != 0)
            {
                temp = (*p[rq[j]])[0];
                rr[colorIndex] = new QBarSet(QString("p%1").arg(o[rq[j]]));
                rr[colorIndex]->append(timeQuantum);
                gantt->append(rr[colorIndex]);
                colorIndex++;
                currTime += timeQuantum;
                p[rq[j]]->remove(0, 1);
                p[rq[j]]->append(temp - timeQuantum);
                for(int i = 0; i < itr1; i++)
                {
                    if(currTime < arr[i])
                    {
                        arrivedProcess = i;
                        break;
                    }
                    if(currTime >= lastArrived)
                    {
                        arrivedProcess = itr1;
                    }
                }
                for(int i = j+1; i < arrivedProcess; i++)
                {
                    q++;
                    rq[q] = i;
                }
                q++;
                rq[q] = rq[j];
            }
        }

        int i = 0;
        while(i < itr1)
        {
            p[i]->setColor(QColor::fromRgb(QRandomGenerator::global()->generate()));
            for(int j = 0; j < colorIndex; j++)
            {
                if(rr[j]->label() == p[i]->label())
                {
                    rr[j]->setColor(p[i]->color());
                }
            }
            i++;
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    else if(algorithmNumber == 4)
    {
        int arrivedProcess = 0;
        float currTime = 0, compTime = 0, turnaround = 0;
        bool enteredArrived = 0;

        for(int i = 0; i < itr2 - 1; i++)
        {
            if(arr[i] != arr[i + 1])
            {
                enteredArrived = 1;
            }
        }

        if(enteredArrived == 1)
        {
            for(int k = 0; k < itr2; k++)
            {
                for(int j = k; j < itr2; j++)
                {
                    if(currTime <= arr[j])
                    {
                        arrivedProcess = j;
                        break;
                    }
                    if(currTime > lastArrived)
                    {
                        arrivedProcess = itr2 - 1;
                        break;
                    }
                }
                for(int i = 0; i < arrivedProcess; i++)
                {
                    for(int j = k; j < arrivedProcess + k - i - 2; j++)
                    {
                        if(j < arrivedProcess)
                        {
                            if (pr[j] > pr[j + 1])
                            {
                                swap(pr[j], pr[j + 1]);
                                swap(p[j], p[j + 1]);
                            }
                        }
                    }
                }
                gantt->append(p[k]);
                currTime += (*p[k])[0];
                compTime = currTime;
                turnaround = compTime - arr[k];
                avgTurn += turnaround;
                avgWait += turnaround - (*p[k])[0];
            }
        }
        // If user didn't provide arrived times, or all arrived at the same time
        else
        {
            for(int i = 0; i < itr3; i++)
            {
                for(int j = 0; j < itr3 - i - 1; j++)
                {
                    if (pr[j] > pr[j + 1])
                    {
                        swap(pr[j], pr[j + 1]);
                        swap(p[j], p[j + 1]);
                    }
                }
            }
            for(int i = 0; i < itr1; i++)
            {
                gantt->append(p[i]);
                currTime += (*p[i])[0];
                compTime = currTime;
                turnaround = compTime - arr[i];
                avgTurn += turnaround;
                avgWait += turnaround - (*p[i])[0];
            }
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    else if(algorithmNumber == 5)
    {
        float sumOfBurst = 0, secondToArrive = 0, currTime = 0, compTime = 0, turnaround = 0, temp = 0, bursts[1000];
        int currMaxPriorityIndex = 0, maxPriority = 0, colorIndex = 0, arrivedProcess = 0;
        int samePriority[1000], sameIndex = 0, nameIterator = -2;
        bool allArrived = 0, finished = 0, out = 0;
        QBarSet *prPre[1000];

        for(int i = 0; i < itr1; i++)
        {
            sumOfBurst += (*p[i])[0];
            bursts[i] = (*p[i])[0];
        }

        for(int i = 0; i < itr3; i++)
        {
            if(pr[i] > maxPriority)
            {
                maxPriority = arr[i];
            }
        }

        for(int i = 0; i < itr3; i++)
        {
            for(int j = 1; j < itr3; j++)
            {
                if(pr[i] == pr[j] && i != j && samePriority[sameIndex - 2] !=j)
                {
                    samePriority[sameIndex] = i;
                    samePriority[++sameIndex] = j;
                    sameIndex ++;
                    break;
                }
            }
        }

        for(int i = 0; i < sameIndex; i++)
        {
            for(int j = 0; j < sameIndex - i - 1; j++)
            {
                if (pr[samePriority[j]] > pr[samePriority[j + 1]])
                {
                    swap(samePriority[j], samePriority[j + 1]);
                }
            }
        }

        int currMaxPriority = maxPriority;

        while(sumOfBurst != currTime)
        {
            for(int i = 0; i < itr1; i++)
            {
                if(currTime < arr[i])
                {
                    arrivedProcess = i;
                    secondToArrive = arr[i];
                    break;
                }
                if(currTime >= lastArrived)
                {
                    arrivedProcess = itr1;
                    allArrived = 1;
                    break;
                }
            }
            for(int i = 0; i < arrivedProcess; i++)
            {
                if(pr[i] < currMaxPriority)
                {
                    currMaxPriority = pr[i];
                    currMaxPriorityIndex = i;
                }

                if((*p[currMaxPriorityIndex])[0] == 0)
                {
                    currMaxPriority = maxPriority;
                }
            }
            if(allArrived == 1)
            {
                for(int i = 0; i < itr3; i++)
                {
                    for(int j = 0; j < itr3 - i - 1; j++)
                    {
                        if (pr[j] > pr[j + 1])
                        {
                            swap(pr[j], pr[j + 1]);
                            swap(p[j], p[j + 1]);
                            swap(bursts[j], bursts[j + 1]);
                        }
                    }
                }
                for(int i = 0; i < itr1; i++)
                {
                    if(pr[i] == pr[i + 1])
                    {
                        nameIterator += 2;
                        out = 0;
                        while((*p[i])[0] != 0 && (*p[i + 1])[0] != 0 && out != 1)
                        {
                            sameIndex = 0 + nameIterator;
                            for(int j = i; j < i + 2; j++)
                            {
                                if(finished == 1 && (*p[j])[0] != 0)
                                {
                                    gantt->append(p[j]);
                                    currTime += (*p[j])[0];
                                    compTime = currTime;
                                    turnaround = compTime - arr[j];
                                    avgTurn += turnaround;
                                    avgWait += turnaround - bursts[j];
                                    finished = 0;
                                    out = 1;
                                    break;
                                }
                                if((*p[j])[0] <= timeQuantum)
                                {
                                    if((*p[j])[0] != 0)
                                    {
                                        gantt->append(p[j]);
                                        currTime += (*p[j])[0];
                                        compTime = currTime;
                                        turnaround = compTime - arr[j];
                                        avgTurn += turnaround;
                                        avgWait += turnaround - bursts[j];
                                        finished = 1;
                                    }
                                }
                                else
                                {
                                    temp = (*p[j])[0];
                                    prPre[colorIndex] = new QBarSet(QString("p%1").arg(samePriority[sameIndex] + 1));
                                    prPre[colorIndex]->append(timeQuantum);
                                    gantt->append(prPre[colorIndex]);
                                    colorIndex++;
                                    p[j]->remove(0, 1);
                                    p[j]->append(temp - timeQuantum);
                                    currTime += timeQuantum;
                                    sameIndex = nameIterator + 1;
                                }
                            }
                        }
                    }
                    else
                    {
                        if((*p[i])[0] != 0 && out != 1)
                        {
                            gantt->append(p[i]);
                            currTime += (*p[i])[0];
                            compTime = currTime;
                            turnaround = compTime - arr[i];
                            avgTurn += turnaround;
                            avgWait += turnaround - bursts[i];
                        }
                        out = 0;
                    }
                }
                break;
            }
            else if ((*p[currMaxPriorityIndex])[0] != 0)
            {
                temp = (*p[currMaxPriorityIndex])[0];
                prPre[colorIndex] = new QBarSet(QString("p%1").arg(currMaxPriorityIndex + 1));
                prPre[colorIndex]->append(secondToArrive - currTime);
                gantt->append(prPre[colorIndex]);
                colorIndex++;
                p[currMaxPriorityIndex]->remove(0,1);
                p[currMaxPriorityIndex]->append(temp - secondToArrive + currTime);
                currTime += secondToArrive - currTime;

                if(secondToArrive - currTime == temp)
                {
                    compTime = currTime;
                    turnaround = compTime - arr[currMaxPriorityIndex];
                    avgTurn += turnaround;
                    avgWait += turnaround - bursts[currMaxPriorityIndex];
                }
            }
        }

        int i = 0;
        while(i < itr1)
        {
            p[i]->setColor(QColor::fromRgb(QRandomGenerator::global()->generate()));
            for(int j = 0; j < colorIndex; j++)
            {
                if(prPre[j]->label() == p[i]->label())
                {
                    prPre[j]->setColor(p[i]->color());
                }
            }
            i++;
        }
        avgTurn = avgTurn / itr1;
        avgTurnaround->setText(QString("%1 ms").arg(avgTurn));
        avgWait = avgWait / itr1;
        avg->setText(QString("%1 ms").arg(avgWait));
    }

    chart->addSeries(gantt);

    for(int i = 0; i < chart->legend()->markers().count(); i++)
    {
        for(int j = i; j < chart->legend()->markers().count(); j++)
        {
            if((chart->legend()->markers())[i]->label() == (chart->legend()->markers())[j]->label() && i != j)
            {
                (chart->legend()->markers())[j]->setVisible(0);
            }
        }
    }

    chart->setTitle("Gantt Chart");
    chart->setAnimationOptions(QChart::AllAnimations);

    QStringList categories;
    categories << "Processes";

    QBarCategoryAxis *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    gantt->attachAxis(axisY);

    QValueAxis *axisX = new QValueAxis();

    axisX->setTitleText("Time (ms)");
    axisX->setTickCount(sumOfBurst + idle + 1);
    if(ceil(sumOfBurst + idle) == sumOfBurst + idle)
    {
        axisX->setLabelFormat("%.0f");
    }
    chart->addAxis(axisX, Qt::AlignBottom);
    gantt->attachAxis(axisX);

    QLabel *m3 = new QLabel("Average turn around time:");
    QLabel *m4 = new QLabel("Average waiting time:");

    layout->addWidget(showGantt, 8, 0, 12, 2);
    layout->addWidget(m3, 21, 0, 1, 1);
    layout->addWidget(avgTurnaround, 21, 1);
    layout->addWidget(m4, 22, 0, 1, 1);
    layout->addWidget(avg, 22, 1);
}

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void restart()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Doesn't handle if the processes entered are not sorted by arrival time in both priority algorithms
// Doesn't handle same priority processes with round robin if not all processes have arrived in preemptive priority algorithm
// User has to enter the values in the order they are asked, e.g. the algorithm before the number of processes
