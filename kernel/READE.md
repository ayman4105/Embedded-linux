
```markdown
# Embedded Linux Boot Guide - NFS & SD Card
## By: Ayman
## Date: March 2026

---

# Table of Contents

1. [Overview](#overview)
2. [Boot Flow](#boot-flow)
3. [Prerequisites](#prerequisites)
4. [Kernel Compilation](#kernel-compilation)
5. [U-Boot Setup](#u-boot-setup)
6. [SD Card Boot](#sd-card-boot)
7. [TFTP Setup](#tftp-setup)
8. [NFS Boot](#nfs-boot)
9. [Boot Script](#boot-script)
10. [Custom Init (Shell)](#custom-init)
11. [QEMU Commands](#qemu-commands)
12. [Troubleshooting](#troubleshooting)
13. [Quick Reference](#quick-reference)

---

# Overview

This document covers two boot methods for embedded Linux:
- **SD Card Boot**: Kernel and rootfs stored on SD card
- **NFS Boot**: Kernel loaded via TFTP, rootfs mounted over network

## Architecture Summary

| Platform | Architecture | Kernel Image | Boot Command | Console |
|----------|-------------|-------------|-------------|---------|
| Vexpress-A9 | ARM32 | `zImage` | `bootz` | `ttyAMA0` |
| Raspberry Pi 3B+ | ARM64 | `Image` | `booti` | `ttyAMA0` |

---

# Boot Flow

## General Boot Flow
```
Power ON
    ↓
U-Boot starts
    ↓
Loads boot.scr (from SD or TFTP)
    ↓
Executes boot script
    ↓
Loads Kernel + DTB
    ↓
Sets bootargs
    ↓
Boots Kernel
    ↓
Kernel mounts rootfs (SD or NFS)
    ↓
Runs /sbin/init
    ↓
Shell / Userspace
```

## Boot Script Flow
```
┌──────────────────────────────────┐
│        Try SD Card (mmc 0:1)     │
└──────────┬───────────┬──────────┘
        Found ✅    Not Found ❌
           │              │
           ▼              ▼
    ┌────────────┐  ┌──────────────┐
    │ Boot from  │  │  Try TFTP    │
    │ SD Card    │  │  Server      │
    └────────────┘  └──┬───────┬───┘
                    Found ✅ Not Found ❌
                       │        │
                       ▼        ▼
                ┌────────────┐ ┌──────────────┐
                │ Boot from  │ │ BOOT FAILED  │
                │ TFTP       │ │ Error Message│
                └────────────┘ └──────────────┘
```

---

# Prerequisites

## Required Packages

```bash
# Cross Compilers
sudo apt install gcc-arm-linux-gnueabi      # ARM32 (Vexpress)
sudo apt install gcc-aarch64-linux-gnu      # ARM64 (Raspberry Pi)

# QEMU
sudo apt install qemu-system-arm

# TFTP Server
sudo apt install tftpd-hpa

# NFS Server
sudo apt install nfs-kernel-server

# U-Boot tools (for mkimage)
sudo apt install u-boot-tools

# Device Tree Compiler
sudo apt install device-tree-compiler
```

## Directory Structure

```
~/ITI/fady/
├── kernel_linux/linux/     # Kernel source code
├── u-boot/                 # U-Boot source code
├── tftp/                   # TFTP server directory
│   ├── zImage
│   ├── vexpress-v2p-ca9.dtb
│   └── boot.scr
├── myinit/                 # Custom init source
│   ├── init.c
│   └── init
├── myrootfs/               # Local rootfs copy
│   ├── sbin/init
│   ├── proc/
│   ├── sys/
│   ├── dev/
│   ├── etc/
│   └── tmp/
└── embedded_linux/
    └── Embedded-linux/
        └── kernel_panicv/  # QEMU working directory
            ├── sd.img
            ├── run.sh
            ├── boot.cmd
            └── boot.scr
```

---

# Kernel Compilation

## For Vexpress (ARM32)

```bash
cd ~/ITI/fady/kernel_linux/linux

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-

# Add your name to kernel version
# Edit Makefile: EXTRAVERSION = -Ayman-v1

make vexpress_defconfig
make menuconfig   # Enable NFS if needed
make -j$(nproc) zImage dtbs
```

### Output Files
```
arch/arm/boot/zImage                          # Kernel image
arch/arm/boot/dts/arm/vexpress-v2p-ca9.dtb   # Device Tree
```

### Required Kernel Config for NFS

```
File systems --->
    [*] Network File Systems --->
        <*> NFS client support
        [*]   NFS client support for NFS version 3
        [*]   Root file system on NFS

