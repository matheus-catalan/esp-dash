#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

void VextON(void);
void VextOFF(void);
void displayReset(void);
void setupDisplay();
void drawProgressBar(int progress, String text);
void drawText(String text);
void drawHomeScreen(String name, String wifi, String ssid, String ip, String db, String passwd);


#endif
