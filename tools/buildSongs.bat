.\tools\buildSoundbank.exe append

.\tools\gbfs .\tools\audio.gbfs .\tools\soundbank.bin .\tools\effect_locations.bin
.\tools\padbin 256 .\tools\Apotris-base.gba
type .\tools\Apotris-base.gba .\tools\audio.gbfs > Apotris.gba

@echo off
set "file=Apotris.gba"
FOR %%A IN ("%file%") DO set size=%%~zA

set /a "power=1"
:loop
if %power% LSS %size% (
  set /a "power=power*2"
  goto :loop
)

if %power% GTR 33554432 (
  echo:
  echo:
  echo "File size too large (%size%, max is 32MB). Try adding fewer songs."
  echo:
  echo:
  pause
) else (
  echo %power%
  .\tools\padbin %power% Apotris.gba
)
