// Fond de base. Le panneau versionné est configuré par kyronext-visual-02.js.
var screens = desktops();
for (var i = 0; i < screens.length; i++) {
    screens[i].wallpaperPlugin = "org.kde.image";
    screens[i].currentConfigGroup = ["Wallpaper", "org.kde.image", "General"];
    screens[i].writeConfig("Image", "file:///usr/share/wallpapers/Kyronext/contents/images/1920x1080.svg");
}