Networking support --->
    Networking options --->
        [*] IP: kernel level autoconfiguration
```

## For Raspberry Pi (ARM64)

```bash
cd ~/ITI/fady/kernel_linux/linux

export ARCH=arm64
export CROSS_COMPILE=~/x-tools/aarch64-rpi3-linux-gnu/bin/aarch64-rpi3-linux-gnu-

make bcm2711_defconfig
make -j$(nproc) Image dtbs
```

### Output Files
```
arch/arm64/boot/Image                              # Kernel image
arch/arm64/boot/dts/broadcom/bcm2837-rpi-3-b-plus.dtb  # Device Tree
```

---

# U-Boot Setup

## Compile U-Boot for Vexpress

```bash
cd ~/ITI/fady/u-boot

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-

make vexpress_ca9x4_defconfig
make -j$(nproc)
```

Output: `u-boot` binary

## Key U-Boot Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `bootargs` | Arguments passed to kernel | `console=ttyAMA0,115200 root=/dev/mmcblk0p2` |
| `bootcmd` | Auto-run command on boot | `load mmc 0:1 0x100000 boot.scr; source 0x100000` |
| `ipaddr` | Board IP address | `192.168.1.100` |
| `serverip` | TFTP/NFS server IP | `192.168.1.1` |

## Useful U-Boot Commands

```bash
# Show all variables
printenv

# Set a variable
setenv bootargs "console=ttyAMA0,115200"

# Save to flash
saveenv

# List files on SD
ls mmc 0:1

# Load file from SD
load mmc 0:1 0x60000000 zImage

# Load file from TFTP
tftp 0x60000000 zImage

# Boot kernel
bootz 0x60000000 - 0x65000000    # ARM32
booti 0x80000 - 0x2600000        # ARM64

# Run a variable as command
run bootcmd

# Ping test
ping 192.168.1.1
```

## bootargs Locations (3 Places)

| Location | Priority | Needs Rebuild? |
|----------|----------|----------------|
| U-Boot Environment | 1 (Highest) | No |
| Device Tree (chosen node) | 2 | Yes (DTB) |
| Kernel Config (CONFIG_CMDLINE) | 3 (Lowest) | Yes (Kernel) |

---

# SD Card Boot

## Create SD Card Image

```bash
# Create 1GB image
dd if=/dev/zero of=sd.img bs=1M count=1024

# Create partitions
# Partition 1: FAT32 (boot) - 200MB
# Partition 2: ext4 (rootfs) - rest
cfdisk sd.img
```

## Mount and Copy Files

```bash
# Setup loop device
sudo losetup -fP sd.img

# Format partitions
sudo mkfs.vfat /dev/loop0p1
sudo mkfs.ext4 /dev/loop0p2

# Mount boot partition
sudo mkdir -p /mnt/boot
sudo mount /dev/loop0p1 /mnt/boot

# Copy boot files
sudo cp zImage /mnt/boot/
sudo cp vexpress-v2p-ca9.dtb /mnt/boot/
sudo cp boot.scr /mnt/boot/

# Mount rootfs partition
sudo mkdir -p /mnt/rootfs
sudo mount /dev/loop0p2 /mnt/rootfs

