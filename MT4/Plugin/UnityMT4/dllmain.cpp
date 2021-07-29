#include "pch.h"

#include <cstdio>
#include <Windows.h>
#include <cstdlib>
#include <tchar.h>

#define MT4_EXPFUNC __declspec(dllexport)

#pragma pack(push,1)
struct RateInfo
{
    __int64           ctm;
    double            open;
    double            low;
    double            high;
    double            close;
    unsigned __int64  vol_tick;
    int               spread;
    unsigned __int64  vol_real;
};
#pragma pack(pop)

struct MqlStr
{
    int               len;
    char* string;
};


static HANDLE _receive_pipe;
static HANDLE _send_pipe;

static void OpenPipes(int wait)
{
    // ハンドルが生きてるなら再作成しないように

    _receive_pipe = CreateNamedPipe(_T("\\\\.\\pipe\\UnityMT4_FromUnity"),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        0,
        0,
        wait,
        nullptr);

    _send_pipe = CreateNamedPipe(_T("\\\\.\\pipe\\UnityMT4_ToUnity"),
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        0,
        0,
        wait,
        nullptr);
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        _receive_pipe = INVALID_HANDLE_VALUE;
        _send_pipe = INVALID_HANDLE_VALUE;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: ;
    }
    return TRUE;
}


MT4_EXPFUNC int __stdcall ConnectInitialize(const int wait)
{
    OpenPipes(wait);
    return _receive_pipe != INVALID_HANDLE_VALUE && _send_pipe != INVALID_HANDLE_VALUE;
}

MT4_EXPFUNC wchar_t* __stdcall GetCommandString(wchar_t* result, int buffer_size)
{
    static const wchar_t* dmy_str = L"some command string";

    swprintf_s(result, buffer_size, L"return is %s", dmy_str);

    return result;
}


static int CompareMqlStr(const void* left, const void* right);


MT4_EXPFUNC int __stdcall GetIntValue(const int ipar)
{
    printf("GetIntValue takes %d\n", ipar);
    return(ipar);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC double __stdcall GetDoubleValue(const double dpar)
{
    printf("GetDoubleValue takes %.8lf\n", dpar);
    return(dpar);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC wchar_t* __stdcall GetStringValue(wchar_t* spar)
{
    wprintf(L"GetStringValue takes \"%s\"\n", spar);
    return(spar);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC double __stdcall GetArrayItemValue(const double* arr, const int arraysize, const int nitem)
{
    //---
    if (arr == nullptr)
    {
        printf("GetArrayItemValue: NULL array\n");
        return(0.0);
    }
    if (arraysize <= 0)
    {
        printf("GetArrayItemValue: wrong arraysize (%d)\n", arraysize);
        return(0.0);
    }
    if (nitem < 0 || nitem >= arraysize)
    {
        printf("GetArrayItemValue: wrong item number (%d)\n", nitem);
        return(0.0);
    }
    //---
    return(arr[nitem]);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC bool _stdcall SetArrayItemValue(double* arr, const int arraysize, const int nitem, const double value)
{
    //---
    if (arr == nullptr)
    {
        printf("GetArrayItemValue: NULL array\n");
        return(FALSE);
    }
    if (arraysize <= 0)
    {
        printf("GetArrayItemValue: wrong arraysize (%d)\n", arraysize);
        return(FALSE);
    }
    if (nitem < 0 || nitem >= arraysize)
    {
        printf("GetArrayItemValue: wrong item number (%d)\n", nitem);
        return(FALSE);
    }
    //---
    arr[nitem] = value;
    return(TRUE);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC double __stdcall GetRatesItemValue(const RateInfo* rates, const int rates_total, const int shift, const int nrate)
{
    if (rates == nullptr)
    {
        printf("GetRatesItemValue: NULL array\n");
        return(0.0);
    }

    if (rates_total < 0)
    {
        printf("GetRatesItemValue: wrong rates_total number (%d)\n", rates_total);
        return(0.0);
    }

    if (shift < 0 || shift >= rates_total)
    {
        printf("GetRatesItemValue: wrong shift number (%d)\n", shift);
        return(0.0);
    }

    if (nrate < 0 || nrate>5)
    {
        printf("GetRatesItemValue: wrong rate index (%d)\n", nrate);
        return(0.0);
    }

    const int nitem = rates_total - 1 - shift;
    switch (nrate)
    {
    case 0: 
        return double(rates[nitem].ctm);
    case 1: 
        return rates[nitem].open;
    case 2: 
        return rates[nitem].low;
    case 3: 
        return rates[nitem].high;
    case 4: 
        return rates[nitem].close;
    case 5: 
        return double(rates[nitem].vol_tick);
    default:;
        return(0.0);
    }
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
MT4_EXPFUNC int __stdcall SortStringArray(MqlStr* arr, const int arraysize)
{
    //---
    if (arr == nullptr)
    {
        printf("SortStringArray: NULL array\n");
        return(-1);
    }
    if (arraysize <= 0)
    {
        printf("SortStringArray: wrong arraysize (%d)\n", arraysize);
        return(-1);
    }
    //---
    qsort(arr, arraysize, sizeof(MqlStr), CompareMqlStr);
    //---
    return(arraysize);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
/*
MT4_EXPFUNC int __stdcall ProcessStringArray(MqlStr* arr, const int arraysize)
{
    int   len1, len2;
    //---
    if (arr == nullptr)
    {
        printf("ProcessStringArray: NULL array\n");
        return(-1);
    }
    if (arraysize <= 0)
    {
        printf("ProcessStringArray: wrong arraysize (%d)\n", arraysize);
        return(-1);
    }
    //---
    for (int i = 0; i < arraysize - 1; i++)
    {
        if (arr[i].string == nullptr)
            len1 = 0;
        else
            len1 = strlen(arr[i].string);

        if (arr[i + 1].string == nullptr)
            len2 = 0;
        else
            len2 = strlen(arr[i + 1].string);
        //--- uninitialized string
        if (arr[i + 1].string == nullptr)
            continue;
        //--- destination string is uninitialized and cannot be allocated within dll
        if (arr[i].string == nullptr)
            continue;
        //--- memory piece is less than needed and cannot be reallocated within dll
        if (arr[i].len < len1 + len2)
            continue;
        //--- final processing
        strcat(arr[i].string, arr[i + 1].string);
    }
    //---
    return(arraysize);
}
*/
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CompareMqlStr(const void* left, const void* right)
{
    const auto leftstr = (MqlStr*)left;
    const auto rightstr = (MqlStr*)right;
    //---
    if (leftstr->string == nullptr)
        return(-1);
    if (rightstr->string == nullptr)
        return(1);
    //---
    return(strcmp(leftstr->string, rightstr->string));
}
//+------------------------------------------------------------------+
