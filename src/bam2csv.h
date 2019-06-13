#ifndef BAM2CSV_H
#define BAM2CSV_H

#include "hpghmapper_worker.h"
#include <QDialog>
#include <QTextCursor>
#include <QThread>

namespace Ui {
class Bam2csv;
}

class Bam2csv : public QDialog
{
    Q_OBJECT

public:
    explicit Bam2csv(QWidget *parent = nullptr);
    ~Bam2csv();

    void set_bam_nproc(int);
    void set_memory_available(int);

private:
    Ui::Bam2csv *ui;

    /** ***********************************************************************************************
      * \fn void bam_2_csv()
      *  \brief Función responsable de mapear los ficheros bam y guardar resultado en ficheros csv
      * ***********************************************************************************************
      */
    void bam_2_csv();

    /** ***********************************************************************************************
      * \fn void widgets_enabling (QList<int>)
      *  \brief Función responsable de habilitar las zonas de interfaz seleccionadas
      *  \param enable	listado de estado de los elementos a habilitar
      * ***********************************************************************************************
      */
    void widgets_enabling (QList<int> enable);

    /** ***********************************************************************************************
      *  \brief variables respondables de gestionar las ventanas de listas de ficheros
      *  \param QString             ruta de directorio seleccionado en explorador
      *  \param QStringList         lista de rutas de ficheros seleccionados en explorador
      *  \param bool                habilación de coloreado de líneas seleccionadas
      *  \param QTextBlockFormat    variable para gestionar el color de las líneas seleccionadas
      *  \param QTextCursor         control de posiciónb del cursor en las listas de ficheros
      * ***********************************************************************************************
      */
    QString           file;
    QStringList       files;
    bool              mc_list;
    bool              hmc_list;
    QTextBlockFormat  color;
    QTextCursor       *cursor_mc;
    QTextCursor       *cursor_hmc;
    QString           path;
    bool              directorio;
    int               memory_available;

    QThread           *hilo_hpgHmapper;
    HPGHmapper_worker *mapper_worker;
    int               contador;

public slots:
    /** ***********************************************************************************************
      * \fn void on_exit_clicked()
      *  \brief Función responsable de ceerrar la ventana y devolver el control al menú prinicipal
      * ***********************************************************************************************
      */
    void on_exit_clicked();

    /** ***********************************************************************************************
      * \fn void on_mc_files_clicked()
      *  \brief Función responsable de abrir el explorador de ficheros para seleccionar el/los ficheros
      *         BAM metilados a mapear con HPG-HMapper
      * ***********************************************************************************************
      */
    void on_mc_files_clicked();

    /** ***********************************************************************************************
      * \fn void on_mc_list_cursorPositionChanged()
      *  \brief Función responsable de marcar en gris el fichero marcado con el ratón en la ventana
      *         que muestra los ficheros BAM metilados a alinear
      * ***********************************************************************************************
      */
    void on_mc_list_cursorPositionChanged();

    /** ***********************************************************************************************
      * \fn void on_mc_delete_clicked()
      *  \brief Función responsable de borrar un fichero marcado de la lista de ficheros mostrados
      * ***********************************************************************************************
      */
    void on_mc_delete_clicked();

    /** ***********************************************************************************************
      * \fn void on_mc_up_clicked()
      *  \brief Función responsable de subir una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_mc_up_clicked();

    /** ***********************************************************************************************
      * \fn void on_mc_down_clicked()
      *  \brief Función responsable de bajar una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_mc_down_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_files_clicked()
      *  \brief Función responsable de abrir el explorador de ficheros para seleccionar el/los ficheros
      *         BAM hidroximetilados a mapear con HPG-HMapper
      * ***********************************************************************************************
      */
    void on_hmc_files_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_list_cursorPositionChanged()
      *  \brief Función responsable de marcar en gris el fichero marcado con el ratón en la ventana
      *         que muestra los ficheros BAM hidroximetilados a alinear
      * ***********************************************************************************************
      */
    void on_hmc_list_cursorPositionChanged();

    /** ***********************************************************************************************
      * \fn void on_hmc_delete_clicked()
      *  \brief Función responsable de borrar un fichero marcado de la lista de ficheros mostrados
      * ***********************************************************************************************
      */
    void on_hmc_delete_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_up_clicked()
      *  \brief Función responsable de subir una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_hmc_up_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_down_clicked()
      *  \brief Función responsable de bajar una posición el fichero marcado en la lista
      * ***********************************************************************************************
      */
    void on_hmc_down_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_ram_slider_valueChanged(int value)
      *  \brief Función responsable de capturar movimiento de slider y actualizar etiqueta adjunta
      *  \param value   valor en la posición actual del slider
      * ***********************************************************************************************
      */
    void on_ram_slider_valueChanged(int value);

    /** ***********************************************************************************************
      * \fn void on_hmc_out_path_clicked()
      *  \brief Función responsable de abrir el explorador de directorios para seleccionar el
      *         directorio donde guardar los resultados del proceso de mapeado
      * ***********************************************************************************************
      */
    void on_hmc_out_path_clicked();

    /** ***********************************************************************************************
      * \fn void on_start_clicked()
      *  \brief Función responsable de poner en marcha el proceso seleccionado
      * ***********************************************************************************************
      */
    void on_start_clicked();

    /** ***********************************************************************************************
      * \fn void on_ejecutable_clicked()()
      *  \brief Función responsable de seleccionar la ruta del ejecutable hpg-hmapper
      * ***********************************************************************************************
      */
    void on_ejecutable_clicked();

    /** ***********************************************************************************************
      * \fn void bam_acabado()
      *  \brief Función responsable de informar de final de proceso de mapeado
      * ***********************************************************************************************
      */
    void bam_acabado();

    /** ***********************************************************************************************
      * \fn void bam_mapeado()
      *  \brief Función responsable de gestionar el bucle de mapeado de las diferentes muestras
      * ***********************************************************************************************
      */
    void bam_mapeado();

private slots:
    /** ***********************************************************************************************
      * \fn void on_stop_clicked()
      *  \brief Función responsable de paralizar el proceso
      * ***********************************************************************************************
      */
    void on_stop_clicked();

    /** ***********************************************************************************************
      * \fn void on_hmc_check_stateChanged(int arg1)
      *  \brief Función responsable de activar la ventana de mapeado con ficheros hmC
      * ***********************************************************************************************
      */
    void on_hmc_check_stateChanged(int arg1);

    /** ***********************************************************************************************
      * \fn void on_bwt_index_clicked()
      *  \brief Función responsable de incluir el cromosoma de referencia en el mepeado
      * ***********************************************************************************************
      */
    void on_bwt_index_clicked();
};

#endif // BAM2CSV_H
