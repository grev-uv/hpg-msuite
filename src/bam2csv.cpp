#include "bam2csv.h"
#include "ui_bam2csv.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QScrollBar>


Bam2csv::Bam2csv(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Bam2csv)
{
    ui->setupUi(this);

    // inicialización de variables
    //---------------------------------------------------------------------------------------------
    directorio        = false;
    // control de coloreado de filas en listas
    mc_list           = false;
    hmc_list          = false;
    // cursores de cada una de las listas
    cursor_mc         = new QTextCursor();
    cursor_hmc        = new QTextCursor();

    contador          = 0;
    ui->progressBar->setMinimum(0);

    // obtención de lista de ejecutables hpg-methyl que se han utilizado en otras ocasiones
    //---------------------------------------------------------------------------------------------
    QFile methyl("./find_mapper.txt");

    // comprueba que el fichero se ha abierto correctamente
    if (!methyl.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             "ERROR Opening file",
                             "An error occurred opening the file: find_mapper.txt"
                             "\nPlease, check the file for corrupted"
                            );
        qDebug() << "ERROR opening file find_mapper.txt";
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
        QList<int> enable_iface = {1,1,1,1,1,
                                   1,0,0,0,0,0,
                                   1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,0,0};
        widgets_enabling(enable_iface);
    }
    else
    {
        QList<int> enable_iface = {0,0,0,0,0,
                                   0,0,0,0,0,0,
                                   0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0};
        widgets_enabling(enable_iface);
    }

    // hilo y conexiones para el proceso de alineamiento
    hilo_hpgHmapper = new QThread();
    mapper_worker   = new HPGHmapper_worker();
    mapper_worker->moveToThread(hilo_hpgHmapper);
    connect(mapper_worker, SIGNAL(bam_mapeado()), SLOT(bam_mapeado()));
    hilo_hpgHmapper->connect(mapper_worker,SIGNAL(bam_solicitado()), SLOT(start()));
    mapper_worker->connect(hilo_hpgHmapper, SIGNAL(started()), SLOT(lectura()));
    hilo_hpgHmapper->connect(mapper_worker, SIGNAL(finished()), SLOT(quit()), Qt::DirectConnection);
    connect(hilo_hpgHmapper, &QThread::finished, mapper_worker, &QObject::deleteLater);
    connect(mapper_worker, SIGNAL(finished()), SLOT(bam_acabado()));
}

Bam2csv::~Bam2csv()
{
    delete ui;
}

//*************************************************************************************************
void Bam2csv::set_bam_nproc(int value)
{
    ui->ncpus_slider->setMaximum(value);
    ui->ncpus_slider->setValue(int(value * 0.5));
}

//*************************************************************************************************
void Bam2csv::set_memory_available(int value)
{
    memory_available = value;

    // muestra los valores de memoria y #procesadores en la barra de estado inferior de la ventana
    ui->statusBar->setText("System available RAM: " +
                           QString::number(memory_available) +
                           "GB - Number of processors: " +
                           QString::number(ui->ncpus_slider->maximum()));
}

//*************************************************************************************************
void Bam2csv::widgets_enabling(QList<int> enable)
{
    ui->mc_files->setEnabled(enable.at(0));
    ui->mc_delete->setEnabled(enable.at(1));
    ui->mc_up->setEnabled(enable.at(2));
    ui->mc_down->setEnabled(enable.at(3));
    ui->mc_list->setEnabled(enable.at(4));
    ui->mc_list->clear();
    ui->hmc_check->setEnabled(enable.at(5));
    ui->hmc_files->setEnabled(enable.at(6));
    ui->hmc_delete->setEnabled(enable.at(7));
    ui->hmc_up->setEnabled(enable.at(8));
    ui->hmc_down->setEnabled(enable.at(9));
    ui->hmc_list->setEnabled(enable.at(10));
    ui->hmc_list->clear();
    ui->hmc_out_path->setEnabled(enable.at(11));
    ui->hmc_out_path_label->setEnabled(enable.at(12));
    ui->hmc_out_path_label->clear();
    ui->bwt_index->setEnabled(enable.at(13));
    ui->bwt_index_label->setEnabled(enable.at(14));
    ui->bwt_index_label->clear();
    ui->label_mc_ncpus->setEnabled(enable.at(15));
    ui->ncpus_slider->setEnabled(enable.at(16));
    ui->ncpus_label->setEnabled(enable.at(17));
    ui->label_mc_ram->setEnabled(enable.at(18));
    ui->ram_slider->setEnabled(enable.at(19));
    ui->ram_label->setEnabled(enable.at(20));
    ui->label_mc_batch->setEnabled(enable.at(21));
    ui->hmc_batch_size_slider->setEnabled(enable.at(22));
    ui->hmc_batch_size_label->setEnabled(enable.at(23));
    ui->label_mc_quality->setEnabled(enable.at(24));
    ui->hmc_quality_slider->setEnabled(enable.at(25));
    ui->hmc_quality_label->setEnabled(enable.at(26));
    ui->label_coverage->setEnabled(enable.at(27));
    ui->min_coverage->setEnabled(enable.at(28));
    ui->coverage_label->setEnabled(enable.at(29));
    ui->progressBar->setEnabled(enable.at(30));
    ui->start->setEnabled(enable.at(31));
    ui->stop->setEnabled(enable.at(32));
}


