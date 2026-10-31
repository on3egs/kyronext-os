# Kyronext Connect — première étape locale

`kyronext-connect-inventory` décrit uniquement la machine courante : noyau,
architecture, modèle disponible et interfaces locales. Il n'émet aucune requête
de découverte, ne parcourt aucun réseau et n'ouvre aucun service d'administration.

```bash
python3 /usr/local/bin/kyronext-connect-inventory
python3 /usr/local/bin/kyronext-connect-inventory --output mon-inventaire.json
```

L'export crée un nouveau fichier privé ; un fichier existant n'est pas écrasé.
Le nom de la machine et ses adresses peuvent être personnels : relire le rapport
avant de le partager. Aucun numéro de série, mot de passe ou secret n'est collecté.
La compatibilité reste `unverified` : identifier un Jetson ne le rend pas compatible.

Étapes futures : consentement explicite de découverte, appairage authentifié,
identités stables, services annoncés et autorisations par action. La présence
sur le réseau local ne constituera pas une preuve de confiance. Aucun modèle
IA ni privilège administrateur permanent ne sera nécessaire aux fonctions locales.
