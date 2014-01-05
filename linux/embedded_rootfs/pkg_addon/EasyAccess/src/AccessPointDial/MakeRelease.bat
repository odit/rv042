md Release

copy .\SSLDrv\objfre_w2k_x86\i386\SSLDrv.sys .\Release\SSLDrv.sys
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\SSLDrv.sys

copy .\XTunnel\ReleaseMinDependency\XTunnel.dll .\Release\XTunnel.dll
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTunnel.dll

copy .\UninstallVTPassage\Release\UninstallVTPassage.exe .\Release\UninstallVTPassage.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\UninstallVTPassage.exe

cabarc n .\Release\XTunnel.cab .\XTunnel\XTunnel.inf .\Release\XTunnel.dll .\Release\SSLDrv.sys .\SSLDrv\Inf\SSLDrv.txt .\Release\UninstallVTPassage.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTunnel.cab
del .\Release\SSLDrv.sys
del .\Release\XTunnel.dll
del .\Release\UninstallVTPassage.exe


