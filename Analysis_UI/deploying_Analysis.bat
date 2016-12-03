@echo off
title Deploying Analysis Program
echo =====================================================
echo This is batch script for deploying Analysis in winodws
echo =====================================================
pause

Set name=Analysis_UI.exe
Set fftwLib=libfftw3-3_x64.dll
Set origin=C:\Users\Triplet-ESR\Desktop\nonLinearFit\build-Analysis_UI-Desktop_Qt_5_7_0_MSVC2015_64bit-Release\release
Set destination=C:\Users\Triplet-ESR\Desktop\Analysis_Release
rem Set destination=C:\Users\Triplet-ESR\Desktop\kaka
Set compiler=C:\Qt\5.7\msvc2015_64\bin\

rem -----------------------------------------
echo Copy *.exe to %destination%
xcopy %origin%\%name% %destination%
xcopy %origin%\%fftwLib% %destination%
pause

rem -----------------------------------------
set /p deployFlag="Deploy: Y/N?"

IF "%deployFlag%"=="Y" (
	rem -----------------------------------------
	echo Use windepoly to create nessacary *.dll
	cd %compiler%
	echo %cd%
	windeployqt.exe %destination%\%name%
)
pause
