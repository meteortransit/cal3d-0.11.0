//----------------------------------------------------------------------------//
// StdAfx.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//

#ifndef STDAFX_H
#define STDAFX_H

//----------------------------------------------------------------------------//
// Defines                                                                    //
//----------------------------------------------------------------------------//

#ifdef _WIN32
#define VC_EXTRALEAN
#endif

//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//

#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>

#include "max.h"
#if MAX_VERSION_MAJOR >= 14
#include "maxscript/maxscript.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\maxwrapper\mxsobjects.h"
#include "maxscript\maxwrapper\mxsmaterial.h"
#else
#include "maxscrpt/maxscrpt.h"
#include "maxscrpt/Strings.h"
#include "maxscrpt/arrays.h"
#include "maxscrpt/numbers.h"
#include "maxscrpt/maxobj.h"
#include "maxscrpt/definsfn.h"
#include "maxscrpt/maxmats.h"
#endif
#include "bipexp.h"
#include "phyexp.h"
#include "decomp.h"

#if MAX_RELEASE >= 4000
#include "iskin.h"
#endif

#include "resource.h"

#ifdef interface
#undef interface
#endif

//----------------------------------------------------------------------------//
// MSVC++ stuff                                                               //
//----------------------------------------------------------------------------//

//{{AFX_INSERT_LOCATION}}

#endif

//----------------------------------------------------------------------------//
