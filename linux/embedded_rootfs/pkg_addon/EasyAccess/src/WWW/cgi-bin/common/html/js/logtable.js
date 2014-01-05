function NA(){return new Array();}
function DCE(x){return document.createElement(x);}
function DGE(x){return document.getElementById(x);}
var c3=new Array();c3[1]=document.createElement("TABLE");c3[2]=document.createElement("TD");c3[3]=document.createElement("TR");c3[4]=document.createElement("TBODY");c3[5]=document.createElement("THEAD");c3[6]=document.createElement("TFOOT");c3[7]=document.createElement("SPAN");c3[8]=document.createElement("INPUT");c3[9]=document.createElement("SELECT");c3[10]=document.createElement("A");c3[30]=document.createElement("OPTION");
function initFireBase(){c2=FireBase;for(var i=0;i<c2.length;i++){if(c2[i].p8){c2[i].f3();c2[i].f30();}}}
function FBase(p1,p2,p16,p17){this.p1=p1;this.p3=FireBase.length;this.p2=p2;this.p5=true;this.p40=false;this.p6=true;this.p15=true;this.p4=20;this.p7=0;this.p8=true;var a=new Array();this.p9=a;a=new Array();this.p10=a;a=new Array();this.p11=a;this.p12=false;a=new Array(p2.length);this.p13=a;a=new Array();this.p14=a;var x=this.p2.length;this.p38=NA();this.p39=NA();var i=x;do{y=x-i;this.p13[y]=".";this.p14[y]="a";this.p38[y]=this.p6;this.p39[y]=this.p15;}while(--i)this.p16=p16;if(p17==null)p17="accesspoint";this.setStyle(p17);a=new Array();this.p18=a;this.p19=false;this.p20="";this.p21=0;this.p22="c";this.p23=0;this.p24="";this.p25="c";this.p26=false;this.p27="&nbsp;&nbsp;&nbsp;Go to: ";}
sp=FBase.prototype;sp.f1=f1;sp.f2=f2;sp.f3=f3;sp.f4=f4;sp.f5=f5;sp.f6=f6;sp.f7=f7;sp.f8=f8;sp.f12=f12;sp.f13=f13;sp.f14=f14;sp.f9=f9;sp.f15=f15;sp.f10=f10;sp.f16=f16;sp.f11=f11;sp.f17=f17;sp.f18=f18;sp.f19=f19;sp.f20=f20;sp.f21=f21;
function f3(){var a=this.f8();a.cellPadding=3;a.cellSpacing=0;a.className=this.p28;for(var i=0;i<this.p13.length;i++){if(this.p13[i]!="."){this.p12=true;break;}}if(this.p12){this.f22=f23;}else if(this.p40){this.f22=f37;}else{this.f22=f22;}if(!this.p15&&!this.p5){this.f30=f19;}if(this.p5&&!this.p15){this.f30=f24;}if(this.p15&&this.p5){this.f30=f25;this.f26=f26;this.f24=f24;}if(this.p15&&!this.p5){this.f30=f27;}if(this.p15||this.p6){this.f28=f28;}else{this.f28=f29;}}
function f7(){var a=this.f8();for(var i =0;i<a.childNodes.length;i++){if(a.childNodes[i].tagName=="TBODY")return a.childNodes[i];}var tb=c3[4].cloneNode(false);a.appendChild(tb);return tb;}
function f8(){var a=document.getElementById(this.p1);if(a==null){a=c3[1].cloneNode(false);document.body.appendChild(a);a.id=this.p1;}return a;}
function f20(){return this.f8().tFoot;}
function f21(){return this.f8().tHead;}
function filterHelp(){alertStr="Include: n";alert(alertStr);}
function f16(a){this.p24=this.f12();this.p23=this.f13();this.p25=a;if(this.p20==this.p24&&this.p21==this.p23&&this.p22==this.p25){return false;}else{return true;}}
function f1(a){if(this.f12()==""&&a!="c"){alert("no value entered");}else if(a=="c"&&this.p22!="c"){this.p26=true;this.p19=false;this.p24="";this.p23=0;this.f14("");this.f9(0);this.f30();this.p26=false;this.p20=this.p24;this.p21=this.p23;this.p22=this.p25=a;}else if(this.f16(a)){this.p19=true;this.p26=true;if(this.f18()){this.f30();this.p20=this.p24;this.p21=this.p23;this.p22=this.p25;}else{alert("No results found");this.f14(this.p20);this.f9(this.p21);this.p24=this.p20;this.p23=this.p21;this.p25=this.p22;if(this.p18.length==0)this.p19=false;}this.p26=false;}}
function f14(x){var a=DGE("table"+this.p3+"_filterText");a.value=x;}
function f9(x){var a=DGE("table"+this.p3+"_filterSelect");a.options.selectedIndex=x;}
function f12(){var a=DGE("table"+this.p3+"_filterText").value;a+="";return a;} //.toUpperCase()
function f13(){var a=DGE("table"+this.p3+"_filterSelect");return a.options[a.options.selectedIndex].value;}
function f10(v){var a=false;if(this.p25=="b"){a=v.indexOf(this.p24)==-1;}else if(this.p25=="a"){a=v.indexOf(this.p24)>-1;}return a}
function f18(){var a=NA();var b=0;var c=this.p23;var d=this.p16.length;var e=parseInt(d/8);var f=d%8;var i=d-f-1;var p16=this.p16;if(f>0){do{var g=i+f;if(this.f10(p16[g][c]+"")){a[b++]=g;}}while(--f)}if(d>=8){do{if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}if(this.f10(p16[i--][c]+"")){a[b++]=(i+1);}}while(--e)}if(a.length==0){return false;}else{a.reverse();this.p18=a;return true;}}
function f29(){var a=c3[5].cloneNode(false);b=c3[3].cloneNode(false);b.className="HeadingStyle";b.style.borderBottomWidth="0";var c=this.p2.length;var i=c;do{var g=c-i;var h=c3[2].cloneNode(false);h.innerHTML=this.p2[g];h.className=this.p10[g];h.noWrap=true;b.appendChild(h);}while(--i)a.appendChild(b);return a;}
function f28(){var a=c3[5].cloneNode(false);if(this.p15){a.appendChild(this.f15());}var b=c3[3].cloneNode(false);b.className="HeadingStyle";b.style.borderBottomWidth="0";var c=this.p2.length;var i=c;p2=this.p2;d=this.p10;do{var g=c-i;var h=c3[2].cloneNode(false);h.className=d[g];h.noWrap=true;if(this.p38[g]==true){var l=c3[10].cloneNode(false);l.href="javascript:c2["+this.p3+"].f5("+g+");";l.innerHTML="<u>"+p2[g]+"</u>";l.className=d[g];h.appendChild(l);}else{h.innerHTML=this.p2[g];}b.appendChild(h);}while(--i)a.appendChild(b);return a;}
function f15(){var x=c3[3].cloneNode(false);var b=c3[2].cloneNode(false);b.className=this.p30;b.colSpan=this.p2.length;var d=c3[7].cloneNode(false);d.innerHTML="Search";var c=c3[8].cloneNode(false);c.type="text";c.id="table"+this.p3+"_filterText";c.size="10";c.value=this.p24;c.className=this.p30;var e=c3[7].cloneNode(false);e.innerHTML="in";var f=c3[9].cloneNode(false);f.id="table"+this.p3+"_filterSelect";f.className=this.p30;var g=this.p2.length;var i=g;do{j=g-i;if(this.p39[j]==true){f.options[f.options.length]=new Option(this.p2[j],j);if(j==this.p23)f.options[j].selected=true;}}while(--i)var l=c3[10].cloneNode(false);l.className=this.p30;l.innerHTML="Find";l.href="javascript:c2["+this.p3+"].f1('a')";var k=c3[10].cloneNode(false);k.className=this.p30;k.innerHTML="Exclude";k.href="javascript:c2["+this.p3+"].f1('b')";var m=c3[10].cloneNode(false);m.className=this.p30;m.innerHTML="Reset";m.href="javascript:c2["+this.p3+"].f1('c')";var n=c3[10].cloneNode(false);n.innerHTML="";n.href="javascript:filterHelp()";b.appendChild(d);b.appendChild(c);b.appendChild(e);b.appendChild(f);b.appendChild(l);b.appendChild(k);b.appendChild(m);var a=this.p9[1];if(a==null||a==""||a<0||a>this.f7().rows.length){a=this.p9[1]!=null?this.p9[1]:0;}var bx=this.p4;var cx=false?this.p18.length:this.p16.length;var dx=false;var ex=false;tablefooter=this.f8().tFoot;
if(!this.p5&&tablefooter!=null){}if(this.p5&&cx>bx){ex=true;if(this.p26){dx=true;ex=false;}if(tablefooter==null||tablefooter.rows.length==1){dx=true;ex=false;}}else if(this.p5&&tablefooter!=null&&tablefooter.rows.length==2){tablefooter.removeChild(tablefooter.rows[1]);}if(tablefooter==null){tablefooter=c3[6].cloneNode(false);var rx=c3[3].cloneNode(false);var tablecell=c3[2].cloneNode(false);}
if(!this.p5){}var gx=cx%bx;var hx=cx-gx;var jx=hx/bx;kx=a==hx?jx:a/bx;if(dx){rx=c3[3].cloneNode(false);var pagelink=new Array();var ix=jx;do{index=jx-ix;mx=(index*bx==a);  pagelink[index]=c3[7].cloneNode(false);}while(--ix)if(gx>0) {mx=a==hx;pagelink[pagelink.length]=this.f2(hx,mx);}var nx=c3[7].cloneNode(false);nx.innerHTML=this.p27;tablecell=c3[2].cloneNode(false);var selects=c3[9].cloneNode(false);selects.onchange = function(){checkMenu(this.options[this.selectedIndex].value)};selects.id="table"+this.p3+"_pageselect";
selects.className=this.p30;var length=pagelink.length;b.appendChild(nx.cloneNode(true));var ix=length;do{index=length-ix;selects.options[selects.options.length]=new Option(index+1,index*bx);}while(--ix);b.appendChild(selects);if(tablefooter.childNodes[1]==null){}else{tablefooter.replaceChild(rx,tablefooter.childNodes[1])}}if(ex){var ox=this.p29+"PageLink_"+this.p3+"_"+this.p7;var px=document.getElementById(ox);var qx=this.f2(this.p7*this.p4,false);qx.innerHTML=this.p7+1;qx.id=ox;px.parentNode.replaceChild(qx,px);var ox=this.p29+"PageLink_"+this.p3+"_"+kx;var vx=document.getElementById(ox);qx=this.f2(kx*this.p4,true);qx.innerHTML=kx+1;qx.id=ox;vx.parentNode.replaceChild(qx,vx);}this.p7=kx;a.className=this.p28;x.appendChild(b);return x;}
function f2(a,b) {var l=c3[10].cloneNode(false);l="c2[0].f30("+a+");";return l;}
function checkMenu(param){window.location="javascript:c2[0].f30("+param+");"}
function f4(a){}
function f5(a){if(this.p16.length>0){this.f6(a,0,this.p16.length-1);for(var f=0;f<this.p14.length;f++){if(f!=a)this.p14[f]="a";}if(this.p14[a]=="a"){this.p14[a]="b";}else{this.p14[a]="a";this.p16.reverse();}if(this.p19){this.f18();}this.f30();}}
function f6(a,b,c){var v=this.p16;var p,l,h,t;if(c-b==1){if(v[b][a]>v[c][a]){t=v[b];v[b]=v[c];v[c]=t;}return;}p=v[parseInt((b+c)/ 2)];v[parseInt((b+c)/ 2)]=v[b];v[b]=p;l=b+1;h=c;do{while(l<=h&&v[l][a]<=p[a]){l++;}while(v[h][a]>p[a]){h--;}if(l<h){t=v[l];v[l]=v[h];v[h]=t;}}while(l<h)v[b]=v[h];v[h]=p;if(b<h-1){this.f6(a,b,h-1);}if(h+1<c){this.f6(a,h+1,c);}}
function f11(){var a=this.f8();if(a.tHead==null){a.createTHead();a.replaceChild(this.f28(),a.tHead);}var b=this.f7();if(this.p37==null){this.p37=c3[4].cloneNode(false);}a.replaceChild(this.p37,b);b.className=this.p36;a.tHead.className=this.p28;a.className=this.p28;}
function f25(a){if(!this.p19){this.f24(a);}else{this.f26(a);}}
function f24(a){if(this.p16.length>0){this.p9[1]=a;if(a==null||a<0||a=="")a=0;var b=this.p16.length-a<this.p4? this.p16.length-a:this.p4;this.p37=c3[4].cloneNode(false);var c=a;var d=0;do{var e=this.f17(c,d);this.p37.appendChild(e);d++;c++;}while(d<b)}this.f11();}
function f26(a){if(this.p16.length>0){if(a==null||a<0||a=="")a=0;this.p9[1]=a;var b=this.p18.length-a<this.p4?this.p18.length-a:this.p4;this.p37=c3[4].cloneNode(false);var c=a;var d=0;do{var e=this.f17(this.p18[c],d);this.p37.appendChild(e);d++;c++;}while(d<b)}this.f11();}
function f27(){this.p37=c3[4].cloneNode(false);if(this.p19&&this.p16.length>0){var a=this.p18.length;var b=a%8;var c=(a-b)/8;var d=0;if(c>0){do{this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));this.p37.appendChild(this.f17(this.p18[d],d++));}while(--c)}if(b>0){d=a-b;do{this.p37.appendChild(this.f17(this.p18[d],d++));}while(--b)}this.f11();}else{this.f19();}}
function f19(){this.p37=c3[4].cloneNode(false);var a=this.p16.length;if(a>0){var b=a%8;var c=(a-b)/8;var i=a-b-1;var p16=this.p16;var d=0;if(c>0){do{this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));this.p37.appendChild(this.f17(d,d++));}while(--c)}if(b>0){d=a-b;do{this.p37.appendChild(this.f17(d,d++));}while(--b)}}this.f11();}
function f17(a,c){var r=c3[3].cloneNode(false);r.name="accesspointRow"+a;var s=c%2==0?"RowEven":"RowOdd";r.id=s;r.setAttribute('onmouseover',function anonymous(){r.className="accesspointRowOn";});if (r.id=='RowOdd') {r.setAttribute('onmouseout',function anonymous(){r.className='accesspointRowOdd';});} if (r.id=='RowEven') {r.setAttribute('onmouseout',function anonymous(){r.className='accesspointRowEven';});} r.className=this.p34+s;var l=this.p2.length;var i=l;do{var t=this.f22(a,l-i,s);r.appendChild(t);}while(--i)return r;}
function f23(rowNum,a,style){var currentRow=this.p16[rowNum];var colNum=a;var theCell=c3[2].cloneNode(false);theCell.align="left";theCell.noWrap=true;theCell.className=this.p11[a]+style;if(this.p13[a]!="."){eval(this.p13[a]);}else{theCell.innerHTML=this.p16[rowNum][a];}return theCell;}
function f22(a,b,c){var d=c3[2].cloneNode(false);d.align="left";d.noWrap=true;d.className=this.p11[b]+c;var e=c3[7].cloneNode(false);e.innerHTML=this.p16[a][b];d.appendChild(e);return d;}

