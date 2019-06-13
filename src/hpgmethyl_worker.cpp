#include "hpgmethyl_worker.h"
#include <QDir>
#include <QDebug>


HPGMethyl_worker::HPGMethyl_worker(QObject *parent)
    : QObject(parent)
{
    aborted      = false;
    working      = false;
    fastq_paired = false;
}

void HPGMethyl_worker::solicitud_alineamiento(QString methyl_path,
                                              QStringList parametros,
                                              QString bam_path,
                                              QStringList lista)
{
    path2methyl  = methyl_path;
    path2bam     = bam_path;

    // parametros recibidos:
    //  0   bwt_path
    //  1   num_procesadores
    //  2   min_CAL
    //  3   CAL factor
    //  4   best_align
    //  5   best_hits
    //  6   best_score
    argumentos   = parametros;

    lista_1      = lista;
    aborted      = false;
    working      = true;
    fastq_paired = false;

    emit alineamiento_solicitado();
}

void HPGMethyl_worker::solicitud_alineamiento(QString methyl_path,
                                              QStringList parametros,
                                              QString bam_path,
                                              QStringList lista_a,
                                              QStringList lista_b)
{
    path2methyl  = methyl_path;
    path2bam     = bam_path;

    // parametros recibidos:
    //  0   bwt_path
    //  1   num_procesadores
    //  2   min_CAL
    //  3   CAL factor
    //  4   best_align
    //  5   best_hits
    //  6   best_score
    //  7   min_dist_paired
    //  8   max_dist_paired
    argumentos   = parametros;

    lista_1      = lista_a;
    lista_2      = lista_b;
    aborted      = false;
    working      = true;
    fastq_paired = true;

    emit alineamiento_solicitado();
}

void HPGMethyl_worker::abort()
{
    if (working)
        aborted = true;
}

void HPGMethyl_worker::lectura()
{
    proceso = new QProcess();
    QStringList args;

    // bucle sobre la lista a alinear para ejecutar el proceso ordenadamente
    for (int i = 0; i < lista_1.size(); i++)
    {
        if (!aborted)
        {
            // crea el directorio particular donde guardar el archivo BAM
            QString name = path2bam + lista_1[i].split("/").last().split(".").first();
            QDir fastq_path (name);
            if (!fastq_path.exists())
                fastq_path.mkpath(".");

            // compone los argumentos
            args.clear();
            args << "bs" <<
                    "-i" << argumentos[0] <<
                    "-f" << lista_1[i];
            if (fastq_paired)
                args << "-j" << lista_2[i];
            args << "-o" << name <<
                    "--cpu-threads" << argumentos[1];
            if (fastq_paired)
            {
                args << "--paired-mode" << "1"; // <<
                //     "--paired-min-distance" << argumentos[7] <<
                //     "--paired-max-distande" << argumentos[8] <<
            }
            args << //"--min_cal-size" << argumentos[2] <<
               //     "--umbral-cal-length-factor" << argumentos[3] <<
               //     "--report-n-best" << argumentos[4] <<
               //     "--report-n-hits" << argumentos[5] <<
               //     "--filter-read-mappings" << argumentos[6] <<
                    "--write-mcontext";

            // proceso de alineamiento
            proceso->execute(path2methyl, args);
            proceso->waitForFinished();                             // espera a la finalización del proceso
            QString result = proceso->readAllStandardError();       // captura de errores del proceso
            proceso->close();                                       // cierre del proceso

            // muestra los erroes que se han producido por consola
            qDebug() << "error alineando: " << result;

            emit fastq_alineado();
        }
        else
            break;
    }

    working = false;
    emit finished();
}

