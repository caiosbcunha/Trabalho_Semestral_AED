@echo off
REM ============================================================
REM  jogar_grafico.bat - Compila e executa a versao RAYLIB
REM  de "Caminho do Conhecimento" (jogo_raylib.c)
REM  Basta dar um duplo-clique neste arquivo.
REM ============================================================
setlocal
cd /d "%~dp0"

echo ============================================
echo   CAMINHO DO CONHECIMENTO (Raylib)
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
        echo        Instale o MSYS2/MinGW e a raylib:
        echo            pacman -S mingw-w64-ucrt-x86_64-raylib
        echo.
        pause
        exit /b 1
    )
)

REM ---- Garante que a pasta de saida existe ----
if not exist "output" mkdir "output"

REM ---- Compila a versao grafica ----
REM  As libs -lopengl32 -lgdi32 -lwinmm sao exigidas pela raylib no Windows.
gcc -Wall -Wextra -o "output\jogo_grafico.exe" "jogo_raylib.c" -lraylib -lopengl32 -lgdi32 -lwinmm -lm
if errorlevel 1 (
    echo.
    echo [ERRO] Falha na compilacao. Veja as mensagens acima.
    echo        Verifique se a raylib esta instalada no MSYS2/UCRT64.
    echo.
    pause
    exit /b 1
)

echo Compilacao concluida com sucesso!
echo.
echo ============================================
echo   Iniciando o jogo grafico...
echo ============================================
echo.

REM ---- Executa a partir de output\ para que os arquivos ----
REM ---- gerados (CSV e resultados.txt) fiquem nessa pasta ----
cd /d "%~dp0output"
".\jogo_grafico.exe"

echo.
echo ============================================
echo   Fim. Arquivos gerados na pasta: output\
echo ============================================
pause
endlocal
