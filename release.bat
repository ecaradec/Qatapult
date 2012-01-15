set YYYYMMDD=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%

set setupdir=setup\%YYYYMMDD%
mkdir setup
mkdir %setupdir%

copy Release\Qatapult.exe workdir\
cd workdir
c:\unxtools\zip.exe -r ..\%setupdir%\Qatapult.zip *.* -x *.db -x settings.ini -x blat.log -x _* -x photos\* -x database\*
cd ..

echo %setupdir%\Qatapult.zip