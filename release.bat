set YYYYMMDD=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%
set HMS=%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%

set setupdir=setup\%YYYYMMDD%%HMS%
rem set setupdir=setup\tmp

mkdir setup
mkdir "%setupdir%"

copy Release\Qatapult.exe workdir\
cd workdir
c:\unxtools\zip.exe -r "..\%setupdir%\Qatapult.zip" *.* -x plugins\* -x *.db -x settings.xml -x blat.log -x _* -x photos\* -x database\* -x dbghelp.dll -x vld_x86.dll -x Microsoft.DTfW.DHL.manifest
cd ..

:end
echo "%setupdir%\Qatapult.zip"
