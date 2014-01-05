/* Common Variable */
var languagetype = 0;
var UI_Version = "v1.9";
var WideLayoutwidth = "92%";	
var NormallLayoutwidth = "85%";
var SmallLayoutwidth = "78%";
var Tableclass = "TableSTA";	
var Tabletileclass="tablestyle";
var Tablealign ="left" ;
var Tableborder = "0" ;
var Tablebordercolor = "#8499A2"; 
var Tablecellspacing = "0";
var Tablecellpadding ="0";
var TableButton = "STbutton";					// Add to list, del add, service management ... 					
var Tablestyle ="border-collapse: collapse";
var Enablelinkcolor ="black";
var Disablelinkcolor ="red";
var Alertcolor="blue";
var Connectlinkcolor ="green";
var Content_title="";
var HasExtendLogo = 0;
var splitVarforSpecialWordField="{[(*-*-*)]}";
var DHCPV6_MAXAVAILABLEIP = 512;

/***************************************************/
/*												   */	
/*            		 Begin of Tree menu		 	          */
/*												   */	
/***************************************************/
var menuheigh=0;
var additional_heigh=0;
function CreateNode(UID, link, string, hidden) //constructor 
{ 
	//constant data 
	this.UID = UID
	this.link = link
	this.string = string		
	this.ishidden = hidden  

	//dynamic data 
	this.id = 0;
	this.isSelect = 0   	
	this.children = new Array 
	this.nChildren = 0 

	//methods 
	this.addChild = addChild 
	this.renderOb = drawNode 
} 

function drawNode() 
{ 
	var diffNode = 0;
	if (this.UID !="" && this.ishidden == 0)
	{
		document.write("<tr>\n")
		document.write("<td width=\"176\">\n");
		document.write("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n");
		document.write("<tr>\n ");	  
		if (this.isSelect)
		{				
			if (this.nChildren == 0)
				diffNode = 1;
			else
			{
				if (this.children[0].isSelect == 1 && this.link == this.children[0].link)
					diffNode = 2;
				else
				{
					if (this.children[0].isSelect == 1)
						diffNode = 4;
					else
						diffNode = 5;
					for (var i=1; i<this.nChildren;i++) 
					{
						if (this.children[i].isSelect == 1)
						{							
							diffNode = 3;
							break;
						}
					}
				}
			}		
			if (diffNode == 1)				// No children iterm
			{
				Content_title = this.string;
				document.write("<td class=\"SelectMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\"")		
				if (this.link == "")
					document.write(">");	
				else	
					document.write(" onClick=\"javacript:location.href='"+this.link+"'\">");		
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"21\" height=\"12\">")
			}
			else if (diffNode == 2)			// This Node's first children iterm has been select and its link is the same as Node ;
			{
				document.write("<td class=\"SelectedMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\">")						
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"4\"><img border=\"0\" src=\"images/cend10.gif\" width=12 height=12><img border=\"0\" src=\"images/_blank.gif\" width=\"5\">")			
			}
			else if (diffNode == 3)			// This Node's children (not first one) has been select
			{
				document.write("<td class=\"SelectedMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\">")			
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"4\"><img border=\"0\" src=\"images/cend10.gif\" width=12 height=12><img border=\"0\" src=\"images/_blank.gif\" width=\"5\">")
			}
			else	 if (diffNode == 4)			// This Node's first children iterm has been select but its link isn't the same as Node ;
			{
				document.write("<td class=\"SelectedMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\">")		
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"4\"><img border=\"0\" src=\"images/cend10.gif\" width=12 height=12><img border=\"0\" src=\"images/_blank.gif\" width=\"5\">")
			}
			else
			{
				Content_title = this.string;
				document.write("<td class=\"SelectedMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\">")		
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"4\"><img border=\"0\" src=\"images/cend10.gif\" width=12 height=12><img border=\"0\" src=\"images/_blank.gif\" width=\"5\">")						
			}
		}
		else
		{
			document.write("<td class=\"UnSelectMenu\" width=\"176\" height=\"24\" align=\"left\" valign=\"middle\" onmouseover=\"this.className='HoverMenu', window.status='"+this.string+"'\" onmouseout=\"this.className='UnSelectMenu'\" ");
			document.write(" onmousedown=\"this.className='PressMenu'\"");
			if (this.link == "")
				document.write(">");	
			else	
				document.write(" onClick=\"javacript:location.href='"+this.link+"'\">");	
			if (this.nChildren > 0)
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"4\"><img border=\"0\" src=\"images/cend31.gif\" width=12 height=12><img border=\"0\" src=\"images/_blank.gif\" width=\"5\">")
			else
				document.write("<img border=\"0\" src=\"images/_blank.gif\" width=\"21\" height=\"12\">")
		}
		if (this.isSelect)
		{
			document.write(this.string)     
			document.write("</b></td>\n")
			document.write("</tr>");
			if (diffNode == 1)
			{
				document.write("</table>\n")
				document.write("</td>\n")
				document.write("</tr>\n")
			}
		}		
		else
		{		
			document.write(this.string) 
			document.write("</td>\n")
			document.write("</tr>");
			document.write("</table>\n")
			document.write("</td>\n")
			document.write("</tr>\n")
		}
		menuheigh += 24;
	}	
}

// Definition of class Item (a icon or link inside a node) 
// ************************************************************* 
 
function CreateItem(UID,link, string, hidden) // Constructor 
{ 
	//constant data
	this.UID = UID
	this.link = link
	this.string = string 
	
	//dynamic data 
	this.id = 0;
	this.isSelect = 0
	this.ishidden = hidden  	

	//methods 
	this.renderOb = drawItem  
} 

function drawItem() 
{ 
	if (this.ishidden == 0)
	{
		menuheigh += 20;
		document.write("<tr>\n");
		if (this.isSelect)	
		{
			document.write("<td class=\"SelectItem\" height=\"20\" onmouseover=\"window.status='"+this.string+"'\" onmouseout=\"window.status=''\">");
			if (this.link != "")
			{	
				document.write('<a href="'+this.link+'" style="text-decoration:none">');
				document.write("<span style=\"color:#FFF;background:#8db71f;\">");
				document.write(this.string);
				document.write("</span>");	
				document.write('</a>');							
			}
			else
			{
				document.write("<span style=\"background:#8db71f;\">");
				document.write(this.string);
				document.write("</span>");	
			}
			Content_title = this.string;
		}	
		else
		{
			document.write("<td class=\"UnSelectItem\" height=\"20\" onmouseover=\"window.status='"+this.string+"';\" onmouseout=\"window.status='';\">");
			if (this.link != "")
			{
				document.write('<a href="'+this.link+'">');
				document.write(this.string);
				document.write('</a>');
			}
			else
			{
				document.write('<img border="0" src="images/_blank.gif" width="10" height="1">'); 
				document.write(this.string);
			}	
		}	
		document.write('</td>\n')  
		document.write("</tr>\n")
	}
	else if (this.isSelect)
		Content_title = this.string;	
}

// Initial selected Node & Item  
function SetItemInit(RootNode, Nodeid, Childid) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == "")
			continue;
		
		if (RootNode.children[i].UID == Nodeid)
		{
			RootNode.children[i].isSelect = 1
			break;
		}
	}
 	if (Childid>=0 && RootNode.children[i].nChildren > 0)
 	{
		for (j=0 ; j < RootNode.children[i].nChildren; j++)  
		{
			if (RootNode.children[i].children[j].UID == Childid)
			{
				RootNode.children[i].children[j].isSelect = 1
				break;
			}
		}
 	}		
} 

// Change Node link: default or changed 
function SetNodelink(RootNode, Nodeid, link) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			RootNode.children[i].link = link
			break;
		}
	}		
} 

// Change Item link: default or changed 
function SetItemlink(RootNode, Nodeid, Childid, link) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			break;
		}
	}
	
	if (RootNode.children[i].nChildren > 0)
	{	
		for (j=0 ; j < RootNode.children[i].nChildren; j++)  
		{
			if (RootNode.children[i].children[j].UID == Childid)
			{
				RootNode.children[i].children[j].link = link	
				break;
			}
		}
	}		
 	else
		RootNode.children[i].link = link
} 

// Change Item str: default or changed
function SetNodeStr(RootNode, Nodeid, Str) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			RootNode.children[i].string = Str
			break;
		}
	}
}
function SetItemStr(RootNode, Nodeid, Childid, Str) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			break;
		}
	}
	
	if (RootNode.children[i].nChildren > 0)
	{	
		for (j=0 ; j < RootNode.children[i].nChildren; j++)  
		{
			if (RootNode.children[i].children[j].UID == Childid)
			{
				RootNode.children[i].children[j].string = Str
				break;
			}
		}
	}		
 	else
		RootNode.children[i].string = Str
}

// Change Node state: visible or hidden
function SetNodeState(RootNode, Nodeid, State) 
{ 
	var i=0;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			break;
		}
	}	
	RootNode.children[i].ishidden = State
} 

// Change Node state: visible or hidden
function SetItemState(RootNode, Nodeid, Childid, State) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].UID == Nodeid)
		{
			break;
		}
	}

	for (j=0 ; j < RootNode.children[i].nChildren; j++)  
	{
		if (RootNode.children[i].children[j].UID == Childid)
		{
			RootNode.children[i].children[j].ishidden = State
			break;
		}
	}
}

// Get Selected Node string
function GetSelectedNodelStr(RootNode) 
{ 
	var i,j;
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (RootNode.children[i].isSelect)
		{
			return RootNode.children[i].string;
		}
	}
	return "";
}

