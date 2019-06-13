#include "fastq2bam.h"
#include "ui_fastq2bam.h"
#include <QProcess>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>

Fastq2bam::Fastq2bam(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Fastq2bam)
{
    ui->setupUi(this);

    // inicialización de variables
    //---------------------------------------------------------------------------------------------
    directorio        = false;
    // control de coloreado de filas en listas
    fastq_list        = false;
    pair_fastq_list   = false;
    // cursores de cada una de las listas
    cursor_fastq      = new QTextCursor();
    cursor_pair_fastq = new QTextCursor();
    contador          = 0;
    ui->progressBar->setMinimum(0);

    // obtención de lista de ejecutables hpg-methyl que se han utilizado en otras ocasiones
    //---------------------------------------------------------------------------------------------
    QFile methyl("./find_methyl.txt");

    // comprueba que el fichero se ha abierto correctamente
    if (!methyl.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             "ERROR Opening file",
                             "An error occurred opening the file: find_methyl.txt"
                             "\nPlease, check the file for corrupted"
                            );
        qDebug() << "ERROR opening file find_methyl.txt";
        return;
    }
    else
    {
        QString line;
        while (!methyl.atEnd())
        {
            line = methyl.readLine();
            ui->ejecutables->addItem(line.split('\n')[0]);
        }
    }
    methyl.close();

    // habilita la interfaz si tiene ejecutables disponibles
    if (ui->ejecutables->count())
    {
        QList<int> enable_iface = {1,1,1,
                                   0,0,0,0,0,0,0,
                                   1,1,1,1,1,1,1,1,1,
                                   1,0,0,0,0,0,0,0,0,0,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   0,0,0,1};
        widgets_enabling(enable_iface);
    }
    else
    {
        QList<int> enable_iface = {0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,1};
        widgets_enabling(enable_iface);
    }

    // hilo y conexiones para el proceso de alineamiento
    hilo_hpgMethyl = new QThread();
    methyl_worker  = new HPGMethyl_worker();
    methyl_worker->moveToThread(hilo_hpgMethyl);
    connect(methyl_worker, SIGNAL(fastq_alineado()), SLOT(fastq_acabado()));
    hilo_hpgMethyl->connect(methyl_worker,SIGNAL(alineamiento_solicitado()), SLOT(start()));
    methyl_worker->connect(hilo_hpgMethyl, SIGNAL(started()), SLOT(lectura()));
    hilo_hpgMethyl->connect(methyl_worker, SIGNAL(finished()), SLOT(quit()), Qt::DirectConnection);
    connect(hilo_hpgMethyl, &QThread::finished, methyl_worker, &QObject::deleteLater);
    connect(methyl_worker, SIGNAL(finished()), SLOT(alineamiento_acabado()));
}

Fastq2bam::~Fastq2bam()
{
    delete ui;
}

void Fastq2bam::on_exit_clicked()
{
    this->hide();
}


//*************************************************************************************************
//**********************************ZONA BWT INDEX*************************************************
//*************************************************************************************************
void Fastq2bam::bwt_enabling(bool enable)
{
    // habilita la zona de interfaz para el fichero índice en HPG-Methyl
    ui->dnaFa_file->setEnabled(enable);
    ui->dnaFa_file_label->setEnabled(enable);
    ui->dnaFa_out_path->setEnabled(enable);
    ui->dnaFa_out_path_label->setEnabled(enable);
    ui->label_index_ratio_2->setEnabled(enable);
    ui->slider_index_ratio->setEnabled(enable);
    ui->label_index_ratio->setEnabled(enable);
}

//*************************************************************************************************
void Fastq2bam::paired_enabling(bool enable)
{
    // habilita la zona de interfaz para el alineamieno de paired-reads en HPG-Methyl
    ui->label_min_distance->setEnabled(enable);
    ui->pair_fastq_min_distance->setEnabled(enable);
    ui->label_max_distance->setEnabled(enable);
    ui->pair_fastq_max_distance->setEnabled(enable);
    ui->pair_fastq_files->setEnabled(enable);
    ui->pair_fastq_delete->setEnabled(enable);
    ui->pair_fastq_up->setEnabled(enable);
    ui->pair_fastq_down->setEnabled(enable);
    ui->pair_fastq_list->setEnabled(enable);
}

//*************************************************************************************************
void Fastq2bam::on_bwt_yes_toggled(bool checked)
{
    // habilita la zona de generación de fichero índice BWT
    if (checked)
        bwt_enabling(true);
}

//*************************************************************************************************
void Fastq2bam::on_bwt_no_toggled(bool checked)
{
    // deshabilita la zona de generación de fichero índice BWT y limpia las selecciones
    if (checked)
    {
        ui->dnaFa_file_label->clear();
        ui->dnaFa_out_path_label->clear();
        bwt_enabling(false);

        if (ui->fastq_list->toPlainText().isEmpty())
            ui->start->setEnabled(false);
    }
}

