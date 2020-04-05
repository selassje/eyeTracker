/*
MIT License

Copyright (c) 2020 Przemyslaw Koziol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef FONT_HPP
#define FONT_HPP

#include "stdafx.h"

class COleFont : public COleDispatchDriver {
public:
    struct ID {
        enum {
            NAME = 0x0,
            SIZE = 0x2,
            BOLD = 0x3,
            ITALIC = 0x4,
            UNDERLINE = 0x5,
            STRIKETHROUGH = 0x6,
            WEIGHT = 0x7,
            CHARSET = 0x8,
        };
    };
    COleFont() {}
    COleFont(LPDISPATCH pDispatch)
        : COleDispatchDriver(pDispatch)
    {
    }
    COleFont(const COleFont& dispatchSrc)
        : COleDispatchDriver(dispatchSrc)
    {
    }

    CString GetName()
    {
        CString result;
        GetProperty(ID::NAME, VT_BSTR, (void*)&result);
        return result;
    }

    void SetName(LPCTSTR propVal)
    {
        SetProperty(ID::NAME, VT_BSTR, propVal); //-V111
    }

    CY GetSize()
    {
        CY result;
        GetProperty(ID::SIZE, VT_CY, (void*)&result);
        return result;
    }

    void SetSize(const CY& propVal)
    {
        SetProperty(ID::SIZE, VT_CY, &propVal); //-V111
    }

    BOOL GetBold()
    {
        BOOL result;
        GetProperty(ID::BOLD, VT_BOOL, (void*)&result);
        return result;
    }

    void SetBold(BOOL propVal)
    {
        SetProperty(ID::BOLD, VT_BOOL, propVal);
    }

    BOOL GetItalic()
    {
        BOOL result;
        GetProperty(ID::ITALIC, VT_BOOL, (void*)&result);
        return result;
    }

    void SetItalic(BOOL propVal)
    {
        SetProperty(ID::ITALIC, VT_BOOL, propVal);
    }

    BOOL GetUnderline()
    {
        BOOL result;
        GetProperty(ID::UNDERLINE, VT_BOOL, (void*)&result);
        return result;
    }

    void SetUnderline(BOOL propVal)
    {
        SetProperty(ID::UNDERLINE, VT_BOOL, propVal);
    }

    BOOL GetStrikethrough()
    {
        BOOL result;
        GetProperty(ID::STRIKETHROUGH, VT_BOOL, (void*)&result);
        return result;
    }

    void SetStrikethrough(BOOL propVal)
    {
        SetProperty(ID::STRIKETHROUGH, VT_BOOL, propVal);
    }

    short GetWeight()
    {
        short result;
        GetProperty(ID::WEIGHT, VT_I2, (void*)&result);
        return result;
    }

    void SetWeight(short propVal)
    {
        SetProperty(ID::WEIGHT, VT_I2, propVal);
    }

    short GetCharset()
    {
        short result;
        GetProperty(ID::CHARSET, VT_I2, (void*)&result);
        return result;
    }

    void SetCharset(short propVal)
    {
        SetProperty(ID::CHARSET, VT_I2, propVal);
    }
};

#endif