//*************************************************************************************************
//**********************************ZONA 5mC FILES SELECTION***************************************
//*************************************************************************************************
void Bam2csv::on_mc_files_clicked()
{
    color.setBackground(Qt::white);
    cursor_mc->select((QTextCursor::LineUnderCursor));
    cursor_mc->setBlockFormat(color);

    // abre ventana de explorador de ficheros para seleccionar el archivo FASTA
    // con el que generar el índice BWT
    files.clear();
    files = QFileDialog::getOpenFileNames( this,
                                           tr("Select BAM 5mC files to map with HPG_HMapper"),
                                           (directorio) ? path : QDir::homePath() ,
                                           "BAM files (*.bam) ;; All files (*.*)"
                                         );
    // escribe en pantalla el nombre de los ficheros seleccionados
    if (!files.isEmpty())
    {
        mc_list = true;
        ui->mc_list->append(files.join('\n'));

        ui->start->setEnabled(true);
        if (ui->hmc_check->isChecked())
            if (QStringList(ui->hmc_list->toPlainText().split('\n'))[0].size() == 0)
                ui->start->setEnabled(false);

        directorio = true;
        path       = file.split(".")[0];
    }
}

//*************************************************************************************************
void Bam2csv::on_mc_list_cursorPositionChanged()
{
    if (mc_list)
    {
        // desmarcar la línea previa quitando color de fondo
        color.setBackground(Qt::white);
        cursor_mc->select((QTextCursor::LineUnderCursor));
        cursor_mc->setBlockFormat(color);

        // adquirir el cursor de la línea seleccionada
        *cursor_mc = ui->mc_list->textCursor();
        cursor_mc->movePosition(QTextCursor::StartOfBlock);
        cursor_mc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        // marcar con color de fondo la línea seleccionada
        color.setBackground(Qt::gray);
        cursor_mc->select((QTextCursor::LineUnderCursor));
        cursor_mc->setBlockFormat(color);
    }
}

//*************************************************************************************************
void Bam2csv::on_mc_delete_clicked()
{
    int line_number = cursor_mc->blockNumber();
    QStringList list = ui->mc_list->toPlainText().split("\n");

    mc_list = false;
    list.removeAt(line_number);
    ui->mc_list->clear();
    if (!list.isEmpty())
    {
        mc_list = true;
        ui->mc_list->append(list.join('\n'));
    }
    else
    {
        ui->stop->setEnabled(false);
        ui->start->setEnabled(false);
    }

    color.setBackground(Qt::white);
    cursor_mc->select((QTextCursor::LineUnderCursor));
    cursor_mc->setBlockFormat(color);

    cursor_mc->movePosition(QTextCursor::Start);
    for (int i = 0; i < line_number; i++)
    {
        cursor_mc->movePosition(QTextCursor::StartOfBlock);
        cursor_mc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor_mc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

        if (i == list.size() - 2)
            break;
    }

    ui->mc_list->setTextCursor(*cursor_mc);
}

