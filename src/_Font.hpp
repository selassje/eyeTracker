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

class COleFont : public COleDispatchDriver {
public:
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
        GetProperty(0x0, VT_BSTR, (void*)&result);
        return result;
    }

    void SetName(LPCTSTR propVal)
    {
        SetProperty(0x0, VT_BSTR, propVal);
    }

    CY GetSize()
    {
        CY result;
        GetProperty(0x2, VT_CY, (void*)&result);
        return result;
    }

    void SetSize(const CY& propVal)
    {
        SetProperty(0x2, VT_CY, &propVal);
    }

    BOOL GetBold()
    {
        BOOL result;
        GetProperty(0x3, VT_BOOL, (void*)&result);
        return result;
    }

    void SetBold(BOOL propVal)
    {
        SetProperty(0x3, VT_BOOL, propVal);
    }

    BOOL GetItalic()
    {
        BOOL result;
        GetProperty(0x4, VT_BOOL, (void*)&result);
        return result;
    }

    void SetItalic(BOOL propVal)
    {
        SetProperty(0x4, VT_BOOL, propVal);
    }

    BOOL GetUnderline()
    {
        BOOL result;
        GetProperty(0x5, VT_BOOL, (void*)&result);
        return result;
    }

    void SetUnderline(BOOL propVal)
    {
        SetProperty(0x5, VT_BOOL, propVal);
    }

    BOOL GetStrikethrough()
    {
        BOOL result;
        GetProperty(0x6, VT_BOOL, (void*)&result);
        return result;
    }

    void SetStrikethrough(BOOL propVal)
    {
        SetProperty(0x6, VT_BOOL, propVal);
    }

    short GetWeight()
    {
        short result;
        GetProperty(0x7, VT_I2, (void*)&result);
        return result;
    }

    void SetWeight(short propVal)
    {
        SetProperty(0x7, VT_I2, propVal);
    }

    short GetCharset()
    {
        short result;
        GetProperty(0x8, VT_I2, (void*)&result);
        return result;
    }

    void SetCharset(short propVal)
    {
        SetProperty(0x8, VT_I2, propVal);
    }
};

#endif