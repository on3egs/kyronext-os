"""Client QMP local : état, capture PNG ou commande moniteur pour les tests."""
import json
import socket
import sys

endpoint = sys.argv[1]
if endpoint.startswith('tcp:'):
    parts = endpoint.split(':')
    if len(parts) != 3 or parts[1] != '127.0.0.1':
        raise SystemExit('QMP TCP autorisé uniquement sur tcp:127.0.0.1:PORT')
    try:
        port = int(parts[2])
        if not 1 <= port <= 65535:
            raise ValueError
    except ValueError:
        raise SystemExit('Port QMP invalide')
    sock = socket.create_connection(('127.0.0.1', port), timeout=30)
else:
    sock = socket.socket(socket.AF_UNIX)
    sock.settimeout(30)
    sock.connect(endpoint)
stream = sock.makefile('rwb', buffering=0)
json.loads(stream.readline())

def command(name, arguments=None):
    payload = {'execute': name}
    if arguments is not None:
        payload['arguments'] = arguments
    stream.write((json.dumps(payload) + '\n').encode())
    while True:
        response = json.loads(stream.readline())
        if 'error' in response:
            raise RuntimeError(response['error'])
        if 'return' in response:
            return response['return']

command('qmp_capabilities')
action = sys.argv[2]
if action == 'screenshot':
    result = command('screendump', {'filename': sys.argv[3], 'format': 'png'})
elif action == 'monitor':
    result = command('human-monitor-command', {'command-line': sys.argv[3]})
elif action == 'shutdown':
    result = command('system_powerdown')
elif action == 'quit':
    result = command('quit')
elif action == 'status':
    result = command('query-status')
else:
    raise SystemExit('Action attendue : status, screenshot, monitor, shutdown ou quit')
print(json.dumps(result, ensure_ascii=False))
