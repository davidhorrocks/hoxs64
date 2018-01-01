#pragma once
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

class CDPI
{
public:
    CDPI();
    
    // Get screen DPI.
    int GetDPIX();
    int GetDPIY();

    // Convert between raw pixels and relative pixels.
    int ScaleX(int x);
    int ScaleY(int y);
    int UnscaleX(int x);
    int UnscaleY(int y);

    // Determine the screen dimensions in relative pixels.
    int ScaledScreenWidth();
    int ScaledScreenHeight();

    // Scale rectangle from raw pixels to relative pixels.
    void ScaleRect(__inout RECT *pRect);

    // Scale Point from raw pixels to relative pixels.
    void ScalePoint(__inout POINT *pPoint);

    // Scale Size from raw pixels to relative pixels.
    void ScaleSize(__inout SIZE *pSize);

    // Determine if screen resolution meets minimum requirements in relative pixels.
    bool IsResolutionAtLeast(int cxMin, int cyMin);

    // Convert a point size (1/72 of an inch) to raw pixels.
    int PointsToPixels(int pt);

    // Invalidate any cached metrics.
    void Invalidate();

private:

    // This function initializes the CDPI Class
    void _Init();

    // This returns a 96-DPI scaled-down equivalent value for nIndex 
    // For example, the value 120 at 120 DPI setting gets scaled down to 96		
    // X and Y versions are provided, though to date all Windows OS releases 
    // have equal X and Y scale values
    int _ScaledSystemMetricX(int nIndex);

    // This returns a 96-DPI scaled-down equivalent value for nIndex 
    // For example, the value 120 at 120 DPI setting gets scaled down to 96		
    // X and Y versions are provided, though to date all Windows OS releases 
    // have equal X and Y scale values
    int _ScaledSystemMetricY(int nIndex);

private:
        
    // Member variable indicating whether the class has been initialized
    bool m_fInitialized; 

    // X and Y DPI values are provided, though to date all 
    // Windows OS releases have equal X and Y scale values
    int m_dpiX;			
    int m_dpiY;
};
