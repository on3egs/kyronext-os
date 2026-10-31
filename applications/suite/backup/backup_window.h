#pragma once

#include "archive_backend.h"

#include <QMainWindow>
#include <atomic>

class QLineEdit;
class QListWidget;
class QPushButton;
class QProgressBar;
class QThread;

class BackupWindow final : public QMainWindow {
public:
    explicit BackupWindow(QWidget *parent = nullptr);
    ~BackupWindow() override;

private:
    QListWidget *sources_ = nullptr;
    QLineEdit *destination_ = nullptr;
    QPushButton *start_ = nullptr;
    QPushButton *cancel_ = nullptr;
    QPushButton *addSource_ = nullptr;
    QPushButton *removeSources_ = nullptr;
    QPushButton *browseDestination_ = nullptr;
    QProgressBar *progress_ = nullptr;
    QThread *worker_ = nullptr;
    std::atomic_bool cancelRequested_{false};
    BackupResult workerResult_;

    void addSource();
    void chooseDestination();
    void startBackup();
    void finishBackup();
    void setBusy(bool busy);
};
