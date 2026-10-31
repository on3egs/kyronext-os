"""Contrôle HTTPS explicite, jamais exécuté automatiquement par les applications."""
import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import importlib.util
import json
from pathlib import Path
import ssl
from urllib.error import HTTPError, URLError
from urllib.request import build_opener, HTTPRedirectHandler, HTTPSHandler, Request

ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / 'iso/config/includes.chroot/usr/lib/kyronext/kyronext_links.py'
spec = importlib.util.spec_from_file_location('kyronext_links', MODULE)
links_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(links_module)


def inspect(link):
    checked = datetime.now(timezone.utc).isoformat(timespec='seconds')
    result = {'id': link['id'], 'url': link['url'], 'checked_at': checked,
              'status': 'to-confirm', 'timeout_seconds': 8,
              'certificate_policy': 'ssl.create_default_context; check_hostname=True; CERT_REQUIRED',
              'tls_verified': None, 'redirects': []}

    class SecureRedirects(HTTPRedirectHandler):
        def redirect_request(self, req, fp, code, msg, headers, newurl):
            links_module.validate_url(newurl)
            result['redirects'].append({'status': code, 'url': newurl})
            return super().redirect_request(req, fp, code, msg, headers, newurl)

    try:
        opener = build_opener(HTTPSHandler(context=ssl.create_default_context()), SecureRedirects())
        request = Request(link['url'], headers={'User-Agent': 'Kyronext-Link-Audit/0.1'})
        with opener.open(request, timeout=8) as response:
            result['http_status'] = response.status
            result['final_url'] = response.url
            result['tls_verified'] = True
            response.read(1)
            result['status'] = 'verified'
            result['detail'] = 'Réponse HTTPS reçue avec validation standard du certificat.'
    except HTTPError as error:
        result['http_status'] = error.code
        result['final_url'] = error.url
        result['tls_verified'] = True
        result['status'] = 'to-confirm' if error.code in (401, 403, 429) else 'error'
        result['detail'] = 'Réponse HTTP ' + str(error.code) + ' ; contenu non confirmé.'
    except (URLError, OSError, ssl.SSLError, links_module.LinkConfigError) as error:
        result['detail'] = type(error).__name__ + ': ' + str(error)[:350]
        if isinstance(error, links_module.LinkConfigError) or isinstance(getattr(error, 'reason', error), ssl.SSLCertVerificationError):
            result['status'] = 'error'
        result['scope'] = 'Échec depuis cet environnement ; ne prouve pas une indisponibilité mondiale.'
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--update-config', action='store_true', help='Reporter ce contrôle daté dans links.json')
    args = parser.parse_args()
    config = json.loads(links_module.SYSTEM_CONFIG.read_text(encoding='utf-8'))
    with ThreadPoolExecutor(max_workers=3) as pool:
        results = list(pool.map(inspect, config['links']))
    log = ROOT / 'logs/links-domain-validation.json'
    log.parent.mkdir(exist_ok=True)
    log.write_text(json.dumps(results, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    status_labels = {'verified': 'Vérifié', 'error': 'Erreur', 'to-confirm': 'À confirmer'}
    lines = ['# Contrôle explicite des domaines Kyronext', '',
             'Les adresses demandées sont conservées sans substitution. Les redirections',
             'sont relevées, jamais substituées à la configuration. TLS utilise la validation',
             'standard de Python et du système ; aucune erreur de certificat n’est ignorée.', '',
             '| Adresse fournie | État | Date UTC | Résultat |',
             '|---|---|---|---|']
    for result in results:
        detail = result['detail'].replace('|', '/').replace('\n', ' ')
        lines.append(f"| {result['url']} | {status_labels[result['status']]} | {result['checked_at']} | {detail} |")
    for result in results:
        if result.get('redirects'):
            lines.append('')
            lines.append('Redirections relevées pour ' + result['url'] + ' : ' + ', '.join(item['url'] for item in result['redirects']))
    lines += ['', 'Un contrôle réussi ne garantit pas l’accessibilité future ni le contenu du site.',
              'Un échec depuis ce poste ne prouve pas une indisponibilité générale.',
              'Rapport détaillé : `logs/links-domain-validation.json`.',
              'Ce script est une opération de développement explicite ; aucun contrôle réseau',
              'n’est exécuté au démarrage de l’accueil, des applications ou du lanceur.']
    (Path(__file__).parent / 'DOMAIN_REPORT.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    if args.update_config:
        by_id = {item['id']: item for item in results}
        for link in config['links']:
            result = by_id[link['id']]
            link['verification'] = {key: result[key] for key in ('status', 'checked_at', 'detail')}
            if 'final_url' in result:
                link['verification']['observed_final_url'] = result['final_url']
        links_module.SYSTEM_CONFIG.write_text(json.dumps(config, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(json.dumps(results, indent=2, ensure_ascii=False))


if __name__ == '__main__':
    main()
