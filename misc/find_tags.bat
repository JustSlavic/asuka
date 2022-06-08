@echo off

pushd ..

echo.
echo ========================= FIND TODO ===================================
echo.
findstr /d:common;src /n /l "@todo" *
rem echo.
rem echo ========================= FIND NOTE ===================================
rem echo.
rem findstr /d:common;src /n /l "@todo" *
echo.
echo ========================= FIND NOCOMMIT ===============================
echo.
findstr /d:common;src /n /l "@nocommit" *
echo.
echo =======================================================================
echo.

popd
