<!--
var mlapmessage="Bottom - Mlap Cookie Not Found" + document.URL;
var mlapcookie = document.cookie;
var cookiestart = mlapcookie.indexOf(" mlap=");
var cookieend;
if ( cookiestart == -1 )
{
  cookiestart = mlapcookie.indexOf("mlap=");
}
if ( cookiestart != -1 )
{
  cookieend = mlapcookie.indexOf(";", cookiestart);
  mlapmessage = "Bottom " + document.URL + " Cookie Value "+ unescape(mlapcookie.substring(cookiestart, cookieend));
}
alert(mlapmessage);
//-->