//*************************************************************************************************
void Fastq2bam::on_dnaFa_file_clicked()
{
    // abre la ventana de explorador de ficheros para seleccionar el archivo FASTA
    // con el que generar el índice BWT
    file = QFileDialog::getOpenFileName( this,
                                         tr("Select a FASTA file for BWT index creation"),
                                         (directorio) ? path : QDir::homePath() ,
                                         "FASTA files (*.fa *.fasta) ;; All files (*.*)"
                                       );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        directorio = true;
        path       = file.split(".")[0];

        if (!ui->dnaFa_out_path_label->text().isEmpty())
            ui->start->setEnabled(true);
    }

    // escribe  en pantalla el nombre del fichero seleccionado
    ui->dnaFa_file_label->setText(file);
}

//*************************************************************************************************
void Fastq2bam::on_dnaFa_out_path_clicked()
{
    // abre ventana de explorador de directorios para seleccionar
    // el directorio donde guardar el índice BWT
    file = QFileDialog::getExistingDirectory( this,
                                              tr("Select a folder to save the BWT index"),
                                              (directorio) ? path : QDir::homePath() ,
                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                                            );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        directorio = true;
        path       = file;

        if (!ui->dnaFa_file_label->text().isEmpty())
            ui->start->setEnabled(true);
    }

    // escribe  en pantalla el nombre del directorio seleccionado
    ui->dnaFa_out_path_label->setText(file);
    ui->bwt_path_label->setText(file + "/");
}

//*************************************************************************************************
//**********************************ZONA FASTQ FILES SELECTION*************************************
//*************************************************************************************************
void Fastq2bam::on_bwt_path_clicked()
{
    // abre ventana de explorador de directorios para seleccionar
    // el directorio donde se halla el índice BWT
    file = QFileDialog::getExistingDirectory( this,
                                              tr("Select the folder where the BWT index is"),
                                              (directorio) ? path : QDir::homePath() ,
                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                                            );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        directorio = true;
        path       = file;
    }

    // escribe  en pantalla el nombre del directorio seleccionado
    ui->bwt_path_label->setText(file + "/");
}

//*************************************************************************************************
void Fastq2bam::on_fastq_files_clicked()
{
    // pone fondo blanco en línea donde se halla el cursor
    color.setBackground(Qt::white);
    cursor_fastq->select((QTextCursor::LineUnderCursor));
    cursor_fastq->setBlockFormat(color);

    // abre ventana de explorador de ficheros para seleccionar los archivos FASTQ
    files.clear();
    files = QFileDialog::getOpenFileNames( this,
                                           tr("Select FASTQ files to align with HPG_Methyl"),
                                           (directorio) ? path : QDir::homePath() ,
                                           "FASTQ files (*.fastq) ;; All files (*.*)"
                                         );
    // escribe en pantalla el nombre de los ficheros seleccionados
    if (!files.isEmpty())
    {
        fastq_list = true;
        ui->fastq_list->append(files.join('\n'));
        ui->start->setEnabled(true);
        ui->progressBar->setEnabled(true);

        directorio = true;
        path       = file.split(".")[0];
    }
}

//*************************************************************************************************
void Fastq2bam::on_fastq_list_cursorPositionChanged()
{
    if (fastq_list)
    {
        //QTextBlockFormat color;

        // desmarca la línea previa quitando color de fondo
        color.setBackground(Qt::white);
        cursor_fastq->select((QTextCursor::LineUnderCursor));
        cursor_fastq->setBlockFormat(color);

        // adquiere el cursor de la línea seleccionada con el ratón
        *cursor_fastq = ui->fastq_list->textCursor();
        cursor_fastq->movePosition(QTextCursor::StartOfBlock);
        cursor_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        // marca con color de fondo la línea seleccionada
        color.setBackground(Qt::gray);
        cursor_fastq->select((QTextCursor::LineUnderCursor));
        cursor_fastq->setBlockFormat(color);
    }
}

//*************************************************************************************************
void Fastq2bam::on_fastq_delete_clicked()
{
    // variables internas
    int line_number  = cursor_fastq->blockNumber();                 // número de línea actual del cursor
    QStringList list = ui->fastq_list->toPlainText().split("\n");   // lista de strings con ficheros

    // deshabilita el cambio de color por acción sobre el cursor
    fastq_list = false;
    // borra el fichero seleccionado
    list.removeAt(line_number);
    // limpia la lista de visualización
    ui->fastq_list->clear();
    // si quedan ficheros en la lista, la copia en la lista de visualización
    if (!list.isEmpty())
    {
        fastq_list = true;
        ui->fastq_list->append(list.join('\n'));
    }
    else
    {
        // si no hay ficheros deshabilita el botón de ejecutar alineamiento
        ui->start->setEnabled(false);
        ui->stop->setEnabled(false);
        ui->progressBar->setEnabled(false);
    }

    // aplica fondo blanco a línea bajo el cursor
    color.setBackground(Qt::white);
    cursor_fastq->select((QTextCursor::LineUnderCursor));
    cursor_fastq->setBlockFormat(color);

    // mueve el cursor al principo de la lista de visualización
    cursor_fastq->movePosition(QTextCursor::Start);
    // traslada el cursor a la posición que tenía antes de borrar
    for (int i = 0; i < line_number; i++)
    {
        cursor_fastq->movePosition(QTextCursor::StartOfBlock);
        cursor_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

        if (i == list.size() - 2)
            break;
    }

    // actualiza la lista con la posición del cursor adecuada para que se resalte
    ui->fastq_list->setTextCursor(*cursor_fastq);
}

