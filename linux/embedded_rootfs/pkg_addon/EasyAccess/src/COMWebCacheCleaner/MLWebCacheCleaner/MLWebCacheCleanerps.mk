
MLWebCacheCleanerps.dll: dlldata.obj MLWebCacheCleaner_p.obj MLWebCacheCleaner_i.obj
	link /dll /out:MLWebCacheCleanerps.dll /def:MLWebCacheCleanerps.def /entry:DllMain dlldata.obj MLWebCacheCleaner_p.obj MLWebCacheCleaner_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del MLWebCacheCleanerps.dll
	@del MLWebCacheCleanerps.lib
	@del MLWebCacheCleanerps.exp
	@del dlldata.obj
	@del MLWebCacheCleaner_p.obj
	@del MLWebCacheCleaner_i.obj
