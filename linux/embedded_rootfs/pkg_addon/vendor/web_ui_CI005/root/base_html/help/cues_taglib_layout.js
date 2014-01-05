/*
 * Copyright 2008 Cisco Systems Inc
 */

/* support for user resizable nav area */
var cuesResizeBox = null;
var cuesResizeLayer = null;
var cuesNavResizing = false;
function icuesResizeLayerMouseDown(evt,obj)
{
  try
  {
    if(evt==null)evt=window.event;
    var obj = (isIE)?evt.srcElement:evt.target;
    if(obj.tagName.toLowerCase()=="img" && parseInt(cuesVersion)>=60)
    {
      obj = icuesFindObjectByTagname(obj, "td");
      obj.style.backgroundColor="";
    }
    if(obj.id != "cuesLayoutTogglerArea")
      return true;
    // check if nav area is hidden
    var navArea = icuesGetNavArea();
    if(navArea != null && navArea.style.display=="none") 
    {
      cuesStopEventBubble(evt);
      return cuesSchedulerNoop();
    }
    if(cuesResizeLayer == null)
      cuesResizeLayer = document.getElementById("cuesResizeLayer");
    cuesResizeLayer.style.display="block";
    cuesResizeLayer.style.cursor = "col-resize";
    // force to be last element in page.  zIndex not enough for Firefox
    document.body.appendChild(cuesResizeLayer);
    if(cuesResizeBox == null)
      cuesResizeBox = document.getElementById("cuesResizeBox");
    cuesResizeBox.style.top = icuesPhysicalTop(obj);
    cuesResizeBox.style.left = icuesPhysicalLeft(obj);
    cuesResizeBox.style.width = obj.offsetWidth;
    cuesResizeBox.style.height = obj.offsetHeight;
    cuesNavResizing = true;
    icuesAttachResizeListeners();
    cuesStopEventBubble(evt);
    return cuesSchedulerNoop();
  }catch(e){}
  return false;
}
function icuesResizeLayerMouseMove(evt)
{
  try
  {
    if(evt==null)evt=window.event;
    if(!cuesNavResizing ||
       evt.clientY < 0 ||
       evt.clientX < 0 ||
       evt.clientY > document.body.offsetHeight ||
       evt.clientX > document.body.offsetWidth)
    {      
      icuesResizeLayerAborted(evt);
    }
    // needed for IE which sends events WAY too often
    if(cuesResizeBox.style.left==evt.clientX+"px") return false;
    cuesResizeBox.style.left = evt.clientX;
    cuesStopEventBubble(evt);
    cuesSchedulerNoop();
  }catch(e){}
  return false;
}
function icuesResizeLayerMouseUp(evt, obj)
{                               
  try
  {
    if(!cuesNavResizing)
      return icuesResizeLayerAborted(evt);
    icuesReleaseResizeListeners();
    if(evt==null)evt=window.event;
    cuesResizeLayer.style.display="none";

    icuesResizeDrawerWidths(Math.max(0, evt.clientX), true);

    try{callbackNavAreaResized();}catch(e){}

    cuesStopEventBubble(evt);
    return cuesSchedulerNoop();
  }catch(e){}
  return false;
}
function icuesResizeDrawerWidths(w, bAllowTile)
{
  try
  {
    var navArea = icuesGetNavArea();
    if(navArea != null)
    {
      if(w==null)
        w = navArea.offsetWidth;

      if (parseInt(cuesVersion)>=60)
      {
        // consider gap at edges of window
        var gapObj = document.getElementById("cuesLayoutLeftBorder");
        if(gapObj != null)
          w -=  gapObj.offsetWidth;
      }
      navArea.width = w;
      var divs = navArea.getElementsByTagName("div");
      var numDivs = divs.length;
      var containerIndex = -1;
      for(var j=0; j<numDivs; j++)
      {
        if(divs[j].className.indexOf("cuesDrawersContainer")==0)
        {
          containerIndex = j;
          divs[j].style.width = w;
        }
        else
        if(divs[j].className.indexOf("cuesDrawersLayer")==0)
          divs[j].style.width = w;
      }
      if(containerIndex != -1 && bAllowTile)
      {
        cuesTileDrawers(divs[containerIndex].id, true);
        // if scrolling nav area and width is reducing, IE may have trouble.
        // client height is reduced to leave space for scroller but no scroller present !!
      }
    }
  }catch(e){}
}
function icuesResizeLayerMouseOut(evt, obj)
{
  try
  {
    if(evt==null)evt=window.event;
    if(!cuesNavResizing ||
       evt.clientY < 0 ||
       evt.clientX < 0 ||
       evt.clientY > document.body.offsetHeight ||
       evt.clientX > document.body.offsetWidth)
    {      
      return icuesResizeLayerAborted(evt);
    }
    cuesStopEventBubble(evt);
    return cuesSchedulerNoop();
  }catch(e){}
  return false;
}
function icuesResizeLayerAborted(evt, obj)
{        
  try
  {
    cuesNavResizing = false;
    cuesResizeLayer.style.display="none";
    icuesReleaseResizeListeners();
    cuesStopEventBubble(evt);
    return cuesSchedulerNoop();
  }catch(e){}
  return false;
}
function icuesReleaseResizeListeners()
{
  if (document.removeEventListener)
  {
    document.removeEventListener('mousemove', icuesResizeLayerMouseMove, false); 
    document.removeEventListener('mouseup', icuesResizeLayerMouseUp, false); 
    document.removeEventListener('mouseout', icuesResizeLayerMouseOut, false); 
  }
  else 
  if (document.detachEvent)
  {
    document.detachEvent('onmousemove', icuesResizeLayerMouseMove);
    document.detachEvent('onmouseup', icuesResizeLayerMouseUp);
    document.detachEvent('onmouseout', icuesResizeLayerMouseOut);
  }    
}
function icuesAttachResizeListeners()
{
  if (document.addEventListener)
  {
    document.addEventListener('mousemove', icuesResizeLayerMouseMove, false); 
    document.addEventListener('mouseup', icuesResizeLayerMouseUp, false); 
    document.addEventListener('mouseout', icuesResizeLayerMouseOut, false); 
  }
  else 
  if (document.attachEvent)
  {
    document.attachEvent('onmousemove', icuesResizeLayerMouseMove);
    document.attachEvent('onmouseup', icuesResizeLayerMouseUp);
    document.attachEvent('onmouseout', icuesResizeLayerMouseOut);
  }
}

