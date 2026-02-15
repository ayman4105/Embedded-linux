
# Embedded Linux – Bootloader Tasks

---

## 1. What is a Bootloader?

A bootloader is a small program that initializes hardware and loads the operating system into memory when the system starts.

---

## 2. Raspberry Pi Boot Chain (Until U-Boot Prompt)

After power-on (the GPU runs first), the fixed code in the Boot ROM executes.  
It loads `bootcode.bin` from the SD card.  

Then:

- `start.elf` initializes hardware.
- It reads `config.txt`, which configures which image to boot (U-Boot or kernel).
- `u-boot.bin` is loaded into memory.

Finally, control is transferred to U-Boot and the **U-Boot prompt** appears.

---

## 2.1 PC Boot Chain (Until OS Running)

After power-on, the PC starts execution on Core 0 (main core) and reads the Boot ROM.

Then:

- Startup code runs (initialize stack pointer, initialize clock, call main → BIOS).
- BIOS initializes hardware (especially DRAM).
- BIOS selects a boot device.
- It loads the bootloader (GRUB or Windows Boot Manager).
- The bootloader loads the kernel into RAM.
- Control is transferred to the kernel.
- The kernel initializes hardware and starts system services.
- The OS becomes fully running.

---

## 3. Difference Between U-Boot and GRUB

- **U-Boot** is mainly used in embedded systems and initializes hardware before loading the kernel.
- **GRUB** is used in PCs and depends on BIOS/UEFI to initialize hardware before loading the kernel.

---

## 4. Required Files in Raspberry Pi Boot Partition

- `bootcode.bin` – First-stage bootloader; initializes SDRAM and basic hardware.
- `start.elf` – GPU firmware; prepares hardware and reads `config.txt`.
- `config.txt` – Configuration file; specifies which kernel/bootloader to load (`kernel=u-boot.bin`).
- `fixup.dat` – Firmware support file.
- `.dtb` file – Device Tree Blob that describes hardware and base addresses.

---

## 5. Build and Test Custom U-Boot in QEMU (Cortex-A9)

### a) Build U-Boot

1. Set default configuration:
   ```bash
   make vexpress_ca9x4_defconfig
````

2. Set cross-compiler:

   ```bash
   export CROSS_COMPILE=arm-linux-gnueabi-
   ```

3. Build:

   ```bash
   make -j
   ```

### b) Run U-Boot in QEMU

```bash
qemu-system-arm -M vexpress-a9 -kernel u-boot -nographic
```

This runs U-Boot on the Vexpress-A9 board inside QEMU without a graphical interface.

---

## 6. Build and Deploy U-Boot on Raspberry Pi 3B+ (AArch64)

### a) Build U-Boot

1. Set architecture and cross-compiler:

   ```bash
   export ARCH=arm
   export CROSS_COMPILE=aarch64-rpi3-linux-gnu-
   export PATH=$PATH:/home/ayman/x-tools/aarch64-rpi3-linux-gnu/bin
   ```

2. Load board configuration:

   ```bash
   make rpi_3_defconfig
   ```

3. Build:

   ```bash
   make -j
   ```

---

### b) Prepare SD Card

Required files:

* `bootcode.bin`
* `start.elf`
* `fixup.dat`
* `config.txt`
* `.dtb` file (Device Tree)
* `u-boot.bin`

The virtual SD card was prepared in the previous task:
[https://github.com/ayman4105/Embedded_linux](https://github.com/ayman4105/Embedded_linux)

---

