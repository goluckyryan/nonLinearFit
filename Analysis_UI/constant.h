#ifndef CONSTANT_H
#define CONSTANT_H

#include <QString>
#include <QStandardPaths>

const QString DESKTOP_PATH = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
const QString HOME_PATH = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
const QString DATA_PATH = HOME_PATH + "ESR_Data_201610/";
const QString LOG_PATH = DATA_PATH + "DAQ_Log";
const QString HALL_DIR_PATH = DATA_PATH + "Hall_data";
const QString HALL_PATH = HALL_DIR_PATH + "/Hall_pars.txt";


const double TORRGRAD = 1e-1; //torrenlence for grad of SSR

const double TIME1 = 5.0;  // start fitting time
const double TIME2 = -3.0; // end time of sample mean and variance

#endif // CONSTANT_H

/**************************
======== TODO--List =======
***************************
1, convert Dialog to mainwindow class ( need edit *.ui file outside Qt )
2, moving average has bug
3, better guest on par
4, when B-plot did not open once, disable print
5, calculate error-weighted mean
6, undecimated DWT
7, non-orthonormal DWT
***************************/
