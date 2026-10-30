// Kyronext Systems — By Manix. API publique Plasma 6, composants KDE standards.
// Script versionné : les profils ayant déjà exécuté kyronext.js le recevront aussi.
(function () {
    var state = new ConfigFile("kyronext-desktoprc", "Branding");
    if (Number(state.readEntry("LayoutVersion") || 0) >= 2) {
        return;
    }

    var wallpaper = "file:///usr/share/wallpapers/Kyronext/contents/images/1920x1080.svg";
    var allDesktops = desktops();
    for (var i = 0; i < allDesktops.length; i++) {
        allDesktops[i].wallpaperPlugin = "org.kde.image";
        allDesktops[i].currentConfigGroup = ["Wallpaper", "org.kde.image", "General"];
        allDesktops[i].writeConfig("Image", wallpaper);
    }

    // Réutiliser le panneau principal ; ne supprimer aucun autre panneau utilisateur.
    var allPanels = panels();
    var panel = null;
    for (var p = 0; p < allPanels.length; p++) {
        if (allPanels[p].location === "bottom" && allPanels[p].screen <= 0) {
            panel = allPanels[p];
            break;
        }
    }
    if (!panel && allPanels.length > 0) {
        panel = allPanels[0];
    }
    if (!panel) {
        panel = new Panel;
        panel.screen = 0;
    }

    panel.location = "bottom";
    panel.alignment = "left";
    panel.lengthMode = "fill";
    panel.offset = 0;
    panel.hiding = "none";
    panel.floating = false;
    panel.opacity = "opaque";
    var geometry = screenGeometry(Math.max(0, panel.screen));
    panel.height = Math.max(42, Math.min(56, Math.round(geometry.height * 0.06)));

    // Retirer les raccourcis du profil Debian qui désignent des applications absentes.
    var oldWidgets = panel.widgets();
    for (var w = 0; w < oldWidgets.length; w++) {
        oldWidgets[w].remove();
    }

    var favorites = [];
    var launchers = [];
    var applications = ["org.kde.dolphin.desktop", "org.kde.konsole.desktop", "systemsettings.desktop"];
    for (var a = 0; a < applications.length; a++) {
        if (applicationExists(applications[a])) {
            favorites.push(applications[a]);
            launchers.push("applications:" + applications[a]);
        }
    }

    var menu = panel.addWidget("org.kde.plasma.kickoff");
    menu.currentConfigGroup = ["General"];
    menu.writeConfig("icon", "kyronext-logo");
    menu.writeConfig("menuLabel", "Kyronext");
    menu.writeConfig("appNameFormat", 0);
    menu.writeConfig("favorites", favorites);
    menu.writeConfig("systemApplications", favorites);
    menu.writeConfig("systemFavorites", ["reboot", "shutdown"]);
    menu.writeConfig("primaryActions", 2);
    menu.reloadConfig();

    var tasks = panel.addWidget("org.kde.plasma.icontasks");
    tasks.currentConfigGroup = ["General"];
    tasks.writeConfig("launchers", launchers);
    tasks.writeConfig("showOnlyCurrentScreen", true);
    tasks.writeConfig("maxStripes", 1);
    tasks.writeConfig("showToolTips", true);
    tasks.writeConfig("wheelEnabled", false);
    tasks.reloadConfig();

    panel.addWidget("org.kde.plasma.systemtray");
    var clock = panel.addWidget("org.kde.plasma.digitalclock");
    clock.currentConfigGroup = ["Appearance"];
    clock.writeConfig("showDate", true);
    clock.writeConfig("dateFormat", "shortDate");
    clock.writeConfig("showSeconds", 0);
    clock.writeConfig("use24hFormat", 2);
    clock.reloadConfig();

    // Ne marquer la migration qu'après la création de tous les éléments.
    state.writeEntry("LayoutVersion", 2);
    state.writeEntry("Edition", "Kyronext OS 0.1 — By Manix");
})();
