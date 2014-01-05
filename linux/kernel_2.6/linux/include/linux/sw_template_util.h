#ifndef __SW_TEMPLATE_UTIL_H__
#define __SW_TEMPLATE_UTIL_H__


int Sw_Template_Trans_Str_2_Hex ( char str )
{

	if(str<='9'&&str>='0')
		return (str - '0');
	if(str<='f'&&str>='a')
		return (str - 'a')+10;
	if(str<='F'&&str>='A')
		return (str - 'A')+10;
	return 0;
}

#endif
