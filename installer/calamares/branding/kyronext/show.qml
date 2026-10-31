import QtQuick 2.0;
import calamares.slideshow 1.0;

Presentation {
    id: presentation

    Timer {
        interval: 12000
        running: presentation.activatedInCalamares
        repeat: true
        onTriggered: presentation.goToNextSlide()
    }

    Slide {
        Rectangle { anchors.fill: parent; color: "#111720" }
        Image {
            x: 0; y: 0; width: parent.width * 0.48; height: parent.height
            source: "desktop.png"
            fillMode: Image.PreserveAspectCrop
            smooth: true
        }
        Rectangle {
            x: parent.width * 0.38; y: 0
            width: parent.width * 0.62; height: parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00111720" }
                GradientStop { position: 0.25; color: "#f6111720" }
                GradientStop { position: 1.0; color: "#ff111720" }
            }
        }
        Column {
            x: parent.width * 0.52; width: parent.width * 0.42
            anchors.verticalCenter: parent.verticalCenter
            spacing: 18
            Text { text: "KYRONEXT OS  /  BIENVENUE"; color: "#ef3d59"; font.pixelSize: 13; font.bold: true }
            Text { text: "Une nouvelle aventure commence"; color: "#f5f7fb"; font.pixelSize: 25; font.bold: true; wrapMode: Text.WordWrap; width: parent.width }
            Text { text: "Merci de découvrir Kyronext OS. Pendant l’installation, quelques images et histoires vous feront visiter le projet."; color: "#c7cfdb"; font.pixelSize: 16; wrapMode: Text.WordWrap; width: parent.width; lineHeight: 1.25 }
        }
    }

    Slide {
        Rectangle { anchors.fill: parent; color: "#111720" }
        Image {
            x: 0; y: 0; width: parent.width * 0.48; height: parent.height
            source: "applications.png"
            fillMode: Image.PreserveAspectCrop
            smooth: true
        }
        Rectangle {
            x: parent.width * 0.38; y: 0
            width: parent.width * 0.62; height: parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00111720" }
                GradientStop { position: 0.25; color: "#f6111720" }
                GradientStop { position: 1.0; color: "#ff111720" }
            }
        }
        Column {
            x: parent.width * 0.52; width: parent.width * 0.42
            anchors.verticalCenter: parent.verticalCenter
            spacing: 18
            Text { text: "DES OUTILS QUI PRENNENT FORME"; color: "#ef3d59"; font.pixelSize: 13; font.bold: true }
            Text { text: "Voici AMP"; color: "#f5f7fb"; font.pixelSize: 25; font.bold: true }
            Text { text: "AMP est le lecteur audio de Kyronext. Cette capture montre son interface réelle dans la version de développement."; color: "#c7cfdb"; font.pixelSize: 16; wrapMode: Text.WordWrap; width: parent.width; lineHeight: 1.25 }
        }
    }

    Slide {
        Rectangle { anchors.fill: parent; color: "#111720" }
        Image {
            x: 0; y: 0; width: parent.width * 0.48; height: parent.height
            source: "desktop.png"
            fillMode: Image.PreserveAspectCrop
            smooth: true
        }
        Rectangle {
            x: parent.width * 0.38; y: 0
            width: parent.width * 0.62; height: parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00111720" }
                GradientStop { position: 0.25; color: "#f6111720" }
                GradientStop { position: 1.0; color: "#ff111720" }
            }
        }
        Column {
            x: parent.width * 0.52; width: parent.width * 0.42
            anchors.verticalCenter: parent.verticalCenter
            spacing: 18
            Text { text: "VOTRE BUREAU"; color: "#ef3d59"; font.pixelSize: 13; font.bold: true }
            Text { text: "Un espace à vous"; color: "#f5f7fb"; font.pixelSize: 25; font.bold: true }
            Text { text: "Kyronext s’appuie sur Debian et KDE Plasma. Le bureau, les raccourcis et les outils évoluent au fil du projet."; color: "#c7cfdb"; font.pixelSize: 16; wrapMode: Text.WordWrap; width: parent.width; lineHeight: 1.25 }
        }
    }

    Slide {
        Rectangle { anchors.fill: parent; color: "#111720" }
        Image {
            x: 0; y: 0; width: parent.width * 0.48; height: parent.height
            source: "kyronext-logo.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        Rectangle {
            x: parent.width * 0.38; y: 0
            width: parent.width * 0.62; height: parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00111720" }
                GradientStop { position: 0.25; color: "#f6111720" }
                GradientStop { position: 1.0; color: "#ff111720" }
            }
        }
        Column {
            x: parent.width * 0.52; width: parent.width * 0.42
            anchors.verticalCenter: parent.verticalCenter
            spacing: 18
            Text { text: "UNE HISTOIRE PERSONNELLE"; color: "#ef3d59"; font.pixelSize: 13; font.bold: true }
            Text { text: "Un rêve, du CPC à Kyronext"; color: "#f5f7fb"; font.pixelSize: 25; font.bold: true; wrapMode: Text.WordWrap; width: parent.width }
            Text { text: "Je code depuis mes 7 ans : j’ai appris le BASIC sur mon CPC 464, puis continué sur un IBM 8081. Fan de K 2000, j’ai porté ce projet seul pendant longtemps, en y consacrant beaucoup de temps et d’argent. Derrière chaque version, il y a des heures d’essais, de corrections et l’envie de partager un rêve. Merci de m’accompagner dans cette aventure. — Manix"; color: "#c7cfdb"; font.pixelSize: 16; wrapMode: Text.WordWrap; width: parent.width; lineHeight: 1.25 }
        }
    }

    Slide {
        Rectangle { anchors.fill: parent; color: "#111720" }
        Image {
            x: 0; y: 0; width: parent.width * 0.48; height: parent.height
            source: "kyronext-logo.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        Rectangle {
            x: parent.width * 0.38; y: 0
            width: parent.width * 0.62; height: parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00111720" }
                GradientStop { position: 0.25; color: "#f6111720" }
                GradientStop { position: 1.0; color: "#ff111720" }
            }
        }
        Column {
            x: parent.width * 0.52; width: parent.width * 0.42
            anchors.verticalCenter: parent.verticalCenter
            spacing: 18
            Text { text: "GRATUIT, ET PARTAGÉ AVEC VOUS"; color: "#ef3d59"; font.pixelSize: 13; font.bold: true; wrapMode: Text.WordWrap; width: parent.width }
            Text { text: "Merci d’être là"; color: "#f5f7fb"; font.pixelSize: 25; font.bold: true }
            Text { text: "Kyronext OS est gratuit. Si vous souhaitez soutenir la suite du projet, une contribution restera entièrement facultative. Le lien sera ajouté lorsqu’il sera prêt. Merci pour votre compréhension. — Manix"; color: "#c7cfdb"; font.pixelSize: 16; wrapMode: Text.WordWrap; width: parent.width; lineHeight: 1.25 }
        }
    }

    function onActivate() { presentation.currentSlide = 0; }
    function onLeave() { }
}
