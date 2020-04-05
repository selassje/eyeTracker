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

#ifndef PICTURE_HPP
#define PICTURE_HPP

#include "stdafx.h"

class CPicture : public COleDispatchDriver {
public:
    enum {
        ID_HANDLE = 0x0,
        ID_HPAL = 0x2,
        ID_TYPE = 0x3,
        ID_WIDTH = 0x4,
        ID_HEIGHT = 0x5
    };

    CPicture() {}
    CPicture(LPDISPATCH pDispatch)
        : COleDispatchDriver(pDispatch)
    {
    }
    CPicture(const CPicture& dispatchSrc)
        : COleDispatchDriver(dispatchSrc)
    {
    }

    long GetHandle()
    {
        long result;
        GetProperty(ID_HANDLE, VT_I4, (void*)&result);
        return result;
    }

    long GetHPal()
    {
        long result;
        GetProperty(ID_HPAL, VT_I4, (void*)&result);
        return result;
    }

    void SetHPal(long propVal)
    {
        SetProperty(ID_HPAL, VT_I4, propVal);
    }

    short GetType()
    {
        short result;
        GetProperty(ID_TYPE, VT_I2, (void*)&result);
        return result;
    }

    long GetWidth()
    {
        long result;
        GetProperty(ID_WIDTH, VT_I4, (void*)&result);
        return result;
    }

    long GetHeight()
    {
        long result;
        GetProperty(ID_HEIGHT, VT_I4, (void*)&result);
        return result;
    }
};

#endif