//*************************************************************************************************
void Bam2csv::on_mc_up_clicked()
{
    if (cursor_mc->blockNumber() > 0)
    {
        int line_number = cursor_mc->blockNumber();
        QStringList list = ui->mc_list->toPlainText().split("\n");
        QString line = list.at(line_number);

        mc_list = false;
        list.removeAt(line_number);
        list.insert(line_number - 1, line);
        ui->mc_list->clear();
        if (!list.isEmpty())
        {
            mc_list = true;
            ui->mc_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_mc->select((QTextCursor::LineUnderCursor));
        cursor_mc->setBlockFormat(color);

        cursor_mc->movePosition(QTextCursor::Start);
        for (int i = 0; i < line_number - 1; i++)
        {
            cursor_mc->movePosition(QTextCursor::StartOfBlock);
            cursor_mc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_mc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }

        ui->mc_list->setTextCursor(*cursor_mc);
    }
}

//*************************************************************************************************
void Bam2csv::on_mc_down_clicked()
{
    QStringList list = ui->mc_list->toPlainText().split("\n");

    if (cursor_mc->blockNumber() < list.size())
    {
        int line_number = cursor_mc->blockNumber();
        QString line = list.at(line_number);

        mc_list = false;
        list.removeAt(line_number);
        list.insert(line_number + 1, line);
        ui->mc_list->clear();
        if (!list.isEmpty())
        {
            mc_list = true;
            ui->mc_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_mc->select((QTextCursor::LineUnderCursor));
        cursor_mc->setBlockFormat(color);

        cursor_mc->movePosition(QTextCursor::Start);
        for (int i = 0; i <= line_number; i++)
        {
            cursor_mc->movePosition(QTextCursor::StartOfBlock);
            cursor_mc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_mc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

            if (i == list.size() - 2)
                break;
        }

        ui->mc_list->setTextCursor(*cursor_mc);
    }
}

//*************************************************************************************************
//**********************************ZONA 5hmC FILES SELECTION**************************************
//*************************************************************************************************
void Bam2csv::on_hmc_files_clicked()
{
    color.setBackground(Qt::white);
    cursor_hmc->select((QTextCursor::LineUnderCursor));
    cursor_hmc->setBlockFormat(color);

    // abre ventana de explorador de ficheros para seleccionar el archivo FASTA
    // con el que generar el índice BWT
    files.clear();
    files = QFileDialog::getOpenFileNames( this,
                                           tr("Select BAM 5hmc files to map with HPG_HMapper"),
                                           (directorio) ? path : QDir::homePath() ,
                                           "BAM files (*.bam) ;; All files (*.*)"
                                         );
    // escribe en pantalla el nombre de los ficheros seleccionados
    if (!files.isEmpty())
    {
        hmc_list = true;
        ui->hmc_list->append(files.join('\n'));

        if (QStringList(ui->hmc_list->toPlainText().split('\n'))[0].size() != 0)
            ui->start->setEnabled(true);
        else
            ui->start->setEnabled(false);

        directorio = true;
        path       = file.split(".")[0];
    }
}

//*************************************************************************************************
void Bam2csv::on_hmc_list_cursorPositionChanged()
{
    if (hmc_list)
    {
        // desmarcar la línea previa quitando color de fondo
        color.setBackground(Qt::white);
        cursor_hmc->select((QTextCursor::LineUnderCursor));
        cursor_hmc->setBlockFormat(color);

        // adquirir el cursor de la línea seleccionada
        *cursor_hmc = ui->hmc_list->textCursor();
        cursor_hmc->movePosition(QTextCursor::StartOfBlock);
        cursor_hmc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        // marcar con color de fondo la línea seleccionada
        color.setBackground(Qt::gray);
        cursor_hmc->select((QTextCursor::LineUnderCursor));
        cursor_hmc->setBlockFormat(color);
    }
}

//*************************************************************************************************
void Bam2csv::on_hmc_delete_clicked()
{
    int line_number = cursor_hmc->blockNumber();
    QStringList list = ui->hmc_list->toPlainText().split("\n");

    hmc_list = false;
    list.removeAt(line_number);
    ui->hmc_list->clear();
    if (!list.isEmpty())
    {
        hmc_list = true;
        ui->hmc_list->append(list.join('\n'));
    }
    else
    {
        ui->start->setEnabled(false);
        ui->stop->setEnabled(false);
    }

    color.setBackground(Qt::white);
    cursor_hmc->select((QTextCursor::LineUnderCursor));
    cursor_hmc->setBlockFormat(color);

    cursor_hmc->movePosition(QTextCursor::Start);
    for (int i = 0; i < line_number; i++)
    {
        cursor_hmc->movePosition(QTextCursor::StartOfBlock);
        cursor_hmc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor_hmc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

        if (i == list.size() - 2)
            break;
    }

    ui->hmc_list->setTextCursor(*cursor_hmc);
}

//*************************************************************************************************
void Bam2csv::on_hmc_up_clicked()
{
    if (cursor_hmc->blockNumber() > 0)
    {
        int line_number = cursor_hmc->blockNumber();
        QStringList list = ui->hmc_list->toPlainText().split("\n");
        QString line = list.at(line_number);

        hmc_list = false;
        list.removeAt(line_number);
        list.insert(line_number - 1, line);
        ui->hmc_list->clear();
        if (!list.isEmpty())
        {
            hmc_list = true;
            ui->hmc_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_hmc->select((QTextCursor::LineUnderCursor));
        cursor_hmc->setBlockFormat(color);

        cursor_hmc->movePosition(QTextCursor::Start);
        for (int i = 0; i < line_number - 1; i++)
        {
            cursor_hmc->movePosition(QTextCursor::StartOfBlock);
            cursor_hmc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_hmc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }

        ui->hmc_list->setTextCursor(*cursor_hmc);
    }
}

//*************************************************************************************************
void Bam2csv::on_hmc_down_clicked()
{
    QStringList list = ui->hmc_list->toPlainText().split("\n");

    if (cursor_hmc->blockNumber() < list.size())
    {
        int line_number = cursor_hmc->blockNumber();
        QString line = list.at(line_number);

        hmc_list = false;
        list.removeAt(line_number);
        list.insert(line_number + 1, line);
        ui->hmc_list->clear();
        if (!list.isEmpty())
        {
            hmc_list = true;
            ui->hmc_list->append(list.join('\n'));
        }

        color.setBackground(Qt::white);
        cursor_hmc->select((QTextCursor::LineUnderCursor));
        cursor_hmc->setBlockFormat(color);

        cursor_hmc->movePosition(QTextCursor::Start);
        for (int i = 0; i <= line_number; i++)
        {
            cursor_hmc->movePosition(QTextCursor::StartOfBlock);
            cursor_hmc->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor_hmc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);

            if (i == list.size() - 2)
                break;
        }

        ui->hmc_list->setTextCursor(*cursor_hmc);
    }
}


//*************************************************************************************************
//**********************************ZONA BAM OPTIONS***********************************************
//*************************************************************************************************
void Bam2csv::on_hmc_out_path_clicked()
{
    // abre ventana de explorador de directorios para seleccionar
    // el directorio donde guardar los ficheros CSV con los cromosomas mapeados
    file = QFileDialog::getExistingDirectory( this,
                                              tr("Select a folder to save the Mapped files"),
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
    ui->hmc_out_path_label->setText(file + "/");
}

//*************************************************************************************************
void Bam2csv::on_ram_slider_valueChanged(int value)
{
    ui->ram_label->setText(QString::number(value) + " %");
}


//*************************************************************************************************
//**********************************ZONA RUNNIG APPLICATIONS***************************************
//*************************************************************************************************
void Bam2csv::on_start_clicked()
{
    // habilita el pulsador de parada de proceso
    ui->stop->setEnabled(true);
    // deshabilita el pulsador de inicio de proceso y exit
    ui->start->setEnabled(false);
    ui->exit->setEnabled(false);

    // proceso de mapeado
    bam_2_csv();
}


//*************************************************************************************************
void Bam2csv::on_stop_clicked()
{
    mapper_worker->abort();
    hilo_hpgHmapper->quit();

    ui->start->setEnabled(true);
    ui->exit->setEnabled(true);
    ui->stop->setEnabled(false);
}


//*************************************************************************************************
void Bam2csv::on_exit_clicked()
{
    this->hide();
}


//*************************************************************************************************
void Bam2csv::on_ejecutable_clicked()
{
    // selecciona la ruta al programa de alineamiento hpg-methyl
    file = QFileDialog::getOpenFileName( this,
                                         tr("Select the HPG-HMAPPER executable"),
                                         QDir::homePath(),
                                         "All files (*)"
                                       );

    if(file.isEmpty() || file.isNull())
        file = "";
    else
    {
        if(file.endsWith("hpg-hmapper"))
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
            QFile methyl("./find_mapper.txt");

            // comprueba que el fichero se ha abierto correctamente
            if (!methyl.open(QIODevice::WriteOnly | QIODevice::Append))
            {
                QMessageBox::warning(this,
                                     "ERROR Opening file",
                                     "An error occurred opening the file: find_mapper.txt"
                                     "\nPlease, check the file for corrupted"
                                    );
                qDebug() << "ERROR opening file find_mapper.txt";
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
                                 "this file is not the executable file: hpg-hmapper"
                                 "\nPlease, select a correct executable"
                                );
            qDebug() << "ERROR this file is not hpg-hmapper";
        }
    }

    // habilita la interfaz si tiene ejecutables disponibles
    //----------------------------------------------------------------------
    if (ui->ejecutables->count() > 0)
    {
        QList<int> enable_iface = {1,1,1,1,1,
                                   1,0,0,0,0,0,
                                   1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,0,0};
        widgets_enabling(enable_iface);
    }
    else
    {
        QList<int> enable_iface = {0,0,0,0,0,
                                   0,0,0,0,0,0,
                                   0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0};
        widgets_enabling(enable_iface);
    }
}


//*************************************************************************************************
void Bam2csv::bam_2_csv()
{
    // variables internas
    // --------------------------------------------------------------------------------------------
    QEventLoop loop;            // delay por bucle

    // ventanas de precaución ante falta de datos previos al proceso
    // --------------------------------------------------------------------------------------------
    // falta la ruta donde guardar los ficheros mapeados CSV
    if (ui->hmc_out_path_label->text().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("BAM to CSV app"),
                             tr("The path to save the CSV mapped files is empty\n"
                                "Please, select a folder to save them")
                            );

        ui->start->setEnabled(true);
        ui->stop->setEnabled(false);

        return;
    }

    // falta la ruta del genoma de referencia
    if (ui->bwt_index_label->text().isEmpty())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "BWT-INDEX is empty",
                                      "There are no bwt-index selected.\n"
                                      "By default, HPG-Hmapper works with human GRCh37.68\n"
                                      "Do you want to continue?",
                                      QMessageBox::Yes|QMessageBox::No);

        // confirmación de inicio de proceso de mapeado
        if (reply == QMessageBox::No)
        {
            ui->start->setEnabled(true);
            ui->stop->setEnabled(false);
            return;
        }
    }

    // listas de ficheros a mapear
    QStringList list_1 = ui->mc_list->toPlainText().split("\n");
    QStringList list_2;
    if (ui->hmc_check->isChecked())
        list_2 = ui->hmc_list->toPlainText().split("\n");
    else {
        QString path = ui->ejecutables->currentText();
        path = path.left(path.lastIndexOf("/"));
        for (int i = 0; i < list_1.size(); i++)
            list_2.append(path + "/map_no_hmc.bam");
    }


    // si las listas no coinciden en longitud
    if (list_1.size() != list_2.size())
    {
        QMessageBox::warning(this,
                             tr("BAM to CSV app"),
                             tr("The 5mC list and 5hmC list must have the same length\n"
                                "Please, adjust the selected files")
                            );

        ui->start->setEnabled(true);
        ui->stop->setEnabled(false);

        return;
    }

    // inicio del proceso de mapeado
    //---------------------------------------------------------------------------------------------
    // se recuerda que la listas deben tener el mismo orden en los ficheros para mapear sobre los
    // mismos BAM metilados e hidroximetilados
    QMessageBox::StandardButton reply;
    QString message;
    if (!ui->hmc_check->isChecked())
        message = "HPG-Hmapper is going to map without hidroximetilation part.\n"
                  "Is it right?";
    else
        message = "Remember, the orden of the lists is very important.\n"
                  "Has each 5mC and 5hmC files the same number of orden?\n"
                  "It means, are the 5mC and 5hmC files paired?";

    reply = QMessageBox::question(this,
                                  "BAM to CSV app",
                                  message,
                                  QMessageBox::Yes|QMessageBox::No);

    // confirmación de inicio de proceso de mapeado
    if (reply == QMessageBox::Yes)
    {
        // deshabilitado de controles de zona de mapeado
        ui->mc_files->setEnabled(false);
        ui->mc_delete->setEnabled(false);
        ui->mc_up->setEnabled(false);
        ui->mc_down->setEnabled(false);
        ui->hmc_files->setEnabled(false);
        ui->hmc_delete->setEnabled(false);
        ui->hmc_up->setEnabled(false);
        ui->hmc_down->setEnabled(false);
        ui->hmc_out_path->setEnabled(false);
        ui->bwt_index->setEnabled(false);

        // moviendo los cursores al inicio de cada lista de ficheros a mapear
        cursor_mc->movePosition(QTextCursor::Start);
        cursor_hmc->movePosition(QTextCursor::Start);
        // coloreado en rojo de la línea de los ficheros mapeando
        color.setBackground(Qt::red);
        cursor_mc->select((QTextCursor::LineUnderCursor));
        cursor_mc->setBlockFormat(color);
        cursor_hmc->select((QTextCursor::LineUnderCursor));
        cursor_hmc->setBlockFormat(color);

        // informa a usuario de inicio del proceso sobre el fichero correspondiente
        ui->statusBar->setText("mapping...");

        // pequeña espera para que se coloreen las líneas
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        // prepara parametros para eviar al worker
        QStringList parametros;
        parametros << QString::number(ui->ncpus_slider->value()) <<
                      QString::number(memory_available) <<
                      QString::number(ui->ram_slider->value()) <<
                      QString::number(ui->hmc_batch_size_slider->value()) <<
                      QString::number(ui->hmc_quality_slider->value()) <<
                      QString::number(ui->min_coverage->value());

        // barra de progreso inicializada a cero;
        ui->progressBar->setMaximum(list_1.size());
        ui->progressBar->setValue(0);

        // lanza el hilo de alineamiento
        mapper_worker->solicitud_mapeado(ui->ejecutables->currentText(),
                                         parametros,
                                         ui->hmc_out_path_label->text(),
                                         ui->bwt_index_label->text(),
                                         list_1,
                                         list_2);
    }
    else
    {
        ui->start->setEnabled(true);
        ui->stop->setEnabled(false);
        return;
    }

}

