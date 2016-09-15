@echo off
    for /R "C:\OCR\Upscale\batch_process" %%f in (*.bmp) do (
    echo %%~nf
    NN_DoubleImage.exe type upscale mode load NN.org %%~nf.bmp %%~nf_Upscaled.bmp
)
pause