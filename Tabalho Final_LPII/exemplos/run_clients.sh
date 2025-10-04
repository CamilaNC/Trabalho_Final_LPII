#!/usr/bin/env bash
# Uso: ./exemplos/run_clients.sh 5
set -euo pipefail

N=${1:-3}
HOST=${HOST:-127.0.0.1}
PORT=${PORT:-9000}
BIN=${BIN:-./bin/cliente_cli}

for i in $(seq 1 "$N"); do
  (
    NICK="bot_$i"
    MSG="Olá de $NICK at $(date +%H:%M:%S)"
    sleep 0.$((RANDOM % 9))
    {
      echo "nick $NICK"
      echo "$MSG"
      echo "tchau de $NICK"
    } | "$BIN" "$HOST" "$PORT" >/dev/null
  ) &
done

wait
echo "Enviadas $N mensagens automatizadas."
