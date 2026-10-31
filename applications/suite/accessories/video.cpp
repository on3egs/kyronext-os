#include <kyronext/ui.h>
#include "help.h"
#include <QApplication>
#include <QAction>
#include <QKeySequence>
#include <QUrl>
#include <QAudioOutput>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMenuBar>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSlider>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QVideoWidget>
#include <algorithm>

class Video final : public QMainWindow {
    QMediaPlayer player; QAudioOutput audio;
    QVideoWidget *screen = new QVideoWidget;
    QLabel *subtitle = new QLabel; QLabel *clock = new QLabel("0:00 / 0:00");
    QPushButton *play = new QPushButton("Lecture / pause");
    QSlider *position = new QSlider(Qt::Horizontal);
    QComboBox *audioTracks = new QComboBox; QComboBox *subtitles = new QComboBox;
public:
    Video() {
        setWindowTitle("Kyronext Video"); resize(960, 680); setMinimumSize(420, 320);
        player.setAudioOutput(&audio); player.setVideoOutput(screen); audio.setVolume(.7);
        auto *body = new QWidget; auto *layout = new QVBoxLayout(body); layout->setContentsMargins(16, 16, 16, 16);
        screen->setMinimumSize(160, 100); layout->addWidget(screen, 1);
        subtitle->setTextFormat(Qt::PlainText); subtitle->setAlignment(Qt::AlignCenter); subtitle->setWordWrap(true);
        subtitle->setAccessibleName("Sous-titre courant"); layout->addWidget(subtitle);
        layout->addWidget(position); position->setRange(0, 10000); position->setEnabled(false); position->setAccessibleName("Position de lecture");
        auto *controls = new QHBoxLayout;
        auto *open = new QPushButton("Ouvrir…"); auto *stop = new QPushButton("Stop"); auto *full = new QPushButton("Plein écran");
        play->setEnabled(false);
        controls->addWidget(open); controls->addWidget(play); controls->addWidget(stop); controls->addWidget(clock); controls->addWidget(full);
        auto *volume = new QSlider(Qt::Horizontal); volume->setRange(0, 100); volume->setValue(70); volume->setMaximumWidth(150); volume->setAccessibleName("Volume");
        controls->addWidget(new QLabel("Volume")); controls->addWidget(volume); layout->addLayout(controls);
        auto *tracks = new QHBoxLayout;
        tracks->addWidget(new QLabel("Audio")); tracks->addWidget(audioTracks, 1); tracks->addWidget(new QLabel("Sous-titres intégrés")); tracks->addWidget(subtitles, 1);
        audioTracks->setAccessibleName("Piste audio"); subtitles->setAccessibleName("Piste de sous-titres"); layout->addLayout(tracks);
        setCentralWidget(body);
        connect(open, &QPushButton::clicked, this, [this]{ choose(); });
        connect(play, &QPushButton::clicked, this, [this]{ if (player.playbackState() == QMediaPlayer::PlayingState) player.pause(); else if (!player.source().isEmpty()) player.play(); });
        connect(stop, &QPushButton::clicked, &player, &QMediaPlayer::stop);
        connect(full, &QPushButton::clicked, this, [this]{ toggleFull(); });
        connect(volume, &QSlider::valueChanged, this, [this](int value){ audio.setVolume(value / 100.0); });
        connect(position, &QSlider::sliderReleased, this, [this]{ if (player.duration() > 0) player.setPosition(qint64(position->value() / 10000.0 * player.duration())); });
        connect(&player, &QMediaPlayer::positionChanged, this, [this](qint64 value){
            if (!position->isSliderDown() && player.duration() > 0) position->setValue(int(value * 10000.0 / player.duration()));
            clock->setText(time(value) + " / " + time(player.duration()));
        });
        connect(&player, &QMediaPlayer::durationChanged, this, [this](qint64 value){ position->setEnabled(value > 0); });
        connect(&player, &QMediaPlayer::seekableChanged, position, &QSlider::setEnabled);
        connect(&player, &QMediaPlayer::tracksChanged, this, [this]{ refreshTracks(); });
        connect(audioTracks, &QComboBox::currentIndexChanged, this, [this](int index){ if (index >= 0) player.setActiveAudioTrack(audioTracks->itemData(index).toInt()); });
        connect(subtitles, &QComboBox::currentIndexChanged, this, [this](int index){ if (index >= 0) player.setActiveSubtitleTrack(subtitles->itemData(index).toInt()); });
        connect(screen->videoSink(), &QVideoSink::subtitleTextChanged, subtitle, &QLabel::setText);
        connect(&player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &message){ statusBar()->showMessage("Lecture impossible : " + message); });
        connect(&player, &QMediaPlayer::playbackStateChanged, this, [this](auto state){ play->setText(state == QMediaPlayer::PlayingState ? "Pause" : "Lecture"); });
        auto *file = menuBar()->addMenu("&Fichier");
        file->addAction("Ouvrir une vidéo…", this, [this]{ choose(); }, QKeySequence::Open);
        file->addAction("Quitter", this, &QWidget::close, QKeySequence::Quit);
        auto *view = menuBar()->addMenu("&Affichage"); view->addAction("Plein écran", this, [this]{ toggleFull(); }, QKeySequence("F11"));
        new QShortcut(QKeySequence("Escape"), this, [this]{ if (isFullScreen()) showNormal(); });
        new QShortcut(QKeySequence("Space"), this, [this]{ play->click(); });
        Kyronext::addAboutMenu(this, "Kyronext Video", "Lecteur local ; formats et pistes selon le moteur Qt Multimedia installé.", "Qt 6 Multimedia / Widgets — LGPL/GPL ; codecs fournis par Debian");
        AccessoryHelp::install(this, "Kyronext Video", "Ouvrez un fichier vidéo local pour démarrer la lecture.\nEspace : lecture/pause. F11 : plein écran ; Échap : revenir à la fenêtre.\nLes pistes audio et sous-titres intégrés apparaissent uniquement si le moteur les détecte. Les sous-titres externes ne sont pas encore pris en charge.\nLes formats disponibles dépendent des codecs Debian installés. Un message d'erreur remplace toute promesse de décodage non disponible.");
        refreshTracks(); statusBar()->showMessage("Ouvrez un fichier local. Aucun téléchargement ni lecture automatique au démarrage.");
    }
    void open(const QString &name) {
        QFileInfo file(name);
        if (!file.isFile() || !file.isReadable()) { Kyronext::showError(this, "Fichier absent ou non lisible."); return; }
        player.stop(); subtitle->clear(); player.setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
        play->setEnabled(true);
        setWindowTitle(file.fileName() + " — Kyronext Video"); player.play();
    }