//*************************************************************************************************
void Fastq2bam::on_fastq_up_clicked()
{
    if (cursor_fastq->blockNumber() > 0)
    {
        // variables internas
        int line_number  = cursor_fastq->blockNumber();                 // número de línea del cursor
        QStringList list = ui->fastq_list->toPlainText().split("\n");   // lista de ficheros
        QString line     = list.at(line_number);                        // fichero en línea seleccionada

        // deshabilita el cambio de color por acción sobre el cursor
        fastq_list = false;
        // borra el fichero seleccionado
        list.removeAt(line_number);
        // inserta el fichero selecionado en una posición más arriba
        list.insert(line_number - 1, line);
        // limpia la lista de visualización
        ui->fastq_list->clear();
        // si quedan ficheros en la lista, la copia en la lista de visualización
        if (!list.isEmpty())
        {
            fastq_list = true;
            ui->fastq_list->append(list.join('\n'));
        }

        // aplica fondo blanco a línea bajo el cursor
        color.setBackground(Qt::white);
        cursor_fastq->select((QTextCursor::LineUnderCursor));
        cursor_fastq->setBlockFormat(color);

        // mueve el cursor al principio de la lista de visualización
        cursor_fastq->movePosition(QTextCursor::Start);
        // traslada el cursor a la posición que ocupa el fichero trasladado
        for (int i = 0; i < line_number - 1; i++)
        {
            cursor_fastq->movePosition(QTextCursor::StartOfBlock);
            cursor_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }

        // actualiza la lista con la posición del cursor adecuada para que se resalte
        ui->fastq_list->setTextCursor(*cursor_fastq);
    }
}

//*************************************************************************************************
void Fastq2bam::on_fastq_down_clicked()
{
    // variables internas
    QStringList list = ui->fastq_list->toPlainText().split("\n");   // lista de ficheros

    // proceder con el desplazamiento abajo si no es el último elemento de la lista
    if (cursor_fastq->blockNumber() < list.size())
    {
        // variables internas
        int line_number = cursor_fastq->blockNumber();  // número de línea del cursor
        QString line    = list.at(line_number);         // fichero en línea seleccionada

        // deshabilita el cambio de color por acción sobre el cursor
        fastq_list = false;
        // borra el fichero seleccionado
        list.removeAt(line_number);
        // inserta el fichero seleccionado en una posición más abajo
        list.insert(line_number + 1, line);
        // limpia la lista de visualización
        ui->fastq_list->clear();
        // se quedan ficheros en la lista, la copia en la lista de visualización
        if (!list.isEmpty())
        {
            fastq_list = true;
            ui->fastq_list->append(list.join('\n'));
        }

        // aplica fondo blanco a línea bajo el cursor
        color.setBackground(Qt::white);
        cursor_fastq->select((QTextCursor::LineUnderCursor));
        cursor_fastq->setBlockFormat(color);

        // mueve el cursor al principio de la lista de visualización
        cursor_fastq->movePosition(QTextCursor::Start);
        // traslada el cursor a la posición que ocupa el fichero trasladado
        for (int i = 0; i <= line_number; i++)
        {
            cursor_fastq->movePosition(QTextCursor::StartOfBlock);
            cursor_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

            // comprueba que no ha alcanzado el final de la lista
            if (i == list.size() - 2)
                break;
        }

        // actualiza la lista con la posición del cursor adecuada para que se resalte
        ui->fastq_list->setTextCursor(*cursor_fastq);
    }
}

//*************************************************************************************************
void Fastq2bam::on_fastq_out_path_clicked()
{
    // abre ventana de explorador de directorios para seleccionar
    // el directorio donde se guardarán los resultados del alineamiento
    file = QFileDialog::getExistingDirectory( this,
                                              tr("Select the folder where the alignment will be saved"),
                                              (directorio) ? path : QDir::homePath() ,
                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                                            );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        directorio = true;
        path       = file;
    }

    // escribe  en pantalla el nombre del directorio seleccionado
    ui->fastq_out_path_label->setText(file + "/");
}

//*************************************************************************************************
//**********************************ZONA PAIRED FILES SELECTION************************************
//*************************************************************************************************
void Fastq2bam::on_pair_fastq_mode_toggled(bool checked)
{
    // habilita la zona de gestión de ficheros para modo paired-end
    if (checked)
        paired_enabling(true);
    else
    {
        // deshabilita la zona y limpia las selecciones realizadas
        ui->pair_fastq_min_distance->setValue(20);
        ui->pair_fastq_max_distance->setValue(30);
        ui->pair_fastq_list->clear();
        paired_enabling(false);
    }
}

