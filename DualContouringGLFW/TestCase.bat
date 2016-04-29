setlocal enabledelayedexpansion
FOR /l %%i IN (1, 1, 3) DO (
    SET folder=Test_%%i_MaxThreads
    mkdir !folder!
    call DualContouringGLFW.exe /test /octreesize 16 >> !folder!\TestSize16@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 32 >> !folder!\TestSize32@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 64 >> !folder!\TestSize64@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 128 >> !folder!\TestSize128@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
)
endlocal
setlocal enabledelayedexpansion
FOR /l %%i IN (1, 1, 3) DO (
    SET folder=Test_%%i_6Threads
    mkdir !folder!
    call DualContouringGLFW.exe /test /octreesize 16 /threads 6 >> !folder!\TestSize16@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 32 /threads 6 >> !folder!\TestSize32@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 64 /threads 6 >> !folder!\TestSize64@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 128 /threads 6 >> !folder!\TestSize128@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
)
endlocal
setlocal enabledelayedexpansion
FOR /l %%i IN (1, 1, 3) DO (
    SET folder=Test_%%i_4Threads
    mkdir !folder!
    call DualContouringGLFW.exe /test /octreesize 16 /threads 4 >> !folder!\TestSize16@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 32 /threads 4 >> !folder!\TestSize32@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 64 /threads 4 >> !folder!\TestSize64@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 128 /threads 4 >> !folder!\TestSize128@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
)
endlocal
setlocal enabledelayedexpansion
FOR /l %%i IN (1, 1, 3) DO (
    SET folder=Test_%%i_2Threads
    mkdir !folder!
    call DualContouringGLFW.exe /test /octreesize 16 /threads 2 >> !folder!\TestSize16@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 32 /threads 2 >> !folder!\TestSize32@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 64 /threads 2 >> !folder!\TestSize64@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
    call DualContouringGLFW.exe /test /octreesize 128 /threads 2 >> !folder!\TestSize128@%DATE:/=-%@%time:~-11,2%-%time:~-8,2%-%time:~-5,2%.log
)
endlocal


