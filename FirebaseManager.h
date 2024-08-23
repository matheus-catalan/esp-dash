#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H
#include "ConfigManager.h"
#include "SensorsManager.h"

String getCurrentTimestamp();
void connectToFirebase(Config &config);
void sendData(Config &config);

#endif
