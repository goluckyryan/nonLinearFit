#ifndef CONSTANT_H
#define CONSTANT_H

#include <QString>

// must have "/" at the end.
//const QString OPENPATH = "C:/Users/Triplet-ESR/Desktop/nonLinearFit/";
//const QString OPENPATH = "C:/Users/goluc/Desktop/nonLinearFit/";
const QString OPENPATH = "/Users/mobileryan/Triplet-ESR/";

const double TORR = 1e-6; //torrelence for fitting SSR
const double TORRGRAD = 1e-1; //torrenlence for grad of SSR

const double TIME1 = 4.0;  // start fitting time
const double TIME2 = -3.0; // end time of sample mean and variance

#endif // CONSTANT_H
