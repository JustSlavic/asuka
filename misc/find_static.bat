@echo off

pushd ..

echo ========================== FINDING "static" ===========================
findstr /d:common;src /n /l "static" *
echo ======================= FINDING STATIC_VARIABLE =======================
findstr /d:common;src /n /l "STATIC_VARIABLE" *
echo ======================= FINDING GLOBAL_VARIABLE =======================
findstr /d:common;src /n /l "GLOBAL_VARIABLE" *
echo =======================================================================

popd