/* simple expand/collapse support */
function cuesMouseOverToggler(evt, obj)
{
  if(obj.attributes["state"].value=="closed")
    obj.src = cuesKnownLocation+cuesImageFolder+"/split-closed-hl-html.gif";
  else
    obj.src = cuesKnownLocation+cuesImageFolder+"/split-opened-hl-html.gif";
  obj.style.cursor = "hand";
}
function cuesMouseOutToggler(evt, obj)
{
  if(obj.attributes["state"].value=="closed")
    obj.src = cuesKnownLocation+cuesImageFolder+"/split-closed-html.gif";
  else
    obj.src = cuesKnownLocation+cuesImageFolder+"/split-opened-html.gif";
  obj.style.cursor = "default";
}
function cuesMouseOverGripper(evt, obj)
{
  obj.style.cursor = "col-resize";
  obj.parentNode.style.backgroundColor="#0088c2";
}
function cuesMouseOutGripper(evt, obj)
{
  obj.style.cursor = "default";
  obj.parentNode.style.backgroundColor="";
}
function cuesToggleNavArea(evt, obj, id)
{
  try
  {
    var newState;
    var contentArea = document.getElementById("cuesLayoutContentArea");
    if(obj.attributes["state"].value=="closed")
    {
      if(isIE)
        newState = "inline";
      else
        newState = "table-cell";
      obj.src = cuesKnownLocation+cuesImageFolder+"/split-opened-html.gif";
      obj.attributes["state"].value="opened";
      obj.title = altCollapseNav;
    }
    else
    {
      newState = "none";
      obj.src = cuesKnownLocation+cuesImageFolder+"/split-closed-html.gif";
      obj.attributes["state"].value="closed";
      obj.title = altExpandNav;
    }

    icuesGetNavArea().style.display=newState;
    try{callbackNavAreaToggled();}catch(e){}
  }
  catch(e){alert(e);}
}
function icuesGetNavArea()
{
  var navArea = null;
  try
  {
    var layoutTbl = document.getElementById("cuesLayout");
    var numCells = layoutTbl.rows[0].cells.length;
    for(var i=0; i<numCells; i++)
    {
      if(layoutTbl.rows[0].cells[i].id=="cuesLayoutTogglerArea")
      {
        navArea = layoutTbl.rows[0].cells[i-1];
        break;
      }
    }
  }
  catch(e){}
  return navArea;
}