# Copy rootfs
sudo cp -a ~/ITI/fady/myrootfs/* /mnt/rootfs/

# Unmount
sudo umount /mnt/boot
sudo umount /mnt/rootfs
sudo losetup -d /dev/loop0
```

## SD Card bootargs

```bash
setenv bootargs console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait rw init=/sbin/init
```

### bootargs Breakdown

| Argument | Meaning |
|----------|---------|
| `console=ttyAMA0,115200` | Serial console, 115200 baud rate |
| `root=/dev/mmcblk0p2` | Root filesystem on SD partition 2 |
| `rootwait` | Wait for SD card to be ready |
| `rw` | Mount as read-write |
| `init=/sbin/init` | First process to run |

---

# TFTP Setup

## Install and Configure TFTP Server

```bash
sudo apt install tftpd-hpa
sudo nano /etc/default/tftpd-hpa
```

```
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/home/ayman/ITI/fady/tftp"
TFTP_ADDRESS="0.0.0.0:69"
TFTP_OPTIONS="--secure"
```

```bash
# Set permissions
sudo chown -R tftp:tftp ~/ITI/fady/tftp
chmod -R 755 ~/ITI/fady/tftp

# Restart
sudo systemctl restart tftpd-hpa
sudo systemctl status tftpd-hpa
```

## Copy Files to TFTP

```bash
mkdir -p ~/ITI/fady/tftp
cp zImage ~/ITI/fady/tftp/
cp vexpress-v2p-ca9.dtb ~/ITI/fady/tftp/
cp boot.scr ~/ITI/fady/tftp/
```

## TFTP Boot in U-Boot

```bash
setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1
tftp 0x60000000 zImage
tftp 0x65000000 vexpress-v2p-ca9.dtb
setenv bootargs "console=ttyAMA0,115200"
bootz 0x60000000 - 0x65000000
```

---

# NFS Boot

## Network Setup Diagram

```
┌──────────────────────────────┐
│         Your Laptop          │
│      tap0: 192.168.1.1      │
│                              │
│  TFTP: ~/ITI/fady/tftp/     │
│  NFS:  /media/ayman/rootfs/ │
└──────────────┬───────────────┘
               │ TAP Bridge
               ▼
┌──────────────────────────────┐
│       QEMU Vexpress         │
│      eth0: 192.168.1.100    │
│                              │
│  Kernel: loaded via TFTP    │
│  RootFS: mounted via NFS    │
└──────────────────────────────┘
```

## Step 1: Setup TAP Network

```bash
# Create TAP interface
sudo ip tuntap add dev tap0 mode tap user $USER

# Assign IP
sudo ip addr add 192.168.1.1/24 dev tap0

# Bring UP
sudo ip link set tap0 up

# Verify
ip addr show tap0
```

### Delete TAP (when needed)

```bash
sudo ip link set tap0 down
sudo ip tuntap del dev tap0 mode tap
```

## Step 2: Setup NFS Server

```bash
sudo apt install nfs-kernel-server

sudo nano /etc/exports
```

```
/media/ayman/rootfs *(rw,sync,no_root_squash,no_subtree_check,insecure)
```

```bash
sudo exportfs -ra
sudo systemctl restart nfs-kernel-server

# Verify
sudo exportfs -v
```

### Test NFS Locally

```bash
sudo mount -t nfs localhost:/media/ayman/rootfs /mnt
ls /mnt
sudo umount /mnt
```

## Step 3: Prepare NFS RootFS

```bash
cd /media/ayman/rootfs

# Create directories
sudo mkdir -p proc sys dev etc etc/init.d tmp bin sbin root

# Copy your custom init
sudo cp ~/ITI/fady/myinit/init sbin/init
sudo chmod +x sbin/init

# Verify structure
ls /media/ayman/rootfs/
```

Expected:
```
bin  dev  etc  proc  root  sbin  sys  tmp
```

## Step 4: NFS bootargs

```bash
setenv bootargs console=ttyAMA0,115200 root=/dev/nfs rootwait rw ip=192.168.1.100:192.168.1.1::255.255.255.0:vexpress:eth0:on nfsroot=192.168.1.1:/media/ayman/rootfs,nfsvers=3,tcp init=/sbin/init
```

### NFS bootargs Breakdown

| Argument | Meaning |
|----------|---------|
| `console=ttyAMA0,115200` | Serial console |
| `root=/dev/nfs` | Root filesystem is NFS |
| `rootwait` | Wait for root device |
| `rw` | Mount read-write |
| `ip=CLIENT:SERVER::MASK:HOSTNAME:DEVICE:AUTOCONF` | Network config |
| `nfsroot=SERVER_IP:PATH,nfsvers=3,tcp` | NFS server path and options |
| `init=/sbin/init` | First process to run |

### ip= Format Explained

```
ip=192.168.1.100:192.168.1.1::255.255.255.0:vexpress:eth0:on
   |             |             |              |        |    |
   Client IP    Server IP    Netmask        Hostname Device Autoconf
```

---

# Boot Script

## boot.cmd vs boot.scr

| | boot.cmd | boot.scr |
|--|----------|----------|
| Type | Text file | Binary file |
| Who reads it | You (human) | U-Boot (machine) |
| Editable | Yes | No |
| U-Boot runs it | No | Yes |
| Think of it as | `.c` source | `.bin` compiled |

## boot.cmd for Vexpress (SD + TFTP + NFS)

```bash
echo ============================================
echo === Ayman Custom Boot Script Vexpress ===
echo ============================================

setenv kernel_addr 0x60000000
setenv fdt_addr 0x65000000
setenv bootargs console=ttyAMA0,115200 root=/dev/nfs rootwait rw ip=192.168.1.100:192.168.1.1::255.255.255.0:vexpress:eth0:on nfsroot=192.168.1.1:/media/ayman/rootfs,nfsvers=3,tcp init=/sbin/init

echo >>> Step 1: Trying SD Card mmc 0:1 ...

if load mmc 0:1 ${kernel_addr} zImage; then if load mmc 0:1 ${fdt_addr} vexpress-v2p-ca9.dtb; then echo >>> SUCCESS Booting from SD Card...; bootz ${kernel_addr} - ${fdt_addr}; else echo !!! DTB not found on SD Card; fi; else echo !!! Kernel not found on SD Card; fi

echo >>> Step 2: Trying TFTP...

setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1

if tftp ${kernel_addr} zImage; then if tftp ${fdt_addr} vexpress-v2p-ca9.dtb; then echo >>> SUCCESS Booting from TFTP...; bootz ${kernel_addr} - ${fdt_addr}; else echo !!! DTB not found on TFTP; fi; else echo !!! Kernel not found on TFTP; fi

echo ============================================
echo !!! BOOT FAILED !!!
echo ============================================
```

## U-Boot Script Rules

| Rule | Wrong ❌ | Correct ✅ |
|------|----------|-----------|
| if/else | Multi-line | One line with `;` |
| echo | `echo "text"` | `echo text` |
| Multi-line | Using `\` | Everything in one line |
| Semicolons | Missing | Required between commands in if |

## Convert to boot.scr

```bash
# For ARM32 (Vexpress)
mkimage -C none -A arm -T script -d boot.cmd boot.scr

# For ARM64 (Raspberry Pi)
mkimage -C none -A arm64 -T script -d boot.cmd boot.scr
```

## Auto-run on boot

```bash
# In U-Boot
setenv bootcmd 'load mmc 0:1 0x62000000 boot.scr; source 0x62000000'
saveenv
```

---

# Custom Init

## init.c - Your Custom Shell

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);

    printf("============================================\n");
    printf("=== Welcome to Ayman's Custom Linux! ===\n");
    printf("============================================\n");

    char cmd[256];

    while (1)
    {
        printf("Ayman@MyLinux # ");
        fflush(stdout);

        if (fgets(cmd, sizeof(cmd), stdin) == NULL)
            continue;

        cmd[strcspn(cmd, "\n")] = 0;

        if (strcmp(cmd, "help") == 0)
        {
            printf("Available commands:\n");
            printf("  help    - Show this message\n");
            printf("  hello   - Print hello message\n");
            printf("  uname   - Show kernel info\n");
            printf("  clear   - Clear screen\n");
            printf("  reboot  - Reboot system\n");
        }
        else if (strcmp(cmd, "hello") == 0)
            printf("Hello from Ayman's Kernel! :)\n");
        else if (strcmp(cmd, "uname") == 0)
        {
            FILE *f = fopen("/proc/version", "r");
            if (f) { char buf[512]; fgets(buf, sizeof(buf), f); printf("%s", buf); fclose(f); }
        }
        else if (strcmp(cmd, "clear") == 0)
            printf("\033[2J\033[H");
        else if (strcmp(cmd, "reboot") == 0)
        {
            printf("Rebooting...\n"); sync(); reboot(0x01234567);
        }
        else if (strlen(cmd) > 0)
            printf("Unknown command: %s\nType 'help'\n", cmd);
    }
    return 0;
}
```

## Compile

```bash
# For ARM32 (Vexpress)
arm-linux-gnueabi-gcc -static -o init init.c

# For ARM64 (Raspberry Pi)
aarch64-linux-gnu-gcc -static -o init init.c

# Verify
file init
```

**Must be statically linked** because rootfs has no `/lib/` shared libraries.

---

# QEMU Commands

## run.sh for Vexpress with Network

```bash
#!/bin/bash

# Cleanup old instances
sudo killall qemu-system-arm 2>/dev/null
sudo ip link set tap0 down 2>/dev/null
sudo ip tuntap del dev tap0 mode tap 2>/dev/null

# Setup TAP
sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.1.1/24 dev tap0
sudo ip link set tap0 up

# Run QEMU
qemu-system-arm -M vexpress-a9 \
    -m 512 \
    -drive file=sd.img,format=raw,if=sd \
    -kernel ~/ITI/fady/u-boot/u-boot \
    -nographic \
    -net nic \
    -net tap,ifname=tap0,script=no,downscript=no
```

## QEMU Options Explained

| Option | Meaning |
|--------|---------|
| `-M vexpress-a9` | Machine type: Vexpress Cortex-A9 |
| `-m 512` | 512MB RAM |
| `-drive file=sd.img,format=raw,if=sd` | SD card image |
| `-kernel u-boot` | Load U-Boot as kernel |
| `-nographic` | No GUI, serial only |
| `-net nic` | Add network card |
| `-net tap,ifname=tap0` | Use TAP network |
| `-serial null -serial stdio` | Redirect serial to terminal |

## Exit QEMU

```
Ctrl+A then X
```

---

# Troubleshooting

## Common Errors and Fixes

| Error | Cause | Fix |
|-------|-------|-----|
| `No ethernet found` | RPi QEMU no network | Use Vexpress for network |
| `TFTP error: reserved memory` | Wrong load address | Use `0x62000000` |
| `Unknown command 'source'` | Old U-Boot | Use `autoscr` instead |
| `syntax error` in script | Multi-line if/else | Put if/else in one line |
| `tap0: Device busy` | Old QEMU still running | `sudo killall qemu-system-arm` |
| `Cannot find device tap0` | TAP not created | `sudo ip tuntap add dev tap0 mode tap` |
| `NFS: access denied` | Wrong exports | Add `insecure` and `*` to exports |
| `Unable to mount root fs via NFS` | NFS config wrong | Check exports, firewall, kernel config |
| `No working init found` | No rootfs or no init | Add rootfs with /sbin/init |
| `init failed (error -2)` | init file not found | Check path: `/sbin/init` not `init` |
| `unknown-block(0,0)` | No `root=` in bootargs | Add `root=/dev/mmcblk0p2` |

## Debug Commands

```bash
# Check NFS server
sudo systemctl status nfs-kernel-server
sudo exportfs -v

# Test NFS locally
sudo mount -t nfs localhost:/media/ayman/rootfs /mnt

# Check TAP
ip addr show tap0
ip tuntap show

# Check TFTP
sudo systemctl status tftpd-hpa
ls ~/ITI/fady/tftp/

# Check Kernel NFS support
grep "CONFIG_NFS" .config | grep "=y"
grep "CONFIG_ROOT_NFS" .config
grep "CONFIG_IP_PNP" .config

# In U-Boot
printenv
ping 192.168.1.1
ls mmc 0:1
```

---

# Quick Reference

## SD Card Boot (Quick)

```bash
# In U-Boot
load mmc 0:1 0x60000000 zImage
load mmc 0:1 0x65000000 vexpress-v2p-ca9.dtb
setenv bootargs console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait rw init=/sbin/init
bootz 0x60000000 - 0x65000000
```

## TFTP Boot (Quick)

```bash
# In U-Boot
setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1
tftp 0x60000000 zImage
tftp 0x65000000 vexpress-v2p-ca9.dtb
setenv bootargs console=ttyAMA0,115200
bootz 0x60000000 - 0x65000000
```

## NFS Boot (Quick)

```bash
# In U-Boot
setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1
tftp 0x60000000 zImage
tftp 0x65000000 vexpress-v2p-ca9.dtb
setenv bootargs console=ttyAMA0,115200 root=/dev/nfs rootwait rw ip=192.168.1.100:192.168.1.1::255.255.255.0:vexpress:eth0:on nfsroot=192.168.1.1:/media/ayman/rootfs,nfsvers=3,tcp init=/sbin/init
bootz 0x60000000 - 0x65000000
```

## Memory Addresses (Vexpress)

| Address | Used For |
|---------|----------|
| `0x60000000` | Kernel (zImage) |
| `0x62000000` | Boot script (boot.scr) |
| `0x65000000` | Device Tree (DTB) |

## Memory Addresses (Raspberry Pi)

| Address | Used For |
|---------|----------|
| `0x80000` | Kernel (Image) |
| `0x2600000` | Device Tree (DTB) |

## bootz vs booti

| Command | Architecture | Image Type |
|---------|-------------|------------|
| `bootz` | ARM32 | zImage (compressed) |
| `booti` | ARM64 | Image (uncompressed) |
| `bootm` | Any (old) | uImage |
```

---

Save it:

```bash
nano ~/ITI/fady/BOOT_GUIDE.md
# Paste everything above
```