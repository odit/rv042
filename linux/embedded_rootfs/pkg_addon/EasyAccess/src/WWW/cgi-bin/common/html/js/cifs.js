var winpops1=0;
var winpops=0;
var blogin='Login';
var bdeletemarked='Delete&nbsp;Marked';
var titlestring=0;
var bsubmit='Submit';
var baddbookmark='Add&nbsp;Bookmark';
var bcancel='Cancel';
var benter='Enter';
var brename='Rename';
var agent = navigator.userAgent.toLowerCase();
var browser=navigator.appName;
var version=navigator.appVersion;
function button(buttonstring){document.write('&nbsp;&nbsp;' + buttonstring + '&nbsp;&nbsp;');}
function bbutton(buttonstring){document.write('<font class=bbuttons>&nbsp;' + buttonstring + '&nbsp;</font>');}
function closechildren(){if (winpops!=0){if(!winpops.closed) winpops.close();}}
function Launchhelp(){
var popurl='/WebHelp/EndUserGuide.htm#netw.htm';
winpops1=window.open(popurl,"OnlineHelp","width=700,height=500,resizable");
}
var thisRow1;
function chgColor(rowNum){
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'onmainbackcifs';
}
function chgColorOut(rowNum){
thisRow1 = eval('"row" + rowNum + "Class"')
document.getElementById(thisRow1).className = 'mainbackcifs';
}
function nothing(){
if (agent.indexOf("msie") != -1){
if (version.indexOf("6.") >= 0){
document.write('<!--[if gte vml 1]><v:line from="1 1" to="1 1"><v:fill on="false" fill=false color="#ffffff"/><v:path textpathok="True"/><v:textpath on="false" string="" style="font:normal normal 1pt Arial"/></v:line><![endif]-->');
}}}
