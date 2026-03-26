# Lab 06 - Understanding Check Answers

### Topic: Build and Boot Your Own Custom Linux Kernel

---

## Question 1: Monolithic Kernel vs Microkernel

### What is the difference between a monolithic kernel and a microkernel? Where does Linux stand?

| Aspect | Monolithic Kernel | Microkernel |
|--------|-------------------|-------------|
| **Architecture** | All OS services run in kernel space | Only essential services in kernel; rest in user space |
| **Components in Kernel** | Process management, memory management, file systems, device drivers, networking | Only IPC, basic scheduling, memory management |
| **Performance** | Faster (no context switching between services) | Slower (frequent IPC between user-space services) |
| **Size** | Large kernel binary | Small kernel binary |
| **Reliability** | One bug can crash entire system | Isolated failures; more stable |
| **Examples** | Linux, Unix, BSD | QNX, MINIX, L4, seL4 |

### Where Does Linux Stand?

Linux is a **monolithic kernel** but with **modular capabilities**:

```
┌─────────────────────────────────────────────────────────┐
│                    USER SPACE                           │
│   Applications, Shell, Libraries (glibc, etc.)          │
├─────────────────────────────────────────────────────────┤
│                    KERNEL SPACE                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │              LINUX KERNEL (Monolithic)          │   │
│  │  • Process Scheduler                            │   │
│  │  • Memory Manager                               │   │
│  │  • VFS (Virtual File System)                    │   │
│  │  • Network Stack                                │   │
│  │  • Device Drivers (can be loadable modules)     │   │
│  └─────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│                     HARDWARE                            │
└─────────────────────────────────────────────────────────┘
```

**Key Point:** Linux supports **loadable kernel modules (LKMs)**, allowing drivers to be loaded/unloaded at runtime without rebooting. This gives it flexibility similar to microkernels while maintaining monolithic performance.

---

## Question 2: Why Linux Over Real-Time Microkernels (QNX)?

### Why does almost every embedded device (phones, TVs, cars, routers) use Linux instead of a true real-time microkernel like QNX?

| Factor | Linux | QNX |
|--------|-------|-----|
| **Cost** | Free (GPL) | Expensive licensing |
| **Source Code** | Open source | Proprietary |
| **Community** | Massive global community | Small, vendor-dependent |
| **Driver Support** | Thousands of drivers available | Limited driver ecosystem |
| **Development Tools** | Free (GCC, GDB, etc.) | Costly proprietary tools |
| **Flexibility** | Highly customizable | Less flexible |
| **Software Ecosystem** | Vast (Android, containers, etc.) | Limited |

### Detailed Reasons:

1. **Cost**: Linux is FREE. QNX requires per-unit licensing fees.

2. **Driver Availability**: Linux has drivers for virtually every hardware component.

3. **Developer Pool**: Millions of Linux developers vs. limited QNX expertise.

4. **Android**: The smartphone revolution was built on Linux kernel.

5. **Good Enough Real-Time**: With `PREEMPT_RT` patches, Linux achieves soft real-time suitable for most applications.

6. **Ecosystem**: Docker, Kubernetes, and cloud-native tools all run on Linux.

### When QNX is Still Used:
- Safety-critical automotive systems (QNX is ASIL-D certified)
- Medical devices requiring hard real-time guarantees
- Nuclear power plant controllers
- Aviation systems

---

## Question 3: Android GKI (Generic Kernel Image)

### What is Android GKI? Why did Google force all vendors to use it from Android 13?

### What is GKI?

**GKI (Generic Kernel Image)** is a unified, Google-maintained Linux kernel that works across all Android devices.