//*************************************************************************************************
void Fastq2bam::on_pair_fastq_list_cursorPositionChanged()
{
    if (pair_fastq_list)
    {
        // desmarca la línea previa quitando color de fondo
        color.setBackground(Qt::white);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);

        // adquiere el cursor de la línea seleccionada
        *cursor_pair_fastq = ui->pair_fastq_list->textCursor();
        cursor_pair_fastq->movePosition(QTextCursor::StartOfBlock);
        cursor_pair_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        // marca con color de fondo la línea seleccionada
        color.setBackground(Qt::gray);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);
    }
}

//*************************************************************************************************
void Fastq2bam::on_pair_fastq_files_clicked()
{
    color.setBackground(Qt::white);
    cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
    cursor_pair_fastq->setBlockFormat(color);

    // abre ventana de explorador de ficheros para seleccionar el archivo FASTA
    // con el que generar el índice BWT
    files.clear();
    files = QFileDialog::getOpenFileNames( this,
                                           tr("Select FASTQ paired files to align with HPG_Methyl"),
                                           (directorio) ? path : QDir::homePath() ,
                                           "FASTQ files (*.fastq) ;; All files (*.*)"
                                         );
    // escribe en pantalla el nombre de los ficheros seleccionados
    if (!files.isEmpty())
    {
        pair_fastq_list = true;
        ui->pair_fastq_list->append(files.join('\n'));

        directorio = true;
        path       = file.split(".")[0];
    }
}

//*************************************************************************************************
void Fastq2bam::on_pair_fastq_delete_clicked()
{
    int line_number = cursor_pair_fastq->blockNumber();
    QStringList list = ui->pair_fastq_list->toPlainText().split("\n");

    pair_fastq_list = false;
    list.removeAt(line_number);
    ui->pair_fastq_list->clear();
    if (!list.isEmpty())
    {
        pair_fastq_list = true;
        ui->pair_fastq_list->append(list.join('\n'));
    }

    color.setBackground(Qt::white);
    cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
    cursor_pair_fastq->setBlockFormat(color);

    cursor_pair_fastq->movePosition(QTextCursor::Start);
    for (int i = 0; i < line_number; i++)
    {
        cursor_pair_fastq->movePosition(QTextCursor::StartOfBlock);
        cursor_pair_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor_pair_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

        if (i == list.size() - 2)
            break;
    }

    ui->pair_fastq_list->setTextCursor(*cursor_pair_fastq);
}

//*************************************************************************************************
void Fastq2bam::on_pair_fastq_up_clicked()
{
    if (cursor_pair_fastq->blockNumber() > 0)
    {
        int line_number = cursor_pair_fastq->blockNumber();
        QStringList list = ui->pair_fastq_list->toPlainText().split("\n");
        QString line = list.at(line_number);

        pair_fastq_list = false;
        list.removeAt(line_number);
        list.insert(line_number - 1, line);
        ui->pair_fastq_list->clear();
        if (!list.isEmpty())
        {
            pair_fastq_list = true;
            ui->pair_fastq_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);

        cursor_pair_fastq->movePosition(QTextCursor::Start);
        for (int i = 0; i < line_number - 1; i++)
        {
            cursor_pair_fastq->movePosition(QTextCursor::StartOfBlock);
            cursor_pair_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_pair_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }

        ui->pair_fastq_list->setTextCursor(*cursor_pair_fastq);
    }
}

//*************************************************************************************************
void Fastq2bam::on_pair_fastq_down_clicked()
{
    QStringList list = ui->pair_fastq_list->toPlainText().split("\n");

    if (cursor_pair_fastq->blockNumber() < list.size())
    {
        int line_number = cursor_pair_fastq->blockNumber();
        QString line = list.at(line_number);

        pair_fastq_list = false;
        list.removeAt(line_number);
        list.insert(line_number + 1, line);
        ui->pair_fastq_list->clear();
        if (!list.isEmpty())
        {
            pair_fastq_list = true;
            ui->pair_fastq_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);

        cursor_pair_fastq->movePosition(QTextCursor::Start);
        for (int i = 0; i <= line_number; i++)
        {
            cursor_pair_fastq->movePosition(QTextCursor::StartOfBlock);
            cursor_pair_fastq->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_pair_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

            if (i == list.size() - 2)
                break;
        }

        ui->pair_fastq_list->setTextCursor(*cursor_pair_fastq);
    }
}

