/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                       http://www.mrpt.org/                                |
   |                                                                           |
   |   Copyright (C) 2005-2011  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */
#ifndef CABOUTBOX_H
#define CABOUTBOX_H

//(*Headers(CAboutBox)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statbmp.h>
//*)

class CAboutBox: public wxDialog
{
public:

    CAboutBox(wxWindow* parent,wxWindowID id = -1);
    virtual ~CAboutBox();

    //(*Identifiers(CAboutBox)
    static const long ID_STATICTEXT1;
    static const long ID_STATICTEXT2;
    static const long ID_STATICBITMAP1;
    static const long ID_STATICLINE1;
    static const long ID_TEXTCTRL1;
    static const long ID_TEXTCTRL2;
    static const long ID_TEXTCTRL3;
    static const long ID_NOTEBOOK1;
    static const long ID_BUTTON1;
    //*)

protected:

    //(*Handlers(CAboutBox)
    void OnInit(wxInitDialogEvent& event);
    void OnButton1Click(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);
    //*)

    //(*Declarations(CAboutBox)
    wxStaticText* lbProgName;
    wxFlexGridSizer* FlexGridSizer1;
    wxTextCtrl* lbLicense;
    wxButton* Button1;
    wxFlexGridSizer* FlexGridSizer4;
    wxStaticLine* StaticLine1;
    wxStaticText* lbBuild;
    wxTextCtrl* TextCtrl1;
    wxNotebook* Notebook1;
    wxTextCtrl* lbInfo;
    wxStaticBitmap* StaticBitmap1;
    //*)

private:

    DECLARE_EVENT_TABLE()
};

#endif