```
BEFORE GKI (Fragmented):
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ Samsung      │  │ Xiaomi       │  │ OnePlus      │
│ Custom Kernel│  │ Custom Kernel│  │ Custom Kernel│
│ (modified)   │  │ (modified)   │  │ (modified)   │
└──────────────┘  └──────────────┘  └──────────────┘
      ↓                 ↓                 ↓
   Different versions, different patches, security nightmare

AFTER GKI (Unified):
┌─────────────────────────────────────────────────────┐
│           Google GKI (Generic Kernel Image)         │
│         Single kernel for ALL Android devices       │
└─────────────────────────────────────────────────────┘
      ↓                 ↓                 ↓
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ Samsung      │  │ Xiaomi       │  │ OnePlus      │
│ Vendor       │  │ Vendor       │  │ Vendor       │
│ Modules      │  │ Modules      │  │ Modules      │
└──────────────┘  └──────────────┘  └──────────────┘
```

### Why Google Forced GKI:

| Problem Before GKI | Solution with GKI |
|-------------------|-------------------|
| Security patches took months to reach devices | Google pushes updates directly |
| Each vendor maintained their own kernel fork | Single kernel source for all |
| Fragmentation nightmare | Unified kernel base |
| Old kernels on new phones | Mandatory up-to-date kernels |
| Difficult to upgrade Android versions | Easier OS upgrades |

### Key Benefits:

1. **Faster Security Updates**: Google can push kernel security patches directly.
2. **Longer Device Support**: Easier to maintain devices for 5+ years.
3. **Reduced Fragmentation**: All devices run similar kernel versions.
4. **Vendor Modules**: Hardware-specific code moves to loadable modules.

---

## Question 4: Why raspberrypi/linux Instead of torvalds/linux?

### Why clone raspberrypi/linux instead of torvalds/linux for RPi?

| Aspect | torvalds/linux (Mainline) | raspberrypi/linux |
|--------|---------------------------|-------------------|
| **Hardware Support** | Generic, limited RPi support | Full RPi-specific drivers |
| **GPU Driver** | Basic or missing | VC4/V3D fully supported |
| **Device Tree** | Generic | RPi-specific DTBs included |
| **Boot Support** | May not boot on RPi | Guaranteed to boot |
| **Optimizations** | None for RPi | RPi-specific optimizations |
| **Patches** | Upstream only | Includes downstream patches |

### Specific Reasons:

1. **Broadcom VideoCore GPU**: Proprietary driver not in mainline.

2. **Device Tree Blobs**: Raspberry Pi Foundation maintains DTBs for all Pi models:
   ```
   bcm2837-rpi-3-b.dtb
   bcm2837-rpi-3-b-plus.dtb
   bcm2711-rpi-4-b.dtb
   ```

3. **Hardware Quirks**: RPi has specific boot requirements and hardware initialization.

4. **Tested & Working**: Guaranteed to work on your Pi hardware.

5. **Downstream Patches**: Some features haven't been upstreamed yet.

### Command Comparison:
```bash
# For Raspberry Pi (recommended):
git clone --depth=1 -b rpi-6.6.y https://github.com/raspberrypi/linux.git

# Mainline (may not boot on RPi):
git clone --depth=1 https://github.com/torvalds/linux.git
```

---

## Question 5: Kernel Image Formats

### Explain the difference between: vmlinux → zImage → Image → uImage → Image.gz

```
┌─────────────────────────────────────────────────────────────────────┐
│                        KERNEL IMAGE FORMATS                         │
└─────────────────────────────────────────────────────────────────────┘

vmlinux (ELF)
    │
    ├──► objcopy ──► Image (ARM64 raw binary)
    │                   │
    │                   └──► gzip ──► Image.gz (compressed)
    │
    └──► Compression + ARM32 decompressor ──► zImage (ARM32)
                                                  │
                                                  └──► mkimage ──► uImage (U-Boot wrapper)
```

| Format | Description | Architecture | Bootloader |
|--------|-------------|--------------|------------|
| **vmlinux** | Uncompressed ELF executable with debug symbols | Any | Cannot boot directly |
| **Image** | Raw binary, stripped of ELF headers | ARM64 (AArch64) | U-Boot (`booti`) |
| **Image.gz** | Gzip compressed Image | ARM64 | U-Boot (needs decompression) |
| **zImage** | Compressed + self-extracting decompressor | ARM32 | U-Boot (`bootz`) |
| **uImage** | zImage/Image + U-Boot header (64-byte header) | ARM32/64 | U-Boot (`bootm`) |

