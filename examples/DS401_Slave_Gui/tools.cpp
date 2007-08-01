#include <iostream>
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/textctrl.h>
#include <iostream>
#include "tools.h"

 void 	IntToString( int i, char* in_pszbuffer )
{
	// permet de copier un int dans la chaîne buffer
	sprintf( in_pszbuffer, "%d", i);
}

char	*wxstr_to_charbuf(wxString	s)
{
	int		i = 0;
	char	*str = NULL;
	
	str = (char*) malloc((s.Length() + 1) * sizeof(char));
 	for(i=0; i<s.Length(); i++)
  	  {
   		str[i] = s.GetChar(i);
  	  }
 	str[s.Length()] = '\0';
 	return(str);
}

void	charbuf_free(char	*s)
{
	free(s);
}
