var fontScale=1; // used inside CSS to scale px-based fonts using current browser setting
function cuesAdjustCSS()
{
  //var t0 = new Date();
  var front, expr, at, bAltered, str, rules, numRules, parts, str2;
  var cuesExpressionRE1 = new RegExp('.*expression',"i");
  var numCSS = document.styleSheets.length;  
  var bSkip = false;
  for(var i=0; i<numCSS; i++)
  {
    bSkip = false;
    if(document.styleSheets[i].id!="cuesCSS" &&
       document.styleSheets[i].id!="cuesCSS2") bSkip=true;
    if(document.styleSheets[i].href != null &&
       document.styleSheets[i].href.indexOf("/cues_utility/")!=-1) bSkip=false;
    if(bSkip) continue;
    if(document.styleSheets[i].rules)
      rules = document.styleSheets[i].rules;
    else
      rules = document.styleSheets[i].cssRules;
    numRules = rules.length;
    for(var j=0; j<numRules; j++)
    {        
      // initially handle only fontSize
      if(rules[j].style.fontSize==null || rules[j].style.fontSize=="") continue;
      str = rules[j].style.cssText+";"; // extra semicolon needed to resolve bug in IE8 beta
      bAltered = false;
      while(cuesExpressionRE1.test(str))
      {
        // rebuild csstext and repeat
        parts = RegExp.rightContext.split(");");
        if(parts.length<2) break;
        try
        {
          str = RegExp.lastMatch.replace("expression","")+eval(parts[0]+")")+";"+parts[1];
          bAltered = true;
        }catch(e){}
      }
      if(bAltered)
        rules[j].style.cssText = str;
    } // end of rules
  } // end of stylesheets
  //var t1 = new Date();
  //cuesLog("elap="+(t1-t0));
}
function cuesAdjustFonts()
{
  // default browser font size is 16px in Firefox and 12pt in IE
  var baseSize = 16;
  // .65 = 10px
  // .70 = 11px default for 5.4
  // .75 = 12px default for 6.0
  // .80 = 13px
  // .85 = 14px
  var ratio = .75;
  var mybaseSize = Math.round(baseSize * ratio);
  var bAdjustCSS = false;
  try
  {
    var baseFont = document.documentElement.currentStyle.fontSize; // only valid in IE
    baseSize = parseInt(baseFont);
    var baseType = baseFont.replace(baseSize+"","");
    if(baseType!="pt") return; // if someone else has set font size, bail out
    fontScale = baseSize/12;
    bAdjustCSS = true;
  }catch(e){}  
  document.documentElement.style.fontSize = Math.round(fontScale*mybaseSize)+"px";
  if(bAdjustCSS)
    cuesAdjustCSS();
}
cuesAdjustFonts();