### Detailed Explanation:

#### vmlinux
```bash
$ file vmlinux
vmlinux: ELF 64-bit LSB executable, ARM aarch64, statically linked, not stripped
$ ls -lh vmlinux
-rwxr-xr-x 1 user user 150M vmlinux   # Very large!
```
- Raw kernel in ELF format
- Contains debug symbols
- Used for debugging/analysis, not booting

#### Image (ARM64)
```bash
$ file Image
Image: Linux kernel ARM64 boot executable Image, little-endian, 4K pages
$ ls -lh Image
-rw-r--r-- 1 user user 25M Image
```
- Raw binary for ARM64
- No self-decompressing code
- Booted with `booti`

#### zImage (ARM32)
```bash
$ file zImage
zImage: Linux kernel ARM boot executable zImage (little-endian)
$ ls -lh zImage
-rw-r--r-- 1 user user 5M zImage
```
- Compressed kernel + decompressor stub
- Self-extracting at boot
- Booted with `bootz`

#### uImage
```bash
$ mkimage -A arm -O linux -T kernel -C none -a 0x80008000 -e 0x80008000 -n "Linux" -d zImage uImage
$ file uImage
uImage: u-boot legacy uImage, Linux/ARM, OS Kernel Image (Not compressed)
```
- 64-byte U-Boot header added
- Contains load address, entry point, checksum
- Booted with `bootm`

---

## Question 6: DTB and fdt_addr_r

### Why fdt_addr_r for DTB? What is DTB?

### What is DTB?

**DTB (Device Tree Blob)** is a compiled binary file that describes the hardware layout of a system.

```
┌─────────────────────────────────────────────────────────────────┐
│                    DEVICE TREE FLOW                             │
└─────────────────────────────────────────────────────────────────┘

   .dts (Source)              .dtb (Binary)
┌─────────────────┐         ┌─────────────────┐
│ /dts-v1/;       │         │                 │
│ / {             │   DTC   │  Binary blob    │
│   compatible =  │ ──────► │  (flattened     │
│   "brcm,bcm2837"│ compile │   device tree)  │
│   ...           │         │                 │
│ };              │         │                 │
└─────────────────┘         └─────────────────┘
```

### Why DTB is Needed:

| Without DTB | With DTB |
|-------------|----------|
| Hardware info hardcoded in kernel | Hardware described externally |
| Different kernel for each board | Same kernel, different DTB |
| Kernel must be recompiled for changes | Just swap DTB file |

### Example DTB Content:
```dts
/ {
    compatible = "raspberrypi,3-model-b-plus", "brcm,bcm2837";
    model = "Raspberry Pi 3 Model B+";
    
    memory@0 {
        device_type = "memory";
        reg = <0x0 0x40000000>;  /* 1GB RAM */
    };
    
    soc {
        uart0: serial@7e201000 {
            compatible = "arm,pl011";
            reg = <0x7e201000 0x200>;
            interrupts = <2 25>;
        };
    };
};
```

### What is fdt_addr_r?

`fdt_addr_r` is the **memory address where U-Boot loads the DTB**.

```
Memory Layout:
┌──────────────────────────────────────┐ High Address
│                                      │
├──────────────────────────────────────┤ 0x05600000 ← fdt_addr_r (DTB)
│   Device Tree Blob (.dtb)            │
├──────────────────────────────────────┤
│                                      │
├──────────────────────────────────────┤ 0x00080000 ← kernel_addr_r (Image)
│   Kernel Image                       │
├──────────────────────────────────────┤
│                                      │
└──────────────────────────────────────┘ Low Address
```

### U-Boot Commands:
```bash
# Load DTB to fdt_addr_r
load mmc 0:1 ${fdt_addr_r} bcm2837-rpi-3-b-plus.dtb

# Boot with kernel and DTB
booti ${kernel_addr_r} - ${fdt_addr_r}
#      ↑ kernel        ↑   ↑ DTB address
#                      │
#                      └── no initramfs (dash means none)
```

---

## Question 7: Bootargs Explained

