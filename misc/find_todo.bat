@echo off

pushd ..

echo =======================================================================
findstr /d:common;src /n /l "@todo" *
echo =======================================================================

popd