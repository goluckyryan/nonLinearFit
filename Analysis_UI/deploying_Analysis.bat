@echo off
title Deploying Analysis Program
echo =====================================================
echo This is batch script for deploying Analysis in winodws
Set desktop=%USERPROFILE%\Desktop
echo Desktop PATH: %Desktop%
echo =====================================================
pause

Set name=Analysis_UI.exe
Set fftwLib=libfftw3-3_x64.dll
Set origin=%Desktop%\nonLinearFit\build-Analysis_UI-Desktop_Qt_5_7_0_MSVC2015_64bit-Release\release
Set destination=%Desktop%\Analysis_Release
rem Set destination=C:\Users\Triplet-ESR\Desktop\kaka
Set compiler=C:\Qt\5.7\msvc2015_64\bin\

rem -----------------------------------------
echo =====================================================
echo Copy *.exe to %destination%
echo =====================================================
xcopy %origin%\%name% %destination%
xcopy %origin%\%fftwLib% %destination%
pause

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
pause
