@echo off

pushd .git
pushd hooks

IF EXIST pre-commit (rename pre-commit tmp-pre-commit) ELSE (rename tmp-pre-commit pre-commit)

popd
popd
