set YYYYMMDD=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%
set TIMETMP=%TIME: =0%
set HMS=%TIMETMP:~0,2%%TIMETMP:~3,2%%TIMETMP:~6,2%

set setupdir=setup\%YYYYMMDD%%HMS%

mkdir setup
mkdir "%setupdir%"

copy Release\Qatapult.exe workdir\
cd workdir
c:\unxtools\zip.exe -r "..\%setupdir%\Qatapult.zip" *.* -x *.psd -x *.db -x settings.xml -x blat.log -x _* -x /Lib/* -x /databases/* -x /photos/* -x /plugins/* -x /skins/alfred-like/* -x dbghelp.dll -x vld_x86.dll -x Microsoft.DTfW.DHL.manifest
cd ..

:end
echo "%setupdir%\Qatapult.zip"
