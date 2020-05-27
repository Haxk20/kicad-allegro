///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/bmpbuttn.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_MIGRATE_SETTINGS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_MIGRATE_SETTINGS_BASE : public DIALOG_SHIM
{
	private:

	protected:
		wxBoxSizer* m_sizer;
		wxStaticText* m_lblWelcome;
		wxStaticText* m_staticText2;
		wxRadioButton* m_btnPrevVer;
		wxComboBox* m_cbPath;
		wxBitmapButton* m_btnCustomPath;
		wxStaticText* m_lblPathError;
		wxRadioButton* m_btnUseDefaults;
		wxStaticLine* m_staticline1;
		wxStdDialogButtonSizer* m_standardButtons;
		wxButton* m_standardButtonsOK;
		wxButton* m_standardButtonsCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnPrevVerSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPathChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPathDefocused( wxFocusEvent& event ) { event.Skip(); }
		virtual void OnChoosePath( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDefaultSelected( wxCommandEvent& event ) { event.Skip(); }


	public:

		DIALOG_MIGRATE_SETTINGS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Configure KiCad Settings Path"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION );
		~DIALOG_MIGRATE_SETTINGS_BASE();

};