//*************************************************************************************************
//**********************************ZONA EJECUCION TRABAJOS****************************************
//*************************************************************************************************
void Fastq2bam::fastq_2_bam_paired()
{
    // variables internas
    // --------------------------------------------------------------------------------------------
    QEventLoop loop;            // delay por bucle
    QProcess   process;         // proceso de control de alineamiento
    QString    result;          // captura de errores en el proceso de alineamiento
    QString    path, path_2;    // ruta y fichero a alinear (ida y vuelta)

    // ventanas de precaución ante falta de datos previos al proceso
    // --------------------------------------------------------------------------------------------
    // falta la ruta al índice BWT
    if (ui->bwt_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to BWT index is empty\n"
                                "Please, select the BWT index folder")
                            );
        return;
    }
    // falta la ruta donde guardar el resultado de alineamiento
    if (ui->fastq_out_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to save the BAM alignments is empty\n"
                                "Please, select a folder to save the BAM files")
                            );
        return;
    }

    // listas de ficheros paired-end a alinear
    QStringList list_1 = ui->fastq_list->toPlainText().split("\n");
    QStringList list_2 = ui->pair_fastq_list->toPlainText().split("\n");

    // si las listas no coinciden en longitud emite aviso de error
    if (list_1.size() != list_2.size())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The FastQ list and PairedFastQ list must have the same length\n"
                                "Please, select the correct files")
                            );
        return;
    }

    // inicio del proceso de alineado
    // --------------------------------------------------------------------------------------------
    // se solicita confirmación sobre el orden correcto de las dos listas para ejecutar el alineado
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "FastQ to BAM app",
                                  "Remember, the orden of the lists is very important.\n"
                                  "Has each FastQ file the same number of orden as its paired?\n"
                                  "It means, are the FastQ lists paired?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // deshabilitado de zona de alineado para no interferir en el proceso de aineamiento
        ui->bwt_path->setEnabled(false);
        ui->fastq_files->setEnabled(false);
        ui->fastq_delete->setEnabled(false);
        ui->fastq_up->setEnabled(false);
        ui->fastq_down->setEnabled(false);
        ui->fastq_out_path->setEnabled(false);
        ui->pair_fastq_mode->setEnabled(false);
        ui->pair_fastq_files->setEnabled(false);
        ui->pair_fastq_delete->setEnabled(false);
        ui->pair_fastq_up->setEnabled(false);
        ui->pair_fastq_down->setEnabled(false);

        // los cursores de las listas se ponen al principio de las listas
        cursor_fastq->movePosition(QTextCursor::Start);
        cursor_pair_fastq->movePosition(QTextCursor::Start);
        // coloreado en rojo el fondo de la línea que se está ejecutando
        color.setBackground(Qt::red);
        cursor_fastq->select((QTextCursor::LineUnderCursor));
        cursor_fastq->setBlockFormat(color);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);

        // se informa del inicio del proceso en la barra de estado inferior de la ventana
        ui->statusBar->setText("aligning...");

        // pequeña espera previa a la ejecución para que se coloree la línea
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        // prepara parametros para eviar al worker
        QStringList parametros;
        parametros << ui->bwt_path_label->text() <<
                      QString::number(ui->fastq_nproc->value()) <<
                      QString::number(ui->fastq_cal_size->value()) <<
                      QString::number(ui->fastq_cal_factor->value()) <<
                      QString::number(ui->fastq_best_align->value()) <<
                      QString::number(ui->fastq_best_hits->value()) <<
                      QString::number(ui->fastq_discard->value()) <<
                      QString::number(ui->pair_fastq_min_distance->value()) <<
                      QString::number(ui->pair_fastq_max_distance->value());

        // barra de progreso a cero;
        ui->progressBar->setMaximum(list_1.size());
        ui->progressBar->setValue(0);

        // lanza el hilo de alineamiento
        methyl_worker->solicitud_alineamiento(ui->ejecutables->currentText(),
                                              parametros,
                                              ui->fastq_out_path_label->text(),
                                              list_1,
                                              list_2);
    }
    else
        return;
}

//*************************************************************************************************
void Fastq2bam::fastq_2_bam()
{
    // variables internas
    // --------------------------------------------------------------------------------------------
    QEventLoop loop;            // delay por bucle
    QProcess   process;         // proceso de control de alineamiento

    // ventanas de precaución ante falta de datos previos al proceso
    // --------------------------------------------------------------------------------------------
    // falta la ruta al índice BWT
    if (ui->bwt_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to BWT index is empty\n"
                                "Please, select the BWT index folder")
                            );
        return;
    }
    // falta la ruta donde guardar el resultado de alineamiento
    if (ui->fastq_out_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to save the BAM alignments is empty\n"
                                "Please, select a folder to save the BAM files")
                            );
        return;
    }

    // deshabilitado de zona de alineado para no interferir en el proceso de aineamiento
    ui->bwt_path->setEnabled(false);
    ui->fastq_files->setEnabled(false);
    ui->fastq_delete->setEnabled(false);
    ui->fastq_up->setEnabled(false);
    ui->fastq_down->setEnabled(false);
    ui->fastq_out_path->setEnabled(false);
    ui->pair_fastq_mode->setEnabled(false);

    // inicio del proceso de alineado
    // --------------------------------------------------------------------------------------------
    // lista de ficheros a alienar
    QStringList list = ui->fastq_list->toPlainText().split("\n");

    ui->statusBar->setText("aligning...");

    // mueve cursor al inicio de la lista
    cursor_fastq->movePosition(QTextCursor::Start);
    // coloreado en rojo el fondo de la línea que se está ejecutando
    color.setBackground(Qt::red);
    cursor_fastq->select((QTextCursor::LineUnderCursor));
    cursor_fastq->setBlockFormat(color);

    // pequeña espera previa a la ejecución para que se coloree la línea
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    // prepara parametros para eviar al worker
    QStringList parametros;
    parametros << ui->bwt_path_label->text() <<
                  QString::number(ui->fastq_nproc->value()) <<
                  QString::number(ui->fastq_cal_size->value()) <<
                  QString::number(ui->fastq_cal_factor->value()) <<
                  QString::number(ui->fastq_best_align->value()) <<
                  QString::number(ui->fastq_best_hits->value()) <<
                  QString::number(ui->fastq_discard->value());

    // barra de progreso a cero;
    contador = 0;
    ui->progressBar->setMaximum(list.size());
    ui->progressBar->setValue(0);

    // lanza el hilo de alineamiento
    methyl_worker->solicitud_alineamiento(ui->ejecutables->currentText(),
                                          parametros,
                                          ui->fastq_out_path_label->text(),
                                          list);
}


