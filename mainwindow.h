#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>
#include <cstdint>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:
    void connect_tpc_server();
    void socket_connect();
    void connection_lost();
    void send_request();
    void update_plot();
    void clear_plot();
    void request_mode(const QString &mode);
    void read_data();
    void ajustar_y();
private:
    Ui::MainWindow *ui;
    QTcpSocket socket;
    QTimer timer;
    uint16_t CRC(uint8_t *data,uint16_t len);
    uint8_t getMSB(uint16_t data);
    uint8_t getLSB(uint16_t data);
    int maxplot_points=10;
    int registercount=0;
    QVector<QVector<double>> plot_x;
    QVector<QVector<double>> plot_y;
    double limitecircular=0;
    void cant_plot(int count);
    QByteArray respuesta;
};
#endif // MAINWINDOW_H
