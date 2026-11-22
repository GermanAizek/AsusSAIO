#include "asussaio.h"

NTSTATUS
AsusSAIOReadPciConfig(
    _In_ UCHAR Bus,
    _In_ UCHAR Device,
    _In_ UCHAR Function,
    _In_ UCHAR Offset,
    _Out_ PULONG Data
)
{
    ULONG slot = (Function << 5) | (Device & 0x1F);
    ULONG bytesRead;

    *Data = 0;

    bytesRead = HalGetBusDataByOffset(
        PCIConfiguration,
        Bus,
        slot,
        Data,
        Offset,
        sizeof(ULONG)
    );

    if (bytesRead == 0) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}
