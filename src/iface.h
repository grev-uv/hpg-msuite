/*
*  iface is the main class to help the align and mapp of fastq and bam files
*  whit HPG-Methyl and HPG-HMapper software.
*  Copyright (C) 2018 Lisardo Fernández Cordeiro <lisardo.fernandez@uv.es>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*
*/

/** \file
*  \brief Programa de ayuda a la manipulación de ficheros FastQ y BAM
*         para su procesamiento con las herramientas HPG-Methyl y HPG-HMapper.
*
*  Este archivo contiene la definición de las funciones para:
*         ..control de ficheros a alinear
*         ..control de ficheros a mapear
*         ..generación de fichero índice BWT
*/

#ifndef IFACE_H
#define IFACE_H

#include "fastq2bam.h"
#include "bam2csv.h"
#include <QMainWindow>
#include <QTextCursor>

namespace Ui {
class IFace;
}

class IFace : public QMainWindow
{
    Q_OBJECT

public:
    explicit IFace(QWidget *parent = nullptr);
    ~IFace();

private slots:
    /** ***********************************************************************************************
      * \fn void on_fastq_2_bam_clicked()
      *  \brief Función responsable de habilitar la zona de selección de ficheros fastq para alineamiento
      * ***********************************************************************************************
      */
    void on_fastq_2_bam_clicked();

    /** ***********************************************************************************************
      * \fn on_bam_2_csv_clicked()
      *  \brief Función responsable de habilitar la zona de selección de ficheros bam para mapearlos
      * ***********************************************************************************************
      */
    void on_bam_2_csv_clicked();

    /** ***********************************************************************************************
      * \fn void on_dmr_clicked()
      *  \brief Función responsable de abrir la aplicación de visualización de señal mC y hmC
      * ***********************************************************************************************
      */
    void on_dmr_clicked();

private:
    Ui::IFace *ui;

    // interfaces para conversión de fastq a bam y bam a csv
    Fastq2bam *fastq2bam;
    Bam2csv   *bam2csv;

    /** ***********************************************************************************************
      *  \brief variable para control de memoria disponible en el sistema
      *  \param memory_available    memoria en GB disponible para parametrizar en HPG-HMapper
      * ***********************************************************************************************
      */
    int memory_available;
};

#endif // IFACE_H
