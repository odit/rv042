var j8Expand;
var j8SelectedFound;
var i, cookie;
var j8Index = 1;
cookie = getCookie(j8Name);
if (!cookie) {
if (!j8Expand) {
j8Expand = new Array();
for (i = 0; i < j8Index; i++)
j8Expand[i] = 0;
// j8Selected = 2;
}}
else {
j8Expand = cookie.split(",");
cookie = getCookie(j8Name);
if (!cookie){ ;}
else
j8Selected = cookie;
}
function getCookie(name) {
var search;
search = name + "=";
offset = document.cookie.indexOf(search) ;
if (offset != -1) {
offset += search.length;
end = document.cookie.indexOf(";", offset);
if (end == -1)
end = document.cookie.length;
return unescape(document.cookie.substring(offset, end));
}
else
return "";
}
function checkBrowser(){
var b = navigator.appName;
if (b == "Netscape"){
this.b = "ns";
}
else if (b == "Microsoft Internet Explorer"){
this.b = "ie";
}
else{
this.b = b;
}
this.dom = document.getElementById?1:0;
this.v = parseInt(navigator.appVersion);
this.ns = (this.b=="ns" && this.v>=4);
this.ns4 = (this.b=="ns" && this.v==4);
this.ns6 = (this.dom && !document.all) ? 1:0;
this.ie = (this.b=="ie" && this.v>=4);
this.ie4 = (navigator.userAgent.indexOf('MSIE 4')>0);
this.ie5 = (navigator.userAgent.indexOf('MSIE 5')>0);
if (this.ie5){
this.v = 5;
}
this.bw = (this.ie5 || this.ie4 || this.ns4 || this.ns6);
return this;
}
var selectedLink;
var selectedLayer;
bw = new checkBrowser();
function b_bgChange(BgColor,TextColor){
if (BgColor){
if (bw.dom||bw.ie4) { this.css.backgroundColor = BgColor; }
else if (bw.ns4) { this.css.bgColor = BgColor; }
selectedLink = this.el.id + '_link';
if (document.getElementById) document.getElementById(selectedLink).style.color= TextColor;
else if (document.all) document.all[selectedLink].style.color= TextColor;
}}
function unselect_Item()
{
if (selectedLink && document.getElementById){
document.getElementById(selectedLink).style.color= this.textUnselected;
}
else if (selectedLink && document.all){
document.all[selectedLink].style.color= this.textUnselected;
}
if (selectedLayer){
selectedLayer.style.background=unselect1;
selectedLayer.selected = false;
}
}
function makeMenuObj(obj,nest,img,offimage,onimage,mouseOverColor,mouseOutColor,doMouseOvers){
nest=(!nest) ? '':'document.'+nest+'.';
this.css=bw.dom? document.getElementById(obj).style:bw.ie4?document.all[obj].style:bw.ns4?eval(nest+"document.layers." +obj):0;
this.el=bw.dom?document.getElementById(obj):bw.ie4?document.all[obj]:bw.ns4?eval(nest+'document.'+obj):0;
this.ref=bw.dom || bw.ie4? document:bw.ns4?this.css.document:0;
this.x=(bw.ns4)? this.css.left:this.css.offsetLeft;
this.y=(bw.ns4)? this.css.top:this.css.offsetTop;
this.height=bw.ns4?this.ref.height:this.el.offsetHeight;
this.hideIt=b_hideIt; this.showIt=b_showIt; this.movey=b_movey;
this.status=0;
this.swap=b_swap;
this.bgChange=b_bgChange;
if(onimage)this.onimage=onimage.src;
this.offimage=offimage;
if(img)this.img=this.ref.images[img];
this.el.mouseOverColor= mouseOverColor;
this.el.mouseOutColor= mouseOutColor;
if(bw.ns4 && !(obj.indexOf('Cont')>-1) ){
this.el.onmouseover= function(){ if(!this.selected) this.bgColor= onselect1; };
this.el.onmouseout= function(){ if(!this.selected) this.bgColor= overselect1; };
}
else if( !(obj.indexOf('Cont')>-1) ){
this.el.onmouseover= function(){ if(!this.selected) {	this.style.background= onselect1; }
if(this.selected) this.style.background= onselect1; };
this.el.onmouseout= function(){ if(!this.selected) {this.style.background= overselect1; }
if(this.selected) this.style.background= select1; };
}
return this;
}
function b_showIt(){this.css.visibility="visible"; this.status=1;}
function b_hideIt(){this.css.visibility="hidden"; this.status=0;}
function b_movey(y){this.y=y; this.css.top=this.y}
function b_swap(on){
if(this.onimage && on){
this.img.src=this.onimage;
}else if(this.onimage && !on){
this.img.src=this.offimage;
}
}
function foldoutMenuObj(name){
this.menus=new Array();
this.name=name;
this.makeStyle=fold_style;
this.make=fold_make;
this.construct=fold_construct;
this.fold=fold;
this.foldstay=fold_stay;
this.foldsub1=fold_sub1;
this.foldsub1stay=fold_sub1_stay;
this.placeAll=fold_placeAll;
this.menus=new Array();
this.a=0; this.b; this.c; this.d; this.e; this.f;
this.initexec='';
this.opn=fold_opn;
this.unselect=unselect_Item;
}
function fold_construct(){
this.container=new makeMenuObj('div'+this.name+'Cont');
this.menu=new Array();
menuheight=0;
for(i=0;i<this.menus.length;i++){
this.menu[i]=new makeMenuObj('div'+this.name+i,'div'+this.name+'Cont',this.menus[i].img,this.menus[i].offimage,this.menus[i].onimage,this.mouseOverColor,this.mouseOutColor,this.doMouseOvers);
this.menu[i].subs=this.menus[i].subs;
this.menu[i].sub=new Array();
for(j=0;j<this.menu[i].subs;j++){
this.menu[i].sub[j]=new makeMenuObj('div'+this.name+i+"_"+j,'div'+this.name+'Cont',this.menus[i][j].img,this.menus[i][j].offimage,this.menus[i][j].onimage,this.mouseOverColor,this.mouseOutColor,this.doMouseOvers);
this.menu[i].sub[j].subs=this.menus[i][j].subs;
this.menu[i].sub[j].sub=new Array();
}
}
this.menus="";
if(this.initexec) eval(this.initexec);
else this.placeAll();
this.container.showIt();
}
function fold_opn(a,b,c,d,e,f){
if(a>-1) this.initexec+="this.fold("+a+","+this.sub1stay+");";
if(b>-1) this.initexec+="this.foldsub1("+a+","+b+","+this.sub2stay+");";
}
function fold_stay(a){
for(z=0;z<this.menu.length;z++){
if(z!=a) this.fold(z,1,1);
}
this.fold(a,1,0);
}
function fold(a,fromtop,noplace){
if(fromtop){
for(b=0;b<this.menu[a].subs;b++){
if(this.menu[a].sub[b].status || noplace){
this.menu[a].sub[b].hideIt();
this.menu[a].swap(0);
this.foldsub1(a,b,1,1);
}else{
this.menu[a].sub[b].showIt();
this.menu[a].swap(1);
}
}
if(!noplace){
if(this.doHighlights){
this.unselect();
this.menu[a].el.selected = true;
selectedLayer= this.menu[a].el;
this.menu[a].bgChange(BGCOSELECT,this.textSelected);
}
this.placeAll();
}
}else this.foldstay(a);
}
function fold_sub1_stay(a,b){
for(z=0;z<this.menu[a].subs;z++){
if(b!=z) this.foldsub1(a,z,1,1);
}
this.foldsub1(a,b,1,0);
}
function fold_sub1(a,b,fromtop,noplace){
if(fromtop){
for(c=0;c<this.menu[a].sub[b].subs;c++){
if(this.menu[a].sub[b].sub[c].status || noplace){
this.menu[a].sub[b].sub[c].hideIt();
this.menu[a].sub[b].swap(0);
this.foldsub2(a,b,c,1,1);
}else{
this.menu[a].sub[b].sub[c].showIt();
this.menu[a].sub[b].swap(1);
}
}
if(!noplace) {
if(this.doHighlights){
this.unselect();
this.menu[a].sub[b].el.selected = true;
selectedLayer= this.menu[a].sub[b].el;
this.menu[a].sub[b].bgChange(BGCOSELECT,this.textSelected);
}
this.placeAll();
}
}else this.foldsub1stay(a,b);
}
function fold_placeAll(){
var calc
menuheight=0;
for(i=0;i<this.menu.length;i++){
calc = menuheight-(28*i)
this.menu[i].movey(calc);
menuheight+=25;
for(j=0;j<this.menu[i].subs;j++){
this.menu[i].sub[j].movey(menuheight);
if(this.menu[i].sub[j].status) menuheight+=22;
}}}
function fold_style(){
}
function fold_make_link(text,lnk,target,offimage,id,cl,ev,acl,fc1){
str2='\t<div id="div'+id+'" class="cl'+cl+'"><a href="';
if(lnk) str2+=lnk+'" ';
else str2+='#" ';
if(!lnk || target){
str2+='onclick="'+ev;
if(!target) str2+='; return false'; str2+='" ';
}
if(target) str2+='target="'+target+'" ';
str2 += 'onMouseOver=\'window.status=\" \"; return true;\' ';
str2 += 'onfocus="if(this.blur)this.blur();" ';
str2 += 'id="div' +id+ '_link" ';
str2 += 'class="cl'+acl+'Links"><nobr>';
if(offimage) str2+= '<img src="'+offimage+'" name="img'+id+'" border="0">';
str2 += text+'</a></nobr><br></div>\n';
return str2;
}
var x ="text+;"
var newnum;
var j8Depth;
var j8Bars;
function j8Click(num) {
var j8Days = 1;
j8Selected = num;
var date = new Date();
date.setTime (date.getTime() + (600 * 10000));
document.cookie = j8Name + "=" + escape(j8Selected) + "; path=/";
}
function fold_make(type,text,lnk,target,offimage,onimage,fc,opn,end){
str="" ; fc=fc?fc+'; ':'';
if(!offimage) offimage=""; if(!onimage) onimage="";
if(this.a==0) str='<div id="div'+this.name+'Cont">\n';
if(type=="top"){
id=this.name+this.a;
if(opn!=j8Selected){str+=fold_make_link(text,lnk,target,offimage,this.name+this.a,this.name,fc+this.name+'.fold('+this.a+','+this.name+'.sub1stay)',this.name,fc);}
if(opn==j8Selected){str+=fold_make_link(text,lnk,target,offimage,this.name+this.a,this.name+'k',fc+this.name+'.fold('+this.a+','+this.name+'.sub1stay)',this.name,fc); this.opn(this.a);}
this.menus[this.a]=new Array();
this.menus[this.a].subs=0;
if(onimage){ this.menus[this.a].onimage=new Image(); this.menus[this.a].onimage.src=onimage; this.menus[this.a].offimage=offimage; this.menus[this.a].img='img'+id}
this.a++; this.b=0;
}else {
id=this.name+(this.a-1)+'_'+(this.b),this.name+'Sub1';
if(opn!=j8Selected) {str+=fold_make_link(text,lnk,target,offimage,id,this.name+'Sub1',fc+this.name+'.foldsub1('+(this.a-1)+','+(this.b)+','+this.name+'.sub2stay)',this.name+'1',fc);}
if(opn==j8Selected) {str+=fold_make_link(text,lnk,target,offimage,id,this.name+'Sub1k',fc+this.name+'.foldsub1('+(this.a-1)+','+(this.b)+','+this.name+'.sub2stay)',this.name+'1',fc);this.opn(this.a-1,this.b);}
this.menus[this.a-1][this.b]=new Array();
if(onimage){ this.menus[this.a-1][this.b].onimage=new Image(); this.menus[this.a-1][this.b].onimage.src=onimage; this.menus[this.a-1][this.b].offimage=offimage; this.menus[this.a-1][this.b].img='img'+id}
this.b++; this.menus[this.a-1].subs=this.b; this.c=0;
}
if(end) str+="</div>";
document.write(str);
}
window.defaultStatus="";