// Layout Menu Area
function initializeTree(RootNode, closelink) 
{  
	var i=0;
	var j=0;
	var limit_heigh = 0;

	document.write("<tr>");
	document.write('<td colspan="2" valign=\"top\">');
	PrintHeadLineBar(closelink);
  	document.write('<tr>');
	document.write('<td valign="top" class="contentframe opacitybg">');
	document.write('<table width=\"170\" border="0" cellspacing="0" cellpadding="0">');
	HasExtendLogo = 0;		
	
	for (i=0 ; i < RootNode.nChildren; i++)  
	{
		if (closelink)
			RootNode.children[i].link = "";
		RootNode.children[i].renderOb();
		if (RootNode.children[i].isSelect && RootNode.children[i].ishidden== 0 &&  RootNode.children[i].nChildren > 0)
		{	
			document.write("<tr>\n")
			document.write("<td>\n")
			document.write("<table width=\"170\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n")
			document.write("<tr>\n")
			document.write("<td align=\"left\" valign=\"top\">")
            		document.write("<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">")
			if (RootNode.children[i].nChildren > 0)
			{
				menuheigh += 4;
				document.write("<tr>");
				document.write("<td class=\"UnSelectItem\" height=\"4\" ></td>");
				document.write("</tr>");
			}			
			for (j=0 ; j < RootNode.children[i].nChildren; j++)
			{
				if (closelink)
					RootNode.children[i].children[j].link = "";
				RootNode.children[i].children[j].renderOb();
			}	
			if (RootNode.children[i].nChildren > 0)
			{
				menuheigh += 4;
				document.write("<tr>");
				document.write("<td class=\"UnSelectItem\" height=\"4\" ></td>");
				document.write("</tr>");
			}
			document.write("</table>\n")
			document.write("</td>\n")
			document.write("</tr>\n")
			document.write("</table>\n")
			document.write("</td>\n")
			document.write("</tr>\n")
			document.write("</table>\n")
			document.write("</td>\n")
			document.write("</tr>\n")
			document.write("<tr>\n")
			document.write("<td height=\"1\" bgcolor=\"#8499A2\"></td>")
			document.write("</tr>\n")	
			menuheigh += 1;
		}
	}		
	if (window.innerHeight)
		limit_heigh = window.innerHeight - 110;
	else
		limit_heigh = document.body.clientHeight - 109;
		
	if (menuheigh < limit_heigh)
		additional_heigh = limit_heigh - menuheigh;
	else	
		additional_heigh = 0;	
	document.write('</table>');
	document.write('</td>');
	document.write('<td valign="middle" align="right"><img border=\"0\" src=\"images/_blank.gif\" width=\"4\"></td>');	
}

// Add New Node or Item  
function addChild(childNode) 
{ 
	childNode.id = this.nChildren
	this.children[this.nChildren] = childNode 
	this.nChildren++ 
	return childNode 
} 

// Auxiliary Functions for Node-Treee backward compatibility 
// ********************************************************* 
 
function gFld(ID,link, string,hidden) 
{ 
	Node = new CreateNode(ID, link, string, hidden) 
	return Node 
} 
 
function gLnk(ID,link, string, hidden) 
{  
  	linkItem = new CreateItem(ID, link, string, hidden)   
  	return linkItem 
} 
 
function insFld(parentNode, childNode) 
{ 
  	return parentNode.addChild(childNode) 
} 
 
function insDoc(parentNode, document) 
{ 
  	parentNode.addChild(document) 
} 

//=============== End of Tree menu ================*/

/***************************************************/
/*												     */	
/*            		Begin of Common function             	     */
/*												     */	
/***************************************************/
// Print Background image
function PrintBackground(extend, width)
{
	var application =navigator.appVersion;
	var appType ="";
	var appName = "";
	var appVersion = 0;
	var BgWidth=0;
	var BgHeight=0;
	var LimitHeight = 0;
	if (window.innerWidth)
	{
		BgHeight = window.innerHeight;
		BgWidth = window.innerWidth;
	}
	else
	{
		BgHeight = document.body.clientHeight;
		BgWidth = document.body.clientWidth;
	}
	document.write('<div class="page">');
	if (extend == "about")
		document.write('<img border="0" id="aboutimg" width="'+BgWidth+'" height="'+BgHeight+'" src="../images/body_bg.jpg">');		
	else if (extend == "help")
		document.write('<img border="0" id="helpimg" width="'+BgWidth+'" height="'+BgHeight+'" src="../images/body_bg.jpg">');		
	else	
		document.write('<img border="0" id="bgimg" width="'+BgWidth+'" height="'+BgHeight+'" src="images/body_bg.jpg">');
	document.write('</div>');
	if (extend == "help")
	{
		appType = application.split(";");
		appName = appType[1].substring(1, 5);
		appVersion = parseInt(appType[1].substring(6, appType[1].length),10);
		if (appName == "MSIE" && appVersion < 7)
			document.write('<table border="0" width="100%" style="background:#c0d3e5;"border-color:#53636a;border-width:1px;>');
		else
			document.write('<table class="helptable" border="0" width="100%">');
	}	 
}
// Print Opacity Disable Mask
function PrintMask()
{
	var BgWidth=0;
	var BgHeight=0;
	var a=PrintMask.arguments; 

	if (window.innerWidth)
	{
		BgHeight = window.innerHeight;
		BgWidth = window.innerWidth;
	}
	else
	{
		BgHeight = document.body.clientHeight;
		BgWidth = document.body.clientWidth;
	}
	document.write('<div id="disablemask" style="display:none">');
	document.write('<table id="masktable" border="'+Tableborder+'" width="'+BgWidth+'" height="'+BgHeight+'" ');
	document.write('style="top: 0px;left: 0px;position: absolute;background-color: #000;opacity:0.75;Filter: Alpha(opacity=75);');
	document.write('width:"'+BgWidth+'"; height:"'+BgHeight+'";');	
	document.write('z-index: 1;>');
	document.write('<tr><td colspan="3" height="30%">&nbsp;</td></tr>');
	document.write('<tr><td width="30%">&nbsp;</td><td align="center" id="ShowProgress">');
   	for(var i=0; i<a.length; i++)
   	{
   		if (i%2 ==0)
   			document.write('<font class="bottomfont">'+a[i]+'</font>');
		else
		{
			document.write('<br><br>');
			document.write('<img src="'+a[i]+'">'); 			
		}
   	}
	document.write('</td><td width="30%">&nbsp;</td></tr>');
	document.write('<tr><td colspan="3" height="35%">&nbsp;</td></tr>');
	document.write('</table>');	
	document.write('</div>');
}
function ChangeBgsize(width)
{
	var obj;
	var TableWidth = 0;	
	var BgWidth=0;
	var BgHeight=0;
	var ContentWidth = 0;
	var LimitWidth = 0;
	var LimitHeight = 0;
	var Scrobarwidth= 0;
		
	if (window.innerWidth)
	{ 
		BgWidth = window.innerWidth;
		BgHeight = window.innerHeight;
		LimitWidth = 203;
		LimitHeight = 111;		
		Scrobarwidth = 30;		
	}	
	else
	{	
		BgWidth = document.body.clientWidth;
		BgHeight = document.body.clientHeight;
		LimitWidth = 204;
		LimitHeight = 109;	
		Scrobarwidth = 30;		
	}	

	if (BgWidth  < 450)
		BgWidth = 450;

	if (BgWidth < screen.availWidth)
		TableWidth = BgWidth - Scrobarwidth;
	else
		TableWidth = screen.availWidth - Scrobarwidth;

	ContentWidth = BgWidth - LimitWidth - Scrobarwidth;
	if (ContentWidth < 700)
		ContentWidth = 700;	

	obj = document.getElementById("content_block");	
	if (obj)
	{
		if (HasExtendLogo == 1)
			obj.style.width = ContentWidth + 30;
		else
			obj.style.width = ContentWidth;
	}

	if (BgHeight - LimitHeight < menuheigh + additional_heigh)
		additional_heigh = 0;
	else
		additional_heigh = BgHeight - LimitHeight - menuheigh;
	
	obj = document.getElementById("mainframe");		
	if (obj)
		obj.style.width = TableWidth;	
		
	obj = document.getElementById("content_height");	
	if (obj)
	{
		obj.style.width = BgWidth - LimitWidth;
		if (BgHeight - LimitHeight < menuheigh)
			obj.style.height = menuheigh;
		else
			obj.style.height = BgHeight - LimitHeight;
	}	
	
	obj = document.getElementById("bgimg");	
	if (obj)
	{
		if (BgHeight < menuheigh + additional_heigh + LimitHeight)
			BgHeight = menuheigh + additional_heigh + LimitHeight + 2;
		obj.style.width= BgWidth;
		obj.style.height= BgHeight;
	}

	obj = document.getElementById("aboutimg");	
	if (obj)
	{
		obj.style.width= document.body.clientWidth;
		obj.style.height= document.body.clientHeight;
	}
	
	obj = document.getElementById("helpimg");	
	if (obj)
	{
		obj.style.width= document.body.clientWidth;
		obj.style.height= document.body.clientHeight;
	}

	obj = document.getElementById("masktable");	
	if (obj)
	{
		obj.style.width= BgWidth;
		obj.style.height= BgHeight;
	}	
}
function PrintWhiteTableBegin()
{
	if (window.ActiveXObject)
		document.write('<div id="whitetable" style="overflow-y:scroll">');		
	else
		document.write('<div id="whitetable" style="overflow-y:auto">');	
	document.write('<table border="0" width="100%" height="100%" align="center" cellspacing = "0" cellpadding ="0">');
	document.write('<tr>');
       document.write('<td valign="top">');
}
function PrintWhiteTableEnd()
{
	document.write('</td>');
	document.write('</tr>');
	document.write('</table>');
	document.write('</div>');
	ChangeWhiteTable();	
}
function ChangeWhiteTable()
{
	var obj;
	obj = document.getElementById("whitetable");	
	if (obj)
	{
		if (window.innerWidth)
			obj.style.height = window.innerHeight - 45;
		else	
			obj.style.height = document.body.clientHeight - 44;
	}	
}
// Print head bar
function PrintHeadLineBar(CloseAllLink)
{
	document.write('<table border="0" id="mainframe" cellpadding="0" cellspacing="0" width="100%" height="100%" style="border-collapse: collapse;">');
	document.write('<tr>');	
	document.write('<td valign="middle" colspan="3">'); 
   	document.write('<table border="0" cellpadding="0" cellspacing="0" width="100%" style="border-collapse: collapse;">');
	document.write('<tr>');
	document.write('<td valign="bottom" class="leftheaderfont" width="62" rowspan="2">');
	document.write('<a><div class="Logo" onClick=window.open(\'http://www.cisco.com/\');></div></a></td>');
	document.write('<td  class="upperheader">Small Business</td>');
	document.write('<td class="rightheaderfont"><b>'+UserName+'</b>');
	document.write('<img border="0" src="images/_blank.gif" width="15" height="1">');
	if (CloseAllLink == 1);
	else document.write('<a href="./cgi-bin/welcome.cgi?&status=logout">');
	document.write('Logout</a>');
	document.write('<img border="0" src="images/_blank.gif" width="15" height="1">');
	document.write('<a href="JavaScript:openTable(\'about.htm\')" title="About">About</a>');
	document.write('<img border="0" src="images/_blank.gif" width="15" height="1">');
	document.write('<a href="JavaScript:callHelp();">Help</a>');
	document.write('</td>');	
    	document.write('</tr>');
	document.write('<tr>');
	document.write('<td valign="top" class="downheader" colspan="2">'+ModelName+'&nbsp;&nbsp;'+ModelType+'</td>');
	document.write('</tr>');
  	document.write('</table>');
	document.write('</td>');	
	document.write('</tr>');	
}

function PrintBeginBlock(hastitle, titlestr, hasSpace)
{
	document.write('<tr><td>');
	document.write('<table id="'+titlestr+'" width="100%" class="block" border="0" cellpadding="0" cellspacing="0">');
   	document.write('<tr>');
	document.write('<td align="left" valign="top">');	
	if (hastitle == 1)
		PrintBarItem(titlestr, hasSpace);
}

function PrintEndBlock(hasLine, hasSpace, isSmall)
{
	document.write('</td>');
	document.write('</tr>');
	if (hasLine == 1)
		PrintLine(hasSpace, isSmall);
	document.write('</table>');
	document.write('</td>');
	document.write('</tr>');	
}

// Print Outline title 
function PrintBarItem(str, hasSpace)
{
	document.write('<font class="Grouptitle">');
	document.write(str);
	document.write('</font><br>');
	if (hasSpace == 1) document.write('<img border="0" src="images/_blank.gif" width="1" height="10">'); 
	else document.write('<img border="0" src="images/_blank.gif" width="1" height="15">');
}

// Print bar-line
function PrintLine(hasSpace, isSmall)
{
	if (isSmall == 1)
	{
		if (hasSpace == 1);
		else
			document.write('<tr><td colspan="2" height="5"></td></tr>');
		document.write('<tr><td colspan="2">');
		document.write('<hr class="line" align="center" size="1" width="100%">');        	
		document.write('</td>');
		document.write('</tr>');
	}
	else
	{
		if (hasSpace == 1);
		else
			document.write('<tr><td colspan="2" height="9"></td></tr>');
		document.write('<tr><td colspan="2">');
		document.write('<hr class="line" align="center" size="1" width="100%">');        	
		document.write('</td>');
		document.write('</tr>');
		document.write('<tr><td colspan="2"><img border="0" src="images/_blank.gif" width="1" height="3"></td></tr>');
	}
}

function PrintBeginContent()
{
	var obj;
	var i=0;
	var j=0;
	var application =navigator.appVersion;
	var appType ="";
	var appName = "";
	var appVersion = 0;
	var BgWidth = 0;
	var BgHeight = 0;
	var Tablewidth = 0;
	var ContentWidth = 0;
	var ContentHight = 0;
	var LimitWidth =0;
	var LimitHeight =0;
	
	if (window.innerWidth)
	{
		BgWidth = window.innerWidth;
		BgHeight = window.innerHeight;
		LimitWidth = 203;
		LimitHeight = 111;
	}	
	else
	{
		BgWidth = document.body.clientWidth;
		BgHeight = document.body.clientHeight; 
		LimitWidth = 204;
		LimitHeight = 109;
	}	
	
	if (BgWidth < 450)
	{
		BgWidth = 450;
		Tablewidth = 425;
	}	
	else if (BgWidth < screen.availWidth)
		Tablewidth = BgWidth - 30;
	else
		Tablewidth = screen.availWidth - 30;
	
	ContentWidth = BgWidth - LimitWidth - 18;
	if (ContentWidth < 700)
		ContentWidth = 700;
	
	if (HasExtendLogo == 1)
		ContentWidth += 18;

	if (BgHeight - LimitHeight < menuheigh)
		ContentHight = menuheigh;
	else
		ContentHight = BgHeight - LimitHeight;
		
	obj = document.getElementById("mainframe");	
	if (obj)
		obj.style.width = Tablewidth;		
		
	obj = document.getElementById("bgimg");	
	if (obj)
	{
		obj.style.width = BgWidth;
		obj.style.height = BgHeight;	
	}	
		
	appType = application.split(";");
	appName = appType[1].substring(1, 5);
	appVersion = parseInt(appType[1].substring(6, appType[1].length),10);
	if (appName == "MSIE" && appVersion < 7)
		document.write('<td valign="top" class="contentframe spacificbg"');
	else
		document.write('<td valign="top" class="contentframe contentbg"');	
	document.write(' width="90%">');
	document.write('<div id="content_height" style="overflow-x:auto;overflow-y:auto;width:'+(BgWidth - LimitWidth)+'px;height:'+ContentHight+'px">');
	document.write('<table id="content_block" width="'+ContentWidth+'">');
	document.write('<tr>');
	document.write('<td>');
	document.write('<table width="100%" border="0" style="'+Tablestyle+'">');
	document.write('<tr>');		
	document.write('<td class="Contenttitle">');
	document.write(Content_title);
	document.write('</td>');
	if (HasExtendLogo == 1)
	{
		document.write('<td style="text-align:right ;position:relative;padding-top:0px; padding-right:0px;">');
		document.write('<img src="images/tlogo.gif" title="TREND MIRO">');
		document.write('<img border="0" src="images/_blank.gif" width="10">');
		document.write('</td>');
	}
	document.write('</tr>');
	document.write('</table>');
	document.write('</td>');
   	document.write('</tr>');
	document.title = "Cisco " + ModelName + " Configuration Utility";
}

function PrintEndContent()
{
	document.write('</table>');
	document.write('<table width="'+NormallLayoutwidth+'" border="0">');
	document.write('<tr>');
	document.write('<td height="5"></td>');
   	document.write('</tr>');	
	document.write('<tr><td style="padding-top:0px; padding-bottom:0px; padding-left:10px;">');
	PrintButton();
	document.write('</td></tr>');
	document.write('<tr>');
	document.write('<td height="8"></td>');
   	document.write('</tr>');		
	document.write('</table>');
	document.write('</div>');
	document.write('</td>');
	document.write('</form>');
    document.write('</tr>');	
}

function PrintEndPopwindow()
{
	document.write('<table width="100%" border="0" cellpadding="0">');
	document.write('<tr><td align="left" style="padding-top:10px;margin-bottom:0px;">');
	PrintButton();	
	document.write('</td></tr>');
	if (window.innerHeight);
	else
	{
		document.write('<tr>');
		document.write('<td height="8" colspan="2"></td>');
   		document.write('</tr>');	
	}
	document.write('</table>');
}
function PrintCloseButton(HelpPage)
{
	document.write('<div align="center">');
	if (HelpPage)
		PrintSTButton("CloseWindow", "Close", "closeWindow()", 1, "", 70);
	else	
		PrintSTButton("CloseWindow", "Close", "closeWindow()", 0, "", 70);
	document.write('</div>');
}
function PrintRight()
{
}

function PrintBottom()
{
	document.write('<tr>');	
	document.write('<td colspan="3" height="8"></td>');
	document.write('</tr>');
	document.write('<tr>');	
	document.write('<td valign="bottom" class="bottomfont" colspan="3" height="12">&copy; 2010 Cisco Systems, Inc. All rights reserved.</td>');
	document.write('</tr>');	
  	document.write('</table>');
}

function PrintButton()
{
	switch(ButtonType)
	{
		case 'Refresh':
			PrintRefreshButton(Reloadlink);
			break;
		case 'RefreshClose':	
			PrintRefreshCloseButton(Reloadlink);
			break;				
		case 'OkCancel':
			PrintOKCancelButton(Savelink, Cancellink);
			break;
		case 'OkCancelClose':	
			PrintOKCancelCloseButton(Savelink, Cancellink);
			break;	
		case 'BackOkCancel':
			PrintBackOKCancelButton(Backlink, Savelink, Cancellink);
			break;	
		case 'ShowTableOkCancel':
			PrintShowTableOKCancelButton(Tablelink, Savelink, Cancellink);
			break;		
		default:
			break;
	}
}
/****************************************************/
/*												      */	
/* 		           Submit Button            	  			      */
/*												      */	
/***************************************************/
function GetReload()
{
	var href = "javascript: location.";
	var UrlString = document.location.href;
	var fileindex = 0, pathindex = 0;
	var tmpurl = "";

	fileindex = UrlString.lastIndexOf(".htm", UrlString.length);
	if (fileindex != -1)
		pathindex = UrlString.lastIndexOf("/", fileindex);

	if (pathindex != -1)
		tmpurl = UrlString.substring(pathindex+1, fileindex);

	if (tmpurl)
		href += "href='"+tmpurl+".htm';";
	else	
		href += "reload();"

	return href;
}

function GetURLParam()
{
	var param = "";
	var UrlString = document.location.href;
	var fileindex = 0;

	fileindex = UrlString.lastIndexOf(".htm#", UrlString.length);
	if (fileindex != -1)
		param = UrlString.substring(fileindex+5, UrlString.length);
		
	return param;
}

function PrintRefreshButton(RefreshStr)
{
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" height="21">');
	document.write('<tr>'); 
	document.write("<td height=\"26\">");
	PrintSTButton("Refresh", "Refresh", "javascript: location.reload();", "", "", 75);
	document.write("</td>");
	document.write('</tr>');
	document.write('</table>');
}

function PrintRefreshCloseButton(RefreshStr)
{
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" height="21">');
	document.write('<tr>'); 
	document.write("<td height=\"26\">");
	PrintSTButton("Refresh", "Refresh", "javascript: location.reload();", "", "", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	PrintSTButton("Close", "Close", "javascript: closeWindow();", "","", 75);
	document.write("</td>"); 
	document.write('</tr>');
	document.write('</table>');
}

function PrintOKCancelButton(YesStr, CancelStr)
{
	var Savelink=YesStr;
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" style="'+Tablestyle+'">');
	document.write('<tr>'); 
	document.write("<td height=\"26\">");
	PrintSTButton("Save", "Save", "chAll_Words();", "", "submit", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	if (CancelStr  != "")
		PrintSTButton("Cancel", "Cancel", CancelStr, "", "", 75);
	else	
		PrintSTButton("Cancel", "Cancel", GetReload(), "", "", 75);
	document.write("</td>");
	document.write('</tr>');
	document.write('</table>');
}
function PrintOKCancelCloseButton(YesStr)
{
	var Savelink=YesStr;
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" height="21">');
	document.write('<tr>'); 
	document.write("<td height=\"26\">");
	PrintSTButton("OK", "OK", "chAll_Words();", "", "submit", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	document.write("<td>");
	PrintSTButton("Cancel", "Cancel", GetReload(), "", "", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	PrintSTButton("Close", "Close", "javascript: closeWindow();", "", "", 75);
	document.write("</td>"); 
	document.write('</tr>');
	document.write('</table>');
}
function PrintBackOKCancelButton(BackStr,YesStr, CancelStr)
{
	var Savelink=YesStr;
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" height="21">');
	document.write('<tr>');
	/*
	document.write("<td height=\"26\">");
	if (BackStr  == "")
		PrintSTButton("Refresh", "Refresh", "javascript: location.reload();", "", "", 75);
	else	
		PrintSTButton("Back", "Back", "javascript:location.href='"+BackStr+"'", "", "", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	*/
	document.write("<td>");	
	document.write("<td>");
	PrintSTButton("Save", "Save", "chAll_Words();", "", "submit", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	if (CancelStr  != "")
		PrintSTButton("Cancel", "Cancel", CancelStr, "", "", 75);
	else	
		PrintSTButton("Cancel", "Cancel", "javascript:location.href='"+BackStr+"'", "", "", 75);
	document.write("</td>");
	document.write('</tr>');
	document.write('</table>');
}

function PrintShowTableOKCancelButton(TableStr,YesStr, CancelStr)
{
	document.write('<table border="0" cellpadding="0" cellspacing="0"  valign="bottom" id="AutoNumber15" height="21">');
	document.write('<tr>'); 
	document.write("<td height=\"26\">");
	if (TableStr != "")
		PrintSTButton("View", "View", "javascript:"+TableStr, "", "", 75);
	else
		PrintSTButton("Refresh", "Refresh", GetReload(), "", "", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");	
	document.write("<td>");
	PrintSTButton("Save", "Save", "chAll_Words();", "", "submit", 75);
	document.write("</td>") 
	document.write('<td width="5"></td>');
	document.write("<td>");
	if (CancelStr  != "")
		PrintSTButton("Cancel", "Cancel", CancelStr, "", "", 75);
	else	
		PrintSTButton("Cancel", "Cancel", GetReload(), "", "", 75);
	document.write("</td>");
	document.write('</tr>');
	document.write('</table>');
}
function PrintPageButton(Type, clickfun, status)
{
	var normal_graph = "_blank.gif";
	var hover_graph = "_blank.gif";
	var press_graph = "_blank.gif";
	
	switch (Type)
	{
		case "first":
			normal_graph = "nb.gif";
			hover_graph = "hb.gif";
			press_graph = "pb.gif";
			break;
		case "previous":
			normal_graph = "nl.gif";
			hover_graph = "hl.gif";
			press_graph = "pl.gif";
			break;	
		case "next":
			normal_graph = "nr.gif";
			hover_graph = "hr.gif";
			press_graph = "pr.gif";
			break;
		case "end":
			normal_graph = "nn.gif";
			hover_graph = "hn.gif";
			press_graph = "pn.gif";
			break;	
		default:
			break;
	}
	document.write('<img border="1" src="images/'+normal_graph+'" width="16" height="16" style="border-color:#53636A"');
	if (status && status == "disabled") document.write('>');
	else
	{
		document.write(' onmouseover="this.src=\'images/'+hover_graph+'\';this.style.borderColor=\'#1FA0D5\'"');
		document.write(' onmouseout="this.src=\'images/'+normal_graph+'\'; this.style.borderColor=\'#53636A\'"');
		document.write(' onmousedown="this.src=\'images/'+press_graph+'\';this.style.cursor=\'pointer\';this.style.color=\'#1FA0D5\';\"');
		document.write(' onmouseup="this.src=\'images/'+normal_graph+'\'; this.style.borderColor=\'#53636A\'"');
		document.write(' onClick="'+clickfun+'" >');
	}
}

function PrintSTButton(Name, StrValue, ClickFun, haspath, type, fixedwidth)
{	
	if (type && type == "reset")
		document.write('<input type="reset" class="STbutton" name="'+Name+'" id="'+Name+'" value="'+StrValue+'"');
	else
		document.write('<input type="button" class="STbutton" name="'+Name+'" id="'+Name+'" value="'+StrValue+'"');
	document.write(' onmouseover="ChangeButtonStyle(this,\'mouseover\'');
	if (haspath)	 document.write(', '+haspath+')"');
	else	 document.write(')"');		
	document.write(' onmouseout="ChangeButtonStyle(this,\'mouseout\'');
	if (haspath)	 document.write(', '+haspath+')"');
	else	 document.write(')"');	
	document.write(' onmousedown="ChangeButtonStyle(this,\'mousedown\'');
	if (haspath)	 document.write(','+haspath+')"');
	else	 document.write(')"');	
	document.write(' onmouseup="ChangeButtonStyle(this,\'mouseup\'');
	if (haspath)	 document.write(','+haspath+');"');
	else	 document.write(')"');	
	if (ClickFun) document.write(' onClick="'+ClickFun+'"');	
	document.write(' style="');
	if (fixedwidth)	
		document.write('width:'+fixedwidth+'px;');
	if (type && type == "submit")
		document.write('border-width:2px;border-style:solid;');
	document.write('" >');	
}
function ChangeButtonStyle(obj, EventType, haspath)
{
	if(EventType == 'mouseover')
	{
		obj.style.color ="#000";
		obj.style.borderColor ="#1FA0D5";
		if (haspath) obj.style.background ="url('../images/BTN1.gif')";
		else obj.style.background ="url('images/BTN1.gif')";
	}
	else if(EventType == 'mouseout')
	{
		window.status = "";
		obj.style.color ="#000";
		obj.style.borderColor ="#53636A";
		if (haspath) obj.style.background ="url('../images/BTN0.gif')";
		else obj.style.background ="url('images/BTN0.gif')";
	}
	else if(EventType == 'mousedown')
	{
		obj.style.color ="#026c99";
		obj.style.borderColor ="#1fa0d5";
		if (haspath) obj.style.background ="url('../images/BTN3.gif')";
		else obj.style.background ="url('images/BTN3.gif')";
	}
	else if(EventType == 'mouseup')
	{
		obj.style.color ="#000";
		obj.style.borderColor ="#1FA0D5";
		if (haspath) obj.style.background ="url('../images/BTN1.gif')";
		else obj.style.background ="url('images/BTN1.gif')";
	}	
}	
function SetButtonStatus(Name, StatusType)
{
	var obj = document.getElementById(Name);
	if (StatusType == 'disabled')
	{
		obj.disabled = true;
		obj.style.borderColor ="#BBC2C5";
		obj.style.background ="url('images/BTN4.gif')";
		obj.style.visibility ="visible";
		obj.style.color= "#8E8E8E";
		obj.style.cursor="default";
	}
	else if (StatusType == 'hidden')
	{
		obj.disabled = false;
		obj.style.borderColor ="#BBC2C5";
		obj.style.background ="url('images/BTN4.gif')";
		obj.style.visibility ="hidden";
		obj.style.color="#8E8E8E";
	}	
	else
	{
		obj.disabled = false;
		obj.style.borderColor ="#53636A";
		obj.style.background ="url('images/BTN0.gif')";
		obj.style.visibility ="visible";
		obj.style.color = "#000";
		obj.style.cursor="pointer";
	}	
}

//=============== End of Common function ================*/

function closeWindow() 
{
	window.close();
	if (window.ActiveXObject);
	else window.location.href = "index.htm";
}
function style_display_on()
{
	if (window.ActiveXObject)		// For IE
		return "block";	
	else if (window.XMLHttpRequest)		// For Mozilla, Firefox 	
		return "table";
}

function MM_preloadImages() { //v3.0
   var d=document; 
   if(d.images)
   { 
   	if(!d.MM_p) 
   		d.MM_p=new Array();
   	var i,j=d.MM_p.length,a=MM_preloadImages.arguments; 
   	for(i=0; i<a.length; i++)
   	{
   		if (a[i].indexOf("#")!=0)
   		{ 
   			d.MM_p[j]=new Image; 
   			d.MM_p[j++].src=a[i];
   		}
   	}
   }	
}

function MM_swapImgRestore() { //v3.0
    var i,x,a=document.MM_sr; 
    for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) 
    	x.src=x.oSrc;
}

function MM_swapImage() { //v3.0
    var i,j=0,x,a=MM_swapImage.arguments; 
    document.MM_sr=new Array; 
    for(i=0;i<(a.length-2);i+=3)
    {
    	if ((x=MM_findObj(a[i]))!=null)
    	{
    		document.MM_sr[j++]=x; 
    		if(!x.oSrc) 
    			x.oSrc=x.src; 
    		x.src=a[i+2];
    	}
    }	
}

function MM_reloadPage(init) {  //reloads the window if Nav4 resized
  if (init==true) with (navigator) {if ((appName=="Netscape")&&(parseInt(appVersion)==4)) {
    document.MM_pgW=innerWidth; document.MM_pgH=innerHeight; onresize=MM_reloadPage; }}
  else if (innerWidth!=document.MM_pgW || innerHeight!=document.MM_pgH) location.reload();
}

MM_reloadPage(true);


function MM_findObj(n, d) { //v4.0
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && document.getElementById) x=document.getElementById(n); return x;
}

function MM_showHideLayers() { //v3.0

  var i,p,v,obj,args=MM_showHideLayers.arguments;

  for (i=0; i<(args.length-2); i+=3) if ((obj=MM_findObj(args[i]))!=null) { v=args[i+2];

    if (obj.style) { obj=obj.style; v=(v=='show')?'visible':(v='hide')?'hidden':v; }

    obj.visibility=v; }

}

function sTrim(I)
{

  var p=0;
  var q=0;
  var sL;
  var sTemp;

  sL=I.value.length;
  sTemp=I.value; 

  if (sL>0)
  {
      while(sTemp.charAt(p)==' ') p++;
	  while(sTemp.charAt((sL-1)-q)==' ') q++;
  }

  if (p==sL)
  	I.value="";
  else
	I.value=sTemp.substring(p,sL-q);
}

function notnull(Input, AlrtStr)	// check input string
{
    var str = Input.value;
	
    if (Input.value=="")
    {
    	 if (AlrtStr != "")
        	alert(AlrtStr);
        Input.value=Input.defaultValue;
        return;
    }

    str = replacechar(str, " ", "_");
    str = replacechar(str, ",", "");
    str = replacechar(str, ";", "");
    Input.value = str;	
}

function validatenum(Input, up, down, AlrtStr)	//check input number
{
    if (Input.value =="")
	return -1;	

    if (isNaN(Input.value) == true)
    {
        Input.value=Input.defaultValue;
	 return -1;	
    }
	
    var d;
    d=parseInt(Input.value,10);
    if (!(d<up && d>= down))
    {
        alert(AlrtStr);
        Input.value=Input.defaultValue;	
        return -1;
    }
    Input.value=d;	
    return 1;	
}

function replacechar(Input, checkstr, replacestr)
{
	var p, q;
	var tempstring, tempname, tempspace="";
	
	if (Input != "")
	{
		tempstring = Input;
		p = tempstring.indexOf(checkstr);
		q = tempstring.length;
		
		if (p<0)
		{
			return Input;
		}
		else if (p==0)
			tempstring = tempstring.substring(p+1,q);
			
		while(1)
		{
			q = tempstring.length;
			p = tempstring.indexOf(checkstr);
			if (p >= 0)
			{
				tempname = tempstring;
				tempstring = tempname.substring(p+1,q);
				if (p < q)
				{
					if (p < q -1)
						tempspace += tempname.substring(0,p) + replacestr;
					else
						tempspace += tempname.substring(0,p);
				}	
			}
			else
				break;
		}
		return tempspace+tempstring;
	}
	else
		return tempspace;
}

function Check_User_Input(e)
{
	var keynum;
	
	if(window.event) // IE
		keynum = e.keyCode;
	else if(e.which) // Netscape/Firefox/Opera
		keynum = e.which;

	if (keynum < 47 || keynum > 57)
	{
		if (keynum != 8     //Backspace
		&& keynum != 13   //Enter 
		&& keynum != 46   //. Key
		&& keynum != 58)  //: Key
		{
			return false;
		}	
	}	

	return true;
}
function Check_Special_Words(word)
{

	var iChars = "'\"\\";
	var iChars2 = " ";
	
	for (var i = 0; i < word.value.length; i++)
	{
		if (iChars.indexOf(word.value.charAt(i)) != -1)
		{
			alert(aCheckName);
			return -1;
		}
	}
	return 1;
}

function Check_illegal_Words(word)
{
	var iChars = "'\"\\";
	
	for (var i = 0; i < word.value.length; i++)
	{
		if (iChars.indexOf(word.value.charAt(i)) != -1)
		{
			alert(aCheckName);
			return -1;
		}
	}
	return 0;
}

function chAll_Words()
{
	var objs1 = document.getElementsByTagName("input"); 
	if (objs1)
	for(var i=0;i < objs1.length; i++)  
	{  
		if(objs1[i].type == "text" && objs1[i].readOnly == false && objs1[i].disabled == false) 
		{
			sTrim(objs1[i]);
			if(objs1[i].value.length>0)
			if(Check_Special_Words(objs1[i]) < 0)
			{						
				objs1[i].select();
				return;
			}
		} 
	}
	var objs2 = document.getElementsByTagName("option"); 
	if (objs2)
	for(var i=0;i < objs2.length; i++)  
	{  
		if(Check_illegal_Words(objs2[i]) <0)
		{
			objs2[i].selected=true;
			return;
		}
	}
	eval(Savelink);
}

function checkDate(I)
{
	if(I == null)
		return -1;
	if(I.value == "")
	{
		alert("Please input Date!");
		I.value=I.defaultValue;
		return -1;
	}
	if(Check_Special_Words_j(I) < 0)
	{
		return -1;
	}
	
	var strIndex, strLen;	
	var nTmp;
	var tmpString;
	var rightString = I.value;
	var ts=new tmpWord(6);
	
	// have two '-'
	
	strLen = rightString.length;
	strIndex = rightString.indexOf(".");
	if(strIndex == 0 || strIndex == -1)
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	ts[1] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;
	
	strLen = rightString.length;
	strIndex = rightString.indexOf(".");
	if(strIndex == 0 || strIndex == -1)
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	ts[2] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;
	
	if(rightString =="")
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	strLen = rightString.length;
	strIndex = rightString.indexOf(".");
	if(strIndex > 0)
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	ts[3] = rightString;
	
	if (!(0<=ts[1] && ts[1]<=9999))
	{
		alert(aYearCheck);
		return -1;
	}
	
	if (!(ts[2]<=12 && ts[2]>=1))
	{
		alert(aMonthCheck);
	  return -1;
	}
	
	if (ts[2]==2)
	{
		if (ts[1]%400 == 0 ||(ts[1]%4 == 0 && ts[1]%100 == 0))//Freburary leap year 29 days
		{
			if (!(ts[3]<=29 && ts[3]>=1))
			{
				alert(aDay29Check);
				return -1;
			}
		}
		else
		{
			if (!(ts[3]<=28 && ts[3]>=1))
			{
				alert(aDay28Check);
				return -1;
			}
		}
	}	
	else if (ts[2]==4 || ts[2]==6 || ts[2]==9 || ts[2]==11)
	{
		if (!(ts[3]<=30 && ts[3]>=1))
		{
			alert(aDay30Check);
			return -1;
		}
	}
	else
	{
		if (!(ts[3]<=31 && ts[3]>=1))
		{
			alert(aDayCheck);
			return -1;
		}
	}
	
	if (ts[2].length==1) 
  {
  	nTmp=ts[2];
  	ts[2]="0"+nTmp;
  }
  if (ts[3].length==1) 
  {
  	nTmp=ts[3];
  	ts[3]="0"+nTmp;
  }
  
	I.value = ts[1] +"." +ts[2] +"." +ts[3];
	return 1;
}

function FilterDate(DateStr)
{
	var index = 0;
	var CompareChar=new Array("A", "D", "F", "J", "M", "N", "S", "O");
	for (var i=0; i<8;i++)
	{
		index = DateStr.indexOf(CompareChar[i],0);
		if (index < 0)
			continue;

		if (index > 0)
			return DateStr.substring(index, DateStr.length);
		else
			return DateStr;
	}
}

function checkDate2(I, year)
{
	if(I == null)
		return -1;
	if(I.value == "")
	{
		alert("Please input Date!");
		I.value=I.defaultValue;
		return -1;
	}
	if(Check_Special_Words_j(I) < 0)
	{
		return -1;
	}
	
	var strIndex, strLen;	
	var nTmp;
	var tmpString;
	var rightString = I.value;
	var ts=new tmpWord(6);

	ts[1] = year; 
	strLen = rightString.length;
	strIndex = rightString.indexOf(".");
	if(strIndex == 0 || strIndex == -1)
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	ts[2] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;	
	
	if(rightString =="")
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	strLen = rightString.length;
	strIndex = rightString.indexOf(".");
	if(strIndex > 0)
	{
		alert("Please input date setting with correct format!");
		return -1;
	}
	ts[3] = rightString;
	/*
	if (!(0<=ts[1] && ts[1]<=9999))
	{
		alert(aYearCheck);
		return -1;
	}
	*/
	if (!(ts[2]<=12 && ts[2]>=1))
	{
		alert(aMonthCheck);
	  return -1;
	}
	
	if (ts[2]==2)
	{
		if (ts[1]%400 == 0 ||(ts[1]%4 == 0 && ts[1]%100 == 0))//Freburary leap year 29 days
		{
			if (!(ts[3]<=29 && ts[3]>=1))
			{
				alert(aDay29Check);
				return -1;
			}
		}
		else
		{
			if (!(ts[3]<=28 && ts[3]>=1))
			{
				alert(aDay28Check);
				return -1;
			}
		}
	}	
	else if (ts[2]==4 || ts[2]==6 || ts[2]==9 || ts[2]==11)
	{
		if (!(ts[3]<=30 && ts[3]>=1))
		{
			alert(aDay30Check);
			return -1;
		}
	}
	else
	{
		if (!(ts[3]<=31 && ts[3]>=1))
		{
			alert(aDayCheck);
			return -1;
		}
	}
	
	if (ts[2].length==1) 
  {
  	nTmp=ts[2];
  	ts[2]="0"+nTmp;
  }
  if (ts[3].length==1) 
  {
  	nTmp=ts[3];
  	ts[3]="0"+nTmp;
  }
  
	I.value = ts[2] +"." +ts[3];
	return 1;
}

function checkDateRange2(StartDate, EndDate, year)
{
	if(checkDate2(StartDate, year) < 0)
	{
		return -1;		
	}
	if(checkDate2(EndDate, year) < 0)
	{
		return -1;		
	}
	
	var strIndex, strLen;
	var tmpString;
	var rightString1 = StartDate.value;
	var rightString2 = EndDate.value;
	var ts1=new tmpWord(6);
	var ts2=new tmpWord(6);

	strLen = rightString1.length;
	strIndex = rightString1.indexOf(".");
	ts1[2] = rightString1.substring(0, strIndex);
	tmpString = rightString1.substring(strIndex +1, strLen);
	rightString1 = tmpString;	
	ts1[3] = rightString1;

	strLen = rightString2.length;
	strIndex = rightString2.indexOf(".");
	ts2[2] = rightString2.substring(0, strIndex);
	tmpString = rightString2.substring(strIndex +1, strLen);
	rightString2 = tmpString;
	ts2[3] = rightString2;
	
	var aStartLargerThanEnd ="Start Date must be earlier than End Date!";
	var year1=0, year2=0;
	var month1 = parseInt(ts1[2], 10);
	var month2 = parseInt(ts2[2], 10);
	var day1 = parseInt(ts1[3], 10);
	var day2 = parseInt(ts2[3], 10);
	/*purpose     : 0012654 author : Ben date : 2010-06-22*/
	/*description : Daylight Savings Time Dates should not be restricted
	if (month1 != 3 || (month1 == 3 && day1 > 14))
	{
		alert("The StartDate should be the 2nd Sunday in March!");
		return -1;
	}
	else if (month2 != 11 || (month2 == 11 && day1 > 7))
	{
		alert("The EndDate should be the 1st Sunday in November!");
		return -1;
	}
	*/	
	if (year1 >year2 )
	{
		alert(aStartLargerThanEnd);
		return -1;
	}
	else if(year1 == year2)
	{
		if(month1 >month2)
		{
			alert(aStartLargerThanEnd);
			return -1;
		}
		else if(month1 ==month2)
		{
			if(day1 >day2)
			{
				alert(aStartLargerThanEnd);
				return -1;				
			}
		}
	}
	
	return 1;
}

function checkTime(I)
{
	if(I == null)
		return -1;
	if(I.value == "")
	{
		alert("Please input Time!");
		I.value=I.defaultValue;
		return -1;
	}
	if(Check_Special_Words_jj(I) < 0)
	{
		return -1;
	}
	var strIndex, strLen;	
	var nTmp;
	var tmpString;
	var rightString = I.value;
	var ts=new tmpWord(6);
	var aFormatOfTime = "Please input time setting with correct format!";
	
	// have two '-'
	
	strLen = rightString.length;
	strIndex = rightString.indexOf(":");
	if(strIndex == 0 || strIndex == -1)
	{
		alert(aFormatOfTime);
		return -1;
	}
	ts[1] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;
	
	strLen = rightString.length;
	strIndex = rightString.indexOf(":");
	if(strIndex == 0 || strIndex == -1)
	{
		alert(aFormatOfTime);
		return -1;
	}
	ts[2] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;
	
	
	if(rightString =="")
	{
		alert(aFormatOfTime);
		return -1;
	}
	strLen = rightString.length;
	strIndex = rightString.indexOf(":");
	if(strIndex > 0)
	{
		alert(aFormatOfTime);
		return -1;
	}
	ts[3] = rightString;	
	
	if (!(ts[1]<=23 && ts[1]>=0))
	{
		alert(aHourCheck);
		return -1; 
	}
	if (!(ts[2]<=59 && ts[2]>=0))
	{
		alert(aMinuteCheck);
		return -1; 
	}
	if (!(ts[3]<=59 && ts[3]>=0))
	{
		alert(aSecondCheck);
		return -1; 
	}
	if (ts[1].length==1) 
	{
		nTmp=ts[1];
		ts[1]="0"+nTmp;
	}
	if (ts[2].length==1) 
	{
		nTmp=ts[2];
		ts[2]="0"+nTmp;
	}
	if (ts[3].length==1) 
	{
		nTmp=ts[3];
		ts[3]="0"+nTmp;
	}	
	
	I.value = ts[1] +":" +ts[2] +":" +ts[3];
	return 1;
}

/***************************************************
*** check time ***
***************************************************/
function Check_Special_Words_j(word)
{
	var iChars = "'\"\\";
	/*var iChars2 = " ";*/
	
	for (var i = 0; i < word.value.length; i++)
	{
		if (iChars.indexOf(word.value.charAt(i)) != -1)
		{
			alert(aCheckName);
			return -1;
		}
	}
	return 1;
}

function Check_Special_Words_jj(word)
{
	var iChars = "'\"\\";
	/*var iChars2 = " ";*/
	
	for (var i = 0; i < word.value.length; i++)
	{
		if (iChars.indexOf(word.value.charAt(i)) != -1)
		{
			alert(aCheckName);
			return -1;
		}
	}
	return 1;
}

function checkHourMinute(I)
{
	if(I == null)
		return -1;
	if(I.value == "")
	{
		alert("Please input Time!");
		I.value=I.defaultValue;
		return -1;
	}
	if(Check_Special_Words_jj(I) < 0)
	{
		return -1;
	}
	var strIndex, strLen;	
	var nTmp;
	var tmpString;
	var rightString = I.value;
	var ts=new tmpWord(6);
	
	// have a ':'
	
	strLen = rightString.length;
	strIndex = rightString.indexOf(":");
	if(strIndex == 0 || strIndex == -1)
	{
		alert("Please input time setting with correct format!");
		return -1;
	}
	ts[1] = rightString.substring(0, strIndex);
	tmpString = rightString.substring(strIndex +1, strLen);
	rightString = tmpString;
		
	if(rightString =="")
	{
		alert("Please input time setting with correct format!");
		return -1;
	}
	strLen = rightString.length;
	strIndex = rightString.indexOf("-");
	if(strIndex > 0)
	{
		alert("Please input time setting with correct format!");
		return -1;
	}
	ts[2] = rightString;	
	
	if (!(ts[1]<=23 && ts[1]>=0))
	  {
		alert(aHourCheck);
		return -1; 
	  }
	  if (!(ts[2]<=59 && ts[2]>=0))
	  {
		alert(aMinuteCheck);
		return -1; 
	  }
	  if (ts[1].length==1) 
	  {
	  	nTmp=ts[1];
	  	ts[1]="0"+nTmp;
	  }
	  if (ts[2].length==1) 
	  {
	  	nTmp=ts[2];
	  	ts[2]="0"+nTmp;
	  }
	
	I.value = ts[1] +":" +ts[2];
	return 1;
}

function checkHourMinuteRange(StartTime, EndTime)
{	
	if(checkHourMinute(StartTime) < 0)
	{
		return -1;		
	}
	if(checkHourMinute(EndTime) < 0)
	{
		return -1;		
	}
	
	var strIndex, strLen;
	var tmpString;
	var rightString1 = StartTime.value;
	var rightString2 = EndTime.value;
	var ts1=new tmpWord(6);
	var ts2=new tmpWord(6);
	
	strLen = rightString1.length;
	strIndex = rightString1.indexOf(":");
	ts1[1] = rightString1.substring(0, strIndex);
	tmpString = rightString1.substring(strIndex +1, strLen);
	rightString1 = tmpString;
	ts1[2] = rightString1;
	
	strLen = rightString2.length;
	strIndex = rightString2.indexOf(":");
	ts2[1] = rightString2.substring(0, strIndex);
	tmpString = rightString2.substring(strIndex +1, strLen);
	rightString2 = tmpString;
	ts2[2] = rightString2;
	
	var aStartLargerThanEnd="Start Time must be earlier than End Time!";
	var hour1 = parseInt(ts1[1], 10);
	var hour2 = parseInt(ts2[1], 10);
	var minute1 = parseInt(ts1[2], 10);
	var minute2 = parseInt(ts2[2], 10);
	if (hour1 >hour2)
	{
		alert(aStartLargerThanEnd);
		return -1;
	}
	else if(hour1 == hour2)
	{
		if(minute1 >= minute2)
		{
			alert(aStartLargerThanEnd);
			return -1;
		}
	}	
}

function hours_minutes_Check(I,mod_1,mod_2)
{
  var d;
  var single;
  if(I.value.length == 0)
   return ;
  var q = I.value.indexOf(':');
 
  if( q == -1 )
  {
  	alert("the time should be like:\'hh:mm\'");
  	I.value="";
  	I.select();
  	return	
  }
  else
  {
  	var hours= I.value.substring(0,q);
  	var minutes= I.value.substring(q+1,I.value.length);
  	
  	if(minutes.indexOf(':')!= -1 )
  	{
  		I.select();
  		I.value="";
  		return ;
  	}
  	 if (isNaN(hours) == true)
  	{
  		I.select();
  		I.value="";
  		return ;
  	}
  	 if (isNaN(minutes) == true)
  	{
  		I.select();
  		I.value="";
  		return ;
  	}
  
 
  if(mod_1 == "h")
  {
  	 d=parseInt(hours,10);
  	 if (!(d<24 && d>=0))
    {
      alert('The value (Hour) is out of range [0~23] !');
    I.select();
    I.value="";
	  return; 
    }
		hours=d;
    if (hours.toString().length==1) 
    {
      single=hours;
      hours="0"+single;
    }  	
  }
   if(mod_2 == "m")
  {
  	d=parseInt(minutes,10);
 
  	if (!(d<60 && d>=0))
    {
      alert('The value (Minute) is out of range [0~59] !');
    I.select();
    I.value="";
	  return;
    }
	  minutes = d;
	 
    if (minutes.toString().length==1) 
    {
      single=minutes;
      minutes="0"+single;
     
    }  	
  }
   I.value = hours.toString()+":"+ minutes.toString() ; 
   return ;
  } 
}
/**************************** Multi-selection operator ************************************/
function UpSel(s,up)
{
	var z;  
	var k;
	var i,j;
	if (s.length > 0)
	{
		tmp=new tmpWord(s.length);
		tmpChanged=new tmpWord(s.length); 
		opvtmp=new tmpWord(s.length);
		opvtmpChanged=new tmpWord(s.length);
		for (i=0; i < s.length; i++)
		{
			tmp[i+1]=s.options[i].text; 
			opvtmp[i+1]=s.options[i].value;	  
		}	
		if(up==1)
		{
			for (i=0; i < s.length; i++)
			{
				if (s.options[i].selected==true)
				{ 
					tmp[0]=s.options[i-up].text;
					opvtmp[0]=s.options[i-up].value;	
					s.options[i-up].value=s.options[i].value;		  		
					s.options[i-up].text=s.options[i].text;
					s.options[i].value=opvtmp[0];
					s.options[i].text=tmp[0];
					s.options[i].selected=false;
					s.options[i-up].selected=true;		
				}
			}
		}
		if(up==-1)
		{
			for (i=s.length-1; i > -1; i--)
			{
				if (s.options[i].selected==true)
				{ 
					tmp[0]=s.options[i-up].text;
					opvtmp[0]=s.options[i-up].value;	
					s.options[i-up].value=s.options[i].value;		  		
					s.options[i-up].text=s.options[i].text;
					s.options[i].value=opvtmp[0];
					s.options[i].text=tmp[0];
					s.options[i].selected=false;
					s.options[i-up].selected=true;		
				}
			}
		}
	}
  	SetButtonStatus("upRate", "disabled");
  	SetButtonStatus("downRate", "disabled");
	if(s.options[0].selected!=true)
  		SetButtonStatus("upRate", "");
	if(s.options[s.length-1].selected!=true)	
  		SetButtonStatus("downRate", "");
}	
function delSel(s,I)
{
	var z;  
	var k;

	if (s.length > 0)
	{
		tmp=new tmpWord(s.length);
		tmpChanged=new tmpWord(s.length); 
		opvtmp=new tmpWord(s.length);
		opvtmpChanged=new tmpWord(s.length); 

		for (var i=0; i < s.length; i++)
		{
			tmp[i+1]=s.options[i].text;
			opvtmp[i+1]=s.options[i].value;
		}	

		for (var i=0; i < s.length; i++)
		{
			if (s.options[i].selected==true)
			{ 
				s.options[i].text="";
				s.options[i].value="";
				tmp[i+1]=" ";
				opvtmp[i+1]=" ";		
				s.options[i].selected=false;	      
			}
		}
		k=1;
		z=0;
		for (var j=1; j<=s.length; j++) 
		{ 
			if (tmp[j]!=" ") 
			{
				tmpChanged[k]=tmp[j];
				opvtmpChanged[k]=opvtmp[j];
				k++;
			}
			else
			{
				z++;
			}
		}
		for (var i=0; i < s.length-z; i++)
		{
			  s.options[i].text=tmpChanged[i+1]; 
			  s.options[i].value=opvtmpChanged[i+1];  	 
		}
		s.length-=z;
	}
	clearContent(s.form,I); 
}

function exPosion(s)
{
	if (s.length > 0)
	{
		tmp=new tmpWord(s.length);
		tmpChanged=new tmpWord(s.length); 
		opvtmp=new tmpWord(s.length);
		opvtmpChanged=new tmpWord(s.length);
		 	
		for (var i=0; i < s.length; i++)
		{
			tmp[i+1]=s.options[i].text;
			opvtmp[i+1]=s.options[i].value;
		}	
			
		for (var i=0; i < s.length; i++)
		{
			s.options[i].text=tmp[s.length-i];
			s.options[i].value=opvtmp[s.length-i];
		}

		for (var i=0; i < s.length; i++)
		{
			tmp[i+1]=" ";
			opvtmp[i+1]=" ";	
		}
	}
}
function selAll(s)
{
	if (s.length>0)
	{
		//exPosion(s);
		for (var i=0; i < s.length; i++)
		s.options[i].selected=true;
	}
}

function tmpWord(n)
{
	this.length=n;
	for (var i=1; i<=n; i++)
		this[i]=0;
	return this;
}

/*********open table*******************/
var wservice_window=null;
var wstatus_window=null;
var wsetting_window=null;
var wabout_window=null;
var whelp_window=null;

function openTable(n)
{	
    switch (n)
    {

	case 'about.htm':
   	        if (wabout_window!=null) closeTable(wabout_window);
		 {
		 	if (TotalPortNumber == 16)
				wabout_window=window.open('016/about.htm', '','menubar=no, width=480, height=425');
			else if (TotalPortNumber == 6)
				wabout_window=window.open('042/about.htm', '','menubar=no, width=480, height=425');
			else
	    			wabout_window=window.open('082/about.htm', '','menubar=no, width=480, height=425');		
   	        }		
		 break;
	case 'Dhcp_table1.htm':
   	        if (wsetting_window!=null) closeTable(wsetting_window);
	    		wsetting_window=window.open(n, '','location=yes, resizable, menubar=no, scrollbars, width=680, height=600');		
		break;		 
	case 'sys_log.htm':
	case 'outgoing_log.htm':
	case 'incoming_log.htm':
	case 'Routing_table.htm':
	case 'UPnP_table.htm':
   	        if (wstatus_window!=null) closeTable(wstatus_window);
	    		wstatus_window=window.open(n, '','location=yes, resizable, menubar=no, scrollbars, width=820');		
		break;
			
    	default:
		if (wsetting_window!=null) closeTable(wsetting_window);
			wsetting_window=window.open(n, '','location=yes, resizable, menubar=no, scrollbars, width=820');		
	    	break;
    }	
}
function callHelp()
{	
	if (!helplink)
		return;

        if (whelp_window!=null) closeTable(whelp_window);
	 {
/*	 	
	 	if (TotalPortNumber == 16)
			whelp_window=window.open('016/'+helplink, '','location=yes, menubar=no, scrollbars=yes , width=640,height=600');
		else if (TotalPortNumber == 6)
			whelp_window=window.open('042/'+helplink, '','location=yes, menubar=no, scrollbars=yes , width=640,height=600');
		else
			whelp_window=window.open('082/'+helplink, '','location=yes, menubar=no, scrollbars=yes , width=640,height=600');
*/
		whelp_window=window.open('help/help_index.htm?url='+helplink, '','location=yes, menubar=no, scrollbars=yes , width=640,height=600');
        }	
}
function closeTable(wThis)
{
    if (wThis!=null)
    {
      wThis.close();
      wThis=null;
    }
}
function chLeave()
{
	 if (wabout_window!=null) closeTable(wabout_window);
	 //if (whelp_window!=null) closeTable(whelp_window);
}

/* 2004/08/18 Eric --> Network Check */
function IpToArray(V, n) // IP (or Mask) value [ex: 192.168.1.1], want to get IP index (1~4)
{
	var ip1,ip2,ip3,ip4;
	var p,q,rightString,tmpString;

	rightString=V;
/**/
	q=rightString.length;
	p=rightString.indexOf(".");
	ip1=rightString.substring(0,p); 
	tmpString=rightString;
	rightString=tmpString.substring(p+1,q); 
/*.*/

	q=rightString.length;
	p=rightString.indexOf(".");
	ip2=rightString.substring(0,p); 
	tmpString=rightString;
	rightString=tmpString.substring(p+1,q); 
/*.*/

	q=rightString.length;
	p=rightString.indexOf(".");
	ip3=rightString.substring(0,p); 
	tmpString=rightString;

/*.*/
	ip4=tmpString.substring(p+1,q); 

/*.*/
	if (n=="1") return ip1;
	if (n=="2") return ip2;
	if (n=="3") return ip3;
	if (n=="4") return ip4;	// get IP index (1~4)
}
function NetworkToArray(ip, mask, n) // IP value [ex: 192.168.1.1], Mask value [ex: 255.255.255.0], want to get NETWORK ID index (1~4)
{
    var ip1,ip2,ip3,ip4;
	var mask1,mask2,mask3,mask4;
    var network1,network2,network3,network4;
	
	ip1=IpToArray(ip, '1');
	ip2=IpToArray(ip, '2');
	ip3=IpToArray(ip, '3');
	ip4=IpToArray(ip, '4');
	
	mask1=IpToArray(mask, '1');
	mask2=IpToArray(mask, '2');
	mask3=IpToArray(mask, '3');
	mask4=IpToArray(mask, '4');
	
	network1 = ip1 & mask1; 
	network2 = ip2 & mask2;
	network3 = ip3 & mask3; 
	network4 = ip4 & mask4; 
	
	if (n=="1") return network1;
	if (n=="2") return network2;
	if (n=="3") return network3;
	if (n=="4") return network4; // get NETWORK ID index (1~4)
}
// 2004/11/30 Eric -->
function BroadcastToArray(ip, mask, n) // IP value [ex: 192.168.1.1], Mask value [ex: 255.255.255.0], want to get BROADCAST ID index (1~4)
{
    var ip1,ip2,ip3,ip4;
	var mask1,mask2,mask3,mask4;
    var network1,network2,network3,network4;
	
	ip1=IpToArray(ip, '1');
	ip2=IpToArray(ip, '2');
	ip3=IpToArray(ip, '3');
	ip4=IpToArray(ip, '4');
	
	mask1=IpToArray(mask, '1');
	mask2=IpToArray(mask, '2');
	mask3=IpToArray(mask, '3');
	mask4=IpToArray(mask, '4');
	
	network1 = ip1 | (255-mask1); 
	network2 = ip2 | (255-mask2);
	network3 = ip3 | (255-mask3); 
	network4 = ip4 | (255-mask4); 
	
	if (n=="1") return network1;
	if (n=="2") return network2;
	if (n=="3") return network3;
	if (n=="4") return network4; // get NETWORK ID index (1~4)
}
var aNetworkRangeCheck="IP Address is out of range "; 
function NetworkRangeCheck(I, ip, mask, n)
{
  var d;
  var netip_start, netip_end;

  d=parseInt(I.value,10);
  netip_start=eval(NetworkToArray(ip, mask, n));
  netip_end=eval(BroadcastToArray(ip, mask, n));
 
  if (n=="4")
  {
      	netip_start++;
      	netip_end--;
  }
  //if DMZ_host ip=0;don't  alert message
  if(!(I.name=="dmzAddr4"&&I.value==0))
  if (!(d<=netip_end && d>=netip_start)) 
  {
    alert(aNetworkRangeCheck+"["+netip_start+"~"+netip_end+"] !");
    I.value=I.defaultValue;
    return;   
  }
  I.value=d;  
  return;	
	
}
function NetworkRangeCheck1(I, ip, mask, n)
{
  var d;
  var netip_start, netip_end;

  d=parseInt(I.value,10);
  netip_start=eval(NetworkToArray(ip, mask, n));
  netip_end=eval(BroadcastToArray(ip, mask, n));
 
  if (n=="4")
  {
      	netip_start++;
      	netip_end--;
  }

  //if DMZ_host ip=0;don't  alert message
  if(!(I.name=="dmzAddr4"&&I.value==0))
  if (!(d<=netip_end && d>=netip_start)) 
  {
    alert(aNetworkRangeCheck+"["+netip_start+"~"+netip_end+"] !");
//    I.value=I.defaultValue;
    return -1;   
  }
  I.value=d;  
  return 1;	
	
}

function IPSaver(CheckIP,ip1,ip2,ip3,ip4)
{
	var ts=new tmpWord(4);
	var rightString;
	var tmpString;
	var i,p,q;
	tmpString=CheckIP.value;
	for(i=1;i<5;i++)
	{
		q=tmpString.length;
		p=tmpString.indexOf(".");
		if(i==4)
			ts[i]=tmpString;
		else
		{
			ts[i]=tmpString.substring(0,p);
			rightString=tmpString;
			tmpString=rightString.substring(p+1,q);
		}
	}
	ip1.value=ts[1];
	ip2.value=ts[2];
	ip3.value=ts[3];
	ip4.value=ts[4];

	return;
}

function IPCheck(CheckIP, Min, Max, AllowScope, ForbiddenIP, NoAlert, IPName)
{
	var i,j,p,q;
  	var tmpString;
	var rightString;
	var ts=new tmpWord(4);
	var ts2=new tmpWord(4);
	var ts3=new Array(254,252,248,240,224,192,128);
	var AlertName = "This IP";

	if (IPName)
		AlertName = IPName;

	if (!Max || isNaN(Max) == true)
		Max = 254;
	
	if (!Min || isNaN(Min) == true)
		Min = 0;	
	
	tmpString=CheckIP.value;
	for(i=1;i<5;i++)
	{
		q=tmpString.length;
		p=tmpString.indexOf(".");
		if(i==4)
		{
			ts[4]=tmpString.substring(0,p);
			rightString = "";
			rightString = tmpString.substring(p,q);
			if (ts[4] == "")
			{
				ts[4]=tmpString;
				rightString = "";
			}

			ts[4] = replacechar(ts[4], " ", "");
			if (isNaN(ts[4]) == true || ts[4].length ==0 )
			{
				if (NoAlert);
				else alert(AlertName+'\'s format is illegal! ');
				return -1;
			}
			
			if(ts[4] < Min || ts[4] > Max)
			{
				if (NoAlert);
				else alert(AlertName+'\'s last value is out of range! ['+Min+'~'+Max+'] ');
				return -1;
			}

			if (rightString.length > 0)
			{
				if (NoAlert);
				else alert('Please input '+AlertName+' with correct format!');
				return -1;
			}
		}
		else if(p<=0)
		{
			if (CheckIP.value == "") return 0;
			else
			{
				if (NoAlert);
				else alert('Please input '+AlertName+' with correct format!');
			}	
			return -1;
		}
		else
		{
			ts[i]=tmpString.substring(0,p);
			rightString=tmpString;
			tmpString=rightString.substring(p+1,q);
			ts[i] = replacechar(ts[i], " ", "");
		}
		
		if(isNaN(ts[i]) == true || ts[i]<0||ts[i]>255)
		{
			if (NoAlert);
			else alert(AlertName+'\'s format is illegal! ');
			return -1;
		}
		if(AlertName == 'Subnet Mask')
		{
			if(ts[1] == 0)
			{
				if (NoAlert);
				else alert(AlertName+'\'s format is illegal! ');
				return -1;
			}
			if((ts[1] != 0) && (ts[1] != 255))
			{
				for(j=0;j<7;j++)
				{
					if(ts[1] == ts3[j])
					{
						break;
					}
					if(j==6)
					{
						if (NoAlert);
						else alert(AlertName+'\'s format is illegal! ');
						return -1;	
					}
				}
				if((ts[2] != 0) || (ts[3] != 0) || (ts[4] != 0))
				{
					if (NoAlert);
					else alert(AlertName+'\'s format is illegal! ');
					return -1;
				}
			}
			if(ts[1] == 255)
			{
				if((ts[2] != 0) && (ts[2] != 255))
				{
					for(j=0;j<7;j++)
					{
						if(ts[2] == ts3[j])
						{
							break;
						}
						if(j==6)
						{
							if (NoAlert);
							else alert(AlertName+'\'s format is illegal! ');
							return -1;	
						}
					}
					if((ts[3] != 0) || (ts[4] != 0))
					{
						if (NoAlert);
						else alert(AlertName+'\'s format is illegal! ');
						return -1;
					}
				}
				if(ts[2] == 255)
				{
					if((ts[3] != 0) && (ts[3] != 255))
					{
						for(j=0;j<7;j++)
						{
							if(ts[3] == ts3[j])
							{
								break;
							}
							if(j==6)
							{
								if (NoAlert);
								else alert(AlertName+'\'s format is illegal! ');
								return -1;	
							}	
						}
						if(ts[4] != 0)
						{
							if (NoAlert);
							else alert(AlertName+'\'s format is illegal! ');
							return -1;
						}
					}
					if(ts[3] == 255)
					{
						if((ts[4] != 0) && (ts[4] != 255))
						{
							for(j=0;j<7;j++)
							{
								if(ts[4] == ts3[j])
								{
									break;
								}
								if(j==6)
								{
									if (NoAlert);
									else alert(AlertName+'\'s format is illegal! ');
									return -1;	
								}
							}
						}
					}
					if((ts[3] == 0) && (ts[4] != 0))
					{
						if (NoAlert);
						else alert(AlertName+'\'s format is illegal! ');
						return -1;	
					}
				}
				if((ts[2] == 0) && ((ts[3] != 0) || (ts[4] != 0)))
				{
					if (NoAlert);
					else alert(AlertName+'\'s format is illegal! ');
					return -1;	
				}
			}
		}
	}	
	if (AllowScope)
	{
		tmpString=AllowScope.value;
		for(i=1;i<5;i++)
		{
			q=tmpString.length;
			p=tmpString.indexOf(".");
			if(i==4)
			{
				ts2[i]=tmpString;
			}
			else
			{
				ts2[i]=tmpString.substring(0,p);
				rightString=tmpString;
				tmpString=rightString.substring(p+1,q);
			}
		}
		for(i=1;i<4;i++)
		{
			if(ts[i]!=ts2[i])
			{
				if (NoAlert);
				else alert(AlertName+'\'s value should be under '+ts2[1]+'.'+ts2[2]+'.'+ts2[3]+'.'+ Min +' ~ '+Max+'! ');
				return -2;
			}
		}
	}
	
	if (ForbiddenIP)
	{
		tmpString=ForbiddenIP.value;
		for(i=1;i<5;i++)
		{
			q=tmpString.length;
			p=tmpString.indexOf(".");
			if(i==4)
			{
				ts2[i]=tmpString;
			}
			else
			{
				ts2[i]=tmpString.substring(0,p);
				rightString=tmpString;
				tmpString=rightString.substring(p+1,q);
			}
		}
		if(ts[1] == ts2[1] && ts[2] == ts2[2] && ts[3] == ts2[3] && ts[4] == ts2[4])
		{
			if (NoAlert);
			else alert(AlertName+' can\'t be '+ts2[1]+'.'+ts2[2]+'.'+ts2[3]+'.'+ts2[4]+'! ');
			return -3;	
		}		
	}	
	CheckIP.value = ts[1] + "." + ts[2] + "." + ts[3] + "." + ts[4];	
	return 0;
}

function PortCheck(I)
{
	return validatenum(I, 65536, 0, aPort0Check);
}

function IPRangeCheck(StartIP, EndIP, AllowScope, ForbiddenIP, InputColume, NoAlert) // Only Support Class C
{
	var i,p,q;
  	var tmpString;
	var rightString;
	var ts=new tmpWord(4);
	var ts2=new tmpWord(4);
	var ts3=new tmpWord(4);	
	var StartIPisRight = 0;

	if (InputColume);
	else if (IPCheck(StartIP, 1, 254, AllowScope, 0, 0, "Start IP") <0) return -1;

	if (StartIP.value != "")
	{
		StartIPisRight = 1;
		tmpString=StartIP.value;
		for(i=1;i<5;i++)
		{
			q=tmpString.length;
			p=tmpString.indexOf(".");

			if(i==4)
				ts[i]=tmpString;
			else
			{
				ts[i]=tmpString.substring(0,p);
				rightString=tmpString;
				tmpString=rightString.substring(p+1,q);
			}
		}
		if (EndIP.value=="")
			EndIP.value= ts[1] + '.' + ts[2] + '.' + ts[3] + '.' + ts[4];
	}
	if (StartIPisRight)
		if (IPCheck(EndIP, 1, 254, StartIP, 0, 0, "End IP") <0) return -1;
	else
		if (IPCheck(EndIP, 1, 254, AllowScope, 0, 0, "End IP") <0) return -1;

	tmpString=EndIP.value;
	for(i=1;i<5;i++)
	{
		q=tmpString.length;
		p=tmpString.indexOf(".");
		if(i==4)
			ts2[i]=tmpString;
		else
		{
			ts2[i]=tmpString.substring(0,p);
			rightString=tmpString;
			tmpString=rightString.substring(p+1,q);
		}		
	}
	if(StartIPisRight == 1 && parseInt(ts2[4],10) < parseInt(ts[4],10))
	{
		if (NoAlert);
		else alert('The End IP should be bigger than Start IP! ');
		return -2;
	}		
	if (ForbiddenIP)
	{
		tmpString=ForbiddenIP.value;
		for(i=1;i<5;i++)
		{
			q=tmpString.length;
			p=tmpString.indexOf(".");
			if(i==4)
			{
				ts3[i]=tmpString;
			}
			else
			{
				ts3[i]=tmpString.substring(0,p);
				rightString=tmpString;
				tmpString=rightString.substring(p+1,q);
			}
		}	
	
		
		if(ts3[4] >= ts[4] && ts3[4] <= ts2[4])
		{
			if (NoAlert);
			else if (StartIP.value == "") alert(aIPAddressStart);
			else alert('This IP Range can\'t contain '+ts3[1]+'.'+ts3[2]+'.'+ts[3]+'.'+ts3[4]+'! ');
			return -3;	
		}		
	}

	return 0;	
}

function MACCheck(CheckMAC, MACName) 
{    
	var tmp=CheckMAC.value.split(":");
	var tmp2=CheckMAC.value.split("-");
	var mac_string ="";
	var AlertName = "This MAC";	

	if (MACName)
		AlertName = AlertName;

	if(tmp.length == 6)
		mac_string=tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
	else if(tmp2.length == 6)
		mac_string=tmp2[0]+tmp2[1]+tmp2[2]+tmp2[3]+tmp2[4]+tmp2[5];
	else
		mac_string=CheckMAC.value;

	mac_string=mac_string.toUpperCase();
	if(mac_string.length != 12)
	{
		if (CheckMAC.value == "") return 0;
		else alert('Please input '+AlertName+' with correct format!');
		return -1;                    
	}

	for (var i=0; i < mac_string.length; i++)
	{
		if( (mac_string.charAt(i) < "A" || mac_string.charAt(i) > "F" ) )
		if( (mac_string.charAt(i) < "0" || mac_string.charAt(i) > "9"))
		{
			alert(AlertName +'\'s format is illegal! ');
			return -2;                  
		}
	}
	return 0;
}

function filterMAC(MAC) 
{
	var type=":";
	var tmp=MAC.split(":");
	var tmp2=MAC.split("-");
	var mac_string;

	if(tmp.length == 6)
		mac_string=tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
	else if(tmp2.length == 6)
		mac_string=tmp2[0]+tmp2[1]+tmp2[2]+tmp2[3]+tmp2[4]+tmp2[5];
	else
		mac_string=MAC;		
	if(mac_string.length == 12)
	{
		mac_string=mac_string.charAt(0)+mac_string.charAt(1)+type+mac_string.charAt(2)+mac_string.charAt(3)+type+
				   mac_string.charAt(4)+mac_string.charAt(5)+type+mac_string.charAt(6)+mac_string.charAt(7)+type+
				   mac_string.charAt(8)+mac_string.charAt(9)+type+mac_string.charAt(10)+mac_string.charAt(11);
		mac_string=mac_string.toUpperCase();                   
	}
	return mac_string;     
}

function CheckIPv6(IP,type) 
{
    var v6rule,v6rule2002;

	v6ruleRegularZip=/^((([0-9a-f]{1,4}:){7}[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){6}:[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){5}:([0-9a-f]{1,4}:)?[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){4}:([0-9a-f]{1,4}:){0,2}[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){3}:([0-9a-f]{1,4}:){0,3}[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){2}:([0-9a-f]{1,4}:){0,4}[0-9a-f]{1,4})|([0-9a-f]{1,4}::([0-9a-f]{1,4}:){0,5}[0-9a-f]{1,4})|(::([0-9a-f]{1,4}:){0,6}[0-9a-f]{1,4})|(([0-9a-f]{1,4}:){1,7}:))$/;
	v6Begin=/^(2002|fc00|fe80)/;

	if (IP.value=="::1" || IP.value=="::"){
		alert("\"::\" or \"::1\" cannot be IPv6 address.");
		return false;
	}

	if (!v6ruleRegularZip.test(IP.value)){
		alert("Please input IPv6 Address with correct format!");
		return false;
	}else return true;

	/*if (type=="local"){
		if (!v6ruleRegularZip.test(IP.value)|| !v6Begin.test(IP.value)){
			alert("Please input IPv6 Address with correct format!");
			return false;
		}else return true;
	}
	else if (type=="global")
	{
		if (!v6ruleRegularZip.test(IP.value)){
			alert("Please input IPv6 Address with correct format!");
			return false;
		}else return true;
	}*/

}
function CheckIPv6Prefix(Prefix) 
{
	if (Prefix.value>128 || Prefix.value<0)
	{
        alert("Please set the prefix length within the range of 0-128.");
		return false;
	}else return true;

}

function CHeckIPv6_smallLarge(small_ip, large_ip, prefix){

    var hex_prefix_small1,hex_prefix_small2,hex_prefix_small3,hex_prefix_small4;
	var hex_prefix_large1,hex_prefix_large2,hex_prefix_large3,hex_prefix_large;
	var i;

	//small_ip   
	i=hex_to_ten(small_ip);	
	hex_prefix_small1=parseInt(i/8);
	hex_prefix_small2=parseInt((i-hex_prefix_small1*8)/4);
	hex_prefix_small3=parseInt((i-hex_prefix_small1*8-hex_prefix_small2*4)/2);
	hex_prefix_small4=parseInt((i-hex_prefix_small1*8-hex_prefix_small2*4-hex_prefix_small3*2));

	//large_ip  
	i=hex_to_ten(large_ip);
	hex_prefix_large1=parseInt(i/8);
	hex_prefix_large2=parseInt((i-hex_prefix_large1*8)/4);
	hex_prefix_large3=parseInt((i-hex_prefix_large1*8-hex_prefix_large2*4)/2);
	hex_prefix_large4=(i-hex_prefix_large1*8-hex_prefix_large2*4-hex_prefix_large3*2);

	switch(prefix){		
		case 3:
			if (hex_prefix_small3!=hex_prefix_large3)
				return false;
		case 2:
			if (hex_prefix_small2!=hex_prefix_large2)
				return false;
		case 1:
			if (hex_prefix_small1!=hex_prefix_large1)
				return false;
		default:
			;
	}

	return true;

}

function hex_to_ten(char_ten){
	var hex_value;

	switch (char_ten)
	{
		case 'a':
			hex_value=10;
			break;
		case 'b':
			hex_value=11;
			break;
		case 'c':
			hex_value=12;
			break;
		case 'd':
			hex_value=13;
			break;
		case 'e':
			hex_value=14;
			break;
		case 'f':
			hex_value=15;
			break;
		default:
			hex_value=char_ten;
	}

	return hex_value;
}

function IPRangeCheck_v6(ip_start, ip_end, v6LanIp, prefix, type)
{	
	var tmp_start=ip_start.value.split(":");
	var tmp_end=ip_end.value.split(":");
	var tmp_lan=v6LanIp.value.split(":");
	var tmp;
	var samefrefix=true;

	if (type=="start")
	{
		if(!CheckIPv6(ip_start,"local"))
		return;
	}
	else if (type=="end")
	{
		if(!CheckIPv6(ip_end,"local"))
		return;
	}
	else
	{	
		if(!CheckIPv6(ip_start,"global"))
			return;
		
		if (type!="zero")
		{
			if(!CheckIPv6(ip_end,"global"))
				return;
		}
	}	

	//ip_start
	tmp=tmp_start.length;
	for (var i=1; i <= tmp_start.length; i++)
	{
		if (ip_start.value.search(tmp_start[i])==0)
		{
			for (var j=7; j > 7-((tmp-1)-i) ; j--) //move 8-(tmp-1)
			{	
				if (tmp_start[(tmp-1)-(7-j)].length<4)
				{	
					switch (tmp_start[(tmp-1)-(7-j)].length)
					{
						case 1:
							tmp_start[j]="000"+tmp_start[(tmp-1)-(7-j)];
							break;
						case 2:
							tmp_start[j]="00"+tmp_start[(tmp-1)-(7-j)];
							break;
						case 3:
							tmp_start[j]="0"+tmp_start[(tmp-1)-(7-j)];
							break;
						default:
							tmp_start[j]="0000";
					}
				}else
					tmp_start[j]=tmp_start[(tmp-1)-(7-j)];
			}

			for (var k=i ; k <= (7-((tmp-1)-i)) ; k++) 
			{	
				tmp_start[k]="0000";
			}
			for (var j=0; j < (7-((tmp-1)-i)) ; j++) //move 8-(tmp-1)
			{	
				switch (tmp_start[j].length)
				{
					case 1:
						tmp_start[j]="000"+tmp_start[j];
						break;
					case 2:
						tmp_start[j]="00"+tmp_start[j];
						break;
					case 3:
						tmp_start[j]="0"+tmp_start[j];
						break;
					default:
						;
				}
			}

			i=9;
		}
	}
	//alert(tmp_start[0]+" "+tmp_start[1]+" "+tmp_start[2]+" "+tmp_start[3]+" "+tmp_start[4]+" "+tmp_start[5]+" "+tmp_start[6]+" "+tmp_start[7]);


	if (type=="zero"){
		
		for (j=0; j<32 ; j++ )
		{
			if(tmp_start[parseInt(j/4)].charAt(j%4) != 0)
				return 0;
		}
		return 1;
	}

	//ip_end
	tmp=tmp_end.length;
	for (var i=1; i <= tmp_end.length; i++)
	{
		if (ip_end.value.search(tmp_end[i])==0)
		{
			for (var j=7; j > 7-((tmp-1)-i) ; j--) //move 8-(tmp-1)
			{	
				if (tmp_end[(tmp-1)-(7-j)].length<4)
				{	
					switch (tmp_end[(tmp-1)-(7-j)].length)
					{
						case 1:
							tmp_end[j]="000"+tmp_end[(tmp-1)-(7-j)];
							break;
						case 2:
							tmp_end[j]="00"+tmp_end[(tmp-1)-(7-j)];
							break;
						case 3:
							tmp_end[j]="0"+tmp_end[(tmp-1)-(7-j)];
							break;
						default:
							tmp_end[j]="0000";
					}
				}else
					tmp_end[j]=tmp_end[(tmp-1)-(7-j)];
			}

			for (var k=i ; k <= (7-((tmp-1)-i)) ; k++) 
			{	
				tmp_end[k]="0000";
			}

			for (var j=0; j < (7-((tmp-1)-i)) ; j++) //move 8-(tmp-1)
			{	
				switch (tmp_end[j].length)
				{
					case 1:
						tmp_end[j]="000"+tmp_end[j];
						break;
					case 2:
						tmp_end[j]="00"+tmp_end[j];
						break;
					case 3:
						tmp_end[j]="0"+tmp_end[j];
						break;
					default:
						;
				}
			}

			i=9;
		}
	}
	//alert(tmp_end[0]+" "+tmp_end[1]+" "+tmp_end[2]+" "+tmp_end[3]+" "+tmp_end[4]+" "+tmp_end[5]+" "+tmp_end[6]+" "+tmp_end[7]);

	if (type=="sort"){
		
		for (tmp=0; j<32 ; j++ )
		{
			if(tmp_start[parseInt(j/4)].charAt(j%4) < tmp_end[parseInt(j/4)].charAt(j%4))
			{	
				return -1;
			}
			else if (tmp_start[parseInt(j/4)].charAt(j%4) > tmp_end[parseInt(j/4)].charAt(j%4))
			{
				return 1;
			}
		}
		if (tmp==32)
			return 0;
	}

if (prefix.value!=128 || type=="same")
{
	//v6_LanIP
	tmp=tmp_lan.length;
	for (var i=1; i <= tmp_lan.length; i++)
	{
		if (v6LanIp.value.search(tmp_lan[i])==0)
		{
			for (var j=7; j > 7-((tmp-1)-i) ; j--) //move 8-(tmp-1)
			{	
				if (tmp_lan[(tmp-1)-(7-j)].length<4)
				{	
					switch (tmp_lan[(tmp-1)-(7-j)].length)
					{
						case 1:
							tmp_lan[j]="000"+tmp_lan[(tmp-1)-(7-j)];
							break;
						case 2:
							tmp_lan[j]="00"+tmp_lan[(tmp-1)-(7-j)];
							break;
						case 3:
							tmp_lan[j]="0"+tmp_lan[(tmp-1)-(7-j)];
							break;
						default:
							tmp_lan[j]="0000";
					}
				}else
					tmp_lan[j]=tmp_lan[(tmp-1)-(7-j)];
			}

			for (var k=i ; k <= (7-((tmp-1)-i)) ; k++) 
			{	
				tmp_lan[k]="0000";
			}

			for (var j=0; j < (7-((tmp-1)-i)) ; j++) //move 8-(tmp-1)
			{	
				switch (tmp_lan[j].length)
				{
					case 1:
						tmp_lan[j]="000"+tmp_lan[j];
						break;
					case 2:
						tmp_lan[j]="00"+tmp_lan[j];
						break;
					case 3:
						tmp_lan[j]="0"+tmp_lan[j];
						break;
					default:
						;
				}
				
			}

			i=9;
		}
	}

	//alert(tmp_lan[0]+" "+tmp_lan[1]+" "+tmp_lan[2]+" "+tmp_lan[3]+" "+tmp_lan[4]+" "+tmp_lan[5]+" "+tmp_lan[6]+" "+tmp_lan[7]);

	if(type=="check_v6Lan_dhcp" || type=="check_range_DMZ")
	{
		//check start under lan
		for (var j=0; j<parseInt(prefix.value/4) ; j++ )
		{	
			if(tmp_start[parseInt(j/4)].charAt(j%4)!=tmp_lan[parseInt(j/4)].charAt(j%4))
			{		

				if (type=="check_v6Lan_dhcp")
				{
					alert("Start IP address should not precede the IPv6 LAN IP Address.");
					return false;
				}
				else if(type=="check_range_DMZ")
				{
					alert("Start IP address should not precede the IPv6 WAN IP Address.");
					return false;
				}
				return false;
			}
		}

		if (!CHeckIPv6_smallLarge(tmp_lan[parseInt(j/4)].charAt(j%4),tmp_start[parseInt(j/4)].charAt(j%4),prefix.value%4))
		{
				if (type=="check_v6Lan_dhcp")
				{
					alert("Start IP address should not precede the IPv6 LAN IP Address.");
					return false;
				}
				else if(type=="check_range_DMZ")
				{
					alert("Start IP address should not precede the IPv6 WAN IP Address.");
					return false;
				}
		}
		
		//check end under lan
		for (var j=0; j<parseInt(prefix.value/4) ; j++ )
		{	
			if(tmp_end[parseInt(j/4)].charAt(j%4)!=tmp_lan[parseInt(j/4)].charAt(j%4))
			{		

				if (type=="check_v6Lan_dhcp")
				{
					alert("End IP address should not precede the IPv6 LAN IP Address.");
					return false;
				}
				else if(type=="check_range_DMZ")
				{
					alert("End IP address should not precede the IPv6 WAN IP Address.");
					return false;
				}
				return false;
			}
		}

		if (!CHeckIPv6_smallLarge(tmp_lan[parseInt(j/4)].charAt(j%4),tmp_end[parseInt(j/4)].charAt(j%4),prefix.value%4))
		{
			if (type=="check_v6Lan_dhcp")
			{
				alert("End IP address should not precede the IPv6 LAN IP Address.");
				return false;
			}
			else if(type=="check_range_DMZ")
			{
				alert("End IP address should not precede the IPv6 WAN IP Address.");
				return false;
			}
		}
	}
	else if (type=="test_under")
	{

		for (var j=0; j<parseInt(prefix.value/4) ; j++ )
		{	
			if(tmp_start[parseInt(j/4)].charAt(j%4)!=tmp_lan[parseInt(j/4)].charAt(j%4))
			{		
				return false;
				break;
			}
		}

		if (!CHeckIPv6_smallLarge(tmp_lan[parseInt(j/4)].charAt(j%4),tmp_start[parseInt(j/4)].charAt(j%4),prefix.value%4))
		{
			return false;
		}

		return true;
	}
	else if (type=="check_v6Lan" || type=="check_v6DMZ" )
	{
		for (var j=0; j<parseInt(prefix.value/4) ; j++ )
		{	
			if(tmp_start[parseInt(j/4)].charAt(j%4)!=tmp_lan[parseInt(j/4)].charAt(j%4))
			{		
				samefrefix=false;
				break;
			}
		}

		if (samefrefix && !CHeckIPv6_smallLarge(tmp_lan[parseInt(j/4)].charAt(j%4),tmp_start[parseInt(j/4)].charAt(j%4),prefix.value%4))
		{
			samefrefix=false;
		}

	}
	else if (type=="same")
	{
		for (var j=0; j<32 ; j++ )
		{	
			if(tmp_start[parseInt(j/4)].charAt(j%4)!=tmp_lan[parseInt(j/4)].charAt(j%4))
			{		
				return false;
				break;
			}
		}

		return true;
	}
	

	//max dhcp available ip is 512 (DHCPV6_MAXAVAILABLEIP)
	var start_value=0,end_value=0,lan_value_s=0,lan_value_e=0,lan_value=0,ip_range_number=0;

	for (var j=parseInt(prefix.value/4); j<32 ;j++ )
	{	
		start_value=hex_to_ten(tmp_start[parseInt(j/4)].charAt(j%4));

		end_value=hex_to_ten(tmp_end[parseInt(j/4)].charAt(j%4));

		ip_range_number=ip_range_number+(end_value-start_value)*Math.pow(16,31-j);

		lan_value=hex_to_ten(tmp_lan[parseInt(j/4)].charAt(j%4));

		lan_value_s=lan_value_s+(lan_value-start_value)*Math.pow(16,31-j);
		lan_value_e=lan_value_e+(lan_value-end_value)*Math.pow(16,31-j);

	}

	if (type!="check_v6Lan" && type!="check_v6DMZ")
	{
		if (ip_range_number > DHCPV6_MAXAVAILABLEIP)
		{
			alert("IP not more than "+DHCPV6_MAXAVAILABLEIP+"!");
			return false;
		}

		//check start ip is small than end ip
		if(ip_range_number<0)
		{	
			alert("Start IP value should be smaller than End IP !");
			return false;
		}
	}



	//Lan and DHCP range has same prefix, lanip < dhcp_start or lanip > dhcp_end	
	if (samefrefix)
	{
		if (lan_value_s>=0 && lan_value_e<=0)
		{	
			if (type=="check_v6Lan_dhcp" || type=="check_v6Lan")
			{
				alert(aDhcpLanIpConflict);
				return false;
			}
			else if (type=="check_range_DMZ" ||  type=="check_v6DMZ" )
			{
				alert(aDMZSubnetConflict);
				return false;
			}
		}
	}

}
	return true;
}

function ip_range_start_get(ip,netmask,startip,output)
{
	var ipaddr=ip.value.split(".");
	var netmask=netmask.value.split(".");
	var start_ip=startip.value.split(".");
	
	if(parseInt(~netmask[3] + 1) == -1)
	{
		netmask[3] = 255;
	}
	else if(parseInt(~netmask[3] + 1) < -1)
	{
		netmask[3] = Math.abs(parseInt(~netmask[3] + 1));
	}
	
	var in_subnet = (ipaddr[3] & netmask[3]) == (start_ip[3] & netmask[3]) &&
			(1 || ipaddr[3] != start_ip[3]) &&
			(ipaddr[3] & netmask[3]) &&
			(ipaddr[3] & netmask[3]) != netmask[3];
	if ( !in_subnet )
	{
		start_ip[3] = (ipaddr[3] & netmask[3])+1;
	}
	if(output)
	{
		start_ip[3] = (ipaddr[3] & netmask[3])+1;
		return start_ip[3];
	}
	else
	{
		return in_subnet;
	}
}

function ip_range_end_get(ip,netmask,endip,output)
{
	var ipaddr=ip.value.split(".");
	var netmask=netmask.value.split(".");
	var end_ip=endip.value.split(".");
	var s_netmask=0;
	
	if(parseInt(netmask[3]) == 0)
	{
		netmask[3] = 255;
		s_netmask = 255;
	}
	else if(parseInt(~netmask[3] + 1) < -1)
	{
		s_netmask = 255-Math.abs(parseInt(~netmask[3] + 1));
		netmask[3] = Math.abs(parseInt(~netmask[3] + 1));
	}
	
	var in_subnet = (ipaddr[3] & netmask[3]) == (end_ip[3] & netmask[3]) &&
			(1 || ipaddr[3] != end_ip[3]) &&
			(ipaddr[3] & netmask[3]) &&
			(ipaddr[3] & netmask[3]) != netmask[3];
	if ( !in_subnet )
	{
		end_ip[3] = (ipaddr[3] & netmask[3] | s_netmask)-1;
	}
	if(output)
	{
		end_ip[3] = (ipaddr[3] & netmask[3] | s_netmask)-1;
		return end_ip[3];
	}
	else
	{
		return in_subnet;
	}
}

function PrintIPTypeTab(tab1,current1,tab2,current2){

	var isIE = navigator.userAgent.search("MSIE") > -1; 
    var isFirefox = navigator.userAgent.search("Firefox") > -1; 
    //var isOpera = navigator.userAgent.search("Opera") > -1; 
    //var isSafari = navigator.userAgent.search("Safari") > -1;

	if (isIE)
	{
		document.write('<div id="IEmenu"><ul>')
	}
	else if (isFirefox)
	{
		document.write('<div id="FFmenu"><ul>')
	}
	else
	{	
		document.write('<div id="menu"><ul>')
	}
	document.write('<ul>');

	//Tab1
	document.write('<li><a title="'+tab1+'" ');
	if (current1=='true')
		document.write(' class="current">');
	else
		document.write(' href="javascript:chChangeIPmode(1);">');
	document.write('<span>&nbsp;'+tab1+'&nbsp;</span></a></li>');
	
	//Tab2
	document.write('<li><a title="'+tab2+'" ');
	if (current2=='true')
		document.write(' class="current">');
	else
		document.write(' href="javascript:chChangeIPmode(2);">');
	document.write('<span>&nbsp;'+tab2+'&nbsp;</span></a></li>');

	document.write('</ul>');
	document.write('</div>');

}