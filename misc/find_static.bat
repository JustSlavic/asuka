@echo off

pushd ..

echo.
echo ======================= FIND KEYWORD "static" ===========================
echo.
findstr /d:common;src /n /l "static" *
echo.
echo ======================= FIND PERSIST VARIABLES ==========================
echo.
findstr /d:common;src /n /l "PERSIST" *
echo.
echo ======================= FIND GLOBAL VARIABLES ===========================
echo.
findstr /d:common;src /n /l "GLOBAL" *
echo.
echo =========================================================================
echo.

popd
