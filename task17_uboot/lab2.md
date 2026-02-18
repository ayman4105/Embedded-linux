
# Embedded Linux – Bootloader Tasks

## 1. Definition
A bootloader is a small program that initializes hardware and loads the operating system into memory when the system starts.

---

## 2. Raspberry Pi Boot Process
After power-on, the system follows these specific stages:

1.  **Boot ROM:** The fixed code in Boot ROM executes first on the GPU.
2.  **bootcode.bin:** It loads `bootcode.bin` from the SD card.
3.  **start.elf:** This GPU firmware initializes hardware and reads `config.txt` to determine the next boot stage (U-Boot or Kernel).
4.  **U-Boot:** Control is transferred to U-Boot, and the U-Boot prompt appears.

---

## 2.1 PC Boot Process (x86 Architecture)
1.  **Power-on:** The PC starts on Core 0 (main core) and reads the Boot ROM.
2.  **Startup Code:** Executes initialization (init Stack Pointer, init clock, call main -> BIOS).
3.  **BIOS/UEFI:** Initializes hardware (specifically DRAM) and selects a boot device.
4.  **Bootloader:** Loads the system bootloader (e.g., GRUB or Windows Boot Manager).
5.  **Kernel:** The bootloader loads the kernel into RAM and transfers control to it.
6.  **Services:** The kernel initializes hardware and starts system services until the OS is fully running.

---

## 3. Comparison: U-Boot vs. GRUB
* **U-Boot:** Mainly used in embedded systems; it is responsible for initializing hardware before loading the kernel.
* **GRUB:** Used in PCs; it depends on BIOS/UEFI to initialize hardware before it loads the kernel.

---

## 4. Key Raspberry Pi Boot Files
* **bootcode.bin:** First-stage bootloader; initializes SDRAM and basic hardware.
* **start.elf:** GPU firmware; prepares hardware and reads `config.txt`.
* **config.txt:** Configuration file; specifies parameters and which image to load (e.g., `kernel=u-boot.bin`).

---

## 5. Build and Run U-Boot on QEMU (Vexpress-A9)

### A. Compilation Steps
1.  **Set default configuration:**
    ```bash
    make vexpress_ca9x4_defconfig
    ```
2.  **Set the cross-compiler and architecture:**
    ```bash
    export CROSS_COMPILE=arm-linux-gnueabi-
    ```
3.  **Build U-Boot:**
    ```bash
    make -j
    ```

### B. Run on QEMU
Execute the following command to run U-Boot without a graphical interface:
```bash
qemu-system-arm -M vexpress-a9 -kernel u-boot -nographic

```

---

## 6. Build and Deploy U-Boot on Raspberry Pi 3B+ (AArch64)

### A. Building Steps

```bash
export ARCH=arm
export CROSS_COMPILE=aarch64-rpi3-linux-gnu-
export PATH=$PATH:/home/ayman/x-tools/aarch64-rpi3-linux-gnu/bin
make rpi_3_defconfig
make -j

```

### B. Required Files for SD Card

To ensure the system boots correctly, the following files must be present on the boot partition:

* `bootcode.bin`
* `start.elf`
* `fixup.dat`
* `config.txt`
* `.dtb` file (Device Tree Binary)
* `u-boot.bin`

> **Note:** The Virtual SD Card preparation was completed in the previous task. Reference: [GitHub Repository](https://github.com/ayman4105/Embedded_linux)


## 7- Which file provides the hardware description to U-Boot on the Raspberry Pi 3B+ and at which stage is it loaded?

The file that provides the hardware description is:

`bcm2710-rpi-3-b-plus.dtb`

This is a **Device Tree Blob (DTB)** file.

### What does it contain?

It describes the hardware of the board, such as:
- CPU
- RAM
- GPIO
- UART
- USB
- MMC (SD card)
- Other peripherals

U-Boot reads this file to understand how the hardware is connected.

---

### At which stage is it loaded?

On the Raspberry Pi 3B+:

1. The GPU firmware (inside the SoC) starts first.
2. It reads `config.txt` from the FAT partition.
3. It loads:
   - `u-boot.bin`
   - The DTB file (`bcm2710-rpi-3-b-plus.dtb`)
4. Then U-Boot starts and uses the DTB to know the hardware layout.

So, the DTB is loaded **before U-Boot runs fully**, during the firmware stage from the SD card.


## 8- How Linux knows where partitions start inside an image file

When we run:

```bash
losetup --partscan --show -f sd.img
````

* `losetup` attaches the image file (`sd.img`) to a loop device (e.g., `/dev/loop5`).
* The `--partscan` option makes Linux read the **partition table** inside the image and create devices for each partition (e.g., `/dev/loop5p1`, `/dev/loop5p2`).

### How it works:

1. **Partition Table (MBR)**

   * Located at the first sector of the image.
   * Contains start sector, size, and type of each partition.

2. **Loop Devices**

   * Linux uses the start sector to map each partition to a loop device.
   * Reads and writes to `/dev/loop5p1` go to the correct offset inside `sd.img`.

3. **Filesystem Info**

   * The first sector of each partition contains filesystem info (Boot Sector for FAT32, Superblock for EXT4) so Linux can access files correctly.


```
