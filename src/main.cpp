#include "iface.h"
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IFace w;

    w.setWindowTitle("From FastQ to DMRs App");
    //w.setStyleSheet("QMainWindow {background: 'grey';}");
    w.setWindowIcon(QIcon(":/icon.png"));

    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            w.size(),
            qApp->desktop()->availableGeometry()
        )
    );

    w.show();

    return a.exec();
}
