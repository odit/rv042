md Release

del .\Release\WindowsVPInstaller.jar
del .\Release\WindowsVPDialer.jar

copy .\SSLDrv\objfre_w2k_x86\i386\SSLDrv.sys .\Release\SSLDrv.sys
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\SSLDrv.sys

copy .\XTunnel\ReleaseMinDependency\XTunnel.dll .\Release\XTunnel.dll
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTunnel.dll

copy .\UninstallVTPassage\Release\UninstallVTPassage.exe .\Release\UninstallVTPassage.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\UninstallVTPassage.exe

copy .\VirtualPassageExe\Release\VirtualPassageExe.exe .\Release\VirtualPassageExe.exe
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\VirtualPassageExe.exe

javac -source 1.3 -target 1.1 .\VirtualPassageExe\VPInstaller.java

jar -cMvf .\Release\WindowsVPInstaller.jar -C .\VirtualPassageExe VPInstaller.class -C .\VirtualPassageExe VPExtracter.class -C .\VirtualPassageExe VPDebug.class
jarsigner -keystore ..\SignCode\mycert.p12 -storetype pkcs12 .\Release\WindowsVPInstaller.jar mycertalias

jar -cmvf .\VirtualPassageExe\version.info .\Release\WindowsVPDialer.jar -C .\Release SSLDrv.sys -C .\Release XTunnel.dll -C .\Release UninstallVTPassage.exe -C .\SSLDrv\Inf SSLDrv.txt -C .\Release VirtualPassageExe.exe

del .\Release\SSLDrv.sys
del .\Release\XTunnel.dll
del .\Release\UninstallVTPassage.exe
del .\Release\VirtualPassageExe.exe
del .\VirtualPassageExe\VPInstaller.class
del .\VirtualPassageExe\VPExtracter.class
del .\VirtualPassageExe\VPDebug.class

