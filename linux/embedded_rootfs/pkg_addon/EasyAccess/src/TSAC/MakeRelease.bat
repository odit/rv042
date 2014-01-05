md Release
copy .\XTSAC\Release\XTSAC.ocx .\Release\XTSAC.ocx
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTSAC.ocx
cabarc n .\Release\XTSAC.cab .\XTSAC\XTSAC.inf .\Release\Msrdp.ocx .\Release\XTSAC.ocx 
signcode -spc ..\SignCode\mycert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\XTSAC.cab
del .\Release\XTSAC.ocx
