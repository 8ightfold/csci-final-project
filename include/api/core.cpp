#include "core.hpp"

#include <algorithm>
#include <locale>

#include <strsafe.h>

namespace api {
    Coords::operator POINT() CNOEXCEPT {
        return { x, y };
    }

    Coords::operator COORD() CNOEXCEPT {
        return { (SHORT)x, (SHORT)y };
    }

    Coords::operator iVec2() CNOEXCEPT {
        return { x, y };
    }

    Coords::operator dVec2() CNOEXCEPT {
        return { (double)x, (double)y };
    }

    Coords& Coords::operator=(POINT p) NOEXCEPT {
        x = p.x;
        y = p.y;
        return *this;
    }

    Coords& Coords::operator=(COORD c) NOEXCEPT {
        x = c.X;
        y = c.Y;
        return *this;
    }

    int Coords::area() CNOEXCEPT {
        return (x * y);
    }

    Coords Coords::operator+(int i) CNOEXCEPT {
        return { x + i, y + i };
    }

    Coords& Coords::operator/=(int i) NOEXCEPT {
        x /= i;
        y /= i;
        return *this;
    }

    Coords& Coords::operator/=(double d) NOEXCEPT {
        x = static_cast<int>(x / d);
        y = static_cast<int>(y / d);
        return *this;
    }

    Coords& Coords::operator/=(Coords c) NOEXCEPT {
        x /= c.x;
        y /= c.y;
        return *this;
    }

    Coords& Coords::operator/=(dVec2 d) NOEXCEPT {
        x = static_cast<int>(x / d.x);
        y = static_cast<int>(y / d.y);
        return *this;
    }

    void throw_last_error(const char* filename, int line, LPTSTR lpszFunction, bool exit_on_error) {
        LPVOID lpMsgBuf;
        LPTSTR lpOutBuf;
        DWORD err_code = GetLastError();

        FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // NOLINT
                (LPTSTR) &lpMsgBuf, 0, NULL);  // NOLINT

        lpOutBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        std::size_t out_size = LocalSize(lpOutBuf) / sizeof(TCHAR);
        StringCchPrintf((LPTSTR)lpOutBuf,out_size,
                        TEXT("os error: \"%s\" failed with error %d; %s"),
                        lpszFunction, err_code, lpMsgBuf);

        std::string throw_buf { lpOutBuf };
        LocalFree(lpMsgBuf);
        LocalFree(lpOutBuf);
        DEBUG_ONLY( std::cout << filename << ": " << line << '\n'; )
        std::cout << throw_buf << std::endl;
        if(exit_on_error) std::cout << "press [esc] to continue...";
        while(not (GetAsyncKeyState(VK_ESCAPE) & 0x1)) {}
        if(exit_on_error) std::exit(-2);
    }
}