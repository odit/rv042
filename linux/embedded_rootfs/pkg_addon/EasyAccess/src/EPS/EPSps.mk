
EPSps.dll: dlldata.obj EPS_p.obj EPS_i.obj
	link /dll /out:EPSps.dll /def:EPSps.def /entry:DllMain dlldata.obj EPS_p.obj EPS_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del EPSps.dll
	@del EPSps.lib
	@del EPSps.exp
	@del dlldata.obj
	@del EPS_p.obj
	@del EPS_i.obj