### Explain bootargs: root= rootfstype= console= init=

`bootargs` is a string passed from the bootloader to the kernel containing configuration parameters.

```bash
bootargs="root=/dev/nfs rootfstype=nfs console=ttyS0,115200 init=/init"
```

### Parameter Breakdown:

#### root=
Specifies where the root filesystem is located.

| Value | Meaning |
|-------|---------|
| `root=/dev/mmcblk0p2` | SD card partition 2 |
| `root=/dev/sda1` | First SATA/USB disk |
| `root=/dev/nfs` | Network File System |
| `root=/dev/ram0` | RAM disk (initramfs) |
| `root=PARTUUID=xxxx` | Partition by UUID |

#### rootfstype=
Specifies the filesystem type of root.

| Value | Meaning |
|-------|---------|
| `rootfstype=ext4` | EXT4 filesystem |
| `rootfstype=nfs` | NFS (network) |
| `rootfstype=squashfs` | Compressed read-only |
| `rootfstype=f2fs` | Flash-friendly FS |

#### console=
Specifies where kernel messages are sent.

```
console=ttyS0,115200     # Serial port 0 at 115200 baud
console=ttyAMA0,115200   # ARM AMBA serial (RPi)
console=tty0             # First virtual terminal (screen)
console=tty0 console=ttyS0,115200  # Multiple consoles
```

#### init=
Specifies the first user-space process to run.

| Value | Meaning |
|-------|---------|
| `init=/sbin/init` | Default init system |
| `init=/init` | Custom init at root |
| `init=/bin/sh` | Drop to shell (debug) |
| `init=/linuxrc` | BusyBox init |

### Complete Example:
```bash
setenv bootargs "console=ttyS0,115200 root=/dev/nfs nfsroot=192.168.2.1:/srv/nfs/rootfs,vers=3,tcp ip=192.168.2.100::192.168.2.1:255.255.255.0::eth0:off rootfstype=nfs rw init=/init"
```

---

## Question 8: Why bootz for ARM32, booti for ARM64?

### Different kernel image formats require different boot commands.

| Command | Architecture | Image Format | Reason |
|---------|--------------|--------------|--------|
| `bootz` | ARM32 | zImage | Handles self-decompressing zImage |
| `booti` | ARM64 | Image | Handles raw ARM64 binary |
| `bootm` | Any | uImage | Handles U-Boot wrapped images |

### Technical Differences:

#### ARM32 (zImage + bootz):
```
zImage Structure:
┌─────────────────────────────┐
│ Decompressor stub (ARM32)   │  ← Self-extracting code
├─────────────────────────────┤
│ Compressed kernel           │
└─────────────────────────────┘

bootz:
1. Loads zImage to memory
2. Validates ARM32 zImage magic number
3. Sets up registers (r0, r1, r2)
4. Jumps to decompressor
5. Decompressor extracts kernel and runs it
```

#### ARM64 (Image + booti):
```
Image Structure:
┌─────────────────────────────┐
│ 64-byte header              │  ← ARM64 header with magic
├─────────────────────────────┤
│ Uncompressed kernel code    │  ← Ready to execute
└─────────────────────────────┘

booti:
1. Loads Image to memory
2. Validates ARM64 magic number (0x644d5241 = "ARM\x64")
3. Sets up registers (x0 = dtb address)
4. Jumps directly to kernel entry point
```

### U-Boot Command Syntax:
```bash
# ARM32
bootz ${kernel_addr_r} - ${fdt_addr_r}

# ARM64
booti ${kernel_addr_r} - ${fdt_addr_r}

# With initramfs
booti ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} ${fdt_addr_r}
```

---

## Question 9: VFS Unable to Mount Root FS

### What causes "VFS: Unable to mount root fs" panic?

This error means the kernel booted successfully but cannot find or mount the root filesystem.

### Common Causes:

| Cause | Solution |
|-------|----------|
| Wrong `root=` parameter | Fix the device path |
| Missing filesystem driver | Enable driver in kernel config |
| Filesystem corrupted | Run fsck |
| NFS server not reachable | Check network and NFS config |
| Wrong `rootfstype=` | Specify correct filesystem type |
| Device not ready | Add `rootwait` or `rootdelay=5` |

