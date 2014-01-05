signcode -spc ..\SignCode\myCert.spc -v ..\SignCode\mykey.pvk -a sha1 .\EndPointSecurity\ReleaseMinDependency\EPS.dll
cabarc n .\Release\EPS.cab .\EndPointSecurity\ReleaseMinDependency\EPS.dll  .\EndPointSecurity\EPS.inf
signcode -spc ..\SignCode\myCert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\EPS.cab

