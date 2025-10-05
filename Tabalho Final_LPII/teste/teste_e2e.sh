#!/usr/bin/env bash
set -euo pipefail

HOST=${HOST:-127.0.0.1}
PORT=${PORT:-9000}
BIN_SERVER=./bin/server_cli
BIN_CLIENT=./bin/cliente_cli
RUN_BOTS=./exemplos/run_clients.sh
LOG_SERVER=logs/server.log
LOG_CLIENT=logs/client.log

msg(){ echo -e "[-] $*"; }
ok(){ echo -e "[OK] $*"; }
die(){ echo -e "[ERRO] $*" >&2; exit 1; }

require() { command -v "$1" >/dev/null 2>&1 || die "ferramenta requerida não encontrada: $1"; }

wait_listen() {
  local port="$1" timeout="${2:-10}" i=0
  while (( i < timeout*10 )); do
    if ss -tnlp 2>/dev/null | grep -q ":$port " || netstat -tnlp 2>/dev/null | grep -q ":$port "; then
      return 0
    fi
    sleep 0.1; ((i++))
  done
  return 1
}

start_server() {
  mkdir -p logs bin
  msg "iniciando servidor na porta $PORT…"
  "$BIN_SERVER" "$PORT" >/dev/null 2>&1 &
  SRV_PID=$!
  disown || true
  wait_listen "$PORT" 10 || die "servidor não ficou em LISTEN na porta $PORT"
  ok "servidor em LISTEN (pid=$SRV_PID)"
}

stop_server() {
  if ps -p "${SRV_PID:-}" >/dev/null 2>&1; then
    msg "parando servidor (pid=$SRV_PID)…"
    kill -TERM "$SRV_PID" 2>/dev/null || true
    sleep 0.5
    kill -KILL "$SRV_PID" 2>/dev/null || true
  fi
}

assert_grep() {
  local pat="$1" file="$2"
  grep -q -- "$pat" "$file" || die "não encontrou '$pat' em $file"
}

assert_count_ge() {
  local pat="$1" file="$2" min="$3"
  local c
  c=$(grep -c -- "$pat" "$file" || true)
  (( c >= min )) || die "esperava >= $min ocorrências de '$pat' em $file (achou $c)"
  ok "$file: '$pat' >= $min (achou $c)"
}

wait_for_count_in_log() {
  local pat="$1" file="$2" min="$3" timeout="${4:-10}"
  local i=0
  while (( i < timeout*10 )); do
    local c
    c=$(grep -c -- "$pat" "$file" 2>/dev/null || echo 0)
    if (( c >= min )); then
      return 0
    fi
    sleep 0.1
    ((i++))
  done
  return 1
}

broadcast_check() {
  msg "teste de broadcast…"
  local fifoA=/tmp/in_cli_A.$$ outA=/tmp/out_cli_A.$$
  mkfifo "$fifoA"
  : > "$outA"

  ( stdbuf -oL cat "$fifoA" | "$BIN_CLIENT" "$HOST" "$PORT" "A" >"$outA" 2>&1 ) &
  local A_PID=$!
  sleep 0.5

  local PING="PING_$(date +%s)"
  echo "$PING" | "$BIN_CLIENT" "$HOST" "$PORT" "B" >/dev/null 2>&1 || die "cliente B não conseguiu enviar"

  sleep 0.5
  echo "/quit" > "$fifoA" || true
  sleep 0.5
  rm -f "$fifoA"

  grep -q -- "$PING" "$outA" || { echo "----- stdout do cliente A -----"; cat "$outA"; echo "--------------------------------"; die "cliente A não recebeu broadcast"; }
  ok "broadcast chegou no cliente A"
  rm -f "$outA"
}

trap 'stop_server' EXIT

require ss || require netstat
[[ -x $BIN_SERVER && -x $BIN_CLIENT ]] || die "compile antes: make all"

rm -f "$LOG_SERVER" "$LOG_CLIENT"

start_server

N=${N:-5}
msg "rodando $N clientes automáticos…"
chmod +x "$RUN_BOTS"
HOST="$HOST" PORT="$PORT" BIN="$BIN_CLIENT" "$RUN_BOTS" "$N"

if ! wait_for_count_in_log "client connected from" "$LOG_SERVER" "$N" 10; then
  echo "---- conteudo atual de $LOG_SERVER (últimas 200 linhas) ----"
  tail -n 200 "$LOG_SERVER" || true
  echo "---- conteudo atual de $LOG_CLIENT (últimas 200 linhas) ----"
  tail -n 200 "$LOG_CLIENT" || true
  die "timeout aguardando $N conexões no servidor (checando logs)"
fi
ok "logs do servidor registraram >= $N conexões"

assert_grep "server: listening on port $PORT" "$LOG_SERVER"
assert_count_ge "client connected from" "$LOG_SERVER" "$N"

if ! wait_for_count_in_log "server: recv line" "$LOG_SERVER" "$((3 * N))" 10; then
  echo "---- conteudo atual de $LOG_SERVER (últimas 200 linhas) ----"
  tail -n 200 "$LOG_SERVER" || true
  die "timeout aguardando $(($3 * N)) recv lines no servidor"
fi
assert_count_ge "server: recv line" "$LOG_SERVER" "$(( 3 * N ))"

broadcast_check

ok "todos os testes passaram"
