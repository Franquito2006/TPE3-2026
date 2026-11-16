#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QColor>
#include <QPen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_connect,&QPushButton::clicked,this,&MainWindow::connect_tpc_server);
    connect(&socket,&QTcpSocket::connected,this,&MainWindow::socket_connect);
    connect(&socket,&QTcpSocket::disconnected,this,&MainWindow::connection_lost);
    connect(ui->pushButton_request,&QPushButton::clicked,this,&MainWindow::send_request);
    connect(&timer,&QTimer::timeout,this,&MainWindow::send_request);
    connect(ui->comboBox_interval,&QComboBox::currentTextChanged,this,&MainWindow::request_mode);

    connect(ui->pushButton_update,&QPushButton::clicked,this,&::MainWindow::update_plot);
    connect(ui->pushButton_clearplot,&QPushButton::clicked,this,&MainWindow::clear_plot);

    connect(&socket,&QTcpSocket::readyRead,this,&MainWindow::read_data);
}

void MainWindow::connect_tpc_server()
{
    if(socket.state()==QAbstractSocket::UnconnectedState)
    {
        QString ip=ui->lineEdit_serverip->text().trimmed();
        bool ok;
        quint16 port=ui->lineEdit_port->text().toUShort(&ok);

        if(ip.isEmpty() || !ok || port==0)
        {
            qDebug()<<"error, ingrese una ip o puerto valido";
            return;
        }
        socket.connectToHost(ip,port);
    }
    else
    {
        socket.disconnectFromHost();
    }
}

void MainWindow::socket_connect()
{
    ui->pushButton_connect->setText("disconnect");
    ui->pushButton_connect->setStyleSheet("background-color: green;""color: white;");
    ui->lineEdit_serverip->setEnabled(false);
    ui->lineEdit_port->setEnabled(false);

    request_mode(ui->comboBox_interval->currentText());
}

void MainWindow::connection_lost()
{
    ui->pushButton_connect->setText("connect");
    ui->pushButton_connect->setStyleSheet("background-color: red;""color:white;");
    ui->lineEdit_serverip->setEnabled(true);
    ui->lineEdit_port->setEnabled(true);

    timer.stop();
    ui->pushButton_request->setEnabled(false);
}

uint16_t MainWindow::CRC(uint8_t *data, uint16_t len)
{
    uint16_t i, j, temp_bit, temp_int, crc;
    crc = 0xFFFF;
    for ( i = 0; i < len; i++ )
    {
        temp_int = *data++;
        crc ^= temp_int;
        for ( j = 0; j < 8; j++ )
        {
            temp_bit = crc & 0x0001;
            crc >>= 1;
            if ( temp_bit != 0 )
                crc ^= 0xA001;
        }
    }
    return crc;
}

uint8_t MainWindow::getLSB(uint16_t data)
{
    return static_cast<uint8_t>(data & 0xFF);
}

uint8_t MainWindow::getMSB(uint16_t data)
{
    return static_cast<uint8_t>((data>>8)&0xFF);
}

void MainWindow::send_request()
{
    if(socket.state()!=QAbstractSocket::ConnectedState)
    {
        qDebug()<<"error debe estar conectado añ¿l servidor para mandar solicitudes";
        return;
    }

    uint8_t frame[8];

    uint8_t slaveaddress=ui->lineEdit_slaveaddress->text().toInt();
    uint16_t dataaddress=ui->lineEdit_startdata->text().toUShort();
    uint16_t count=ui->lineEdit_count->text().toUShort();

    cant_plot(count);
    frame[0]=slaveaddress;
    frame[1]=0x03;
    frame[2]=getMSB(dataaddress);
    frame[3]=getLSB(dataaddress);
    frame[4]=getMSB(count);
    frame[5]=getLSB(count);

    uint16_t crc=CRC(frame,6);
    frame[6]=getLSB(crc);
    frame[7]=getMSB(crc);

    QByteArray trama(reinterpret_cast<char*>(frame),8);

    qint64 enviar=socket.write(trama);
}

