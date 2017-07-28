@echo OFF

:: ===========================================================================
:: Datovka build script
:: The script builds Portable Datovka binary and creates and archive (*.zip)
:: with all dependencies.
:: CZ.NIC, z.s.p.o. 2017
:: ===========================================================================

:: ---------------------------------------------------------------------------
:: How to use it?
:: ---------------------------------------------------------------------------
:: First, install Qt and 7-ZIP tools on you computer.
:: Set path to Qt, Qt compiler executables to Windows Environment Variables (section PATH).
:: See https://www.computerhope.com/issues/ch000549.htm for exmaple.
:: Set to PATH following paths (5.9.1 replace for your version of Qt): 
::      "C:\Qt\5.9.1\mingw53_32\bin\" and "C:\Qt\Tools\mingw530_32\bin\" 
:: Set your path to 7ZIP executables into variables below:
set ZIPPATH="C:\Program Files (x86)\7-Zip\7z.exe"
:: You must have built libisds, openssl and other dependencies in the folder "mingw32built"   
:: Then you can run this script.
:: ---------------------------------------------------------------------------

:: Obtain current version from datovka.pro
cd ..
findstr /C:"VERSION =" datovka.pro > version.txt
set "string=var1;var2;var3;"
for /f "tokens=1,2,3 delims= " %%i in (version.txt) do set "variable1=%%i" &set "variable2=%%j" &set "VERSION=%%k"
endlocal
del version.txt

::  Define package name, User can change it if needed 
set DATOVKAPZIP="datovka-portable-%VERSION%-windows.zip"

@echo --------------------------------------------------------------------
@echo This batch creates Datovka packages for Windows in several steps:
@echo 1) Build Datovka binary (datovka.exe) with QT tool (requires Qt)
@echo 2) Create application packages to "packages" folder
@echo 3) Create installation package (*.exe) to "packages" folder (requires NSIS tool)
@echo 4) Create ZIP packages to "packages" folder (requires 7-ZIP tool)
@echo ---------------------------------------------------------------------
@echo WARNING:
@echo You must set path to Qt, Qt compiler executables to Windows Environment
@echo Variables (section PATH) otherwise the script will not run correctly.
@echo Add to Windows PATH following paths (replace 5.9.1 for your Qt version): 
@echo   "C:\Qt\5.9.1\mingw53_32\bin\"
@echo   "C:\Qt\Tools\mingw530_32\bin\"
@echo ---------------------------------------------------------------------
@echo Current path to NSIS:  %NSISPATH%
@echo Current path to 7-ZIP: %ZIPPATH% 
@echo NOTE: If paths are wrong, change these in the script.
@echo ---------------------------------------------------------------------
@echo Datovka version to build: %VERSION%
@echo ---------------------------------------------------------------------
@echo.
pause

if exist packages (
  rmdir /S /Q packages
)

:: Datovka portable version
@echo =====================================
@echo Datovka portable package (v%VERSION%)  
@echo =====================================
@echo Build Datovka binary ... 
mingw32-make.exe clean
lupdate datovka.pro
lrelease datovka.pro
qmake.exe datovka.pro -r -spec win32-g++ PORTABLE_APPLICATION=1 
mingw32-make.exe -j 2
mingw32-make.exe clean
@echo Build done.
@echo.
@echo -------------------------------------------------------------
@echo Create application bundle and copy all files and libraries...
:: Create app packege folder
set DATOVKAPORTPATH=packages\datovka-%VERSION%-portable
mkdir %DATOVKAPORTPATH%
mkdir "%DATOVKAPORTPATH%\locale"
:: copy all required app files and libraries
copy "release\datovka-portable.exe" %DATOVKAPORTPATH%
copy "AUTHORS" %DATOVKAPORTPATH%
copy "copyING" %DATOVKAPORTPATH%
copy "Changelog" %DATOVKAPORTPATH%
copy "scripts\datovka-portable-log.bat" %DATOVKAPORTPATH%
copy "locale\datovka_cs.qm" "%DATOVKAPORTPATH%\locale"
for /R "mingw32built\bin\" %%x in (*.dll) do copy "%%x" %DATOVKAPORTPATH% /Y
windeployqt --release "%DATOVKAPORTPATH%\datovka-portable.exe"
copy "%DATOVKAPORTPATH%\translations\qt_cs.qm" "%DATOVKAPORTPATH%\locale\qtbase_cs.qm"
rmdir /S /Q "%DATOVKAPORTPATH%\translations"
@echo Bundle done.
@echo.
@echo -----------------------------------------------------
@echo Run 7-ZIP and create ZIP archive of portable Datovka ...
cd packages
start /wait /Min "Create portable Datovka ZIP archive" %ZIPPATH% a -tzip %DATOVKAPZIP% datovka-%VERSION%-portable
cd ..
@echo ZIP archive done.
@echo.
rmdir /S /Q release
rmdir /S /Q debug
cd scripts