#include "backup_window.h"

#include <QApplication>
#include <kyronext/ui.h>

int main(int argc, char **argv) {
    QApplication application(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Backup"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(application, Kyronext::paletteFromSettings());
    BackupWindow window;
    window.show();
    return application.exec();
}
