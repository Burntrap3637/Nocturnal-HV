#include <deps/includes.h>

int main( ) {
    int regs[ 4 ]{};

    PROCESSOR_NUMBER pn{};
    GetCurrentProcessorNumberEx( &pn );
    printf( "[*] Running on processor group=%u number=%u\n", pn.Group, pn.Number );

    // check hypervisor present bit
    __cpuid( regs, 1 );
    printf( "[*] CPUID(1) ECX = 0x%08x\n", regs[ 2 ] );
    bool hv_present = ( regs[ 2 ] >> 31 ) & 1;  // save it before regs is overwritten
    printf( "[*] Hypervisor present bit: %s\n", hv_present ? "YES" : "NO" );

    // vendor string
    __cpuid( regs, 0x40000000 );
    printf( "[*] CPUID(0x40000000) EAX=0x%x EBX=0x%x ECX=0x%x EDX=0x%x\n",
        regs[ 0 ], regs[ 1 ], regs[ 2 ], regs[ 3 ] );

    char vendor[ 13 ]{};
    memcpy( vendor + 0, &regs[ 1 ], 4 );
    memcpy( vendor + 4, &regs[ 2 ], 4 );
    memcpy( vendor + 8, &regs[ 3 ], 4 );
    vendor[ 12 ] = '\0';
    printf( "[*] Hypervisor vendor: \"%s\"\n", vendor );

    // HV interface
    __cpuid( regs, 0x40000001 );
    printf( "[*] CPUID(0x40000001) EAX=0x%x\n", regs[ 0 ] );

    // use saved hv_present, not stale regs[2]
    if ( hv_present && strcmp( vendor, "Nocturnal   " ) == 0 )
        printf( "[+] Nocturnal hypervisor is running!\n" );
    else
        printf( "[-] Nocturnal not detected — is the driver loaded?\n" );

    return std::getchar( );
}