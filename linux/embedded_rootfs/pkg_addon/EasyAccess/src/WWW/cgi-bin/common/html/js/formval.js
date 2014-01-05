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
var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
var ipArray = tfld.match(ipPattern);
if (ipArray == null){
msg (ifld, "deny", "badval");
vfld.focus();
return false;
}else{
for (i = 1; i <= 4; i++){
thisSegment = ipArray[i];
if (thisSegment > 255){
msg (ifld, "deny", "badval");
vfld.focus();
i = 5;
return false;
}
if ((i == 1) && (thisSegment > 255)){
msg (ifld, "deny", "badval");
vfld.focus();
i = 5;
return false;
}}
msg (ifld, "warn", "toolbar");
return true;
};}
function validatenameorip(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
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
function notnull(vfld, ifld, reqd){
var stat = commonCheck (vfld, ifld, reqd);
if (stat != proceed) return stat;
var tfld = trim(vfld.value);
//msg (ifld, "warn", "toolbar");
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
ret = validateportrange(tfld);
}else
{ret = isbelow65535(tfld);}
if(ret == false){ msg (ifld, "error", "badval");vfld.focus();return false;}
else {
return true;
}
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
if (secdVal > fistVal)
return true;
else
return false;
}}


