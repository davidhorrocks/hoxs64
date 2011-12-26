#include <windows.h>
#include <windowsx.h>
#include "CDPI.h"
//+---------------------------------------------------------------------------
//	
//  Class:
//      CDPI
//
//  Implements:
//      Several utility functions useful for DPI aware applications.  
//		this helps with the conversions for apps which assume 96 DPI to	
//		convert to relative pixels where:
//			
//			relative pixel = 1 pixel at 96 DPI and scaled based on actual DPI.
// 
//
//  Synopsis:
//		This class provides utility functions for the following common tasks:
//		-- Get the screen DPI setting
//		-- Get the effective screen resolution (based on real resolution & DPI)
//		-- Convert from 96-DPI pixels to DPI-relative pixels 
//		-- Convert common structures (POINT, RECT, SIZE) to DPI-scaled values
//
//
//	This file is part of the Microsoft Windows SDK Code Samples.
//	
//	Copyright (C) Microsoft Corporation.  All rights reserved.
//	
//	This source code is intended only as a supplement to Microsoft
//	Development Tools and/or on-line documentation.  See these other
//	materials for detailed information regarding Microsoft code samples.
//	
//	THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//----------------------------------------------------------------------------

CDPI::CDPI() : m_fInitialized(false), m_dpiX(96), m_dpiY(96) { }
    
    // Get screen DPI.
    int CDPI::GetDPIX() { _Init(); return m_dpiX; }
    int CDPI::GetDPIY() { _Init(); return m_dpiY; }

    // Convert between raw pixels and relative pixels.
    int CDPI::ScaleX(int x) { _Init(); return MulDiv(x, m_dpiX, 96); }
    int CDPI::ScaleY(int y) { _Init(); return MulDiv(y, m_dpiY, 96); }
    int CDPI::UnscaleX(int x) { _Init(); return MulDiv(x, 96, m_dpiX); }
    int CDPI::UnscaleY(int y) { _Init(); return MulDiv(y, 96, m_dpiY); }

    // Determine the screen dimensions in relative pixels.
    int CDPI::ScaledScreenWidth() { return _ScaledSystemMetricX(SM_CXSCREEN); }
    int CDPI::ScaledScreenHeight() { return _ScaledSystemMetricY(SM_CYSCREEN); }

    // Scale rectangle from raw pixels to relative pixels.
    void CDPI::ScaleRect(__inout RECT *pRect)
    {
        pRect->left = ScaleX(pRect->left);
        pRect->right = ScaleX(pRect->right);
        pRect->top = ScaleY(pRect->top);
        pRect->bottom = ScaleY(pRect->bottom);
    }

    // Scale Point from raw pixels to relative pixels.
    void CDPI::ScalePoint(__inout POINT *pPoint)
    {
        pPoint->x = ScaleX(pPoint->x);
        pPoint->y = ScaleX(pPoint->y);        
    }

    // Scale Size from raw pixels to relative pixels.
    void CDPI::ScaleSize(__inout SIZE *pSize)
    {
        pSize->cx = ScaleX(pSize->cx);
        pSize->cy = ScaleX(pSize->cy);		
    }

    // Determine if screen resolution meets minimum requirements in relative pixels.
    bool CDPI::IsResolutionAtLeast(int cxMin, int cyMin) 
    { 
        return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin); 
    }

    // Convert a point size (1/72 of an inch) to raw pixels.
    int CDPI::PointsToPixels(int pt) { return MulDiv(pt, m_dpiY, 72); }

    // Invalidate any cached metrics.
    void CDPI::Invalidate() { m_fInitialized = false; }

    // This function initializes the CDPI Class
    void CDPI::_Init()
    {
        if (!m_fInitialized)
        {
            HDC hdc = GetDC(NULL);
            if (hdc)
            {
                // Initialize the DPI member variable
                // This will correspond to the DPI setting
                // With all Windows OS's to date the X and Y DPI will be identical					
                m_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
                m_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(NULL, hdc);
            }
            m_fInitialized = true;
        }
    }

    // This returns a 96-DPI scaled-down equivalent value for nIndex 
    // For example, the value 120 at 120 DPI setting gets scaled down to 96		
    // X and Y versions are provided, though to date all Windows OS releases 
    // have equal X and Y scale values
    int CDPI::_ScaledSystemMetricX(int nIndex) 
    { 
        _Init(); 
        return MulDiv(GetSystemMetrics(nIndex), 96, m_dpiX); 
    }

    // This returns a 96-DPI scaled-down equivalent value for nIndex 
    // For example, the value 120 at 120 DPI setting gets scaled down to 96		
    // X and Y versions are provided, though to date all Windows OS releases 
    // have equal X and Y scale values
    int CDPI::_ScaledSystemMetricY(int nIndex) 
    { 
        _Init(); 
        return MulDiv(GetSystemMetrics(nIndex), 96, m_dpiY); 
    }
