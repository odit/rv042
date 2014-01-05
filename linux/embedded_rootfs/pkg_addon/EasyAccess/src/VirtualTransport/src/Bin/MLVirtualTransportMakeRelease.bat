signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\VirtualTransportUninstall.exe
signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\SpOrder.dll
signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\MLVTNsp.dll
signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\MLVTLsp.dll
signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\MLVT.dll
cabarc n .\MenloLSP.cab .\MLVT.dll .\VirtualTransportUninstall.exe .\SpOrder.dll .\MLVTLsp.dll .\MLVTNsp.dll .\MenloLSP.inf
signcode -spc ..\..\..\SignCode\mycert.spc -v ..\..\..\SignCode\mykey.pvk -a sha1 .\MenloLSP.cab
copy .\MenloLSP.cab ..\..\Release\MenloLSP.cab
del .\MenloLSP.cab