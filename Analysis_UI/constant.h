#ifndef CONSTANT_H
#define CONSTANT_H

#include <QString>
#include <QStandardPaths>

const QString DESKTOP_PATH = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);

const double TORR = 1e-6; //torrelence for fitting SSR
const double TORRGRAD = 1e-1; //torrenlence for grad of SSR

const double TIME1 = 5.0;  // start fitting time
const double TIME2 = -3.0; // end time of sample mean and variance

#endif // CONSTANT_H
