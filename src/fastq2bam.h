#ifndef FASTQ2BAM_H
#define FASTQ2BAM_H

#include "hpgmethyl_worker.h"
#include <QDialog>
#include <QTextCursor>
#include <QThread>

namespace Ui {
class Fastq2bam;
}

class Fastq2bam : public QDialog
{
    Q_OBJECT

public:
    explicit Fastq2bam(QWidget *parent = nullptr);
    ~Fastq2bam();

    void set_fastq_nproc(int);

private slots:
    /** ***********************************************************************************************
      * \fn void on_exit_clicked()
      *  \brief Función responsable de ceerrar la ventana y devolver el control al menú prinicipal
      * ***********************************************************************************************
      */
    void on_exit_clicked();

    /** ***********************************************************************************************
      * \fn void on_bwt_yes_toggled(bool checked)
      *  \brief Función responsable de habilitar la zona de selección de fichero fastA para generar
      *         el índice BWT
      *  \param checked	opción marcada
      * ***********************************************************************************************
      */
    void on_bwt_yes_toggled(bool checked);

    /** ***********************************************************************************************
      * \fn void on_bwt_no_toggled(bool checked)
      *  \brief Función responsable de deshabilitar la zona de selección de ficheros fastA
      *  \param checked opción marcada
      * ***********************************************************************************************
      */
    void on_bwt_no_toggled(bool checked);

    /** ***********************************************************************************************
      * \fn void on_dnaFa_file_clicked()
      *  \brief Función responsable de abrir explorador de ficheros para seleccionar el fichero
      *         dna.fa de referencia para crear el índice BWT
      * ***********************************************************************************************
      */
    void on_dnaFa_file_clicked();

    /** ***********************************************************************************************
      * \fn void on_dnaFa_out_path_clicked()
      *  \brief Función responsable de abrir explorador de directorios para seleccionar el directorio
      *         donde guardar el índice BWT a crear
      * ***********************************************************************************************
      */
    void on_dnaFa_out_path_clicked();

    /** ***********************************************************************************************
      * \fn void on_bwt_path_clicked()
      *  \brief Función responsable de abrir explorador de directorios para seleccionar el directorio
      *         donde se encuentra el índice BWT
      * ***********************************************************************************************
      */
    void on_bwt_path_clicked();

    /** ***********************************************************************************************
      * \fn void on_fastq_files_clicked()
      *  \brief Función responsable de abrir el explorador de ficheros para seleccionar el/los ficheros
      *         fastQ a alinear con HPG-Methyl
      * ***********************************************************************************************
      */
    void on_fastq_files_clicked();

    /** ***********************************************************************************************
      * \fn void on_fastq_list_cursorPositionChanged()
      *  \brief Función responsable de marcar en gris el fichero marcado con el ratón en la ventana
      *         que muestra los ficheros fastQ a alinear
      * ***********************************************************************************************
      */
    void on_fastq_list_cursorPositionChanged();

    /** ***********************************************************************************************
      * \fn void on_fastq_delete_clicked()
      *  \brief Función responsable de borrar un fichero marcado de la lista de ficheros fastQ mostrados
      * ***********************************************************************************************
      */
    void on_fastq_delete_clicked();

    /** ***********************************************************************************************
      * \fn void on_fastq_up_clicked()
      *  \brief Función responsable de subir una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_fastq_up_clicked();

    /** ***********************************************************************************************
      * \fn void on_fastq_down_clicked()
      *  \brief Función responsable de bajar una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_fastq_down_clicked();

    /** ***********************************************************************************************
      * \fn void on_fastq_out_path_clicked()
      *  \brief Función responsable de abrir el explorador de directorios para seleccionar el
      *         directorio donde guardar los resultados del proceso de alineamiento
      * ***********************************************************************************************
      */
    void on_fastq_out_path_clicked();

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_mode_toggled(bool checked)
      *  \brief Función responsable de habilitar la zona de selección de ficheros FastQ en caso de
      *         estar procesados como paired-end
      *  \param checked opción marcada
      * ***********************************************************************************************
      */
    void on_pair_fastq_mode_toggled(bool checked);

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_files_clicked()
      *  \brief Función responsable de abrir el explorador de ficheros para seleccionar el/los ficheros
      *         fastQ paired-end a alinear con HPG-Methyl
      * ***********************************************************************************************
      */
    void on_pair_fastq_files_clicked();

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_list_cursorPositionChanged()
      *  \brief Función responsable de marcar en gris el fichero marcado con el ratón en la ventana
      *         que muestra los ficheros fastQ paired-end a alinear
      * ***********************************************************************************************
      */
    void on_pair_fastq_list_cursorPositionChanged();

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_delete_clicked()
      *  \brief Función responsable de borrar un fichero marcado de la lista de ficheros mostrados
      * ***********************************************************************************************
      */
    void on_pair_fastq_delete_clicked();

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_up_clicked()
      *  \brief Función responsable de subir una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_pair_fastq_up_clicked();

