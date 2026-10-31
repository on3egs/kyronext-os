#pragma once
#include <QString>
class QApplication;
class QCheckBox;
class QMainWindow;
class QWidget;
namespace Kyronext {
enum class Palette { Red, Amber, HighContrast };
enum class Density { Compact, Comfortable };
void configureApplication(const QString &appName, const QString &version);
Palette paletteFromSettings();
void setPreferredPalette(Palette palette);
bool lightModeFromSettings();
void setLightModeEnabled(bool enabled);
Density densityFromSettings();
void setPreferredDensity(Density density);
bool reducedMotionFromSettings();
void setReducedMotionEnabled(bool enabled);
bool manualReducedMotionChoice(const QCheckBox *control);
void setManualReducedMotionChoice(QCheckBox *control, bool enabled);
void displayReducedMotionChoice(QCheckBox *control, bool forced);
void applyTheme(QApplication &app, Palette palette = Palette::Red);
void addAboutMenu(QMainWindow *window, const QString &appName,
                  const QString &description, const QString &dependencies);
void showError(QWidget *parent, const QString &message);
void notify(QWidget *parent, const QString &message);
QWidget *emptyState(const QString &title, const QString &description, QWidget *parent = nullptr);
}
