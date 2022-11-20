@echo off

set FPATH=".\test.f"
set OFILE=".\20223402-output"
set CTOOL="iverilog"

set SIMTOOL="vvp"

if exist %OFILE% (
    del  %OFILE%
)

%CTOOL% -c %FPATH% -o %OFILE%
%SIMTOOL% %OFILE%
gtkwave ".\cache.dmp"

set FPATH=
set OFILE=

set CTOOL=
set SIMTOOL=

