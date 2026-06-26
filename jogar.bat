@echo off
REM ============================================================
REM  jogar.bat - Compila e executa "Caminho do Conhecimento"
REM  Basta dar um duplo-clique neste arquivo.
REM ============================================================
setlocal
cd /d "%~dp0"

echo ============================================
echo   CAMINHO DO CONHECIMENTO
echo   Compilando...
echo ============================================
echo.

REM ---- Garante que o gcc esta disponivel ----
where gcc >nul 2>nul
if errorlevel 1 (
    if exist "C:\msys64\ucrt64\bin\gcc.exe" (
        set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    ) else (
        echo [ERRO] gcc nao encontrado no PATH.
        echo        Instale o MSYS2/MinGW e adicione ao PATH,
        echo        ou ajuste o caminho dentro deste .bat.
        echo.
        pause
        exit /b 1
    )
)

REM ---- Garante que a pasta de saida existe ----
if not exist "output" mkdir "output"

REM ---- Compila jogo.c para output\jogo.exe ----
gcc -Wall -Wextra -o "output\jogo.exe" "jogo.c"
if errorlevel 1 (
    echo.
    echo [ERRO] Falha na compilacao. Veja as mensagens acima.
    echo.
    pause
    exit /b 1
)

echo Compilacao concluida com sucesso!
echo.
echo ============================================
echo   Iniciando o jogo...
echo ============================================
echo.

REM ---- Executa a partir de output\ para que os arquivos ----
REM ---- gerados (CSV e resultados.txt) fiquem nessa pasta ----
REM  Usa ".\" para forcar a execucao a partir da pasta atual.
cd /d "%~dp0output"
".\jogo.exe"

echo.
echo ============================================
echo   Fim. Arquivos gerados na pasta: output\
echo   (perguntas.csv, historico_respostas.csv,
echo    estatisticas_casas.csv, resultados.txt)
echo ============================================
pause
endlocal
