#!/bin/bash
# ==============================================
# Teste automático - Mini Web Server (Etapa 3)
# ==============================================

PORT=9000
BIN=./program
WWW_DIR=www
LOG_FILE=log.txt

# Função de limpeza
cleanup() {
  echo ""
  echo "[CLEANUP] Encerrando servidor..."
  pkill -f "$BIN server" >/dev/null 2>&1
}
trap cleanup EXIT

# Garante build
echo "[BUILD] Compilando..."
make -s

# Cria diretório www e conteúdo de teste
mkdir -p $WWW_DIR
echo "<html><body><h1>Index OK</h1></body></html>" > $WWW_DIR/index.html
echo "<html><body><h1>About OK</h1></body></html>" > $WWW_DIR/about.html

# Inicia servidor em background protegido contra SIGHUP
echo "[SERVER] Iniciando servidor na porta $PORT..."
nohup $BIN server $PORT $WWW_DIR 50 > server.log 2>&1 &
SERVER_PID=$!
sleep 1

# Confirma que está rodando
if ps -p $SERVER_PID > /dev/null; then
  echo "[SERVER OK] PID=$SERVER_PID"
else
  echo "[ERRO] Servidor não iniciou corretamente."
  echo "--- LOG DE INICIALIZAÇÃO ---"
  cat server.log || true
  exit 1
fi

# Executa múltiplos clientes concorrentes
echo "[TEST] Enviando 10 requisições HTTP concorrentes..."
for i in $(seq 1 10); do
  (
    $BIN client 127.0.0.1 $PORT /index.html > /dev/null 2>&1
    $BIN client 127.0.0.1 $PORT /about.html > /dev/null 2>&1
    $BIN client 127.0.0.1 $PORT /notfound.html > /dev/null 2>&1
  ) &
done

wait

# Aguarda escrita de logs
sleep 1

# Para servidor
echo "[STOP] Parando servidor..."
echo "stop" | $BIN server $PORT >/dev/null 2>&1 || pkill -f "$BIN server" >/dev/null 2>&1
sleep 1

# Verifica logs
if [ -f "$LOG_FILE" ]; then
  echo "[LOG] Exibindo últimas linhas do log:"
  tail -n 10 "$LOG_FILE"
else
  echo "[ERRO] Arquivo de log não encontrado!"
fi

echo "[DONE] Teste finalizado com sucesso."
