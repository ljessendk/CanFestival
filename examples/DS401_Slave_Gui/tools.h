#ifndef TOOLS_H_
#define TOOLS_H_

	#include <wx/wxprec.h>
	#include <wx/wx.h>
	#include <wx/textctrl.h>
	#include <iostream>

	void	IntToString( int i, char* in_pszbuffer );
	char	*wxstr_to_charbuf(wxString	s);
	void	charbuf_free(char	*s);

#endif /*TOOLS_H_*/
