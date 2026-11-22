#include "asussaio.h"

BOOLEAN
AsusSAIOValidateIOPortAccess(
    _In_ USHORT Port,
    _In_ BOOLEAN IsWrite
)
{
    // Delay / diagnostic ports
    if (Port == 0x80 || Port == 0x81 || Port == 0x84 || Port == 0xEB) {
        return TRUE;
    }

    // PCI Configuration Ports 0xCF8 - 0xCFF
    if (Port >= 0xCF8 && Port <= 0xCFF) {
        return TRUE;
    }

    // CMOS Real Time Clock 0x70 - 0x71
    if (Port == 0x70 || Port == 0x71) {
        return TRUE;
    }

    // SuperIO Configuration Ports
    if (Port == 0x2E || Port == 0x2F || Port == 0x4E || Port == 0x4F) {
        return TRUE;
    }

    // ASUS EC Mailbox ports
    if (Port == 0x25C || Port == 0x25D) {
        return TRUE;
    }

    // Disallow critical system ports (PIC, PIT, keyboard controller, DMA)
    if (Port <= 0x6F) {
        return FALSE;
    }

    // Allow read from diagnostics/general I/O, restrict unwhitelisted writes
    if (!IsWrite) {
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
AsusSAIOReadPort(
    _In_ USHORT Port,
    _In_ UCHAR AccessType,
    _Out_ PULONG Data
)
{
    *Data = 0;

    // FIX: Validate port access permissions to prevent system controller tampering
    if (!AsusSAIOValidateIOPortAccess(Port, FALSE)) {
        return STATUS_ACCESS_DENIED;
    }

    switch (AccessType) {
    case 1:
        *Data = READ_PORT_UCHAR((PUCHAR)(ULONG_PTR)Port);
        break;
    case 2:
        *Data = READ_PORT_USHORT((PUSHORT)(ULONG_PTR)Port);
        break;
    case 4:
        *Data = READ_PORT_ULONG((PULONG)(ULONG_PTR)Port);
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AsusSAIOWritePort(
    _In_ USHORT Port,
    _In_ UCHAR AccessType,
    _In_ ULONG Data
)
{
    // FIX: Restrict port writes to whitelisted diagnostic and safe hardware ports
    if (!AsusSAIOValidateIOPortAccess(Port, TRUE)) {
        return STATUS_ACCESS_DENIED;
    }

    switch (AccessType) {
    case 1:
        WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)Port, (UCHAR)Data);
        break;
    case 2:
        WRITE_PORT_USHORT((PUSHORT)(ULONG_PTR)Port, (USHORT)Data);
        break;
    case 4:
        WRITE_PORT_ULONG((PULONG)(ULONG_PTR)Port, Data);
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AsusSAIOHandleEcMailbox(
    _Inout_ PASUSSAIO_EC_REQUEST Request
)
{
    ULONG retry = 0;
    UCHAR status = 0;

    if (Request == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // FIX: Ensure EC ports 0x25C/0x25D are validated
    if (!AsusSAIOValidateIOPortAccess(0x25C, TRUE) || !AsusSAIOValidateIOPortAccess(0x25D, TRUE)) {
        return STATUS_ACCESS_DENIED;
    }

    // Wait for EC port 0x25C to become ready
    while (retry < 50) {
        status = READ_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x25C);
        if ((status & 1) != 0) {
            break;
        }
        KeStallExecutionProcessor(100);
        retry++;
    }

    // Write command to 0x25D
    WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x25D, Request->Command);
    KeStallExecutionProcessor(50);
    WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x25C, Request->SubCommand);

    Request->Status = READ_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x25C);
    Request->Response = READ_PORT_ULONG((PULONG)(ULONG_PTR)0x25D);

    return STATUS_SUCCESS;
}
