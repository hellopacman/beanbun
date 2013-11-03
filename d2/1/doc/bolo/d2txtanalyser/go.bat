@echo off

copy d2txtanalyser\d2txtanalyser___Win32_Release\d2txtanalyser.exe . > nul

for %%f in ("log_*.txt") do del %%f > nul
d2txtanalyser.exe

pause
