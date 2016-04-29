setlocal enabledelayedexpansion
FOR /l %%i IN (1, 1, 3) DO (
    SET folder=Test_%%i_8Threads
    mkdir !folder!
    call DualContouringGLFW.exe /test /octreesize 16 /threads 8 >> !folder!\TestSize16@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 32 /threads 8 >> !folder!\TestSize32@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 64 /threads 8 >> !folder!\TestSize64@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 128 /threads 8 >> !folder!\TestSize128@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
)
endlocal