### Debugging Flowchart:

```
"VFS: Unable to mount root fs"
            │
            ▼
┌─────────────────────────────────┐
│ Is root= parameter correct?    │
│ e.g., root=/dev/mmcblk0p2      │
└─────────────────────────────────┘
            │ No → Fix bootargs
            ▼ Yes
┌─────────────────────────────────┐
│ Does the device exist?          │
│ Check kernel logs for device    │
└─────────────────────────────────┘
            │ No → Add driver to kernel
            ▼ Yes
┌─────────────────────────────────┐
│ Is filesystem driver compiled?  │
│ CONFIG_EXT4_FS=y ?             │
└─────────────────────────────────┘
            │ No → Enable in menuconfig
            ▼ Yes
┌─────────────────────────────────┐
│ For NFS: Is server reachable?  │
│ Check IP, firewall, exports    │
└─────────────────────────────────┘
```

### Example Fixes:

```bash
# Add delay for slow devices
bootargs="root=/dev/sda1 rootwait rootdelay=5"

# Specify filesystem type
bootargs="root=/dev/mmcblk0p2 rootfstype=ext4"

# For NFS, add complete configuration
bootargs="root=/dev/nfs nfsroot=192.168.2.1:/srv/nfs/rootfs,vers=3,tcp ip=dhcp"
```

---

## Question 10: Why Static Linking for Custom Init?

### Why does custom init.c need -static? What if not?

### The Problem:

When the kernel runs `/init`, there is **no filesystem fully set up yet**. Dynamic libraries (`libc.so`, `ld-linux.so`) are not available!

### Static vs Dynamic Linking:

```
DYNAMIC LINKING (Default):
┌─────────────────┐
│    init         │ ──► Needs ld-linux.so
│  (small binary) │ ──► Needs libc.so.6
└─────────────────┘ ──► Needs libpthread.so
        │
        ▼
    FAILS! Libraries not available at early boot

STATIC LINKING (-static):
┌─────────────────────────────────────┐
│              init                    │
│  ┌─────────────────────────────┐    │
│  │ All library code included   │    │
│  │ (libc, etc.)                │    │
│  └─────────────────────────────┘    │
└─────────────────────────────────────┘
        │
        ▼
    WORKS! No external dependencies
```

### Compilation:

```bash
# WRONG - Dynamic linking (will fail)
aarch64-linux-gnu-gcc init.c -o init
$ file init
init: ELF 64-bit LSB executable, dynamically linked, interpreter /lib/ld-linux-aarch64.so.1

# CORRECT - Static linking
aarch64-linux-gnu-gcc -static init.c -o init
$ file init
init: ELF 64-bit LSB executable, statically linked
```

### What Happens Without -static:

```
Kernel: Starting /init...
Kernel: Trying to load /lib/ld-linux-aarch64.so.1
Kernel: File not found!
Kernel panic - not syncing: Requested init /init failed (error -2)
                                                        ^^^^^^^^
                                                        ENOENT (No such file)
```

### Error -2 Explained:
- Error code **-2** = `ENOENT` = "No such file or directory"
- The kernel finds `init` but cannot find the dynamic linker

---

## Question 11: init=/bin/sh Still Panics?

### You passed init=/bin/sh but it still panics. Why?

### Possible Causes:

#### 1. /bin/sh Doesn't Exist
```bash
# Your rootfs might not have /bin/sh
ls -la /path/to/rootfs/bin/sh
# If missing → panic!
```

#### 2. /bin/sh is Dynamically Linked
```bash
$ file /bin/sh
/bin/sh: ELF 64-bit LSB executable, dynamically linked, interpreter /lib/ld-linux-aarch64.so.1
```
If dynamic linker (`/lib/ld-linux-aarch64.so.1`) is missing → panic!

#### 3. Wrong Architecture
```bash
# If your kernel is ARM64 but sh is ARM32:
$ file /bin/sh
/bin/sh: ELF 32-bit LSB executable, ARM, ...
# Kernel panic - can't execute wrong architecture!
```

