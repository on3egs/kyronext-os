#include "browser.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

int main(int argc,char **argv) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);Navigator::registerStartScheme();QApplication app(argc,argv);
    if(Navigator::unsafeEnvironment()){QMessageBox::critical(nullptr,"Kyronext Navigator","Demarrage refuse : options externes du moteur ou du sandbox detectees. Retirez-les ; les protections ne seront pas contournees.");return 2;}
#ifdef Q_OS_UNIX
    if(geteuid()==0){QMessageBox::critical(nullptr,"Kyronext Navigator","Demarrage refuse en administrateur. Utilisez un compte ordinaire avec le sandbox actif.");return 2;}
#endif
    QCoreApplication::setOrganizationName("Kyronext Systems");QCoreApplication::setApplicationName("Kyronext Navigator");QSettings().setValue("logging/enabled",false);
    Kyronext::configureApplication("Kyronext Navigator","0.1.0");Kyronext::applyTheme(app,Kyronext::paletteFromSettings());
    auto *window=new Navigator::BrowserWindow(app.arguments().contains("--private"));window->show();return app.exec();
}
