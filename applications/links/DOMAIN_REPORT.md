# Contrôle explicite des domaines Kyronext

Les adresses demandées sont conservées sans substitution. Les redirections
sont relevées, jamais substituées à la configuration. TLS utilise la validation
standard de Python et du système ; aucune erreur de certificat n’est ignorée.

| Adresse fournie | État | Date UTC | Résultat |
|---|---|---|---|
| https://www.kitt-franco-belge.be | Vérifié | 2026-10-31T14:17:41+00:00 | Réponse HTTPS reçue avec validation standard du certificat. |
| https://kironext.be | À confirmer | 2026-10-31T14:17:41+00:00 | URLError: <urlopen error [Errno 11001] getaddrinfo failed> |
| https://kyronext.eu | Vérifié | 2026-10-31T14:17:41+00:00 | Réponse HTTPS reçue avec validation standard du certificat. |

Redirections relevées pour https://www.kitt-franco-belge.be : https://kitt-franco-belge.be/

Un contrôle réussi ne garantit pas l’accessibilité future ni le contenu du site.
Un échec depuis ce poste ne prouve pas une indisponibilité générale.
Rapport détaillé : `logs/links-domain-validation.json`.
Ce script est une opération de développement explicite ; aucun contrôle réseau
n’est exécuté au démarrage de l’accueil, des applications ou du lanceur.