//*************************************************************************************************
void Fastq2bam::fastq_2_bam_bwt_index()
{
    // variables internas
    QProcess process;               // proceso para control de ejecución de alineamiento
    QString  result;                // captura de errores en el proceso
    bool     fastq_process = false; // proceso de alineado posterior a generación de índice BWT

    // ventanas de precaución ante falta de datos previos al proceso
    // --------------------------------------------------------------------------------------------
    // falta el archivo fastA para generación del índice
    if (ui->dnaFa_file_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to dna.fa file is empty\n"
                                "Please, select a dna.fa file to create a BWT index")
                            );
        return;
    }
    // falta el directorio donde guardar el fichero índice BWT
    if (ui->dnaFa_out_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("FastQ to BAM app"),
                             tr("The path to BWT index is empty\n"
                                "Please, select a BWT index folder where save it")
                            );
        return;
    }

    // comprobación automática de existencia de ficheros a alinear
    // --------------------------------------------------------------------------------------------
    // lista de fichero fastQ en ventana de visualización
    QStringList list = ui->fastq_list->toPlainText().split("\n");

    // si no está vacía se solicita confirmación de alineamiento posterior a generación de índice BWT
    if (list[0].size())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "FastQ to BAM app",
                                      "There are a list of FastQ files.\n"
                                      "Do you want to aling them after BWT index creation?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            // si se procede al alineamiento se comprueba que no faltan datos por rellenar
            if (ui->fastq_out_path_label->text().isEmpty())
            {
                QMessageBox::warning(this,
                                     tr("FastQ to BAM app"),
                                     tr("The path to save the BAM alignments is empty\n"
                                        "Please, select a folder to save the BAM files")
                                    );
                return;
            }
            fastq_process = true;
        }
    }

    // proceso de alineamiento
    // --------------------------------------------------------------------------------------------
    // deshabilitar zona de alineamiento para no interferir con la ejecución del proceso
    ui->bwt_yes->setEnabled(false);
    ui->bwt_no->setEnabled(false);
    ui->dnaFa_file->setEnabled(false);
    ui->dnaFa_out_path->setEnabled(false);
    ui->slider_index_ratio->setEnabled(false);
    ui->bwt_path->setEnabled(false);
    ui->fastq_files->setEnabled(false);
    ui->fastq_delete->setEnabled(false);
    ui->fastq_up->setEnabled(false);
    ui->fastq_down->setEnabled(false);
    ui->fastq_out_path->setEnabled(false);
    ui->pair_fastq_mode->setEnabled(false);

    // ejecución síncrona del proceso de generación del índice BWT
    process.execute("/home/lifercor/programas/Bio-Informatica/hpg-methyl2b/hpg-methyl-master/bin/hpg-methyl",
                    QStringList() << "build-index" <<
                                     "-g" << ui->dnaFa_file_label->text() <<
                                     "-i" << ui->dnaFa_out_path_label->text() <<
                                     "-r" << QString::number(ui->slider_index_ratio->value()) <<
                                     "--bs-index"
                   );
    process.waitForFinished();                  // espera a la finalización del proceso
    result = process.readAllStandardError();    // captura de errores del proceso
    process.close();                            // cierre del proceso
    // mostrar posibles errores en ventana de atención
    if (!result.isEmpty())
        QMessageBox::warning(this,
                             tr("Creating BWT index ERROR"),
                             result
                            );

    // alineamiento posterior
    // --------------------------------------------------------------------------------------------
    if (fastq_process)
    {
        // seleccionar si es fastq o paired-fastQ comprobando si la lista de paired tiene elementos
        if (QStringList(ui->pair_fastq_list->toPlainText().split("\n")).size() > 0 && ui->pair_fastq_mode->isChecked())
            fastq_2_bam_paired();
        else
            fastq_2_bam();
    }

    // habiltar zona de alineamiento para empezar otro proceso
    ui->bwt_yes->setEnabled(true);
    ui->bwt_no->setEnabled(true);
    ui->bwt_no->setChecked(true);
    ui->bwt_path->setEnabled(true);
    ui->fastq_files->setEnabled(true);
    ui->fastq_delete->setEnabled(true);
    ui->fastq_up->setEnabled(true);
    ui->fastq_down->setEnabled(true);
    ui->fastq_out_path->setEnabled(true);
    ui->pair_fastq_mode->setEnabled(true);
}

