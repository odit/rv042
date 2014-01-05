emptyString = /^\s*$/
function trim(str){
return str.replace(/^\s+|\s+$/g, '')
};
function msg(fld, msgtype, message){
var dispmessage;
if (emptyString.test(message))
dispmessage = "&nbsp;";
else
dispmessage = message;
var elem = document.getElementById(fld);
elem.className = dispmessage;
};
var proceed = 2;
function commonCheck(vfld, ifld, reqd){
if (!document.getElementById)
return true;
var elem = document.getElementById(ifld);
if (!elem.firstChild && (typeof elem.innerHTML != "string"))
return true;
if (emptyString.test(vfld.value)){
if (reqd){
msg (ifld, "error", "badval");
vfld.focus();
return false;
}
else{
msg(ifld, "warn", "toolbar");
return true;
}
}
return proceed;
}
function validatePresent(vfld, ifld ){
var stat = commonCheck (vfld, ifld, true);
if (stat != proceed) return stat;
msg (ifld, "warn", "toolbar");
return true;
};
function validateip  (vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var newvar=tfld.lastIndexOf(':');
if (newvar == -1){var returnval = validateipv4(vfld, ifld, reqd);
return returnval;
}
else{
var returnval = validateipv6(vfld, ifld, reqd);
return returnval;}
}
function validateipv4(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
var ipArray = tfld.match(ipPattern);
if (ipArray == null){
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}else{
for (i = 0; i < 4; i++){
thisSegment = ipArray[i];
if (thisSegment > 255){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;
}
if ((i == 0) && (thisSegment > 255)){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;
}}
msg (ifld, "warn", "toolbar");
return 1;
}}
function validateipv6(vfld, ifld, reqd)
{
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var ipPattern1 = /^[0-9a-fA-F]{1,4}(\:[0-9a-fA-F]{1,4}){7}$/;
var ipArray = tfld.match(ipPattern1);
if (ipArray != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern2 = /^[0-9a-fA-F]{1,4}:(\:[0-9a-fA-F]{1,4}){1,6}$/;
var ipArray2 = tfld.match(ipPattern2);
if (ipArray2 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern3 = /^([0-9a-fA-F]{1,4}:){2}(\:[0-9a-fA-F]{1,4}){1,5}$/;
var ipArray3 = tfld.match(ipPattern3);
if (ipArray3 != null) {
return 2;}
var ipPattern4 = /^([0-9a-fA-F]{1,4}:){3}(\:[0-9a-fA-F]{1,4}){1,4}$/;
var ipArray4 = tfld.match(ipPattern4);
if (ipArray4 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern5 = /^([0-9a-fA-F]{1,4}:){4}(\:[0-9a-fA-F]{1,4}){1,3}$/;
var ipArray5 = tfld.match(ipPattern5);
if (ipArray5 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern6 = /^([0-9a-fA-F]{1,4}:){5}(\:[0-9a-fA-F]{1,4}){1,2}$/;
var ipArray6 = tfld.match(ipPattern6);
if (ipArray6 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern7 = /^([0-9a-fA-F]{1,4}:){6}(\:[0-9a-fA-F]{1,4}){1}$/;
var ipArray7 = tfld.match(ipPattern7);
if (ipArray7 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern8 = /^([0-9a-fA-F]{1,4}:){1,7}(\:){1}$/;
var ipArray8 = tfld.match(ipPattern8);
if (ipArray8 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
var ipPattern9 = /^(\:\:ffff\:(((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5])))$/;
var ipArray9 = tfld.match(ipPattern9);
if (ipArray9 != null) {
msg (ifld, "warn", "toolbar");
return 2;}
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}
function validateipv6withprefix (vfld, ifld, reqd)
{
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var ipPattern1 = /^[0-9a-fA-F]{1,4}(\:[0-9a-fA-F]{1,4}){7}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray = tfld.match(ipPattern1);
if (ipArray != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern2 = /^[0-9a-fA-F]{1,4}:(\:[0-9a-fA-F]{1,4}){1,6}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray2 = tfld.match(ipPattern2);
if (ipArray2 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern3 = /^([0-9a-fA-F]{1,4}:){2}(\:[0-9a-fA-F]{1,4}){1,5}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray3 = tfld.match(ipPattern3);
if (ipArray3 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern4 = /^([0-9a-fA-F]{1,4}:){3}(\:[0-9a-fA-F]{1,4}){1,4}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray4 = tfld.match(ipPattern4);
if (ipArray4 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern5 = /^([0-9a-fA-F]{1,4}:){4}(\:[0-9a-fA-F]{1,4}){1,3}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray5 = tfld.match(ipPattern5);
if (ipArray5 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern6 = /^([0-9a-fA-F]{1,4}:){5}(\:[0-9a-fA-F]{1,4}){1,2}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray6 = tfld.match(ipPattern6);
if (ipArray6 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern7 = /^([0-9a-fA-F]{1,4}:){6}(\:[0-9a-fA-F]{1,4}){1}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray7 = tfld.match(ipPattern7);
if (ipArray7 != null) {
msg (ifld, "warn", "toolbar");
return true;}
var ipPattern8 = /^([0-9a-fA-F]{1,4}:){1,7}(\:){1}\/(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray8 = tfld.match(ipPattern8);
if (ipArray8 != null) {
msg (ifld, "warn", "toolbar");
return true;}
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}
function validateprefix(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var ipPattern = /^(\d|[1-9]\d|1[0-1]\d|12[0-8])$/;
var ipArray = tfld.match(ipPattern);
if (ipArray == null){
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}else {
msg (ifld, "warn", "toolbar");
return true;
}}
function validatenameoripv6(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var newvar=tfld.lastIndexOf('.');
var startvar=newvar+1;
var endvar=newvar+4;
var temps=tfld.substring(startvar,endvar);
if (temps.search(/^[A-Za-z]+$/)!=-1){
var iChars = "*|,\"<>[]{}`\';\/()@&$#%";
var iChars2 = " ";
for (var i = 0; i < tfld.length; i++){
if (iChars.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;}
if (iChars2.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();i = 4;
return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}else {
var returnval = validateipv6(vfld, ifld, reqd);
return returnval;
}}
function validatenameoripv4(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var newvar=tfld.lastIndexOf('.');
var startvar=newvar+1;
var endvar=newvar+4;
var temps=tfld.substring(startvar,endvar);
if (temps.search(/^[A-Za-z]+$/)!=-1){
var iChars = "*|,\"<>[]{}`\';\/()@&$#%";
var iChars2 = " ";
for (var i = 0; i < tfld.length; i++){
if (iChars.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;}
if (iChars2.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();i = 4;
return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}else {
var returnval = validateipv4(vfld, ifld, reqd);
return returnval;
}}
function validatenameorip(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var newvar=tfld.lastIndexOf('.');
var startvar=newvar+1;
var endvar=newvar+4;
var temps=tfld.substring(startvar,endvar);
if (temps.search(/^[A-Za-z]+$/)!=-1){
var iChars = "*|,\"<>[]{}`\';\/()@&$#%";
var iChars2 = " ";
for (var i = 0; i < tfld.length; i++){
if (iChars.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;}
if (iChars2.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();i = 4;
return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}else {
var returnval = validateip(vfld, ifld, reqd);
return returnval;
}}
function notnull(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
msg (ifld, "warn", "toolbar");
return true;
}
function notnullspaces	(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
var iChars6 = " ";
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
for (var i = 0; i < tfld.length; i++){
if (iChars6.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}
function country (vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
if (tfld.length == 1){
msg (ifld, "deny", "badval");return false;}
else {msg (ifld, "warn", "toolbar");
return true;}
}
function asciicharacters (vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
var iChars6 = " ";
var iChars7 = "*|,\":<>[]{}`\';\/()@&$#%";
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
for (var i = 0; i < tfld.length; i++){
if (iChars6.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");return false;}
if (iChars7.indexOf(tfld.charAt(i)) != -1) {
msg (ifld, "deny", "badval");return false;}
}
msg (ifld, "warn", "toolbar");
return true;}
function validateurl  (vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var newvar=tfld.lastIndexOf('.');
var startvar=newvar+1;
var endvar=newvar+4;
var temps=tfld.substring(startvar,endvar);
if (temps.search(/^[A-Za-z]+$/)!=-1 || (tfld.indexOf('.') ==-1)){
var iChars = "*|,\"<>[]{}`\';~^()@&$#%";
var iChars2 = " ";
var iChars3 = ":";
var newvar2=1;
if (tfld.indexOf('/') !=-1){newvar2=tfld.indexOf('/');}
if (newvar2 > 3 && newvar2 < 7){
msg (ifld, "deny", "badval");return false;
}
for (var i = 0; i < tfld.length; i++){
if (iChars.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}else{
var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
var ipArray = tfld.match(ipPattern);
if (ipArray == null){
if ((tfld.indexOf('/') !=-1) || (tfld.indexOf('?') !=-1))
{msg (ifld, "warn", "toolbar");return true;}
msg (ifld, "deny", "badval");
return false;}
else{
for (i = 0; i < 4; i++){
thisSegment = ipArray[i];
if (thisSegment > 255){
msg (ifld, "deny", "badval");
i = 4;
return false;
}
if ((i == 0) && (thisSegment > 255)){
msg (ifld, "deny", "badval");
i = 4;
return false;
}}
if (tfld == "0.0.0.0")
msg (ifld, "warn", "toolbar");
else if (tfld == "255.255.255.255")
msg (ifld, "warn", "toolbar");
else
msg (ifld, "warn", "toolbar");
return true;
};}}
function valnumber (vfld, ifld, reqd, startn, endn){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var numberss = /^\d+$/;
var numArray = tfld.match(numberss);
if (numArray == null){
msg (ifld, "deny", "badval");
return false;
}else{
if (numArray <= endn && numArray >= startn){
msg (ifld, "warn", "toolbar");
i = tfld.length;
return true;}
else{
msg (ifld, "deny", "badval");
return false;
};}}
function validatenameoripwithport(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var temppos=tfld.lastIndexOf(':');
var tempip;
if(temppos != -1){
tempip=tfld.substr(0, temppos);
var startvar=temppos+1;
var tempport=tfld.substr(startvar);
var numberss = /^\d+$/;
var numArray = tempport.match(numberss);
if (numArray == null){
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}}
else { tempip = tfld;}
var stat = commonCheck (tempip, ifld, reqd);
if (stat != proceed) return stat;
var tfld = tempip;
var newvar=tfld.lastIndexOf('.');
var startvar=newvar+1;
var endvar=newvar+4;
var temps=tfld.substring(startvar,endvar);
if (temps.search(/^[A-Za-z]+$/)!=-1){
var iChars = "*|,\":<>[]{}`\';\/()@&$#%";
var iChars2 = " ";
for (var i = 0; i < tfld.length; i++){
if (iChars.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;}
if (iChars2.indexOf(tfld.charAt(i)) != -1){
msg (ifld, "deny", "badval");
vfld.focus();i = 4;
return false;}
}
msg (ifld, "warn", "toolbar");
return true;
}else{
var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
var ipArray = tfld.match(ipPattern);
if (ipArray == null){
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}else{
for (i = 0; i < 4; i++){
thisSegment = ipArray[i];
if (thisSegment > 255){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;
}
if ((i == 0) && (thisSegment > 255)){
msg (ifld, "deny", "badval");
vfld.focus();
i = 4;
return false;
}}
if (tfld == "0.0.0.0")
msg (ifld, "warn", "toolbar");
else if (tfld == "255.255.255.255")
msg (ifld, "warn", "toolbar");
else
msg (ifld, "warn", "toolbar");
return true;
};}}

function validateport(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
var ret;
var temppos=tfld.lastIndexOf('-');
if(temppos != -1) {
if(temppos == 0) {
return false;
}
ret =  validateportrange(tfld);
}else
{ret = isbelow65535(tfld);}
if(ret == false){ msg (ifld, "error", "badval");vfld.focus();return false;}
else {return true;}
}
function isbelow65535(sText){
var ValidChars = "0123456789";
var Char;
var blnResult = true;
for (i = 0; i < sText.length; i++)
{
Char = sText.charAt(i);
if (ValidChars.indexOf(Char) == -1)
blnResult= false;
}
if ((sText <= 65535) && (sText >= 0)){
return true;
} else { return false; }
return blnResult;
}

function validateportrange(t2Val)
{
var regFormat=/^\d{0,5}\-\d{0,5}$/;
if (regFormat.test(t2Val))
{
var splitVal=t2Val.split("-");
var fistVal = Number(splitVal[0]);
var secdVal = Number(splitVal[1]);
if(isbelow65535(secdVal) == false)
return false;
if(isbelow65535(fistVal) == false)
return false;
if (secdVal >= fistVal)
return true;
else
return false;
}}