function setStyle(s){this.setHeaderStyle(s);this.setHeaderLinkStyle(s);this.setRowStyle(s);this.setDataRowLinkStyle(s+"RowLink");this.setFooterRowStyle(s);this.setFilterStyle(s+"Filter");for(var i=0;i<this.p2.length;i++){}for(var i=0;i<this.p2.length;i++){this.p11[i]=s;}this.setTableStyle(s+"Table");this.setTableBodyStyle(s+"Table");if(this.setEditableCellStyle){this.setTextBoxStyle(s+"TextBox");this.setEditableCellStyles(s+"EditableCell");}this.p29=s;}
function setFilterStyle(s){this.p30=s;}
function setHeaderStyle(s){this.p31=s;}
function setFooterRowStyle(s){this.p32=s;}
function setHeaderLinkStyle(s){this.p33=s;}
function setDataRowLinkStyle(s){this.p35=s;}function setTableStyle(s){this.p28=s;}
function setTableBodyStyle(s){this.p36=s;}
function setRowStyle(s){this.p34=s;}
function setColumnHeaderStyle(a,b){var i=this.p2.length;if(typeof(a)=="string"){do{if(this.p2[i]==a)this.p10[i]=b;}while(--i);}else if(typeof(a)=="number"){this.p10[a]=b;}}
function setDataCellStyle(a,b){var i=this.p2.length;if(typeof(a)=="string"){do{if(this.p2[i]==a)this.p11[i]=b;}while(--i);}else if(typeof(a)=="number"){this.p11[a]=b;}}
function setRenderer(colNumber,renderingString){this.p13[colNumber]=renderingString;}
function removeRenderer(colNumber){this.p13[colNumber]=".";}
function setFiltering(b){var l=this.p2.length;var i=l;do{x=l-i;this.p39[x]=b;}while(--i);this.p15=b;}
function setColumnFilterable(a,f){this.p39[a]=f; if(f==true)this.p15=f;}
function setSort(b){var l=this.p2.length;var i=l;do{x=l-i;this.p38[x]=b;}while(--i);this.p6=b;}
function setColumnSortable(a,b){this.p38[a]=b;if(b==true)this.p6=b;}
function setPaginate(b){this.p5=b;}
function setPaginateRows(n){this.p4=n;}
function getRow(i){return this.p16[i];}
function addRow(a){this.p16[this.p16.length]=a;}
function remake(){this.f3();this.f30();}
function setRowData(a){this.p16=a;}
function getRowData(){return this.p16;}
function setColumnNames(a){this.p2=a;}
function setTableId(x){this.p1=x;}
function setActive(){this.p8=true;}
function setInactive(){this.p8=false;}
sp.setRenderer=setRenderer;sp.removeRenderer=removeRenderer;sp.setStyle=setStyle;sp.setHeaderStyle=setHeaderStyle;sp.setHeaderLinkStyle=setHeaderLinkStyle;sp.setTableStyle=setTableStyle;sp.setRowStyle=setRowStyle;sp.setDataRowLinkStyle=setDataRowLinkStyle;sp.setColumnHeaderStyle=setColumnHeaderStyle;sp.setFooterRowStyle=setFooterRowStyle;sp.setDataCellStyle=setDataCellStyle;sp.setFilterStyle=setFilterStyle;sp.setTableBodyStyle=setTableBodyStyle;sp.setSort=setSort;sp.setColumnSortable=setColumnSortable;sp.setFiltering=setFiltering;sp.setColumnFilterable=setColumnFilterable;sp.setPaginate=setPaginate;sp.setPaginateRows=setPaginateRows;sp.getRow=getRow;sp.addRow=addRow;sp.remake=remake;sp.setRowData=setRowData;sp.setColumnNames=setColumnNames;sp.setTableId=setTableId;sp.setActive=setActive;sp.setInactive=setInactive;sp.getRowData=getRowData;
function SortableTable(oTable, oSortTypes) {
this.element = oTable;
this.tHead = oTable.tHead;
this.tBody = oTable.tBodies[0];
this.document = oTable.ownerDocument || oTable.document;
this.sortColumn = null;
this.descending = null;
var oThis = this;
this._headerOnclick = function (e) {
oThis.headerOnclick(e);
};
var win = this.document.defaultView || this.document.parentWindow;
this._onunload = function () {
oThis.destroy();
};
if (win && typeof win.attachEvent != "undefined") {
win.attachEvent("onunload", this._onunload);
}
this.initHeader(oSortTypes || []);
}
SortableTable.gecko = navigator.product == "Gecko";
SortableTable.msie = /msie/i.test(navigator.userAgent);
SortableTable.removeBeforeSort = SortableTable.gecko;
SortableTable.prototype.onsort = function () {};
SortableTable.prototype.defaultDescending = false;
SortableTable.prototype._sortTypeInfo = {};
SortableTable.prototype.initHeader = function (oSortTypes) {
var cells = this.tHead.rows[0].cells;
var l = cells.length;
var img, c;
for (var i = 0; i < l; i++) {
c = cells[i];
if (oSortTypes[i] != null && oSortTypes[i] != "None") {
img = this.document.createElement("IMG");
img.src = "/images/shim.gif";
//c.appendChild(img);
if (oSortTypes[i] != null)
c._sortType = oSortTypes[i];
if (typeof c.addEventListener != "undefined")
c.addEventListener("click", this._headerOnclick, false);
else if (typeof c.attachEvent != "undefined")
c.attachEvent("onclick", this._headerOnclick);
else
c.onclick = this._headerOnclick;
}else{
c.setAttribute( "_sortType", oSortTypes[i] );
c._sortType = "None";
}
}
this.updateHeaderArrows();
};
SortableTable.prototype.uninitHeader = function (){
var cells = this.tHead.rows[0].cells;
var l = cells.length;
var c;
for (var i = 0; i < l; i++) {
c = cells[i];
if (c._sortType != null && c._sortType != "None") {
c.removeChild(c.lastChild);
if (typeof c.removeEventListener != "undefined")
c.removeEventListener("click", this._headerOnclick, false);
else if (typeof c.detachEvent != "undefined")
c.detachEvent("onclick", this._headerOnclick);
c._sortType = null;
c.removeAttribute( "_sortType" );
}
}
};
SortableTable.prototype.updateHeaderArrows = function () {};
SortableTable.prototype.headerOnclick = function (e) {
var el = e.target || e.srcElement;
while (el.tagName != "TD")
el = el.parentNode;
this.sort(SortableTable.msie ? SortableTable.getCellIndex(el) : el.cellIndex);
};
SortableTable.getCellIndex = function (oTd) {
var cells = oTd.parentNode.childNodes
var l = cells.length;
var i;
for (i = 0; cells[i] != oTd && i < l; i++);
return i;
};
SortableTable.prototype.getSortType = function (nColumn){
var cell = this.tHead.rows[0].cells[nColumn];
var val = cell._sortType;
if (val != "")
return val;
return "String";
};
SortableTable.prototype.sort = function (nColumn, bDescending, sSortType){
if (sSortType == null)
sSortType = this.getSortType(nColumn);
if (sSortType == "None")
return;
if (bDescending == null){
if (this.sortColumn != nColumn)
this.descending = this.defaultDescending;
else
this.descending = !this.descending;
}else
this.descending = bDescending;
this.sortColumn = nColumn;
if (typeof this.onbeforesort == "function")
this.onbeforesort();
var f = this.getSortFunction(sSortType, nColumn);
var a = this.getCache(sSortType, nColumn);
var tBody = this.tBody;
a.sort(f);
if (this.descending)
a.reverse();
if (SortableTable.removeBeforeSort){
var nextSibling = tBody.nextSibling;
var p = tBody.parentNode;
p.removeChild(tBody);
}
var l = a.length;
for (var i = 0; i < l; i++)
tBody.appendChild(a[i].element);
if (SortableTable.removeBeforeSort){
p.insertBefore(tBody, nextSibling);
}
this.updateHeaderArrows();
this.destroyCache(a);
if (typeof this.onsort == "function")
this.onsort();
};
SortableTable.prototype.asyncSort = function (nColumn, bDescending, sSortType) {
var oThis = this;
this._asyncsort = function () {
oThis.sort(nColumn, bDescending, sSortType);
};
window.setTimeout(this._asyncsort, 1);
};
SortableTable.prototype.getCache = function (sType, nColumn) {
var rows = this.tBody.rows;
var l = rows.length;
var a = new Array(l);
var r;
for (var i = 0; i < l; i++) {
r = rows[i];
a[i] = {
value: this.getRowValue(r, sType, nColumn),
element: r
};
};
return a;
};
SortableTable.prototype.destroyCache = function (oArray) {
var l = oArray.length;
for (var i = 0; i < l; i++) {
oArray[i].value = null;
oArray[i].element = null;
oArray[i] = null;
}
};
SortableTable.prototype.getRowValue = function (oRow, sType, nColumn) {
if (this._sortTypeInfo[sType] && this._sortTypeInfo[sType].getRowValue)
return this._sortTypeInfo[sType].getRowValue(oRow, nColumn);
var s;
var c = oRow.cells[nColumn];
if (typeof c.innerText != "undefined")
s = c.innerText;
else
s = SortableTable.getInnerText(c);
return this.getValueFromString(s, sType);
};
SortableTable.getInnerText = function (oNode) {
var s = "";
var cs = oNode.childNodes;
var l = cs.length;
for (var i = 0; i < l; i++) {
switch (cs[i].nodeType) {
case 1:
s += SortableTable.getInnerText(cs[i]);
break;
case 3:
s += cs[i].nodeValue;
break;
}
}
return s;
};
SortableTable.prototype.getValueFromString = function (sText, sType) {
if (this._sortTypeInfo[sType])
return this._sortTypeInfo[sType].getValueFromString( sText );
return sText;
};
SortableTable.prototype.getSortFunction = function (sType, nColumn) {
if (this._sortTypeInfo[sType])
return this._sortTypeInfo[sType].compare;
return SortableTable.basicCompare;
};
SortableTable.prototype.destroy = function () {
this.uninitHeader();
var win = this.document.parentWindow;
if (win && typeof win.detachEvent != "undefined") {
win.detachEvent("onunload", this._onunload);
}
this._onunload = null;
this.element = null;
this.tHead = null;
this.tBody = null;
this.document = null;
this._headerOnclick = null;
this.sortTypes = null;
this._asyncsort = null;
this.onsort = null;
};
SortableTable.prototype.addSortType = function (sType, fGetValueFromString, fCompareFunction, fGetRowValue) {
this._sortTypeInfo[sType] = {
type:				sType,
getValueFromString:	fGetValueFromString || SortableTable.idFunction,
compare:			fCompareFunction || SortableTable.basicCompare,
getRowValue:		fGetRowValue
};
};
SortableTable.prototype.removeSortType = function (sType) {
delete this._sortTypeInfo[sType];
};
SortableTable.basicCompare = function compare(n1, n2) {
if (n1.value < n2.value)
return -1;
if (n2.value < n1.value)
return 1;
return 0;
};
SortableTable.idFunction = function (x) {
return x;
};
SortableTable.toUpperCase = function (s) {
return s.toUpperCase();
};
SortableTable.toDate = function (s) {
var parts = s.split("-");
var d = new Date(0);
d.setFullYear(parts[0]);
d.setDate(parts[2]);
d.setMonth(parts[1] - 1);
return d.valueOf();
};
SortableTable.prototype.addSortType("String");
