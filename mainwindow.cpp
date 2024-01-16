#include "mainwindow.h"
#include "ui_mainwindow.h"

#define HEATER 2
#define COOLER 3

static double tempT=25, setPoint = 0, kp_values, kd_values, ki_values,temperT=0;
static bool warm = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    wiringPiSetup();
    pinMode(HEATER,OUTPUT);
    digitalWrite(HEATER,HIGH);
    pinMode(COOLER,OUTPUT);
    digitalWrite(COOLER,HIGH);

    ui->heatT->hide();
    ui->coolT->hide();

    Timer = new  QTimer(this);
    connect(Timer, SIGNAL(timeout()),this, SLOT(timer_TimeOut_event_slot()));
    Timer->setInterval(1000);//ms
    Timer->start();
    warm = false;


    ui->CustomPlot->addGraph();
    ui->CustomPlot->graph(0)->setPen(QPen(Qt::red));
    ui->CustomPlot->graph(0)->setAntialiasedFill(false);

    ui->CustomPlot->addGraph();
    ui->CustomPlot->graph(1)->setPen(QPen(Qt::blue));


    /* Configure x-Axis as time in secs */
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->CustomPlot->xAxis->setTicker(timeTicker);
    ui->CustomPlot->axisRect()->setupFullAxesBox();

    /* Configure x and y-Axis to display Labels */
    ui->CustomPlot->xAxis->setTickLabelFont(QFont(QFont().family(),8));
    ui->CustomPlot->yAxis->setTickLabelFont(QFont(QFont().family(),8));
    ui->CustomPlot->xAxis->setLabel("Time ");
    ui->CustomPlot->yAxis->setLabel("Temperature");


    /* Make top and right axis visible, but without ticks and label */
    ui->CustomPlot->xAxis2->setVisible(true);
    ui->CustomPlot->yAxis->setVisible(true);
    ui->CustomPlot->xAxis2->setTicks(false);
    ui->CustomPlot->yAxis2->setTicks(false);
    ui->CustomPlot->xAxis2->setTickLabels(false);
    ui->CustomPlot->yAxis2->setTickLabels(false);

    /* Set up and initialize the graph plotting timer */
    connect(&timer_plot, SIGNAL(timeout()),this,SLOT(realTimePlot()));
    timer_plot.start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_SetPoint_valueChanged(double arg1)
{
    const char TempMax=45;
    const char TempA=25;

    ui->lblSetPoint->setText(" ");
    //ui->SetPoint->getText
    if(arg1<=TempA){
        ui->lblSetPoint->setText("La temperatura tiene que ser mayor a 25°C");
    }
    else if (arg1<=TempMax) {
    warm = true;
    ui->label->setText("En proceso");
    setPoint = arg1;
    QString StringSP = QString::number(setPoint);
    ui->lblSetPoint->setText(StringSP);
    digitalWrite(HEATER,HIGH);
    digitalWrite(COOLER,HIGH);
    }
    else{
        ui->lblSetPoint->setText("La temperatura tiene que ser menor a 45°C");
    }
}

void MainWindow::realTimePlot()
{
   // static QElapsedTimer time;//(QTime::currentTime());
   // time.start();
    static int x =0;
    static double y =0;

   ui->CustomPlot->graph(0)->addData(x, temperT);
   ui->CustomPlot->graph(1)->addData(x, setPoint);

    x=x+1; //Update every  timeout of the timer_plot.start(1000);

   // y=y+0.5;
    /* make key axis range scroll right with the data at a constant range of 8. */
    ui->CustomPlot->graph(0)->rescaleValueAxis();
    ui->CustomPlot->yAxis->setRange(-10,50);
    ui->CustomPlot->xAxis->setRange(x, 8, Qt::AlignRight);
    ui->CustomPlot->replot();
}


void MainWindow::timer_TimeOut_event_slot()
{
    //Ganancias
    double kp=kp_values;  //Proporcional
    double kd=kd_values; 	//Derivativa
    double ki=ki_values;  //Integral
    double e;   //Error
    double u;   //Tempera
    double dx; //Derivada
    double ix;	//Integral
    double eprev=0; //Error previo
    double sume=0;

    QDir dir("/sys/bus/w1/devices");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList list = dir.entryList();

    if (list.size() > 0) {
        QFile file("/sys/bus/w1/devices/" + list.at(0) + "/w1_slave");

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QString line = stream.readLine();
            QString temp = line.mid(line.indexOf('=') + 1);
            QString alls = stream.readAll();
            temp = alls.mid(alls.indexOf('=') + 1);

            double temperature = temp.toDouble() / 1000.0;
            temperT = temperature;
            int temper = int(temperature);
            double checkT = tempT-temperature;
            // Update the label with the temperature
            //ui->Temperature->setText(QString::number(temperature));
            ui->currentTp->display(temperature);
            ui->progressBar->setValue(temper);

    //Control
    // Error
    e=setPoint-temperature;
    // Derivada del error
    dx=e-eprev;
    // Integral del error
    ix=e+sume;
    sume=ix;
    // Error previo
    eprev=tempT-temperature;;
    // Entrada a la planta
    u=kp*e+kd*eprev+ki*ix;

    QString Stringu = QString::number(u);
    ui->label_7->setText(Stringu);

    QString StringuP = QString::number(e);
    ui->label_8->setText(StringuP);

if(temperature < (setPoint-0.3)){
    if(warm == false){
        digitalWrite(HEATER,HIGH);
        digitalWrite(COOLER,HIGH);
    }
    else if(abs(u) < abs(e)){
        digitalWrite(HEATER,LOW);
        digitalWrite(COOLER,HIGH);
        ui->label->hide();
        ui->coolT->hide();
        ui->heatT->show();
        //delay(100);
    }
    else{
        digitalWrite(HEATER,HIGH);
        digitalWrite(COOLER,LOW);
        ui->label->hide();
        ui->heatT->hide();
        ui->coolT->show();
        //delay(100);
    }
}
else {
    digitalWrite(HEATER,HIGH);
    digitalWrite(COOLER,LOW);
    ui->label->hide();
    ui->heatT->hide();
    ui->coolT->hide();
}

 tempT = temperature;
          /*  if(checkT<-0.1){
                //ui->label->setText("Calentando");
                ui->label->hide();
                ui->coolT->hide();
                ui->heatT->show();
                tempT = temperature;
            }
            else if (checkT>0.1) {
                ui->label->hide();
                ui->heatT->hide();
                ui->coolT->show();
                tempT = temperature;
            }
            else {
                //ui->label->setText("Estable");
                //ui->label->show();
                ui->heatT->hide();
                ui->coolT->hide();
                tempT = temperature;
            }*/
        }
    }
}



void MainWindow::on_kd_value_valueChanged(double arg1)
{
    kd_values=arg1;
}

void MainWindow::on_kp_value_valueChanged(double arg1)
{
    kp_values=arg1;
}

void MainWindow::on_ki_value_valueChanged(double arg1)
{
    ki_values=arg1;
}
