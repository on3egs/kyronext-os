# Accueil Kyronext

Petite application PyQt6 native, sans animation imposée ni service privilégié.
Elle propose uniquement des outils réellement inclus dans le Live : Dolphin,
Konsole, Paramètres KDE et diagnostic matériel local dans Konsole.

Le lanceur est `iso/config/includes.chroot/usr/local/bin/kyronext-welcome`.
Les icônes vectorielles originales sont dans
`iso/config/includes.chroot/usr/share/kyronext/welcome/`.

- L'autostart appelle `kyronext-welcome --autostart` : l'accueil apparaît une
  fois par session Live, avec état local dans `$XDG_STATE_HOME/kyronext/`.
- Le menu « Accueil Kyronext » appelle `kyronext-welcome` sans option :
  l'utilisateur peut rouvrir l'accueil même après l'avoir fermé.
- Les programmes sont lancés avec une liste d'arguments fixe, sans shell,
  téléchargement, commande administrateur ou modification des pilotes.
- Les actions indisponibles donnent une erreur française locale. Les dimensions
  s'adaptent à l'espace écran ; une zone défilante conserve les actions accessibles
  sur 800×600 et sur les affichages à mise à l'échelle élevée.

Les composants Qt proviennent du paquet Debian `python3-pyqt6` déjà retenu.
Aucune nouvelle dépendance Python n'est ajoutée. Les fonctionnalités Guardian
avancées, le Cockpit et l'installation sur disque ne sont pas présentés comme
opérationnels par cet accueil.
