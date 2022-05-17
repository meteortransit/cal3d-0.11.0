//----------------------------------------------------------------------------//
// MaxMaterialExport.cpp                                                      //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//

#include "StdAfx.h"
#include "MaxMaterialExport.h"
#include "Exporter.h"
#include "MaxInterface.h"

#if defined(_UNICODE)
#define tstring wstring
#else
#define tstring string
#endif

//----------------------------------------------------------------------------//
// Constructors                                                               //
//----------------------------------------------------------------------------//

CMaxMaterialExport::CMaxMaterialExport()
{
}

//----------------------------------------------------------------------------//
// Destructor                                                                 //
//----------------------------------------------------------------------------//

CMaxMaterialExport::~CMaxMaterialExport()
{
}

//----------------------------------------------------------------------------//
// Following methods have to be implemented to make it a valid plugin         //
//----------------------------------------------------------------------------//

const TCHAR *CMaxMaterialExport::AuthorName()
{
	return _T("IMVU Inc; Bruno 'Beosil' Heidelberger");
}

const TCHAR *CMaxMaterialExport::CopyrightMessage()
{
	return _T("Copyright (c) 2010 IMVU Inc; Copyright (C) 2001 Bruno 'Beosil' Heidelberger");
}

int CMaxMaterialExport::DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// create an export interface for 3d studio max
	CMaxInterface maxInterface;
	if(!maxInterface.Create(ei, i))
	{
		MessageBoxA(0, "Max Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	// create an exporter instance
	if(!theExporter.Create(&maxInterface))
	{
		MessageBoxA(0, "Exporter Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	// export the materials
    std::tstring tname(name);
    std::string cname(tname.begin(), tname.end());
	if(!theExporter.ExportMaterial(cname.c_str()))
	{
		MessageBoxA(0, "Material Export Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	return 1;
}

const TCHAR *CMaxMaterialExport::Ext(int i)
{
	switch(i)
	{
	case 0:
		return _T("xrf");
	default:
		return _T("");
	}
}

int CMaxMaterialExport::ExtCount()
{
	return 1;
}

const TCHAR *CMaxMaterialExport::LongDesc()
{
	return _T("Cal3D Material File");
}

const TCHAR *CMaxMaterialExport::OtherMessage1()
{
	return _T("");
}

const TCHAR *CMaxMaterialExport::OtherMessage2()
{
	return _T("");
}

const TCHAR *CMaxMaterialExport::ShortDesc()
{
	return _T("Cal3D Material File");
}

void CMaxMaterialExport::ShowAbout(HWND hWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDialog dlg(IDD_ABOUT);
	dlg.DoModal();
}

unsigned int CMaxMaterialExport::Version()
{
	return 50;
}

//----------------------------------------------------------------------------//
bool CMaxMaterialExport::ExportMaterialFromMaxscriptCall(const char* fullpathfilename, StdMat* _stdmatfrommaxscript)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// create an export interface for 3d studio max
	CMaxInterface maxInterface;
	
	//Set the tab of materials into our Max interface
	if(! maxInterface.Create(NULL, GetCOREInterface(), _stdmatfrommaxscript))
	{
		MessageBoxA(0, "Max Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	// create an exporter instance
	if(!theExporter.Create(&maxInterface))
	{
		MessageBoxA(0, "Exporter Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	// export the materials
	if(!theExporter.ExportMaterialFromMaxscriptCall(fullpathfilename))
	{
		MessageBoxA(0, "Material Exporter Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	return 1;	
}