#include "iface.h"
#include "ui_iface.h"
#include <QList>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QProcess>
#include <QScrollBar>

IFace::IFace(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IFace)
{
    ui->setupUi(this);

    // inicialización de ventanas
    fastq2bam = new Fastq2bam(this);
    bam2csv   = new Bam2csv(this);

    // obtención de características hardware del equipo
    //---------------------------------------------------------------------------------------------
    // comprueba la memoria disponible del equipo
    QProcess p;
    p.start("awk", QStringList() << "/MemAvailable/ {print $2}" << "/proc/meminfo");
    p.waitForFinished();
    QString data = p.readAllStandardOutput();
    p.close();
    // se asigna a la variable el valor en GB
    memory_available = data.toInt()/(1024 * 1024);

    // comprueba el número de procesadores disponible en el equipo
    p.start("nproc", QStringList() << "--all");
    p.waitForFinished();
    data = p.readAllStandardOutput();
    p.close();

    // se actualizan los valores máximos de los selectores de número de procesadores a emplear en
    // los procesos de alineamiento o mapeado
    fastq2bam->set_fastq_nproc(data.toInt());
    bam2csv->set_bam_nproc(data.toInt());
    bam2csv->set_memory_available(memory_available);

    // muestra los valores de memoria y #procesadores en la barra de estado inferior de la ventana
    ui->statusBar->showMessage("System available RAM: " +
                               QString::number(memory_available) +
                               "GB - Number of processors: " +
                               data);
}

IFace::~IFace()
{
    delete ui;
}

//*************************************************************************************************
//**********************************MENU PRINCIPAL*************************************************
//*************************************************************************************************
void IFace::on_fastq_2_bam_clicked()
{
    // abre la ventana de control
    fastq2bam->show();
}

//*************************************************************************************************
void IFace::on_bam_2_csv_clicked()
{
    // abre la ventana de control
    bam2csv->show();
}

//*************************************************************************************************
void IFace::on_dmr_clicked()
{
    // lanza la visualización y detección de DMRs
    QProcess p;
    int dato = p.execute("/home/lifercor/programas/Bio-Informatica/qt/HPG_Dhunter/HPG_Dhunter_v1-3_Esc/HPG_Dhunter/hpg_dhunter");
    p.waitForFinished(-1); // evitamos problemas de terminación más allá de los 30 segundos
    p.close();

    qDebug() << "info sobre la ejecución de la visualización: " << dato;
}

