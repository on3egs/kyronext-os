# Modules et évolutions

Les sources de fabrication sont dans iso/ ; build/, logs/ et releases/ sont
des sorties locales exclues du dépôt. Les scripts n'agissent que sur des
fichiers et dans une VM Debian dédiée. Aucun disque physique n'est attaché.

La première version conserve les applications KDE : Dolphin pour Files,
System Settings pour Settings et Konsole pour le terminal. Les noms Center,
Software, Update, Security et Studio décrivent des modules futurs, sans
application indépendante opérationnelle à ce stade.

L'accueil initial est une petite application Python/PyQt6 sans privilèges.
Un fond SVG original et une mise à jour de configuration Plasma conservent
la disposition KDE standard. Aucun remplacement du shell n'est prévu en 0.1.

L'IA future sera désactivable, sans effet sur le fonctionnement du bureau.
L'interface utilisateur communiquera avec un moteur local non privilégié.
Les interfaces de texte, voix et mémoire locale seront séparées ; aucun
modèle n'est embarqué ni téléchargé ici. Une demande d'action système sera
distincte d'une réponse conversationnelle et nécessitera une autorisation
explicite par une couche de permissions dédiée avant toute exécution.
Ollama, Vosk, Faster-Whisper et Piper restent des candidats à évaluer,
pas des dépendances installées ni des interfaces déjà opérationnelles.

Calamares sera expérimenté après validation du Live, uniquement avec un
disque virtuel jetable. Chiffrement, récupération et mises à jour signées
nécessitent leur propre conception et validation. Les mécanismes APT de
Debian sont conservés ; aucune infrastructure de mise à jour Kyronext
indépendante n'est annoncée.
