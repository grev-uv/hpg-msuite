#ifndef HPGMETHYL_WORKER_H
#define HPGMETHYL_WORKER_H

#include <QObject>
#include <QProcess>


class HPGMethyl_worker : public QObject
{
    Q_OBJECT

public:
    HPGMethyl_worker(QObject *parent = nullptr);

    /**
     * @brief Solicita al worker que comience
     * @param methyl_path   ruta del ejecutable
     * @param parametros    opciones para la ejecución
     * @param bam_path      ruta para guardas los ficheros mapeados
     * @param lista         ficheros fastq para opción sin pareado
     * @param lista_1       ficheros fastq
     * @param lista_2       ficheros fastq pareados
     */
    void solicitud_alineamiento(QString methyl_path,
                                QStringList parametros,
                                QString bam_path,
                                QStringList lista);

    void solicitud_alineamiento(QString methyl_path,
                                QStringList parametros,
                                QString bam_path,
                                QStringList lista_1,
                                QStringList lista_2);

    /**
     * @brief Solicita al worker que se detenga
     */
    void abort();

    /**
     * @brief objeto para lanzar proceso externo de alineamiento
     */
    QProcess *proceso;

signals:
    /**
     * @brief Esta señal se emite cuando se le solicita al proceso que se active
     */
    void alineamiento_solicitado();

    /**
     * @brief Esta señal se emite cuando se acaba el alineamiento de cada fastq
     */
    void fastq_alineado();

    /**
     * @brief Esta señal se emite cuando el proceso termina o se aborta
     */
    void finished();

public slots:
    /**
     * @brief ejecuta el trabajo de alineamiento sobre todos los elementos de la lista
     */
    void lectura();

private:
    bool aborted;
    bool working;
    bool fastq_paired;
    QStringList lista_1;
    QStringList lista_2;
    QStringList argumentos;
    QString path2methyl;
    QString path2bam;
};

#endif // HPGMETHYL_WORKER_H
