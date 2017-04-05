#ifndef CONSTANT_H
#define CONSTANT_H

#include "constant.h"
#include <QString>
#include <QStandardPaths>

// any global variabel has to be defined in constant.h and constant.cpp

extern const QString DESKTOP_PATH ;
extern const QString HOME_PATH ;
//extern const QString CONFIG_PATH ;

extern QString DATA_PATH ;
extern QString CHEMICAL_PIC_PATH ;
extern QString SAMPLE_PIC_PATH ;
extern QString HALL_PATH ;
extern QString LOG_PATH ;
extern QString DB_PATH ;

extern double TORRGRAD ;
extern double TIME1 ;
extern double TIME2 ;

#endif // CONSTANT_H

/**************************
======== TODO--List =======
***************************
1, convert Dialog to mainwindow class ( need edit *.ui file outside Qt )
2, moving average has bug
3, better guest on par
4, calculate error-weighted mean
5, undecimated DWT
6, non-orthonormal DWT
***************************/
