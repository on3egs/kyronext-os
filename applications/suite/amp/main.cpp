#include "playlist.h"
#include "processed_audio.h"
#include "visualizer.h"
#include <QGroupBox>
#include <QComboBox>
#include <QSignalBlocker>
#include <QScrollArea>
#include <kyronext/ui.h>
#include <QApplication>
#include <QAudioOutput>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QShortcut>
#include <QSlider>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QCheckBox>
#include <limits>

class AmpWindow final : public QMainWindow {
public:
    AmpWindow() {
        setWindowTitle(QStringLiteral("Kyronext AMP"));
        setWindowIcon(QIcon(QStringLiteral(":/amp/resources/kyronext-amp.svg")));
        resize(760, 640);
        setMinimumSize(480, 420);
        processed = new Amp::ProcessedAudio(this);
        output = new QAudioOutput(this);
        player = new QMediaPlayer(this);
        player->setAudioOutput(output);
        auto *body = new QWidget(this);
        auto *layout = new QVBoxLayout(body);
        layout->setContentsMargins(20, 16, 20, 16);
        brand = new QLabel(QStringLiteral("KYRONEXT / AMP"), body);
        brand->setStyleSheet(QStringLiteral("font-size:20px;font-weight:700;color:#ef3c60"));
        layout->addWidget(brand);
        auto *track = new QHBoxLayout;
        cover = new QLabel(body);
        cover->setFixedSize(96, 96);
        cover->setAlignment(Qt::AlignCenter);
        cover->setText(QStringLiteral("AMP"));
        cover->setStyleSheet(QStringLiteral("background:#20242c;border:1px solid #515762;font-size:24px;color:#ef3c60"));
        cover->setAccessibleName(tr("Pochette du morceau"));
        track->addWidget(cover);
        auto *information = new QVBoxLayout;
        title = new QLabel(tr("Choisissez votre musique"), body);
        title->setWordWrap(true);
        title->setStyleSheet(QStringLiteral("font-size:19px;font-weight:600"));
        details = new QLabel(tr("Fichiers locaux • playlist M3U"), body);
        details->setWordWrap(true);
        information->addWidget(title);
        information->addWidget(details);
        information->addStretch();
        track->addLayout(information, 1);
        layout->addLayout(track);
        position = new QSlider(Qt::Horizontal, body);
        position->setAccessibleName(tr("Position de lecture"));
        position->setRange(0, 1000);
        position->setEnabled(false);
        layout->addWidget(position);
        time = new QLabel(QStringLiteral("0:00 / 0:00"), body);
        time->setAlignment(Qt::AlignRight);
        layout->addWidget(time);
        auto *transport = new QHBoxLayout;
        auto button = [&](const QString &text, auto action) {
            auto *control = new QPushButton(text, body);
            control->setAccessibleName(text);
            transport->addWidget(control);
            connect(control, &QPushButton::clicked, this, action);
            return control;
        };
        previous = button(tr("Précédent"), [this] { previousTrack(); });
        play = button(tr("Lecture"), [this] { togglePlayback(); });
        stop = button(tr("Stop"), [this] { if(dspActive){processed->seek(0);processed->pause();position->setValue(0);time->setText(QStringLiteral("0:00 / ")+Amp::displayTime(processed->duration()));play->setText(tr("Lecture"));}else player->stop(); });
        next = button(tr("Suivant"), [this] { nextTrack(true); });
        layout->addLayout(transport);
        auto *sound = new QHBoxLayout;
        mute = new QCheckBox(tr("Muet"), body);
        sound->addWidget(mute);
        sound->addWidget(new QLabel(tr("Volume"), body));
        volume = new QSlider(Qt::Horizontal, body);
        volume->setRange(0, 100);
        volume->setAccessibleName(tr("Volume"));
        QSettings settings;
        volume->setValue(settings.value(QStringLiteral("audio/volume"), 65).toInt());
        sound->addWidget(volume, 1);
        layout->addLayout(sound);
        auto setVolume = [this](int value) {
            // Cubic taper gives more useful control near silence than a linear fader.
            const float fraction = float(value) / 100.0f;
            output->setVolume(fraction * fraction * fraction);
            processed->setVolume(fraction * fraction * fraction);
        };
        setVolume(volume->value());
        connect(volume, &QSlider::valueChanged, this, setVolume);
        connect(mute, &QCheckBox::toggled, this, [this](bool value){output->setMuted(value);processed->setMuted(value);});
        addDspControls(layout,body);
        auto *options = new QHBoxLayout;
        shuffle = new QCheckBox(tr("Aléatoire"), body);
        repeat = new QCheckBox(tr("Répéter la playlist"), body);
        options->addWidget(shuffle);
        options->addWidget(repeat);
        options->addStretch();
        layout->addLayout(options);
        auto *search = new QLineEdit(body);
        search->setPlaceholderText(tr("Filtrer la playlist"));
        search->setAccessibleName(tr("Filtrer les fichiers de la playlist"));
        layout->addWidget(search);
        list = new QListWidget(body);
        list->setAccessibleName(tr("Playlist musicale"));
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        playlistStack = new QStackedWidget(body);
        playlistStack->addWidget(Kyronext::emptyState(
            tr("Votre playlist est vide"),
            tr("Ajoutez des fichiers audio ou importez une playlist M3U pour commencer."),
            playlistStack));
        playlistStack->addWidget(list);
        layout->addWidget(playlistStack, 1);
        auto *files = new QHBoxLayout;
        auto *add = new QPushButton(tr("Ajouter des fichiers…"), body);
        auto *remove = new QPushButton(tr("Retirer de la playlist"), body);
        files->addWidget(add);
        files->addWidget(remove);
        layout->addLayout(files);
        auto *scroll=new QScrollArea(this);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setWidget(body);setCentralWidget(scroll);
        connect(add, &QPushButton::clicked, this, [this] { addFiles(); });
        connect(remove, &QPushButton::clicked, this, [this] { removeSelected(); });
        connect(search, &QLineEdit::textChanged, this, [this](const QString &query) {
            for (int i = 0; i < list->count(); ++i)
                list->item(i)->setHidden(!list->item(i)->text().contains(query, Qt::CaseInsensitive));
        });
        connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) { playTrack(list->row(item)); });
        connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 elapsed) {
            if(dspActive)return;
            if (!position->isSliderDown() && player->duration() > 0)
                position->setValue(int(double(elapsed) * 1000.0 / double(player->duration())));
            const qint64 remaining = qMax<qint64>(0, player->duration() - elapsed);
            time->setText(tr("%1 / %2   •   reste %3").arg(Amp::displayTime(elapsed), Amp::displayTime(player->duration()), Amp::displayTime(remaining)));
        });
        connect(player, &QMediaPlayer::durationChanged, this, [this] { if(!dspActive)position->setEnabled(player->isSeekable()); });
        connect(player, &QMediaPlayer::seekableChanged, this, [this](bool value){if(!dspActive)position->setEnabled(value);});
        connect(position, &QSlider::sliderReleased, this, [this] {
            if(dspActive)processed->seek(qint64(double(position->value())*processed->duration()/1000.0));
            else player->setPosition(qint64(double(position->value()) * double(player->duration()) / 1000.0));
        });
        connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
            if(dspActive)return;
            play->setText(state == QMediaPlayer::PlayingState ? tr("Pause") : tr("Lecture"));
        });
        connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus state) {
            if (!dspActive && state == QMediaPlayer::EndOfMedia) nextTrack(false);
        });
        connect(player, &QMediaPlayer::metaDataChanged, this, [this] { updateMetadata(); });
        connect(player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &error) {
            if(dspActive)return;
            statusBar()->showMessage(tr("Lecture impossible : %1. Vérifiez le fichier et les codecs installés.").arg(error));
        });
        auto *fileMenu = menuBar()->addMenu(tr("&Fichier"));
        fileMenu->addAction(tr("&Ajouter des fichiers…"), this, [this] { addFiles(); }, QKeySequence::Open);
        fileMenu->addAction(tr("Importer une playlist M3U…"), this, [this] { importPlaylist(); });
        fileMenu->addAction(tr("Exporter la playlist M3U…"), this, [this] { exportPlaylist(); }, QKeySequence::Save);
        fileMenu->addSeparator();
        fileMenu->addAction(tr("&Quitter"), this, &QWidget::close, QKeySequence::Quit);
        auto *appearance = menuBar()->addMenu(tr("&Apparence"));
        appearance->addAction(tr("Graphite / rouge"), this, [this] { applyPalette(Kyronext::Palette::Red); });
        appearance->addAction(tr("Graphite / ambre"), this, [this] { applyPalette(Kyronext::Palette::Amber); });
        Kyronext::addAboutMenu(this, QStringLiteral("Kyronext AMP"), tr("Lecteur audio local et playlists. Version de développement 0.1.0."), QStringLiteral("Qt 6 Widgets et Multimedia — licences Qt applicables ; codecs fournis par le système. Créations Kyronext : droits réservés."));
        auto *fullscreen = new QShortcut(QKeySequence(Qt::Key_F11), this);
        connect(fullscreen,&QShortcut::activated,this,[this]{if(isFullScreen())showNormal();else showFullScreen();});
        auto *space = new QShortcut(QKeySequence(Qt::Key_Space), this);
        connect(space, &QShortcut::activated, this, [this] { if (!qobject_cast<QLineEdit *>(focusWidget())) togglePlayback(); });
        applyPalette(Kyronext::paletteFromSettings());
        updateControls();
        statusBar()->showMessage(tr("Lecture standard. Activez le DSP pour traitér le signal et afficher ses visualisations."));
    }

    void appendPaths(const QStringList &paths) {
        int ignored = 0;
        for (const QString &path : paths) {
            const QUrl url = QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath());
            if (!Amp::isReadableLocalAudio(url) || tracks.size() >= 10000) { ++ignored; continue; }
            tracks.append(url);
            auto *item = new QListWidgetItem(QFileInfo(path).fileName(), list);
            item->setToolTip(url.toLocalFile());
            playlistDirty = true;
        }
        updateControls();
        if (ignored) statusBar()->showMessage(tr("%1 fichier(s) ignoré(s) : accès impossible ou limite de 10 000 entrées.").arg(ignored));
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        if (playlistDirty && !tracks.isEmpty()) {
            const auto answer = QMessageBox::question(this, tr("Playlist modifiée"), tr("Exporter votre playlist avant de quitter ?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
            if (answer == QMessageBox::Cancel || (answer == QMessageBox::Save && !exportPlaylist())) { event->ignore(); return; }
        }
        processed->stop();
        player->stop();
        QSettings settings;
        settings.setValue(QStringLiteral("audio/volume"), volume->value());
        event->accept();
    }

private:
    void addDspControls(QVBoxLayout *layout,QWidget *body) {
        dspMode=new QCheckBox(tr("Lecture DSP : égaliseur et visualisations (recharge le morceau)"),body);
        layout->addWidget(dspMode);
        eqPanel=new QGroupBox(tr("Égaliseur 10 bandes - signal PCM réel"),body);
        auto *outer=new QVBoxLayout(eqPanel);auto *top=new QHBoxLayout;
        auto *enabled=new QCheckBox(tr("Égaliseur actif"),eqPanel);enabled->setChecked(true);top->addWidget(enabled);
        auto *preset=new QComboBox(eqPanel);preset->addItems({tr("Neutre"),tr("Rock"),tr("Pop"),tr("Jazz"),tr("Classique"),tr("Vocal"),tr("Personnalisé")});top->addWidget(preset);
        top->addWidget(new QLabel(tr("Préampli"),eqPanel));auto *pre=new QSlider(Qt::Horizontal,eqPanel);pre->setRange(-24,6);pre->setValue(0);pre->setAccessibleName(tr("Préamplification en decibels"));top->addWidget(pre);auto *preValue=new QLabel(tr("0 dB"),eqPanel);top->addWidget(preValue);outer->addLayout(top);
        processed->device.setPreamp(0);
        connect(pre,&QSlider::valueChanged,this,[this,preValue](int value){processed->device.setPreamp(value);preValue->setText(QString::number(value)+" dB");});
        connect(enabled,&QCheckBox::toggled,this,[this](bool value){processed->device.setEqEnabled(value);});
        auto *bands=new QHBoxLayout;const QStringList names={"31","63","125","250","500","1k","2k","4k","8k","16k"};
        for(int i=0;i<10;++i){auto *column=new QVBoxLayout;auto *value=new QLabel("0 dB",eqPanel);value->setAlignment(Qt::AlignCenter);auto *slider=new QSlider(Qt::Vertical,eqPanel);slider->setRange(-12,12);slider->setValue(0);slider->setFixedHeight(76);slider->setAccessibleName(names[i]+" Hz gain dB");gainSliders[i]=slider;column->addWidget(value);column->addWidget(slider,0,Qt::AlignHCenter);auto *frequency=new QLabel(names[i],eqPanel);frequency->setAlignment(Qt::AlignCenter);column->addWidget(frequency);bands->addLayout(column);connect(slider,&QSlider::valueChanged,this,[this,i,value,preset](int gain){processed->device.setBand(i,gain);value->setText(QString::number(gain)+" dB");if(!applyingPreset){QSignalBlocker block(preset);preset->setCurrentIndex(6);}});}
        outer->addLayout(bands);eqPanel->setEnabled(false);eqPanel->setVisible(false);layout->addWidget(eqPanel);
        connect(preset,qOverload<int>(&QComboBox::currentIndexChanged),this,[this,pre](int index){static constexpr int presets[6][10]={{0,0,0,0,0,0,0,0,0,0},{4,3,2,-2,-3,-2,1,3,4,4},{-1,1,3,4,2,-1,-2,-2,1,2},{3,2,1,0,-1,-1,0,1,2,3},{3,2,1,0,-1,0,1,2,3,3},{-4,-3,-2,0,2,4,4,2,0,-2}};if(index>=6)return;applyingPreset=true;for(int i=0;i<10;++i)gainSliders[i]->setValue(presets[index][i]);pre->setValue(index==0?0:-6);applyingPreset=false;});
        auto *visualOptions=new QHBoxLayout;auto *visualEnabled=new QCheckBox(tr("Visualisations"),body);const bool reducedMotion=Kyronext::reducedMotionFromSettings();visualEnabled->setChecked(!reducedMotion);visualEnabled->setEnabled(!reducedMotion);visualEnabled->setToolTip(reducedMotion?tr("Désactivées par le réglage global de réduction des effets visuels."):tr("Désactiver pour réduire l'activité graphique."));auto *mode=new QComboBox(body);mode->addItems({tr("Spectre"),tr("Oscilloscope"),tr("Vu-mètre"),tr("Scanner réactif")});visualOptions->addWidget(visualEnabled);visualOptions->addWidget(mode);visualOptions->addStretch();layout->addLayout(visualOptions);visual=new AudioVisualizer(processed,body);layout->addWidget(visual);
        connect(mode,qOverload<int>(&QComboBox::currentIndexChanged),visual,&AudioVisualizer::setMode);connect(visualEnabled,&QCheckBox::toggled,this,[this](bool value){visual->setActive(value && dspMode->isChecked());});visual->setActive(false);
        connect(dspMode,&QCheckBox::toggled,this,[this,visualEnabled](bool value){eqPanel->setEnabled(value);eqPanel->setVisible(value);visual->setActive(value && visualEnabled->isChecked());if(current>=0)playTrack(current);else statusBar()->showMessage(value?tr("Mode DSP sélectionné. Ajoutez un fichier pour commencer."):tr("Lecture standard sans égaliseur."));});
        connect(processed,&Amp::ProcessedAudio::ready,this,[this]{position->setEnabled(true);play->setText(tr("Pause"));statusBar()->showMessage(tr("DSP actif : PCM traité, limiteur de sécurité en sortie. Visualisations du signal réel."));});
        connect(processed,&Amp::ProcessedAudio::positionChanged,this,[this](qint64 elapsed){if(!dspActive)return;if(!position->isSliderDown() && processed->duration()>0)position->setValue(int(elapsed*1000/processed->duration()));time->setText(Amp::displayTime(elapsed)+" / "+Amp::displayTime(processed->duration()));if(processed->device.limitedCount()>0)statusBar()->showMessage(tr("Limiteur actif : réduisez le préampli pour éviter la saturation (%1 échantillons limites).").arg(processed->device.limitedCount()));});
        connect(processed,&Amp::ProcessedAudio::ended,this,[this]{if(dspActive)nextTrack(false);});
        connect(processed,&Amp::ProcessedAudio::failed,this,[this](const QString &error){dspActive=false;QSignalBlocker block(dspMode);dspMode->setChecked(false);eqPanel->setEnabled(false);eqPanel->setVisible(false);visual->setActive(false);processed->stop();if(current>=0){player->setSource(tracks[current]);player->play();}statusBar()->showMessage(tr("DSP indisponible : %1. Lecture standard sans égaliseur.").arg(error));});
    }
    void applyPalette(Kyronext::Palette palette) {
        Kyronext::setPreferredPalette(palette);
        Kyronext::applyTheme(*qApp, palette);
        const QString accent = palette == Kyronext::Palette::Amber ? QStringLiteral("#e9ad52") : QStringLiteral("#ef3c60");
        brand->setStyleSheet(QStringLiteral("font-size:20px;font-weight:700;color:%1").arg(accent));
        cover->setStyleSheet(QStringLiteral("background:#20242c;border:1px solid #515762;font-size:24px;color:%1").arg(accent));
    }
    void addFiles() {
        appendPaths(QFileDialog::getOpenFileNames(this, tr("Ajouter de la musique"), {}, tr("Audio (*.mp3 *.wav *.flac *.ogg *.opus *.m4a *.aac);;Tous les fichiers (*)")));
    }
    void importPlaylist() {
        const QString path = QFileDialog::getOpenFileName(this, tr("Importer une playlist"), {}, tr("Playlists M3U (*.m3u *.m3u8)"));
        if (path.isEmpty()) return;
        QString message;
        const auto loaded = Amp::loadM3u(path, &message);
        QStringList paths;
        for (const auto &url : loaded) paths.append(url.toLocalFile());
        appendPaths(paths); // Import only: never starts playback automatically.
        if (!message.isEmpty()) QMessageBox::information(this, tr("Import de playlist"), message);
    }
    bool exportPlaylist() {
        QString path = QFileDialog::getSaveFileName(this, tr("Exporter la playlist"), {}, tr("Playlist UTF-8 (*.m3u8);;Playlist M3U (*.m3u)"));
        if (path.isEmpty()) return false;
        if (QFileInfo(path).suffix().isEmpty()) path += QStringLiteral(".m3u8");
        QString error;
        if (!Amp::saveM3u(path, tracks, &error)) {
            QMessageBox::warning(this, tr("Export impossible"), error);
            return false;
        }
        playlistDirty = false;
        statusBar()->showMessage(tr("Playlist enregistrée : %1").arg(path));
        return true;
    }
    void removeSelected() {
        const int row = list->currentRow();
        if (row < 0) return;
        if (row == current) { processed->stop(); player->stop(); player->setSource({}); current = -1; }
        else if (row < current) --current;
        tracks.removeAt(row);
        delete list->takeItem(row);
        playlistDirty = true;
        updateControls();
    }
    void updateControls() {
        const bool available = !tracks.isEmpty();
        playlistStack->setCurrentIndex(available ? 1 : 0);
        play->setEnabled(available); stop->setEnabled(available);
        previous->setEnabled(available); next->setEnabled(available);
    }
    void playTrack(int index) {
        if (index < 0 || index >= tracks.size()) return;
        if (!Amp::isReadableLocalAudio(tracks[index])) {
            statusBar()->showMessage(tr("Le fichier n'est plus accessible. Il reste dans la playlist pour que vous puissiez le retrouver."));
            return;
        }
        current = index;
        list->setCurrentRow(current);
        cover->clear(); cover->setText(QStringLiteral("AMP"));
        title->setText(QFileInfo(tracks[current].toLocalFile()).fileName());
        details->setText(tr("Lecture des métadonnées…"));
        processed->stop();player->stop();
        dspActive=dspMode->isChecked();
        if(dspActive){player->setSource(tracks[current]);position->setEnabled(false);statusBar()->showMessage(tr("Décodage PCM DSP (maximum 128 Mio)..."));processed->load(tracks[current]);}
        else {player->setSource(tracks[current]);player->play();}
        statusBar()->showMessage(tracks[current].toLocalFile());
    }
    void togglePlayback() {
        if(dspActive && current>=0){if(processed->playing()){processed->pause();play->setText(tr("Lecture"));}else{processed->play();play->setText(tr("Pause"));}return;}
        if (player->playbackState() == QMediaPlayer::PlayingState) player->pause();
        else if (current >= 0 && player->source() == tracks.value(current)) player->play();
        else playTrack(list->currentRow() >= 0 ? list->currentRow() : 0);
    }
    void nextTrack(bool manual) {
        const int index = Amp::nextIndex(current, tracks.size(), repeat->isChecked() || manual, shuffle->isChecked(), QRandomGenerator::global()->generate());
        if (index >= 0) playTrack(index);
        else {processed->pause();player->stop();play->setText(tr("Lecture"));}
    }
    void previousTrack() {
        if(dspActive && processed->device.positionMs()>3000){processed->seek(0);return;}
        if (!dspActive && player->position() > 3000) { player->setPosition(0); return; }
        if (!tracks.isEmpty()) playTrack(current > 0 ? current - 1 : tracks.size() - 1);
    }
    void updateMetadata() {
        const QMediaMetaData data = player->metaData();
        const QString name = data.stringValue(QMediaMetaData::Title);
        if (!name.isEmpty()) title->setText(name);
        QStringList lines;
        const QString artist = data.stringValue(QMediaMetaData::ContributingArtist);
        const QString album = data.stringValue(QMediaMetaData::AlbumTitle);
        if (!artist.isEmpty()) lines.append(artist);
        if (!album.isEmpty()) lines.append(album);
        details->setText(lines.isEmpty() ? tr("Métadonnées non disponibles") : lines.join(QStringLiteral(" • ")));
        const QImage art = qvariant_cast<QImage>(data.value(QMediaMetaData::CoverArtImage));
        if (!art.isNull()) cover->setPixmap(QPixmap::fromImage(art).scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    Amp::ProcessedAudio *processed=nullptr;
    QCheckBox *dspMode=nullptr;
    QGroupBox *eqPanel=nullptr;
    AudioVisualizer *visual=nullptr;
    std::array<QSlider*,10> gainSliders{};
    bool dspActive=false,applyingPreset=false;
    QMediaPlayer *player = nullptr;
    QAudioOutput *output = nullptr;
    QListWidget *list = nullptr;
    QStackedWidget *playlistStack = nullptr;
    QLabel *title = nullptr, *details = nullptr, *cover = nullptr, *time = nullptr, *brand = nullptr;
    QSlider *position = nullptr, *volume = nullptr;
    QPushButton *play = nullptr, *stop = nullptr, *previous = nullptr, *next = nullptr;
    QCheckBox *mute = nullptr, *shuffle = nullptr, *repeat = nullptr;
    QList<QUrl> tracks;
    int current = -1;
    bool playlistDirty = false;
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext AMP"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    AmpWindow window;
    for (const auto &path : app.arguments().mid(1)) {
        const QString suffix = QFileInfo(path).suffix().toLower();
        if (suffix == QStringLiteral("m3u") || suffix == QStringLiteral("m3u8")) {
            QString error;
            const auto imported = Amp::loadM3u(path, &error);
            QStringList localPaths;
            for (const auto &url : imported) localPaths.append(url.toLocalFile());
            window.appendPaths(localPaths);
            if (!error.isEmpty()) QMessageBox::information(&window, QObject::tr("Import de playlist"), error);
        } else window.appendPaths({path});
    }
    window.show();
    return app.exec();
}
