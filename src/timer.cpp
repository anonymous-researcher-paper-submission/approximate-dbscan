#include "timer.h"

chrono::duration<double> hashTime = chrono::duration<double>::zero();
chrono::duration<double> queryTime = chrono::duration<double>::zero();
chrono::duration<double> bsElapsed = chrono::duration<double>::zero();
chrono::duration<double> queryElapsed = chrono::duration<double>::zero();

chrono::duration<double> ccbsTime = chrono::duration<double>::zero();
chrono::duration<double> ccQueryTotal = chrono::duration<double>::zero();
chrono::duration<double> ccHashTime = chrono::duration<double>::zero();
chrono::duration<double> ccGroupTime = chrono::duration<double>::zero();
chrono::duration<double> ccQueryTime = chrono::duration<double>::zero();

chrono::duration<double> spTime = chrono::duration<double>::zero();
