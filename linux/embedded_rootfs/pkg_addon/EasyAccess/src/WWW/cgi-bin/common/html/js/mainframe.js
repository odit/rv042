var winpopsmain=0;
var winpops=0;
var heights=410;
function no_error(){return true;}
function resizer(){
window.onerror=no_error;
heights=document.body.scrollHeight;
if (100>heights){setTimeout("resizerwait()",400); }
if (heights==null){setTimeout("resizerwait()",400);}
parent.resizeme(heights);
}
function resizerwait(){
heights=document.body.scrollHeight;
if (100>heights){heights=530;}
if (heights==null){heights=530;}
parent.resizeme(heights);
}
var stradddomain='Access&nbsp;&#187;&nbsp;Add&nbsp;Domain';
var streditdomain='Access&nbsp;&#187;&nbsp;Edit&nbsp;Domain';
var strdomains='Access&nbsp;&#187;&nbsp;Domains';
var strnewcsr='General&nbsp;&#187;&nbsp;Create&nbsp;CSR';
var strsslcert='General&nbsp;&#187;&nbsp;Certificates';
var strmycert='Current&nbsp;Certificates';
var strviewcert='General&nbsp;&#187;&nbsp;View&nbsp;Certificates';
var strnetworktools='General&nbsp;&#187;&nbsp;Network&nbsp;Tools';
var strpassword='General&nbsp;&#187;&nbsp;Password';
var strsettings='General&nbsp;&#187;&nbsp;File&nbsp;Settings';
var struserlist='Status&nbsp;and&nbsp;Logs&nbsp;&#187;&nbsp;Active&nbsp;Users';
var strconnectionlist='Status&nbsp;and&nbsp;Logs&nbsp;&#187;&nbsp;Active&nbsp;Users&nbsp;&#187;&nbsp;Active&nbsp;Connections';
var strrestart='General&nbsp;&#187;&nbsp;Restart';
var strclientroutes='Virtual&nbsp;Passage&nbsp;&#187;&nbsp;Client&nbsp;Routes';
var strtcptunneling='Access&nbsp;&#187;&nbsp;TCP&nbsp;Tunneling&nbsp;to&nbsp;SSL&nbsp;Enable&nbsp;Network&nbsp;Servers';
var streditglobal='Access&nbsp;&#187;&nbsp;Users&nbsp;and&nbsp;Groups&nbsp;&#187;&nbsp;Global';
var streditglobalpolicies='Global&nbsp;Policies';
var streditglobalbookmarks='Global&nbsp;Bookmarks';
var streditgroup='Access&nbsp;&#187;&nbsp;Users&nbsp;and&nbsp;Groups&nbsp;&#187;&nbsp;Groups';
var streditgrouppolicies='Group&nbsp;Policies';
var streditgroupbookmarks='Group&nbsp;Bookmarks';
var stredituser='Access&nbsp;&#187;&nbsp;Users&nbsp;and&nbsp;Groups&nbsp;&#187;&nbsp;Users';
var stredituserattributes='Access&nbsp;&#187;&nbsp;Users&nbsp;and&nbsp;Groups&nbsp;&#187;&nbsp;Users&nbsp;&#187;&nbsp;Login&nbsp;Policies';
var strresource='Access&nbsp;&#187;&nbsp;Network&nbsp;Resources&nbsp;for&nbsp;User and Group&nbsp;Policies';
var streditresource='Access&nbsp;&#187;&nbsp;Network&nbsp;Resources&nbsp;&#187;&nbsp;Edit&nbsp;Resource&nbsp;Object';
var straddresource='Access&nbsp;&#187;&nbsp;Network&nbsp;Resources&nbsp;&#187;&nbsp;Add&nbsp;Resource&nbsp;Object';
var stredituserpolicies='User&nbsp;Policies';
var stredituserbookmarks='User&nbsp;Bookmarks';
var strusersandgroups='Access&nbsp;&#187;&nbsp;Users&nbsp;and&nbsp;Groups&nbsp;&#187;&nbsp;Policies&nbsp;and&nbsp;Bookmarks';
var strstatus='Status&nbsp;and&nbsp;Logs&nbsp;&#187;&nbsp;Status';
var strlogconfig='Status&nbsp;and&nbsp;Logs&nbsp;&#187;&nbsp;Configuration';
var streventlog='Status&nbsp;and&nbsp;Logs&nbsp;&#187;&nbsp;Event&nbsp;Log';
var strsslvpnclient='Virtual&nbsp;Passage&nbsp;&#187;&nbsp;Client&nbsp;Addresses';
var straddtunnel='Add&nbsp;Tunnel';
var straddresource='Add&nbsp;Resource';
var stradduser='Add&nbsp;User';
var straddgroup='Add&nbsp;Group';
var strstaticroutes='Network&nbsp;&#187;&nbsp;Routes';
var stretchosts='Network&nbsp;&#187;&nbsp;Host&nbsp;Resolution';
var strnetworkinterface='Network&nbsp;&#187;&nbsp;Interfaces';
var strifacesettings='Network&nbsp;&#187;&nbsp;IP&nbsp;Address&nbsp;Configuration';
var strdnssettings='Network&nbsp;&#187;&nbsp;DNS&nbsp;Settings';
var strdate='General&nbsp;&#187;&nbsp;Date&nbsp;';
var streditglobaldesktopicons='Global&nbsp;Desktop&nbsp;Icons';
var bsubmit='Submit';
var blogin='Login';
var bexport='Export';
var bimport='Import';
var brestore='Restore';
var bsavenow='Save&nbsp;Now';
var brestart='Restart';
var benter='Enter';
var bupload='Upload';
var bupgrade='Upgrade';
var bcancel='Cancel';
var bclearlog='Clear&nbsp;Log';
var bnewcsr='New&nbsp;CSR/CRT';
var bdelete='Delete';
var breboot='Reboot';
var badduser='Add&nbsp;User';
var baddgroup='Add&nbsp;Group';
var bexpandall='Expand&nbsp;All';
var bcloseall='Close&nbsp;All';
var baddpolicy='Add&nbsp;Policy';
var baddbookmark='Add&nbsp;Bookmark';
var bdeletegroup='Delete&nbsp;Group';
var bdeleteuser='Delete&nbsp;User';
var badddomain='Add&nbsp;Domain';
var baddtunnel='Add&nbsp;Tunnel';
var badd='Add';
var ssuccess='Update&nbsp;Successful.';
var ssuccess1='Update&nbsp;Successful. <a href="/cgi-bin/restart"><font class=linkblue><u>Restart</u></font></a> SSL VPN software now.';
var serror='Update&nbsp;failed.';
var serror2='Update&nbsp;failed.';
var blank='';
var bselectactive='Enable&nbsp;Cert';
var buttonstring=0;
var titlestring=0;
var agent = navigator.userAgent.toLowerCase();
var browser=navigator.appName;
var version=navigator.appVersion;
function title(titlestring){document.write('<font  class="headingstylenomargin">&nbsp;' + titlestring + '&nbsp;</font>');}
function button(buttonstring){document.write('<font class=buttons>&nbsp;&nbsp;' + buttonstring + '&nbsp;&nbsp;</font>');}
function bbutton(buttonstring){document.write('<font class=bbuttons>&nbsp;' + buttonstring + '&nbsp;</font>');}
function error(buttonstring){document.write('<nobr><font class=badval>Error: </font><font class=toolbar> ' + buttonstring + ' </font></nobr>');}
function success(buttonstring){document.write('<font class=badval style="color:#00aa00">Status: </font><font class=toolbar> ' + buttonstring + ' </font>');}
function bbutton2(buttonstring,formname){
if (agent.indexOf("msie") != -1){
document.write('<font class=bbuttons>&nbsp;' + buttonstring + '&nbsp;</font>'); }
else{
document.write('<a href="JavaScript:'+formname+'();"><font class=bbuttons>&nbsp;' + buttonstring + '&nbsp;</font></a>');
}}
docObj = (document.all) ? "document.all." : "document."
function closechildren(){
if (winpopsmain!=0){if(!winpopsmain.closed) winpopsmain.close();}
if (winpops!=0){if(!winpops.closed) winpops.close();}}
var thisRow1;
function chgColor(rowNum) {
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'OnRowStyle2';
}
function chgColorOut(rowNum) {
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'OddRowStyle2';
}
function chgeven(rowNum) {
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'OnRowStyle2';
}
function chgevenOut(rowNum) {
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'EvenRowStyle2';
}
function chgmenu(rowNum) {
thisRow = eval(docObj + "row" + rowNum + "Class")
thisRow.bgColor = "#afafaf"
}
function chgmenuOut(rowNum) {
thisRow = eval(docObj + "row" + rowNum + "Class")
thisRow.bgColor = "#eeebe9"
}
function nothing(){}