//*************************************************************************************************
void Bam2csv::bam_mapeado()
{
    QEventLoop loop;
    // pequeña espera previa antes de la siguiente iteración
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();

    // coloreado en amarillo de las líneas mapeadas
    color.setBackground(Qt::yellow);
    cursor_mc->select((QTextCursor::LineUnderCursor));
    cursor_mc->setBlockFormat(color);
    cursor_hmc->select((QTextCursor::LineUnderCursor));
    cursor_hmc->setBlockFormat(color);

    // desplazamiento de los cursores a la siguiente línea
    cursor_mc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
    cursor_hmc->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);

    // coloreado en rojo de la línea de los ficheros mapeando
    color.setBackground(Qt::red);
    cursor_mc->select((QTextCursor::LineUnderCursor));
    cursor_mc->setBlockFormat(color);
    cursor_hmc->select((QTextCursor::LineUnderCursor));
    cursor_hmc->setBlockFormat(color);

    // actualizar barra de progreso
    contador += 1;
    ui->progressBar->setValue(contador);
}

//*************************************************************************************************
void Bam2csv::bam_acabado()
{
    // habilita los controles de la zona de mapeado
    ui->mc_files->setEnabled(true);
    ui->mc_delete->setEnabled(true);
    ui->mc_up->setEnabled(true);
    ui->mc_down->setEnabled(true);
    if (ui->hmc_check->isChecked())
    {
        ui->hmc_files->setEnabled(true);
        ui->hmc_delete->setEnabled(true);
        ui->hmc_up->setEnabled(true);
        ui->hmc_down->setEnabled(true);
    }
    else {
        ui->hmc_files->setEnabled(false);
        ui->hmc_delete->setEnabled(false);
        ui->hmc_up->setEnabled(false);
        ui->hmc_down->setEnabled(false);
    }
    ui->hmc_out_path->setEnabled(true);
    ui->bwt_index->setEnabled(true);

    // informa de la conclusión del proceso de mapeado sobre toda la lista
    QMessageBox::information(this,
                             tr("BAM to CSV app"),
                             tr("The mapping process is finished!")
                            );

    ui->statusBar->setText("all .bam files mapped and stored as a .csv files");

//    contador = 0;
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->start->setEnabled(true);
    ui->stop->setEnabled(false);
    ui->exit->setEnabled(true);
}

void Bam2csv::on_hmc_check_stateChanged(int arg1)
{
    ui->hmc_files->setEnabled(arg1);
    ui->hmc_delete->setEnabled(arg1);
    ui->hmc_up->setEnabled(arg1);
    ui->hmc_down->setEnabled(arg1);
    ui->hmc_list->setEnabled(arg1);
}

void Bam2csv::on_bwt_index_clicked()
{
    // abre ventana de explorador de directorios para seleccionar
    // el directorio donde guardar los ficheros CSV con los cromosomas mapeados
    file = QFileDialog::getExistingDirectory( this,
                                              tr("Select the folder where bwt-index is"),
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
    ui->bwt_index_label->setText(file + "/");
}
