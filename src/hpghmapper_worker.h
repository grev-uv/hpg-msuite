#ifndef HPGHMAPPER_WORKER_H
#define HPGHMAPPER_WORKER_H

#include <QObject>
#include <QProcess>

class HPGHmapper_worker : public QObject
{
    Q_OBJECT

public:
    HPGHmapper_worker(QObject *parent = nullptr);

    /**
     * @brief Solicita al worker que comience
     * @param mapper_path   ruta del ejecutable
     * @param parametros    opciones para la ejecución
     * @param csv_path      ruta para guardas los ficheros mapeados
     * @param lista_1       ficheros mC
     * @param lista_2       ficheros hmC
     */
    void solicitud_mapeado(QString mapper_path,
                           QStringList parametros,
                           QString csv_path,
                           QString bwt_path,
                           QStringList lista_1,
                           QStringList lista_2);

    /**
     * @brief Solicita al worker que se detenga
     */
    void abort();

    /**
     * @brief objeto para lanzar proceso externo de mapeado
     */
    QProcess *proceso;

signals:
    /**
     * @brief Esta señal se emite cuando se le solicita al proceso que se active
     */
    void bam_solicitado();

    /**
     * @brief Esta señal se emite cuando se acaba el alineamiento de cada fastq
     */
    void bam_mapeado();

    /**
     * @brief Esta señal se emite cuando el proceso termina o se aborta
     */
    void finished();

public slots:
    /**
     * @brief ejecuta el trabajo de mapeado sobre todos los elementos de la lista
     */
    void lectura();

private:
    bool aborted;
    bool working;
    QStringList lista_1;
    QStringList lista_2;
    QStringList argumentos;
    QString path2mapper;
    QString path2csv;
    QString path2bwt;
};

#endif // HPGHMAPPER_WORKER_H
