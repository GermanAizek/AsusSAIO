#include "asussaio.h"

NTSTATUS
AsusSAIOReadPort(
    _In_ USHORT Port,
    _In_ UCHAR AccessType,
    _Out_ PULONG Data
)
{
    *Data = 0;

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