#### 4. Not Executable
```bash
# Missing execute permission
$ ls -la /bin/sh
-rw-r--r-- 1 root root 125488 /bin/sh
# Needs: chmod +x /bin/sh
```

#### 5. Root Filesystem Not Mounted
If `root=` parameter is wrong, `/bin/sh` cannot be found.

### Solutions:

```bash
# Use BusyBox static binary as init
bootargs="init=/bin/busybox sh"

# Or create static shell
aarch64-linux-gnu-gcc -static shell.c -o /rootfs/bin/sh

# Or use a minimal init that execs sh
# init.c:
#include <unistd.h>
int main() {
    char *argv[] = {"/bin/sh", NULL};
    execv("/bin/sh", argv);
    return 1;
}
```

---

## Question 12: Static Linking Deep Dive

### Why must your custom init program be statically linked? What happens if you forget -static?

### Detailed Explanation:

#### The Boot Sequence:
```
1. Bootloader loads kernel + DTB
2. Kernel initializes hardware
3. Kernel mounts root filesystem
4. Kernel executes /init (PID 1)
        │
        ▼
   ┌────────────────────────────────────────┐
   │ At this point:                         │
   │ • No dynamic linker loaded             │
   │ • No shared libraries mapped           │
   │ • /lib/ld-linux.so not available       │
   │ • libc.so not available                │
   └────────────────────────────────────────┘
```

#### Dynamic Executable Requirements:
```
Dynamic init binary needs:
├── /lib/ld-linux-aarch64.so.1  (dynamic linker)
├── /lib/libc.so.6              (C library)
├── /lib/libpthread.so.0        (threading, if used)
└── /lib/libm.so.6              (math, if used)
```

#### What Happens Without -static:

```c
// init.c
#include <stdio.h>
int main() {
    printf("Hello from init!\n");
    while(1);
    return 0;
}
```

```bash
# Compile without -static
aarch64-linux-gnu-gcc init.c -o init

# Check dependencies
aarch64-linux-gnu-readelf -d init | grep NEEDED
 0x0000000000000001 (NEEDED)    Shared library: [libc.so.6]

# Check interpreter
aarch64-linux-gnu-readelf -l init | grep interpreter
      [Requesting program interpreter: /lib/ld-linux-aarch64.so.1]
```

**Boot Result:**
```
Kernel panic - not syncing: Requested init /init failed (error -2)
```

### Correct Approach:

```bash
# Compile WITH -static
aarch64-linux-gnu-gcc -static init.c -o init

# Verify no dependencies
aarch64-linux-gnu-readelf -d init | grep NEEDED
# (no output - no shared library dependencies)

$ file init
init: ELF 64-bit LSB executable, ARM aarch64, statically linked
```

### Size Comparison:
```bash
# Dynamic
$ ls -lh init_dynamic
-rwxr-xr-x 1 user user 8.2K init_dynamic

# Static
$ ls -lh init_static
-rwxr-xr-x 1 user user 760K init_static
```

Static binaries are larger but completely self-contained!

---

## Summary Table

| Question | Key Answer |
|----------|------------|
| Q1 | Linux is monolithic but modular (LKMs) |
| Q2 | Cost, ecosystem, driver support, developer availability |
| Q3 | GKI = unified kernel for faster security updates |
| Q4 | RPi-specific drivers, DTBs, and hardware support |
| Q5 | vmlinux (ELF) → Image (raw) → zImage (compressed+decompressor) |
| Q6 | DTB describes hardware; fdt_addr_r is memory address for DTB |
| Q7 | root=device, rootfstype=fs, console=output, init=first process |
| Q8 | bootz handles self-decompressing ARM32; booti handles raw ARM64 |
| Q9 | Wrong root path, missing drivers, NFS issues, device not ready |
| Q10 | No dynamic linker at early boot; static includes all code |
| Q11 | /bin/sh missing, dynamically linked, wrong arch, not executable |
| Q12 | Dynamic linker not available; kernel can't resolve dependencies |

---