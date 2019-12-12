#include "hpghmapper_worker.h"
#include <QDir>
#include <QDebug>

HPGHmapper_worker::HPGHmapper_worker(QObject *parent)
    : QObject(parent)
{
    aborted = false;
    working = false;
}

void HPGHmapper_worker::solicitud_mapeado(QString mapper_path,
                                          QStringList parametros,
                                          QString csv_path,
                                          QString bwt_path,
                                          QStringList lista_a,
                                          QStringList lista_b)
{
    path2mapper = mapper_path;

    // parametros recibidos:
    //  0   num_procesadores
    //  1   memory_available
    //  2   ram_%
    //  3   batch_size
    //  4   min_quality
    //  5   min_coverage
    argumentos  = parametros;

    path2csv    = csv_path;
    path2bwt    = bwt_path;
    lista_1     = lista_a;
    lista_2     = lista_b;

    aborted     = false;
    working     = true;

    emit bam_solicitado();
}

void HPGHmapper_worker::abort()
{
    if (working)
        aborted = true;
}

void HPGHmapper_worker::lectura()
{
    proceso = new QProcess();
    QStringList args;

    // bucle sobre la lista a alinear para ejecutar el proceso ordenadamente
    for (int i = 0; i < lista_1.size(); i++)
    {
        if (!aborted)
        {
            // crea el directorio particular donde guardar el archivo BAM
            QStringList folder = lista_1[i].split("/");
            QString name       = path2csv + folder[folder.size() - 2];
            QDir fastq_path (name);
            if (!fastq_path.exists())
                fastq_path.mkpath(".");

            // compone los argumentos
            args.clear();
            args << "-mc" << lista_1[i] <<
                    "-hmc" << lista_2[i] <<
                    "-o" << name + "/" <<
                    "--num-threads" << argumentos[0] <<
                    "--memory" << QString::number(int(argumentos[1].toDouble() * argumentos[2].toDouble() * 0.01)) + ".0G" <<
                    "--output-format" << "csv" <<
                    "--batch-size" << argumentos[3] <<
                    "--quality" << argumentos[4] <<
                    "--coverage" << argumentos[5];

            if (!path2bwt.isEmpty())
                args << "--bwt-index" << path2bwt;

            // proceso de alineamiento
            proceso->execute(path2mapper, args);
            proceso->waitForFinished();                             // espera a la finalización del proceso
            QString result = proceso->readAllStandardError();       // captura de errores del proceso
            proceso->close();                                       // cierre del proceso

            // muestra los erroes que se han producido por consola
            if (!result.isEmpty())
                qDebug() << "error alineando: " << result;

            emit bam_mapeado();
        }
        else
            break;
    }

    working = false;
    emit finished();
}
