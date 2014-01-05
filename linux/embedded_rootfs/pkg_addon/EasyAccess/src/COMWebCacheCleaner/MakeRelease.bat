signcode -spc ..\SignCode\myCert.spc -v ..\SignCode\mykey.pvk -a sha1 .\MLWebCacheCleaner\ReleaseUMinDependency\MLWebCacheCleaner.Dll
cabarc n .\Release\MLWebCacheCleaner.cab .\MLWebCacheCleaner\ReleaseUMinDependency\MLWebCacheCleaner.dll  .\MLWebCacheCleaner\MLWebCacheCleaner.inf
signcode -spc ..\SignCode\myCert.spc -v ..\SignCode\mykey.pvk -a sha1 .\Release\MLWebCacheCleaner.cab

