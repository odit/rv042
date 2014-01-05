function doser() {
	top.searchText=document.searchForm.userText.value;
        cleanInput();
        //remove <> from words
	top.searchText = top.searchText.replace(/</g," ");
	top.searchText = top.searchText.replace(/>/g," ");
        top.searchText =trimString(top.searchText);

        window.open('search_results.htm','contentiframe');
}
function trimString (str) {
  str = this != window? this : str;
  return str.replace(/^\s+/g, '').replace(/\s+$/g, '');
}
// Function will ensure that all characters in the search string are recognizable.
function cleanInput() {
	
	//if the search string is all spaces, do not do a search
	var ind = 0;
	while (ind < top.searchText.length && 
		(top.searchText.charAt(ind) == ' ' || top.searchText.charCodeAt(ind) == 12288)) {
		ind++;
	}
	if (ind == top.searchText.length)
		top.searchText = "";
	else if (ind > 0)
		top.searchText = top.searchText.substring(ind);
	ind = top.searchText.length-1;
	while (ind > -1 && 
		(top.searchText.charAt(ind) == ' ' || top.searchText.charCodeAt(ind) == 12288)) {
		ind--;
	}
	if (ind == -1)
		top.searchText = "";
	else if (ind < top.searchText.length-1);
		top.searchText = top.searchText.substring(0,ind+1);

	if (top.searchText.length > 0) {

		cleanISOInput();

		if (top.charsetEncoding == "shift_jis" 
		 || top.charseteEncoding == "big5"
		 || top.charsetEncoding == "gb3212"
		 || top.charsetEncoding == "euc_kr"
		 || top.charsetEncoding == undefined) 
			cleanCKJVInput();
		
		//get rid of repeated spaces
		top.searchText = top.searchText.replace(/[ ]+/g," ");
		top.searchText = top.searchText.replace(/\"/g,"'");
	}
}
//deals with ascii representation of letters and symbols
function cleanISOInput() {

	//get rid of repeated spaces
	top.searchText = top.searchText.replace(/[ ]+/g," ");

	//remove symbols from beginning and end of words 
	top.searchText = top.searchText.replace(/[~!@#$%^&*\(\)_+`\-=\{\}|\[\]\\:\";'<>?,.\/]+ /gi," ");	
	top.searchText = top.searchText.replace(/ [~!@#$%^&*\(\)_+`\-=\{\}|\[\]\\:\";'<>?,.\/]+/gi," ");	
	top.searchText = top.searchText.replace(/[~!@#$%^&*\(\)_+`\-=\{\}|\[\]\\:\";'<>?,.\/]+$|^[~!@#$%^&*\(\)_+`\-=\{\}|\[\]\\:\";'<>?,.\/]+/gi,"");

	//separate words containing '/', ':', '-', '[', or ']' into two words
	top.searchText = top.searchText.replace(/\b[\/:\-\[\]]\b/g," ");
	
	//remove parenthesis from words ending in (s) or (es)
	top.searchText = top.searchText.replace(/\(s\)$/gi,"s");
	top.searchText = top.searchText.replace(/\(s\) /gi,"s ");
	top.searchText = top.searchText.replace(/\(es\)$/gi,"es");
	top.searchText = top.searchText.replace(/\(es\) /gi,"es ");
}

// when document.charset == "shift_jis"
function cleanCKJVInput() {

	var i = 0;
	var currCharCode;
	
	//remove leading and trailing symbols
	while ((top.searchText.charCodeAt(i) > 65279 && top.searchText.charCodeAt(i) < 65520)
		|| (top.searchText.charCodeAt(i) > 12288 && top.searchText.charCodeAt(i) < 12302)
		|| top.searchText.charCodeAt(i) == 8216
		|| top.searchText.charCodeAt(i) == 8217
		|| top.searchText.charCodeAt(i) == 8221) {
		i++;	
	}
	if (i == top.searchText.length)
		top.searchText = "";
	else if (i > 0)
		top.searchText = top.searchText.substring(i);
	i = top.searchText.length-1;
	while ((top.searchText.charCodeAt(i) > 65279 && top.searchText.charCodeAt(i) < 65520)
		|| (top.searchText.charCodeAt(i) > 12288 && top.searchText.charCodeAt(i) < 12302)
		|| top.searchText.charCodeAt(i) == 8216
		|| top.searchText.charCodeAt(i) == 8217
		|| top.searchText.charCodeAt(i) == 8221) {
		i--;
	}
	if (i == -1)
		top.searchText = "";
	else if (i < top.searchText.length-1);
		top.searchText = top.searchText.substring(0,i+1);
	
	for (i=0; i<top.searchText.length-1; i++) {
		currCharCode = top.searchText.charCodeAt(i);				
		// replace full-width spaces with normal spaces
		if (currCharCode == 12288) {
			top.searchText = top.searchText.substring(0,i) 
						+ String.fromCharCode(32) 
						+ top.searchText.substring(i+1,top.searchText.length);
		}
	
		// add spaces between each japanese character
		else if ((currCharCode > 12351 && currCharCode < 12544)
			|| (currCharCode > 16621 && currCharCode < 40960)) {
				top.searchText = top.searchText.substring(0,i+1)
							+ String.fromCharCode(32)
							+ top.searchText.substring(i+1,top.searchText.length);
		}
	}
}

