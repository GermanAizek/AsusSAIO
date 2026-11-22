#ifndef ASUSSAIO_H
#define ASUSSAIO_H

#include <ntddk.h>
#include <intrin.h>

#ifndef _MSC_VER
#ifndef __try
#define __try
#define __except(x) if (0)
#endif
#endif

#define ASUSSAIO_DEVICE_NAME            L"\\Device\\AsusSAIO"
#define ASUSSAIO_DOS_DEVICE_NAME        L"\\DosDevices\\AsusSAIO"
#define ASUSSAIO_DEVICE_TYPE            0x8010

// IOCTL definitions
#define IOCTL_ASUSSAIO_GET_VERSION          0x80102040
#define IOCTL_ASUSSAIO_GET_STATUS           0x80102044
#define IOCTL_ASUSSAIO_CPUID                0x80102068
#define IOCTL_ASUSSAIO_EC_MAILBOX           0x80102070
#define IOCTL_ASUSSAIO_READ_POWER_LIMIT_1   0x80102074
#define IOCTL_ASUSSAIO_READ_POWER_LIMIT_2   0x80102078
#define IOCTL_ASUSSAIO_READ_RAPL_ENERGY     0x8010207C
#define IOCTL_ASUSSAIO_READ_CORE_VOLTAGE    0x80102080
#define IOCTL_ASUSSAIO_READ_CORE_RATIO      0x80102084
#define IOCTL_ASUSSAIO_READ_TEMPERATURE     0x80102088
#define IOCTL_ASUSSAIO_READ_PACKAGE_TEMP    0x8010208C
#define IOCTL_ASUSSAIO_READ_CORE_FREQ       0x80102090
#define IOCTL_ASUSSAIO_READ_AMD_FREQ        0x80102094

#pragma pack(push, 1)

// Port I/O request
typedef struct _ASUSSAIO_PORT_IO_REQUEST {
    USHORT Port;
    ULONG Data;
    UCHAR AccessType;
} ASUSSAIO_PORT_IO_REQUEST, *PASUSSAIO_PORT_IO_REQUEST;

// Physical memory map request
typedef struct _ASUSSAIO_MEM_REQUEST {
    ULONG PhysicalAddress;
    ULONG Data;
    UCHAR Length;
} ASUSSAIO_MEM_REQUEST, *PASUSSAIO_MEM_REQUEST;

// MSR request
typedef struct _ASUSSAIO_MSR_REQUEST {
    ULONG MsrRegister;
    ULONG64 Value;
} ASUSSAIO_MSR_REQUEST, *PASUSSAIO_MSR_REQUEST;

// CPUID request (0x80102068)
typedef struct _ASUSSAIO_CPUID_REQUEST {
    ULONG Function;
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
} ASUSSAIO_CPUID_REQUEST, *PASUSSAIO_CPUID_REQUEST;

// EC Mailbox request (0x80102070)
typedef struct _ASUSSAIO_EC_REQUEST {
    UCHAR Command;
    UCHAR SubCommand;
    UCHAR Data[8];
    UCHAR Reserved;
    UCHAR Status;
    ULONG Response;
} ASUSSAIO_EC_REQUEST, *PASUSSAIO_EC_REQUEST;

// PCI config request
typedef struct _ASUSSAIO_PCI_REQUEST {
    UCHAR Bus;
    UCHAR Device;
    UCHAR Function;
    UCHAR Offset;
    ULONG Data;
} ASUSSAIO_PCI_REQUEST, *PASUSSAIO_PCI_REQUEST;

#pragma pack(pop)

// Device Extension
typedef struct _ASUSSAIO_DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING SymbolicLinkName;
    BOOLEAN SymbolicLinkCreated;
    KMUTEX Mutex;
    LONG OpenCount;
} ASUSSAIO_DEVICE_EXTENSION, *PASUSSAIO_DEVICE_EXTENSION;

// Function declarations
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD AsusSAIOUnload;
DRIVER_DISPATCH AsusSAIODispatch;

// msr.c
NTSTATUS AsusSAIOReadMsr(
    _In_ ULONG MsrRegister,
    _Out_ PULONG64 Value
);

NTSTATUS AsusSAIOWriteMsr(
    _In_ ULONG MsrRegister,
    _In_ ULONG64 Value
);

// mem.c
NTSTATUS AsusSAIOReadPhysicalMemory(
    _In_ ULONG PhysicalAddress,
    _In_ UCHAR Length,
    _Out_ PULONG Data
);

// io.c
NTSTATUS AsusSAIOReadPort(
    _In_ USHORT Port,
    _In_ UCHAR AccessType,
    _Out_ PULONG Data
);

NTSTATUS AsusSAIOWritePort(
    _In_ USHORT Port,
    _In_ UCHAR AccessType,
    _In_ ULONG Data
);

NTSTATUS AsusSAIOHandleEcMailbox(
    _Inout_ PASUSSAIO_EC_REQUEST Request
);

// pci.c
NTSTATUS AsusSAIOReadPciConfig(
    _In_ UCHAR Bus,
    _In_ UCHAR Device,
    _In_ UCHAR Function,
    _In_ UCHAR Offset,
    _Out_ PULONG Data
);

#endif // ASUSSAIO_H
