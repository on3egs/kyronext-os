# Kyronext Update — préversion de consultation

Cette application lance exclusivement `apt-get -s -o Debug::NoLocking=1 upgrade`.
Elle lit les index APT déjà présents, n'actualise pas les dépôts, n'élève pas ses
privilèges et n'installe, ne retire ni ne télécharge aucun paquet. Le résultat
brut de la simulation et les nombres d'opérations sont affichés.

Une simulation dépend de l'âge des index APT locaux. Le texte APT est conservé
afin que l'utilisateur puisse vérifier les versions et dépôts affichés. Les
mises à jour restent à appliquer avec les mécanismes Debian/KDE maintenus ;
cette version n'intègre aucune fonction d'installation.

## Compilation et tests

```bash
cmake -S applications/suite -B build/suite -G Ninja -DBUILD_TESTING=ON
cmake --build build/suite --target kyronext-update test-kyronext-update --parallel 1
ctest --test-dir build/suite -R kyronext-update-simulation-parser --output-on-failure
```

L'application n'est pas encore empaquetée ni intégrée à l'ISO 0.1.
