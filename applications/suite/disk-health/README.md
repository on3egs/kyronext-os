# Kyronext Disk Health

Petit outil local de diagnostic SMART, disponible hors ligne et en lecture seule.
Il énumère des périphériques SATA/SCSI, NVMe, MMC et VirtIO visibles dans
`/sys/block`, puis appelle `smartctl -j -H` avec des arguments fixes. Il ne
utilise ni shell, ni `sudo`, ni commande d'écriture. Le rapport ne montre pas les
numéros de série.

`smartmontools` est facultatif et n'est pas imposé par le paquet Kyronext. En
son absence, l'application explique le manque. Selon le contrôleur et les règles
d'accès de la session, SMART peut rester inaccessible sans privilège ; dans ce
cas l'état demeure « Inconnu ». Un résultat vert ne garantit pas qu'un disque ne
tombera pas en panne : conservez des sauvegardes.

Compilation dans la suite :

```bash
cmake -S applications/suite -B build/suite -G Ninja
cmake --build build/suite --target kyronext-disk-health test-disk-health
ctest --test-dir build/suite -R disk-health --output-on-failure
```

L'outil n'est pas encore intégré à l'ISO et n'a pas été essayé sur un disque
physique Kyronext.
