# Identité du démarrage Kyronext

Le master `kyronext-boot.svg` est une création originale graphite, blanche et
rouge. Il ne reprend aucun logo, image, son ou marque graphique de série.
Le fond ne contient aucune donnée de noyau ou de fabrication à mettre à jour.

Depuis Debian, à la racine du projet :

```bash
bash branding/boot/render.sh
```

Le rendu utilise `rsvg-convert`, fourni par le paquet officiel Debian
`librsvg2-bin`. Il produit deux PNG dans
`iso/config/includes.binary/boot/kyronext/` : `boot-grub.png` en 800×600 et
`boot-bios.png` en 640×480. Ces fichiers rendus sont livrés avec les sources
pour que le hook de fabrication n'ait pas besoin d'installer un moteur SVG.

Le hook `0950-boot-branding.hook.binary` doit être exécuté depuis la racine
d'un arbre ISO déjà généré, après copie de `includes.binary`. Il remplace les
fonds `boot/grub/splash.png` et `isolinux/splash.png`, le thème GRUB et la
présentation Syslinux. Cette interface convient à live-build et au script
de remaster. Le hook échoue clairement si ses PNG sont absents.

Les entrées Live et de secours sont conservées. Les chemins `linux` et
`initrd`, leurs commandes et l'image EFI restent inchangés. Les drapeaux de
diagnostic des entrées de secours restent présents. Le hook ajoute aux
lignes Live les seules options communes absentes : utilisateur/nom d'hôte
Kyronext, locale/clavier français, absence d'autologin console et exclusion
du composant sudo de live-config. Cela corrige l'absence de ces options dans
l'entrée de secours générée par Debian ; aucun contournement de permissions
n'est ajouté. Le mode de secours reste à valider graphiquement séparément.

Validation nécessaire : observer séparément les menus BIOS et UEFI, leur
sélection, puis le démarrage du Live. Une validation syntaxique ne valide
pas les dimensions du rendu, les polices ou le démarrage réel.

Le placement Syslinux tient compte du décalage vertical appliqué aussi aux
lignes d'aide : les positions relatives restent dans les 30 lignes de la
sortie VGA 640×480. Voir les [sources Syslinux](https://kernel.googlesource.com/pub/scm/boot/syslinux/syslinux/+/master/doc/menu.txt).

Contrôle des paramètres et de l'idempotence :

```bash
python3 branding/boot/check-hook.py
```
