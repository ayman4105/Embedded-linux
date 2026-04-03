# Systemd Boot Configuration for Qt Weather Application

## Table of Contents
- [Overview](#overview)
- [What is systemd?](#what-is-systemd)
- [Core Concepts](#core-concepts)
  - [Units](#units)
  - [Dependencies vs Ordering](#dependencies-vs-ordering)
  - [Requires vs Wants](#requires-vs-wants)
  - [Targets](#targets)
  - [Symlinks and Enable/Disable](#symlinks-and-enabledisable)
- [Project Scenario](#project-scenario)
- [File Hierarchy](#file-hierarchy)
- [Unit Files Explained](#unit-files-explained)
  - [1. Network Configuration](#1-network-configuration)
  - [2. Loop Mount Unit](#2-loop-mount-unit)
  - [3. Preparation Service](#3-preparation-service)
  - [4. Qt Application Service](#4-qt-application-service)
  - [5. Custom Target](#5-custom-target)
- [Dependency Chain](#dependency-chain)
- [Setup Instructions](#setup-instructions)
- [Verification Commands](#verification-commands)
- [Troubleshooting](#troubleshooting)

---

## Overview

This project implements a **systemd-based startup design** for an embedded Linux system where a Qt Weather Application starts automatically at boot. The system ensures that all required runtime dependencies (network, filesystem, environment) are ready before the application launches.

### What We Built
| Unit Type | File | Purpose |
|-----------|------|---------|
| `.network` | `10-static.network` | Static IP configuration |
| `.mount` | `mnt-qtdata.mount` | Loop-mount filesystem image |
| `.service` | `qtapp-prepare.service` | Environment preparation |
| `.service` | `qtapp.service` | Qt application launcher |
| `.target` | `qtapp.target` | Groups all units together |

---

## What is systemd?

systemd is the first process that runs on a Linux system (PID 1). It is the parent of all other processes and is responsible for:

- **Starting services** like web servers, databases, and network daemons
- **Mounting filesystems** like partitions and image files
- **Configuring network** interfaces
- **Managing the boot order** so everything starts in the correct sequence

Before systemd, Linux used **SysVinit** which started everything sequentially (one after another). systemd can start things **in parallel**, making boot significantly faster.

---

## Core Concepts

### Units

A **Unit** is the smallest entity that systemd manages. Every task you want systemd to perform must be written in a unit file.

Unit files are stored in these locations:
```
/lib/systemd/system/          → Units that come with the OS (original files)
/usr/lib/systemd/system/      → Units installed by packages
/etc/systemd/system/          → Units created by the administrator (highest priority)
/etc/systemd/network/         → Network configuration files for systemd-networkd
```

#### Unit Types Used in This Project

| Extension | Type | Description | Example |
|-----------|------|-------------|---------|
| `.service` | Service | A program or daemon that runs | `qtapp.service` |
| `.mount` | Mount | Mounts a filesystem to a directory | `mnt-qtdata.mount` |
| `.target` | Target | Groups units as a synchronization point | `qtapp.target` |
| `.network` | Network | Network interface configuration | `10-static.network` |

---

### Dependencies vs Ordering

This is the **most important concept** in systemd. They are two **separate** things:

#### Dependency (What do I need?)
```
Requires=B    → "If B fails, I fail too" (strict dependency)
Wants=B       → "I want B, but if B fails, I still run" (soft dependency)
```

#### Ordering (When do I start?)
```
After=B       → "Wait for B to finish, then start me"
Before=B      → "Start me before B starts"
```

#### Why They Are Separate

`Requires=B` does **NOT** mean B starts before you. Both could start at the same time.

`After=B` does **NOT** mean you need B. If B doesn't exist, you still start.

**To get the behavior "I need B and B must finish first", you must use BOTH:**
```ini
Requires=B
After=B
```

---

### Requires vs Wants

| | Requires | Wants |
|---|----------|-------|
| **Strictness** | Strict | Soft |
| **If dependency fails** | This unit also fails | This unit still runs |
| **Use case** | Critical dependencies | Optional dependencies |
| **Example** | App requires database | Server wants logging |

In our project:
- `qtapp.service` uses `Requires=qtapp-prepare.service` because the app **cannot** run without preparation
- `qtapp.target` uses `Wants=network-online.target` because we **want** the network but the target itself can still be reached

---

### Targets

A **target** is a unit that groups other units together. It doesn't run anything by itself. It acts as a **synchronization point** in the boot process.

#### Built-in Targets in Linux

| Target | Description |
|--------|-------------|
| `multi-user.target` | System is ready for text-based login, all basic services are running |
| `graphical.target` | Desktop environment is ready (comes after multi-user.target) |
| `network-online.target` | Network is fully configured and an IP address is assigned |

#### Boot Flow
```
sysinit.target → basic.target → multi-user.target → graphical.target
                                                          ↓
                                                    qtapp.target (our custom target)
                                                          ↓
                                              ┌───────────┼───────────┐
                                              ↓           ↓           ↓
                                         mnt-qtdata   qtapp-prepare  qtapp
                                           .mount      .service     .service
```

---

### Symlinks and Enable/Disable

When you run `systemctl enable some.service`, systemd does **NOT** copy the file. It creates a **symbolic link (symlink)** inside a `.wants` directory.

#### How enable works:
```
Original file:  /lib/systemd/system/some.service
Symlink:        /etc/systemd/system/graphical.target.wants/some.service → /lib/systemd/system/some.service
```

The `.wants` directory is the **playlist** for that target. When systemd reaches that target, it starts everything linked inside `.wants`.

#### How disable works:
It simply **deletes the symlink**. The original file remains untouched.

#### Why symlinks instead of copies?
- Only one copy of the file exists (the original)
- Any changes to the original are automatically reflected
- No risk of having two different versions of the same file

---

## Project Scenario

We have an **embedded Linux system** running a **Qt Weather Application** that:
- Fetches weather data from an online API (needs network)
- Reads image assets from a loop-mounted filesystem image
- Needs environment preparation before it can start

### Boot Sequence
```
1. System boots → systemd starts (PID 1)
2. systemd-networkd reads 10-static.network → eth0 gets static IP
3. network-online.target becomes ready
4. graphical.target starts → pulls in qtapp.target
5. qtapp.target starts → pulls in all our units
6. mnt-qtdata.mount → mounts /opt/qtdata.img to /mnt/qtdata
7. qtapp-prepare.service → checks that images exist in /mnt/qtdata
8. qtapp.service → launches the WeatherApp on screen
9. qtapp.target is reached → full stack is running
```

---

## File Hierarchy

```
/
├── etc/
│   └── systemd/
│       ├── network/
│       │   └── 10-static.network          ← Network configuration (static IP)
│       └── system/
│           ├── mnt-qtdata.mount           ← Loop mount unit
│           ├── qtapp-prepare.service      ← Preparation service
│           ├── qtapp.service              ← Qt application service
│           ├── qtapp.target               ← Custom target (groups everything)
│           └── graphical.target.wants/
│               └── qtapp.target → ../qtapp.target  ← Created by 'enable'
│
├── opt/
│   └── qtdata.img                         ← Filesystem image (ext4, 64MB)
│
├── mnt/
│   └── qtdata/                            ← Mount point (images appear here)
│       ├── image.png
│       ├── image-1.png
│       └── image-2.png
│
├── usr/
│   └── local/
│       └── bin/
│           ├── appweatherapp              ← Qt application binary
│           └── qtapp-prepare.sh           ← Preparation script
│
└── run/
    └── qtapp/                             ← Runtime directory (created by prep script)
```

---

## Unit Files Explained

### 1. Network Configuration

**File:** `/etc/systemd/network/10-static.network`

```ini
[Match]
Name=enp0s31f6

[Network]
Address=192.168.1.100/24
Gateway=192.168.1.1
DNS=8.8.8.8
```

#### Section Breakdown

**[Match]** — Determines which interface this config applies to
- `Name=enp0s31f6` → Only apply to the wired ethernet interface

**[Network]** — The actual network settings
- `Address=192.168.1.100/24` → Static IP address. The `/24` means subnet mask 255.255.255.0 (first 3 octets define the network, last octet identifies the device)
- `Gateway=192.168.1.1` → The router's IP. Any traffic going outside the local network (192.168.1.x) is sent to this address
- `DNS=8.8.8.8` → Google's DNS server for resolving domain names to IP addresses

#### How it works
The service `systemd-networkd` reads all `.network` files from `/etc/systemd/network/` and applies them. When the interface gets its IP, `network-online.target` becomes ready.

---

### 2. Loop Mount Unit

**File:** `/etc/systemd/system/mnt-qtdata.mount`

```ini
[Unit]
Description=Mount Qt application data image

[Mount]
What=/opt/qtdata.img
Where=/mnt/qtdata
Type=ext4
Options=loop

[Install]
WantedBy=qtapp.target
```

#### Section Breakdown

**[Unit]**
- `Description` → Human-readable description shown in `systemctl status`

**[Mount]**
- `What=/opt/qtdata.img` → The filesystem image file to mount
- `Where=/mnt/qtdata` → The directory where the image contents become accessible
- `Type=ext4` → The filesystem format inside the image
- `Options=loop` → Tells the kernel this is an image file, not a real partition. The kernel uses a **loop device** to treat the file as a block device

**[Install]**
- `WantedBy=qtapp.target` → When enabled, this unit gets linked into qtapp.target's wants

#### Critical Naming Rule
The mount unit filename **must** match the `Where=` path:
- Replace every `/` with `-`
- Remove the leading `/`
- `Where=/mnt/qtdata` → filename must be `mnt-qtdata.mount`

If the name doesn't match, systemd will ignore it.

#### How the image was created
```bash
# Create a 64MB empty file
sudo dd if=/dev/zero of=/opt/qtdata.img bs=1M count=64

# Format it with ext4 filesystem
sudo mkfs.ext4 /opt/qtdata.img

# Create mount point
sudo mkdir -p /mnt/qtdata

# Temporarily mount to copy files
sudo mount -o loop /opt/qtdata.img /mnt/qtdata

# Copy application images into the image
sudo cp ~/ITI/anas/Qt/WeatherApp/image*.png /mnt/qtdata/

# Unmount
sudo umount /mnt/qtdata
```

---

### 3. Preparation Service

**File:** `/usr/local/bin/qtapp-prepare.sh`

```bash
#!/bin/bash

echo "Preparing Qt application environment..."
mkdir -p /run/qtapp

if [ ! -f /mnt/qtdata/image.png ]; then
    echo "ERROR: image.png not found"
    exit 1
fi

echo "Environment ready."
exit 0
```

**File:** `/etc/systemd/system/qtapp-prepare.service`

```ini
[Unit]
Description=Prepare environment for Qt application
Requires=mnt-qtdata.mount
After=mnt-qtdata.mount

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/usr/local/bin/qtapp-prepare.sh

[Install]
WantedBy=qtapp.target
```

#### Section Breakdown

**[Unit]**
- `Requires=mnt-qtdata.mount` → If the mount fails, this service also fails (strict dependency)
- `After=mnt-qtdata.mount` → Wait for the mount to complete before running (ordering). The script checks files inside `/mnt/qtdata/`, so the mount must be done first

**[Service]**
- `Type=oneshot` → This service runs a task and exits. It doesn't stay running like a daemon
- `RemainAfterExit=yes` → After the script exits successfully, keep the service status as **active**. This is important because `qtapp.service` depends on this service — if it showed as "inactive", systemd would think it's not ready
- `ExecStart=/usr/local/bin/qtapp-prepare.sh` → The script to execute

**Exit Codes:**
- `exit 0` → Success. Service status becomes **active (exited)**
- `exit 1` → Failure. Service status becomes **failed**, and dependent services won't start

---

### 4. Qt Application Service

**File:** `/etc/systemd/system/qtapp.service`

```ini
[Unit]
Description=Qt Weather Application Service
Requires=qtapp-prepare.service
After=qtapp-prepare.service
Wants=network-online.target
After=network-online.target
RequiresMountsFor=/mnt/qtdata

[Service]
Type=simple
User=ayman
Environment=DISPLAY=:0
Environment=WAYLAND_DISPLAY=wayland-0
Environment=XDG_RUNTIME_DIR=/run/user/1000
Environment=QT_QPA_PLATFORM=wayland
ExecStart=/usr/local/bin/appweatherapp
Restart=on-failure

[Install]
WantedBy=qtapp.target
```

#### Section Breakdown

**[Unit]**
- `Requires=qtapp-prepare.service` → Cannot run without successful preparation
- `After=qtapp-prepare.service` → Wait for preparation to complete
- `Wants=network-online.target` → Request network readiness (soft dependency)
- `After=network-online.target` → Wait for network to be ready
- `RequiresMountsFor=/mnt/qtdata` → This is a smart shortcut. It automatically adds both `Requires=` and `After=` for any mount unit that provides `/mnt/qtdata`. So if the mount fails, this service won't start

**[Service]**
- `Type=simple` → The application starts and keeps running (unlike oneshot)
- `User=ayman` → Run the app as user `ayman`, not as root. This is necessary because the display server (Wayland) runs under the user session, and root cannot access it
- `Environment=DISPLAY=:0` → The X11 display identifier (fallback)
- `Environment=WAYLAND_DISPLAY=wayland-0` → The Wayland display socket name
- `Environment=XDG_RUNTIME_DIR=/run/user/1000` → Directory containing Wayland socket files. `1000` is the user ID of `ayman`
- `Environment=QT_QPA_PLATFORM=wayland` → Tell Qt to use the Wayland display protocol instead of X11 (xcb)
- `ExecStart=/usr/local/bin/appweatherapp` → Path to the Qt application binary
- `Restart=on-failure` → If the app crashes, systemd automatically restarts it

---

### 5. Custom Target

**File:** `/etc/systemd/system/qtapp.target`

```ini
[Unit]
Description=Qt Application Full Stack Target
Requires=mnt-qtdata.mount
Requires=qtapp-prepare.service
Requires=qtapp.service
Wants=network-online.target
After=mnt-qtdata.mount
After=qtapp-prepare.service
After=qtapp.service
After=network-online.target

[Install]
WantedBy=graphical.target
```

#### Section Breakdown

**[Unit]**
- Three `Requires=` lines → The target needs all three units. If any fails, the target fails
- `Wants=network-online.target` → Request network readiness
- Four `After=` lines → The target is not considered "reached" until all four units are done

**[Install]**
- `WantedBy=graphical.target` → This is what integrates our entire stack into the normal boot flow. When `graphical.target` is reached during boot, it pulls in `qtapp.target`, which pulls in everything else

---

## Dependency Chain

### Visual Dependency Graph

```
graphical.target
    │
    │ (WantedBy)
    ▼
qtapp.target
    │
    ├── Requires + After ──→ mnt-qtdata.mount
    │                            │
    │                            └── Mounts /opt/qtdata.img → /mnt/qtdata
    │
    ├── Requires + After ──→ qtapp-prepare.service
    │                            │
    │                            ├── Requires + After → mnt-qtdata.mount
    │                            └── Runs /usr/local/bin/qtapp-prepare.sh
    │
    ├── Requires + After ──→ qtapp.service
    │                            │
    │                            ├── Requires + After → qtapp-prepare.service
    │                            ├── Wants + After → network-online.target
    │                            ├── RequiresMountsFor → /mnt/qtdata
    │                            └── Runs /usr/local/bin/appweatherapp
    │
    └── Wants + After ─────→ network-online.target
                                 │
                                 └── systemd-networkd reads 10-static.network
```

### Execution Order

```
Step 1: systemd-networkd configures static IP
Step 2: network-online.target is reached
Step 3: mnt-qtdata.mount mounts the image
Step 4: qtapp-prepare.service runs checks
Step 5: qtapp.service launches WeatherApp
Step 6: qtapp.target is reached (all done)
```

### Failure Scenarios

| What Fails | What Happens |
|-----------|--------------|
| Mount fails | Preparation service won't start → App won't start → Target fails |
| Preparation fails | App won't start → Target fails |
| Network is slow | App waits for network-online.target before starting |
| App crashes | systemd restarts it automatically (Restart=on-failure) |

---

## Setup Instructions

### Step 1: Create the filesystem image
```bash
sudo dd if=/dev/zero of=/opt/qtdata.img bs=1M count=64
sudo mkfs.ext4 /opt/qtdata.img
sudo mkdir -p /mnt/qtdata
sudo mount -o loop /opt/qtdata.img /mnt/qtdata
sudo cp ~/ITI/anas/Qt/WeatherApp/image*.png /mnt/qtdata/
sudo umount /mnt/qtdata
```

### Step 2: Copy the application binary
```bash
sudo cp ~/ITI/anas/Qt/WeatherApp/weatherapp/build/Desktop_Qt_6_10_2-Debug/appweatherapp /usr/local/bin/
sudo chmod +x /usr/local/bin/appweatherapp
```

### Step 3: Create the preparation script
```bash
sudo nano /usr/local/bin/qtapp-prepare.sh
# (paste script content)
sudo chmod +x /usr/local/bin/qtapp-prepare.sh
```

### Step 4: Create the network configuration
```bash
sudo nano /etc/systemd/network/10-static.network
# (paste network config)
```

### Step 5: Create all systemd unit files
```bash
sudo nano /etc/systemd/system/mnt-qtdata.mount
sudo nano /etc/systemd/system/qtapp-prepare.service
sudo nano /etc/systemd/system/qtapp.service
sudo nano /etc/systemd/system/qtapp.target
```

### Step 6: Reload and enable everything
```bash
sudo systemctl daemon-reload
sudo systemctl enable systemd-networkd
sudo systemctl enable mnt-qtdata.mount
sudo systemctl enable qtapp-prepare.service
sudo systemctl enable qtapp.service
sudo systemctl enable qtapp.target
```

### Step 7: Start and verify
```bash
sudo systemctl start qtapp.target
systemctl list-dependencies qtapp.target
```

---

## Verification Commands

### Check dependency tree
```bash
systemctl list-dependencies qtapp.target
```

### Check individual unit status
```bash
systemctl status mnt-qtdata.mount
systemctl status qtapp-prepare.service
systemctl status qtapp.service
systemctl status qtapp.target
```

### Check logs for any unit
```bash
journalctl -u qtapp.service -n 30
journalctl -u qtapp-prepare.service -n 20
```

### Verify the mount is working
```bash
mount | grep qtdata
ls /mnt/qtdata/
```

### Verify network configuration
```bash
systemctl status systemd-networkd
ip addr show enp0s31f6
```

### Check what graphical.target pulls in
```bash
systemctl list-dependencies graphical.target | grep qtapp
```

---

## Troubleshooting

### Problem: Qt app fails with "could not connect to display"
**Cause:** The app doesn't have access to the display server
**Solution:** Make sure these lines are in `qtapp.service`:
```ini
User=ayman
Environment=DISPLAY=:0
Environment=WAYLAND_DISPLAY=wayland-0
Environment=XDG_RUNTIME_DIR=/run/user/1000
Environment=QT_QPA_PLATFORM=wayland
```

### Problem: Mount unit is ignored
**Cause:** Filename doesn't match the `Where=` path
**Solution:** For `Where=/mnt/qtdata`, filename must be `mnt-qtdata.mount`

### Problem: Changes to unit files are not applied
**Cause:** systemd caches unit files in memory
**Solution:** Run `sudo systemctl daemon-reload` after any change

### Problem: Service not starting at boot
**Cause:** Service is not enabled (no symlink in `.wants` directory)
**Solution:** Run `sudo systemctl enable <unit-name>`

---

## Author
Ayman — ITI Embedded Linux Track
