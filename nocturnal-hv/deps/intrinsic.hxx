#pragma once

extern "C"
void
_sgdt(
    _Out_ void* descriptor
);

namespace std {
    void* memcpy(
        void* dest,
        const void* src,
        size_t len
    ) {
        char* d = ( char* )dest;
        const char* s = ( const char* )src;
        while ( len-- )
            *d++ = *s++;
        return d - reinterpret_cast< char >( dest );
    }

    __forceinline bool strcmp( const char* a, const char* b ) {
        while ( *a && *b ) {
            if ( *a != *b )
                return false;
            a++;
            b++;
        }
        return *a == *b;
    }
}

unsigned long strtoul(
    const char* str,
    char** endptr,
    int base
) {
    while ( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
        str++;

    bool negative = false;
    if ( *str == '-' ) {
        negative = true;
        str++;
    }
    else if ( *str == '+' ) {
        str++;
    }

    if ( base == 0 ) {
        if ( *str == '0' ) {
            str++;
            if ( *str == 'x' || *str == 'X' ) {
                base = 16;
                str++;
            }
            else {
                base = 8;
            }
        }
        else {
            base = 10;
        }
    }
    else if ( base == 16 ) {
        if ( *str == '0' && ( *( str + 1 ) == 'x' || *( str + 1 ) == 'X' ) ) {
            str += 2;
        }
    }

    unsigned long result = 0;
    bool valid_digit_found = false;

    while ( *str ) {
        int digit;

        if ( *str >= '0' && *str <= '9' ) {
            digit = *str - '0';
        }
        else if ( *str >= 'a' && *str <= 'z' ) {
            digit = *str - 'a' + 10;
        }
        else if ( *str >= 'A' && *str <= 'Z' ) {
            digit = *str - 'A' + 10;
        }
        else {
            break;
        }

        if ( digit >= base ) {
            break;
        }

        valid_digit_found = true;

        if ( result > ( ULONG_MAX - digit ) / base ) {
            result = ULONG_MAX;
            break;
        }

        result = result * base + digit;
        str++;
    }

    if ( endptr ) {
        *endptr = const_cast< char* >( valid_digit_found ? str : str - valid_digit_found );
    }

    return negative ? static_cast< unsigned long >( -static_cast< long >( result ) ) : result;
}