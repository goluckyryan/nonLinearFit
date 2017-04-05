#include "constant.h"

const QString DESKTOP_PATH = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
const QString HOME_PATH = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
//const QString CONFIG_PATH = DESKTOP_PATH + "AnalysisProgram.ini";

QString DATA_PATH;
QString CHEMICAL_PIC_PATH;
QString SAMPLE_PIC_PATH;
QString HALL_PATH;
QString LOG_PATH;
QString DB_PATH;

double TORRGRAD = 1e-1; //torrenlence for grad of SSR
double TIME1 = 5.0;  // start fitting time
double TIME2 = -3.0; // end time of sample mean and variance
