# Sécurité initiale

Les mots de passe root et Live sont verrouillés. L'autologin graphique ouvre uniquement le compte kyronext ; les consoles ne s'ouvrent pas automatiquement. La règle sudo sans mot de passe du Live est désactivée. Polkit conserve les actions ordinaires du bureau, à valider au démarrage.

Aucun service d'administration distant n'est ajouté au Live, aucune télémétrie imposée et aucun moteur IA privilégié. APT vérifie les signatures Debian. Les futurs changements sensibles devront être autorisés et réversibles. Chiffrement, récupération et Secure Boot ne sont pas intégrés ou validés dans cette préversion.
