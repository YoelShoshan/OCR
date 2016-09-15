@echo off
for /R ".\" %%f in (*.boxUnsorted*) do (
	echo %%~nf
%%	tesseract C:\OCR\croppedReceipts_RAW\bmp\%%~nf.bmp C:\OCR\croppedReceipts_RAW\boxes\%%~nf -l heb-seg -psm 6 makebox

)
pause