//*************************************************************************************************
void Fastq2bam::on_start_clicked()
{
    ui->start->setEnabled(false);
    ui->stop->setEnabled(true);
    ui->exit->setEnabled(false);

    // proceso de alineamiento
    // con generación de fichero índice BWT
    if (ui->bwt_yes->isChecked())
        fastq_2_bam_bwt_index();
    // con reads paired-end
    else if (ui->pair_fastq_mode->isChecked())
        fastq_2_bam_paired();
    // con reads single-end
    else
        fastq_2_bam();
}

//*************************************************************************************************
void Fastq2bam::widgets_enabling(QList<int> enable)
{
    ui->label_bwt->setEnabled(enable.at(0));
    ui->bwt_yes->setEnabled(enable.at(1));
    ui->bwt_no->setEnabled(enable.at(2));
    ui->dnaFa_file->setEnabled(enable.at(3));
    ui->dnaFa_file_label->setEnabled(enable.at(4));
    ui->dnaFa_out_path->setEnabled(enable.at(5));
    ui->dnaFa_out_path_label->setEnabled(enable.at(6));
    ui->label_index_ratio_2->setEnabled(enable.at(7));
    ui->slider_index_ratio->setEnabled(enable.at(8));
    ui->label_index_ratio->setEnabled(enable.at(9));
    ui->bwt_path->setEnabled(enable.at(10));
    ui->bwt_path_label->setEnabled(enable.at(11));
    ui->fastq_files->setEnabled(enable.at(12));
    ui->fastq_delete->setEnabled(enable.at(13));
    ui->fastq_up->setEnabled(enable.at(14));
    ui->fastq_down->setEnabled(enable.at(15));
    ui->fastq_list->setEnabled(enable.at(16));
    ui->fastq_out_path->setEnabled(enable.at(17));
    ui->fastq_out_path_label->setEnabled(enable.at(18));
    ui->pair_fastq_mode->setEnabled(enable.at(19));
    ui->pair_fastq_mode->setChecked(false);
    ui->label_min_distance->setEnabled(enable.at(20));
    ui->pair_fastq_min_distance->setEnabled(enable.at(21));
    ui->label_max_distance->setEnabled(enable.at(22));
    ui->pair_fastq_max_distance->setEnabled(enable.at(23));
    ui->pair_fastq_files->setEnabled(enable.at(24));
    ui->pair_fastq_delete->setEnabled(enable.at(25));
    ui->pair_fastq_up->setEnabled(enable.at(26));
    ui->pair_fastq_down->setEnabled(enable.at(27));
    ui->pair_fastq_list->setEnabled(enable.at(28));
    ui->label_fastq_nproc->setEnabled(enable.at(29));
    ui->fastq_nproc->setEnabled(enable.at(30));
    ui->label_min_cal->setEnabled(enable.at(31));
    ui->fastq_cal_size->setEnabled(enable.at(32));
    ui->label_cal_factor->setEnabled(enable.at(33));
    ui->fastq_cal_factor->setEnabled(enable.at(34));
    ui->label_report->setEnabled(enable.at(35));
    ui->fastq_best_align->setEnabled(enable.at(36));
    ui->label_best_alig->setEnabled(enable.at(37));
    ui->fastq_best_hits->setEnabled(enable.at(38));
    ui->label_best_hits->setEnabled(enable.at(39));
    ui->fastq_best_score->setEnabled(enable.at(40));
    ui->label_discard->setEnabled(enable.at(41));
    ui->fastq_discard->setEnabled(enable.at(42));
    ui->label_location->setEnabled(enable.at(43));
    ui->progressBar->setEnabled(enable.at(44));
    ui->start->setEnabled(enable.at(45));
    ui->stop->setEnabled(enable.at(46));
    ui->exit->setEnabled(enable.at(47));
    ui->dnaFa_file_label->clear();
    ui->dnaFa_out_path_label->clear();
    ui->bwt_path_label->clear();
    ui->fastq_list->clear();
    ui->fastq_out_path_label->clear();
    ui->pair_fastq_list->clear();
}

//*************************************************************************************************
void Fastq2bam::set_fastq_nproc(int value)
{
    ui->fastq_nproc->setMaximum(value);
    ui->fastq_nproc->setValue(int(value * 0.5));

    // muestra los valores de memoria y #procesadores en la barra de estado inferior de la ventana
    ui->statusBar->setText("System Number of processors: " + QString::number(value));
}

