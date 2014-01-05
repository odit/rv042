md Release

copy .\SSLDrv\objfre_w2k_x86\i386\SSLDrv.sys .\Release\SSLDrv.sys
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\SSLDrv.sys

copy .\XTunnel\ReleaseMinDependency\XTunnel.dll .\Release\XTunnel.dll
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTunnel.dll

copy .\UninstallVTPassage\Release\UninstallVTPassage.exe .\Release\UninstallVTPassage.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\UninstallVTPassage.exe

copy .\VPDesktopClient\Release\VPDesktopClient.exe .\Release\VPDesktopClient.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\VPDesktopClient.exe

copy .\VPDesktopClientInstall\Release\VPDesktopClientInstall.exe .\Release\VPDesktopClientInstall.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\VPDesktopClientInstall.exe

copy .\VPDesktopClientInstall\VPUnInstall.ico .\Release\VPUnInstall.ico

copy .\VPDesktopClient\VPDesktopClient.exe.manifest .\Release\VPDesktopClient.exe.manifest

cabarc n .\Release\VPInstall.cab .\VPDesktopClientInstall\VPDesktopClientInstall.inf .\Release\XTunnel.dll .\Release\SSLDrv.sys .\SSLDrv\Inf\SSLDrv.txt .\Release\UninstallVTPassage.exe .\Release\VPDesktopClient.exe .\Release\VPDesktopClientInstall.exe .\Release\VPUnInstall.ico .\Release\VPDesktopClient.exe.manifest 
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\VPInstall.cab
del .\Release\SSLDrv.sys
del .\Release\XTunnel.dll
del .\Release\UninstallVTPassage.exe
del .\Release\VPDesktopClient.exe
del .\Release\VPDesktopClientInstall.exe
del .\Release\VPUnInstall.ico
del .\Release\VPDesktopClient.exe.manifest