    /** ***********************************************************************************************
      * \fn void on_pair_fastq_down_clicked()
      *  \brief Función responsable de bajar una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_pair_fastq_down_clicked();

    /** ***********************************************************************************************
      * \fn void on_ejecutable_clicked()()
      *  \brief Función responsable de seleccionar la ruta del ejecutable hpg-methyl
      * ***********************************************************************************************
      */
    void on_ejecutable_clicked();

    /** ***********************************************************************************************
      * \fn void on_start_clicked() and one more
      *  \brief Funciones responsables de iniciar o para el proceso de aliineamiento
      * ***********************************************************************************************
      */
    void on_start_clicked();
    void on_stop_clicked();

    /** ***********************************************************************************************
      * \fn void fastq_acabado()
      *  \brief Función responsable de gestionar el bucle de alineamiento de las diferentes muestras
      * ***********************************************************************************************
      */
    void fastq_acabado();

    /** ***********************************************************************************************
      * \fn void alineamiento_acabado()
      *  \brief Función responsable de informar de final de proceso de alineamiento
      * ***********************************************************************************************
      */
    void alineamiento_acabado();

private:
    Ui::Fastq2bam *ui;

    /** ***********************************************************************************************
      * \fn void widgets_enabling (QList<int>)
      *  \brief Función responsable de habilitar las zonas de interfaz seleccionadas
      *  \param enable	listado de estado de los elementos a habilitar
      * ***********************************************************************************************
      */
    void widgets_enabling (QList<int> enable);


    /** ***********************************************************************************************
      * \fn void bwt_enabling (bool)
      *  \brief Función responsable de habilitar la zona de selección de índice BWT
      *  \param enable	estado de los elementos a habilitar
      * ***********************************************************************************************
      */
    void bwt_enabling (bool);

    /** ***********************************************************************************************
      * \fn void paired_enabling (bool)
      *  \brief Función responsable de habilitar la zona de selección de ficheros paired_end
      *  \param enable  estado de los elementos a habilitar
      * ***********************************************************************************************
      */
    void paired_enabling (bool);

    /** ***********************************************************************************************
      * \fn void fastq_2_bam_bwt_index ()
      *  \brief Función responsable de procesar el alineamiento con generación previa del índice BWT
      * ***********************************************************************************************
      */
    void fastq_2_bam_bwt_index ();

    /** ***********************************************************************************************
      * \fn void fastq_2_bam_paired ()
      *  \brief Función responsable de procesar el alineamiento con ficheros paired-end
      * ***********************************************************************************************
      */
    void fastq_2_bam_paired ();

    /** ***********************************************************************************************
      * \fn void fastq_2_bam ()
      *  \brief Función responsable de procesar el alineamiento de ficheros fastQ single-end
      * ***********************************************************************************************
      */
    void fastq_2_bam ();

    /** ***********************************************************************************************
      *  \brief variables respondables de gestionar las ventanas de listas de ficheros
      *  \param QString             ruta de directorio seleccionado en explorador
      *  \param QStringList         lista de rutas de ficheros seleccionados en explorador
      *  \param bool                habilación de coloreado de líneas seleccionadas
      *  \param QTextBlockFormat    variable para gestionar el color de las líneas seleccionadas
      *  \param QTextCursor         control de posiciónb del cursor en las listas de ficheros
      * ***********************************************************************************************
      */
    QString          file;
    QStringList      files;
    bool             fastq_list;
    bool             pair_fastq_list;
    QTextBlockFormat color;
    QTextCursor      *cursor_fastq;
    QTextCursor      *cursor_pair_fastq;
    QString          path;
    bool             directorio;

    /** ***********************************************************************************************
      *  \brief variables respondables de gestionar los trabajos
      *  \param QThread             hilo de ejecución de alineamiento
      *  \param HPGMethyl_worker    proceso de alineamiento a enviar al hilo
      *  \param contador            gestión de la barra de progreso
      * ***********************************************************************************************
      */
    QThread          *hilo_hpgMethyl;
    HPGMethyl_worker *methyl_worker;
    int              contador;
};

#endif // FASTQ2BAM_H
