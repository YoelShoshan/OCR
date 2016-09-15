@echo off
for /R "C:\OCR\croppedReceipts_RAW\bmp" %%f in (*.bmp) do (
	echo %%~nf
	tesseract C:\OCR\croppedReceipts_RAW\bmp\%%~nf.bmp C:\OCR\croppedReceipts_RAW\boxes\%%~nf -l heb-seg -psm 6 makebox

)
pause