//*************************************************************************************************
void Fastq2bam::on_ejecutable_clicked()
{
    // selecciona la ruta al programa de alineamiento hpg-methyl
    file = QFileDialog::getOpenFileName( this,
                                         tr("Select the HPG-METHYL executable"),
                                         QDir::homePath(),
                                         "All files (*)"
                                       );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        if (file.endsWith("hpg-methyl"))
        {
            if(ui->ejecutables->count() == 0)
                ui->ejecutables->addItem(file);
            else
            {
                // comprueba que no existen duplicados
                bool existe = false;
                for (int i = 0; i < ui->ejecutables->count(); i++)
                {
                    ui->ejecutables->setCurrentIndex(i);
                    if (!QString::compare(file, ui->ejecutables->currentText(), Qt::CaseSensitive) && !existe)
                        existe = true;
                }

                if (!existe)
                {
                    ui->ejecutables->addItem(file);
                    ui->ejecutables->setCurrentIndex(ui->ejecutables->count() - 1);
                }
            }

            // guarda los ficheros utilizados en un archivo para recordarlos
            //-----------------------------------------------------------------------
            QFile methyl("./find_methyl.txt");

            // comprueba que el fichero se ha abierto correctamente
            if (!methyl.open(QIODevice::WriteOnly | QIODevice::Append))
            {
                QMessageBox::warning(this,
                                     "ERROR Opening file",
                                     "An error occurred opening the file: find_methyl.txt"
                                     "\nPlease, check the file for corrupted"
                                    );
                qDebug() << "ERROR opening file find_methyl.txt";
                return;
            }
            else
            {
                file = file + '\n';
                methyl.write(file.toStdString().c_str());
            }
            methyl.close();
        }
        else
        {
            QMessageBox::warning(this,
                                 "ERROR",
                                 "this file is not the executable file: hpg-methyl"
                                 "\nPlease, select a correct executable"
                                );
            qDebug() << "ERROR this file is not hpg-methyl";
        }
    }

    // habilita la interfaz si tiene ejecutables disponibles
    //----------------------------------------------------------------------
    if (ui->ejecutables->count() > 0)
    {
        QList<int> enable_iface = {1,1,1,
                                   0,0,0,0,0,0,0,
                                   1,1,1,1,1,1,1,1,1,
                                   1,0,0,0,0,0,0,0,0,0,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   0,0,0,1};
        widgets_enabling(enable_iface);
    }
    else
    {
        QList<int> enable_iface = {0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,1};
        widgets_enabling(enable_iface);
    }
}

//*************************************************************************************************
void Fastq2bam::fastq_acabado()
{
    QEventLoop loop;
    // pequeña espera previa antes de la siguiente iteración
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();

    // coloreado en amarillo del fondo de la línea del fichero alienado
    color.setBackground(Qt::yellow);
    cursor_fastq->select((QTextCursor::LineUnderCursor));
    cursor_fastq->setBlockFormat(color);

    // desplazamiento del cursor a la siguiente línea a alinear
    cursor_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);

    // coloreado en rojo el fondo de la línea que se está ejecutando
    color.setBackground(Qt::red);
    cursor_fastq->select((QTextCursor::LineUnderCursor));
    cursor_fastq->setBlockFormat(color);

    if (ui->pair_fastq_mode->isChecked())
    {
        // coloreado en amarillo del fondo de la línea del fichero alienado
        color.setBackground(Qt::yellow);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);

        // desplazamiento del cursor a la siguiente línea a alinear
        cursor_pair_fastq->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);

        // coloreado en rojo el fondo de la línea que se está ejecutando
        color.setBackground(Qt::red);
        cursor_pair_fastq->select((QTextCursor::LineUnderCursor));
        cursor_pair_fastq->setBlockFormat(color);
    }

    // actualizar barra de progreso
    contador += 1;
    ui->progressBar->setValue(contador);

}

void Fastq2bam::alineamiento_acabado()
{
    // habilitar los controles de la zona de alineamiento
    ui->bwt_path->setEnabled(true);
    ui->fastq_files->setEnabled(true);
    ui->fastq_delete->setEnabled(true);
    ui->fastq_up->setEnabled(true);
    ui->fastq_down->setEnabled(true);
    ui->fastq_out_path->setEnabled(true);
    ui->pair_fastq_mode->setEnabled(true);
    if (ui->pair_fastq_mode->isChecked())
    {
        ui->pair_fastq_files->setEnabled(true);
        ui->pair_fastq_delete->setEnabled(true);
        ui->pair_fastq_up->setEnabled(true);
        ui->pair_fastq_down->setEnabled(true);
    }

    // ventana emergente informando del final del proceso
    QMessageBox::information(this,
                             tr("FastQ to BAM app"),
                             tr("The alignment process is finished!")
                             );

    // informa de conclusión de alineado sobre el fichero en curso en la barra de estado
    ui->statusBar->setText("all .fastq files aligned and stored as a .bam files");

    ui->progressBar->setValue(ui->progressBar->maximum());
}

void Fastq2bam::on_stop_clicked()
{
    methyl_worker->abort();
    hilo_hpgMethyl->quit();
    system("killall -9 hpg-methyl");

    ui->start->setEnabled(true);
    ui->exit->setEnabled(true);
}
