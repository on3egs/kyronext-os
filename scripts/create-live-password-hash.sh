#!/bin/bash
set -euo pipefail
umask 077

cd -- "$(dirname -- "$0")/.."
output=${1:-build/live-password.hash}
case "$output" in
	/*) ;;
	*) output="$PWD/$output" ;;
esac
[[ ! -e "$output" && ! -L "$output" ]] || { echo 'Le fichier existe déjà ; refus de l’écraser.' >&2; exit 1; }
command -v openssl >/dev/null || { echo 'openssl est requis.' >&2; exit 1; }

IFS= read -r -s -p 'Mot de passe Live : ' password
printf '\n' >&2
IFS= read -r -s -p 'Confirmer : ' confirmation
printf '\n' >&2
[[ -n "$password" && "$password" == "$confirmation" ]] || { unset password confirmation; echo 'Saisie vide ou différente.' >&2; exit 1; }
hash=$(printf '%s' "$password" | openssl passwd -6 -stdin)
unset password confirmation
[[ $hash =~ ^\$6\$(rounds=[0-9]+\$)?[A-Za-z0-9./]{1,16}\$[A-Za-z0-9./]{86}$ ]] || { unset hash; echo 'OpenSSL a produit un hash inattendu.' >&2; exit 1; }
mkdir -p -- "$(dirname -- "$output")"
set -o noclobber
printf '%s\n' "$hash" > "$output"
set +o noclobber
unset hash
chmod 0600 -- "$output"
printf 'Hash privé créé dans %s (le mot de passe et le hash ne sont pas affichés).\n' "$output"