private:
    static QString time(qint64 milliseconds) { auto seconds = std::max<qint64>(0, milliseconds / 1000); return QString::number(seconds / 60) + ":" + QString::number(seconds % 60).rightJustified(2, '0'); }
    void choose() { auto name = QFileDialog::getOpenFileName(this, "Ouvrir une vidéo locale", {}, "Vidéos (*.mp4 *.mkv *.webm *.avi *.mov *.ogv);;Tous les fichiers (*)"); if (!name.isEmpty()) open(name); }
    void toggleFull() { if (isFullScreen()) showNormal(); else showFullScreen(); }
    void refreshTracks() {
        const QSignalBlocker blockAudio(audioTracks), blockSub(subtitles);
        audioTracks->clear(); subtitles->clear(); subtitles->addItem("Désactivés", -1);
        auto add = [](QComboBox *combo, const QList<QMediaMetaData> &tracks) {
            for (int i = 0; i < tracks.size(); ++i) {
                auto title = tracks[i].stringValue(QMediaMetaData::Title);
                auto language = tracks[i].stringValue(QMediaMetaData::Language);
                combo->addItem(QString("%1 — %2 %3").arg(i + 1).arg(title, language), i);
            }
        };
        add(audioTracks, player.audioTracks()); add(subtitles, player.subtitleTracks());
        audioTracks->setEnabled(audioTracks->count() > 0); subtitles->setEnabled(subtitles->count() > 1);
        audioTracks->setCurrentIndex(player.activeAudioTrack()); subtitles->setCurrentIndex(player.activeSubtitleTrack() + 1);
    }
};
int main(int argc, char **argv) {
    QApplication app(argc, argv); Kyronext::configureApplication("Kyronext Video", "0.1.0");
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    Video window; if (argc == 2) window.open(QString::fromLocal8Bit(argv[1]));
    window.show(); return app.exec();
}
