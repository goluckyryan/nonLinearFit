@echo off
title Deploying Analysis Program
echo =====================================================
echo This is batch script for deploying Analysis in winodws
Set desktop=%USERPROFILE%\Desktop
echo Desktop PATH: %Desktop%
echo =====================================================

set /p bit="  32-bit or 64-bit Application? (32/64)"
set /p version="   version? (5_9_2)"
set /p qt_bin=" Qt bin? (5.9.2)"

Set name=Analysis_UI.exe
Set source=%Desktop%\nonLinearFit\Analysis_UI
IF "%bit%"=="64" (
	Set fftwLib=libfftw3-3_x64.dll
	Set origin=%Desktop%\nonLinearFit\build-Analysis_UI-Desktop_Qt_%version%_MSVC2015_64bit-Release\release
	Set compiler=C:\Qt\%qt_bin%\msvc2015_64\bin\
	Set destination=%Desktop%\Analysis_Release_64
)

IF "%bit%"=="32" (
	Set fftwLib=libfftw3-3.dll
	Set origin=%Desktop%\nonLinearFit\build-Analysis_UI-Desktop_Qt_%version%_MinGW_32bit-Release\release
	Set compiler=C:\Qt\%qt_bin%\mingw53_32\bin\
	Set destination=%Desktop%\Analysis_Release_32
)

rem -----------------------------------------
echo =====================================================
echo %bit%-bit Application, Copy files to %destination%
echo =====================================================
xcopy /I/Y %origin%\%name% %destination%
rem xcopy /I/Y %source%\%fftwLib% %origin%
xcopy /I/Y %origin%\%fftwLib% %destination%

rem -----------------------------------------
echo =====================================================
set /p deployFlag="  Deploy Application? (Y/N)"
echo =====================================================

IF "%deployFlag%"=="Y" (
	rem -----------------------------------------
	echo Use windepoly to create nessacary *.dll
	cd %compiler%
	echo %cd%
	windeployqt.exe %destination%\%name%
)

IF "%bit%"=="32" (
echo ***********************************
echo When deploying 32-bit using minGW, some *.dll may be not copied.
echo Please look at %compiler% to copy non-Qt *.dll
echo ***********************************
)

echo --------------- bat script finished. ----------------
pause
