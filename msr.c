#include "asussaio.h"

BOOLEAN
AsusSAIOIsSafeMsr(
    _In_ ULONG MsrRegister,
    _In_ BOOLEAN IsWrite
)
{
    // Block critical system control MSRs to prevent LPE
    if (MsrRegister == 0xC0000080 || // IA32_EFER
        MsrRegister == 0xC0000081 || // IA32_STAR
        MsrRegister == 0xC0000082 || // IA32_LSTAR (syscall target)
        MsrRegister == 0xC0000083 || // IA32_CSTAR
        MsrRegister == 0xC0000084 || // IA32_FMASK
        MsrRegister == 0xC0000100 || // FS_BASE
        MsrRegister == 0xC0000101 || // GS_BASE
        MsrRegister == 0xC0000102 || // KERNEL_GS_BASE
        (MsrRegister >= 0x174 && MsrRegister <= 0x176)) { // IA32_SYSENTER_*
        return FALSE;
    }

    // Whitelist allowed monitoring MSRs
    switch (MsrRegister) {
    case 0x000000E7: // IA32_MPERF
    case 0x000000E8: // IA32_APERF
    case 0x00000150: // IA32_PACKAGE_THERM_MARGIN
    case 0x00000198: // MSR_PERF_STATUS
    case 0x0000019C: // IA32_THERM_STATUS
    case 0x000001A2: // MSR_TEMPERATURE_TARGET
    case 0x000001AD: // MSR_TURBO_RATIO_LIMIT
    case 0x000001B1: // IA32_PACKAGE_THERM_STATUS
    case 0x00000606: // MSR_RAPL_POWER_UNIT
    case 0x00000611: // MSR_PKG_ENERGY_STATUS
    case 0xC0010064: // AMD COFVID_STATUS
        return !IsWrite; // Read-only

    case 0x00000199: // MSR_PERF_CTL
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

NTSTATUS
AsusSAIOReadMsr(
    _In_ ULONG MsrRegister,
    _Out_ PULONG64 Value
)
{
    *Value = 0;

    if (!AsusSAIOIsSafeMsr(MsrRegister, FALSE)) {
        return STATUS_ACCESS_DENIED;
    }

    __try {
        *Value = __readmsr(MsrRegister);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AsusSAIOWriteMsr(
    _In_ ULONG MsrRegister,
    _In_ ULONG64 Value
)
{
    if (!AsusSAIOIsSafeMsr(MsrRegister, TRUE)) {
        return STATUS_ACCESS_DENIED;
    }

    __try {
        __writemsr(MsrRegister, Value);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}
