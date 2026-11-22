# AsusSAIO

AsusSAIO (AsusSAIO.sys) - kernel mode driver used by ASUS System Analysis and ASUS System Control Interface utilities.

My opensource driver does not use the original source codes and is a reverse-engineered driver using [Ghidra](https://en.wikipedia.org/wiki/Ghidra) and [Radare2](https://en.wikipedia.org/wiki/Radare2). The rights have not been violated, and the new driver cannot be signed due to the EV certificate. I hope that the vulnerabilities can be closed using my security patches.

The main goal of the project is to recreate a secure driver with fixed vulnerabilities and errors made by past developers.

### Functions

Driver AsusSAIO.sys allows ASUS system management tools to interact with system hardware directly:

- Reading and writing CPU Model Specific Registers (MSR)
- Direct x86 I/O port read and write
- Embedded Controller (EC) / SuperIO Mailbox communication
- Physical memory mapping via MmMapIoSpace
- PCI configuration space inspection

Fixed vulnerabilities in opensource driver AsusSAIO.sys:

- [x] [CVE-2018-18537](https://nvd.nist.gov/vuln/detail/CVE-2018-18537)
  - Insufficient Access Control: Device object `\Device\AsusSAIO` opened with world-accessible DACL (fixed using `IoCreateDeviceSecure` with SDDL `D:P(A;;GA;;;SY)(A;;GA;;;BA)`)
- [x] Arbitrary MSR Read/Write: IOCTLs 0x8010207C-0x80102094 (fixed with whitelist of monitoring MSRs, blocking IA32_LSTAR and system registers)
- [x] Arbitrary Physical Memory Mapping via `MmMapIoSpace` (IOCTLs 0x80102074, 0x80102078)
  - Unrestricted Physical Memory Mapping: (fixed with physical address range validation, blocking low memory < 1MB, and integer overflow checks)
- [x] Unrestricted Port I/O and EC Mailbox commands (IOCTL 0x80102070)
  - Arbitrary I/O port read/write and EC Mailbox: (fixed with I/O port whitelist, blocking critical system controllers and restricting EC ports 0x25C/0x25D)
- [ ] Arbitrary PCI Configuration space access

Fixed no publish vulnerabilities and bugs which contributors to this project found:

- Device deletion bypassed on symbolic link deletion failure ([view code](main.c))
- Device creation with weak permissions ([view code](main.c))
- Arbitrary CPU MSR register tampering ([view code](msr.c))
- Out-of-bounds physical memory mapping ([view code](mem.c))
- Unfiltered write to critical x86 I/O ports ([view code](io.c))
- Unrestricted PCI configuration modification ([view code](pci.c))

Authors:

- 2026 - Herman Semenoff <<GermanAizek@yandex.ru>> ([germanaizek.github.io](https://germanaizek.github.io))
- ASUSTeK Computer Inc. <<security@asus.com>>
