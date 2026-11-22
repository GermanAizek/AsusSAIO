#include "asussaio.h"
#include <wdmsec.h>

static const GUID GUID_DEVCLASS_ASUSSAIO = {
    0x4d36e97d, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 }
};

static ASUSSAIO_DEVICE_EXTENSION g_DevExt;

NTSTATUS
AsusSAIODispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
)
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    ULONG_PTR information;
    PVOID buffer;
    ULONG inputLength;
    ULONG outputLength;
    ULONG ioControlCode;

    UNREFERENCED_PARAMETER(DeviceObject);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    status = STATUS_SUCCESS;
    information = 0;

    switch (irpSp->MajorFunction) {
    case IRP_MJ_CREATE:
        InterlockedIncrement(&g_DevExt.OpenCount);
        status = STATUS_SUCCESS;
        break;

    case IRP_MJ_CLOSE:
        InterlockedDecrement(&g_DevExt.OpenCount);
        status = STATUS_SUCCESS;
        break;

    case IRP_MJ_DEVICE_CONTROL:
        buffer = Irp->AssociatedIrp.SystemBuffer;
        inputLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        outputLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode) {
        case IOCTL_ASUSSAIO_GET_VERSION:
            if (outputLength < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *(PULONG)buffer = 0x10001;
                information = sizeof(ULONG);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_GET_STATUS:
            if (outputLength < sizeof(LONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *(PLONG)buffer = g_DevExt.OpenCount;
                information = sizeof(LONG);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_CPUID:
            if (inputLength < sizeof(ULONG) || outputLength < 4 * sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                int cpuInfo[4] = { 0 };
                ULONG function = *(PULONG)buffer;
                __cpuid(cpuInfo, (int)function);
                ((PULONG)buffer)[0] = (ULONG)cpuInfo[0];
                ((PULONG)buffer)[1] = (ULONG)cpuInfo[1];
                ((PULONG)buffer)[2] = (ULONG)cpuInfo[2];
                ((PULONG)buffer)[3] = (ULONG)cpuInfo[3];
                information = 4 * sizeof(ULONG);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_EC_MAILBOX:
            if (inputLength < sizeof(ASUSSAIO_EC_REQUEST) || outputLength < sizeof(ASUSSAIO_EC_REQUEST)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                status = AsusSAIOHandleEcMailbox((PASUSSAIO_EC_REQUEST)buffer);
                if (NT_SUCCESS(status)) {
                    information = sizeof(ASUSSAIO_EC_REQUEST);
                }
            }
            break;

        case IOCTL_ASUSSAIO_READ_POWER_LIMIT_1:
        case IOCTL_ASUSSAIO_READ_POWER_LIMIT_2:
            if (inputLength < 3 * sizeof(ULONG) || outputLength < 3 * sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                PULONG p = (PULONG)buffer;
                ULONG pciData = 0;
                ULONG mmioData = 0;
                ULONG64 msrVal = 0;
                AsusSAIOReadPciConfig(0, 0, 0, 0x48, &pciData);
                AsusSAIOReadMsr(0x606, &msrVal);
                if (pciData != 0) {
                    ULONG addr = (pciData & 0xFFFFFFFE) + ((ioControlCode == IOCTL_ASUSSAIO_READ_POWER_LIMIT_1) ? 0x59A0 : 0x59A4);
                    AsusSAIOReadPhysicalMemory(addr, 4, &mmioData);
                }
                p[1] = mmioData;
                information = 3 * sizeof(ULONG);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_RAPL_ENERGY:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 msrUnit = 0, msrEnergy = 0;
                AsusSAIOReadMsr(0x606, &msrUnit);
                AsusSAIOReadMsr(0x611, &msrEnergy);
                *(PULONG64)buffer = msrEnergy;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_CORE_VOLTAGE:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 msrVal = 0;
                AsusSAIOReadMsr(0x198, &msrVal);
                *(PULONG64)buffer = msrVal;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_CORE_RATIO:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 msrVal = 0;
                AsusSAIOReadMsr(0x198, &msrVal);
                *(PULONG64)buffer = (msrVal >> 8) & 0xFF;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_TEMPERATURE:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 msrTarget = 0, msrTherm = 0;
                AsusSAIOReadMsr(0x1A2, &msrTarget);
                AsusSAIOReadMsr(0x19C, &msrTherm);
                *(PULONG64)buffer = (msrTarget >> 16) & 0xFF;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_PACKAGE_TEMP:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 msrPkg = 0;
                AsusSAIOReadMsr(0x1B1, &msrPkg);
                *(PULONG64)buffer = (msrPkg >> 16) & 0x7F;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_CORE_FREQ:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 aperf = 0, mperf = 0;
                AsusSAIOReadMsr(0xE8, &aperf);
                AsusSAIOReadMsr(0xE7, &mperf);
                *(PULONG64)buffer = aperf;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        case IOCTL_ASUSSAIO_READ_AMD_FREQ:
            if (outputLength < sizeof(ULONG64)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG64 cofvid = 0;
                AsusSAIOReadMsr(0xC0010064, &cofvid);
                *(PULONG64)buffer = cofvid;
                information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID
AsusSAIOUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNICODE_STRING symbolicLinkName;

    RtlInitUnicodeString(&symbolicLinkName, ASUSSAIO_DOS_DEVICE_NAME);
    IoDeleteSymbolicLink(&symbolicLinkName);

    // FIX: Unconditionally delete device object to prevent resource leak on unload
    if (DriverObject->DeviceObject != NULL) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicLinkName;
    PDEVICE_OBJECT deviceObject = NULL;

    // SDDL Security String:
    // D:P         -> DACL Protected
    // (A;;GA;;;SY)-> Allow Generic All to Local System
    // (A;;GA;;;BA)-> Allow Generic All to Built-in Administrators
    DECLARE_CONST_UNICODE_STRING(sddlSecureString, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

    UNREFERENCED_PARAMETER(RegistryPath);

    RtlInitUnicodeString(&deviceName, ASUSSAIO_DEVICE_NAME);

    // FIX: Secure device creation to mitigate CVE-2018-18537
    status = IoCreateDeviceSecure(
        DriverObject,
        sizeof(ASUSSAIO_DEVICE_EXTENSION),
        &deviceName,
        ASUSSAIO_DEVICE_TYPE,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &sddlSecureString,
        &GUID_DEVCLASS_ASUSSAIO,
        &deviceObject
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    RtlZeroMemory(&g_DevExt, sizeof(ASUSSAIO_DEVICE_EXTENSION));
    g_DevExt.DeviceObject = deviceObject;
    KeInitializeMutex(&g_DevExt.Mutex, 0);

    RtlInitUnicodeString(&symbolicLinkName, ASUSSAIO_DOS_DEVICE_NAME);
    g_DevExt.SymbolicLinkName = symbolicLinkName;

    status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        return status;
    }

    g_DevExt.SymbolicLinkCreated = TRUE;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = AsusSAIODispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = AsusSAIODispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AsusSAIODispatch;
    DriverObject->DriverUnload = AsusSAIOUnload;

    return STATUS_SUCCESS;
}