void MainWindow::request_mode(const QString &mode)
{
    if(socket.state()!=QAbstractSocket::ConnectedState)
    {
        timer.stop();
        return;
    }
    if(mode=="Manual Request")
    {
        timer.stop();
        ui->pushButton_request->setEnabled(true);
    }
    else
    {
        ui->pushButton_request->setEnabled(false);

        if(mode=="0.5 seg")
        {
            timer.start(500);
        }
        if(mode=="1 seg")
        {
            timer.start(1000);
        }
        if(mode=="5 seg")
        {
            timer.start(5000);
        }
        if(mode=="10 seg")
        {
            timer.start(10000);
        }
        if(mode=="30 seg")
        {
            timer.start(30000);
        }
    }
}

void MainWindow::cant_plot(int count)
{
    if(count==registercount)
    {
        return;
    }

    ui->widget_grafica->clearGraphs();

    registercount=count;

    plot_x.resize(count);
    plot_y.resize(count);

    for(int i=0;i<count;i++)
    {
        ui->widget_grafica->addGraph();
        ui->widget_grafica->graph(i)->setName("registro"+QString::number(i+1));
        QColor color=QColor::fromHsv((i*137)%360,200,220);
        ui->widget_grafica->graph(i)->setPen(QPen(color,2));
    }

    ui->widget_grafica->legend->setVisible(true);
    ui->widget_grafica->replot();
}

void MainWindow::update_plot()
{
    bool ok;
    int points=ui->lineEdit_plotpoint->text().toInt(&ok);

    if(!ok || points<=0)
    {
        qDebug()<<"ingrese una cantidad de puntos valida";
        return;
    }

    maxplot_points=points;

    for(int i=0;i<registercount;i++)
    {
        while(plot_x[i].size()>maxplot_points)
        {
            plot_x[i].removeFirst();
            plot_y[i].removeFirst();
        }

        ui->widget_grafica->graph(i)->setData(plot_x[i],plot_y[i]);
    }
    ui->widget_grafica->replot();
}

void MainWindow::clear_plot()
{
    limitecircular=0;

    for(int i=0;i<registercount;i++)
    {
        plot_x[i].clear();
        plot_y[i].clear();
        ui->widget_grafica->graph(i)->setData(plot_x[i],plot_y[i]);
    }
    ui->widget_grafica->replot();
}

void MainWindow::read_data()
{
    QByteArray datos=socket.readAll();

    respuesta.append(datos);
    if(respuesta.size()<3)
    {
        qDebug()<<"formato recibido invalido";
        return;
    }

    uint8_t count=static_cast<uint8_t>(respuesta[2]);

    int longitud=3+count+2;
    for(int i=0;i<registercount;i++)
    {
        int pos=3+i*2;
        uint8_t msb=static_cast<uint8_t>(respuesta[pos]);
        uint8_t lsb=static_cast<uint8_t>(respuesta[pos+1]);

        uint16_t valor=(static_cast<uint16_t>(msb)<<8)|lsb;

        plot_x[i].append(limitecircular);
        plot_y[i].append(valor);

        while (plot_x[i].size()>maxplot_points)
        {
            plot_x[i].removeFirst();
            plot_y[i].removeFirst();
        }
        ui->widget_grafica->graph(i)->setData(plot_x[i],plot_y[i]);
    }

    limitecircular++;
    respuesta.remove(0,longitud);
    ui->widget_grafica->xAxis->setRange(limitecircular-maxplot_points,limitecircular);
    ajustar_y();
    ui->widget_grafica->replot();
}

void MainWindow::ajustar_y()
{
    double min=0;
    double max=0;
    bool primervalor=true;

    for(int i=0;i<registercount;i++)
    {
        for(int j=0;j<plot_y[i].size();j++)
        {
            double valor=plot_y[i][j];
            if(primervalor)
            {
                min=valor;
                max=valor;
                primervalor=false;
            }
            else
            {
                if(valor<min)
                {
                    min=valor;
                }
                if(valor>max)
                {
                    max=valor;
                }
            }
        }
    }
    if(primervalor)
    {
        return;
    }
     double distancia=max-min;
    double margen=distancia*0.10;

    if(distancia==0)
    {
        margen=max*0.10+1;
    }

    ui->widget_grafica->yAxis->setRange(min-margen,max+margen);
}

MainWindow::~MainWindow()
{
    delete ui;
}
