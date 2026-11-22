#include "asussaio.h"

NTSTATUS
AsusSAIOReadMsr(
    _In_ ULONG MsrRegister,
    _Out_ PULONG64 Value
)
{
    *Value = 0;

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
    __try {
        __writemsr(MsrRegister, Value);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}
