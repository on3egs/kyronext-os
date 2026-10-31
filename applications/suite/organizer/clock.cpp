#include "clock_window.h"
int main(int argc,char **argv){QApplication app(argc,argv);Kyronext::configureApplication("Kyronext Clock","0.1.0");Kyronext::applyTheme(app,Kyronext::paletteFromSettings());ClockWindow window;window.show();return app.exec();}
