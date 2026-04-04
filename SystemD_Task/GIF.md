# Splash Screen GIF with systemd User Service

## Table of Contents
- [Overview](#overview)
- [How It Works](#how-it-works)
- [System Service vs User Service](#system-service-vs-user-service)
- [File Hierarchy](#file-hierarchy)
- [Files Explained](#files-explained)
  - [1. Splash GIF Image](#1-splash-gif-image)
  - [2. Splash Screen Script](#2-splash-screen-script)
  - [3. User Service Unit](#3-user-service-unit)
- [Problems We Faced and How We Solved Them](#problems-we-faced-and-how-we-solved-them)
  - [Problem 1: --on-top option not found](#problem-1---on-top-option-not-found)
  - [Problem 2: GIF not visible because WeatherApp covers it](#problem-2-gif-not-visible-because-weatherapp-covers-it)
  - [Problem 3: Permission denied on /dev/dri](#problem-3-permission-denied-on-devdri)
  - [Problem 4: Service runs before desktop is ready](#problem-4-service-runs-before-desktop-is-ready)
  - [Problem 5: Ordering cycle detected](#problem-5-ordering-cycle-detected)
  - [Problem 6: Service does not start after reboot](#problem-6-service-does-not-start-after-reboot)
- [Dependency Chain](#dependency-chain)
- [Setup Instructions](#setup-instructions)
- [Verification Commands](#verification-commands)
- [Integration with Qt Application Stack](#integration-with-qt-application-stack)

---

## Overview

This component adds a **splash screen GIF** that plays automatically every time the user logs in. The GIF displays fullscreen for a few seconds and then disappears, after which the Qt Weather Application launches.

The splash screen uses:
- **mpv** as the media player to display the GIF fullscreen
- **A bash script** to control the timing (show GIF → wait → close GIF)
- **A systemd user service** to start the script automatically after login

---

## How It Works

```
User types password
        ↓
GNOME desktop starts
        ↓
graphical-session.target is reached
        ↓
splash-screen.service starts (user service)
        ↓
Script waits 2 seconds (display server stabilization)
        ↓
mpv opens the GIF fullscreen
        ↓
GIF plays for 7 seconds
        ↓
mpv is killed → GIF disappears
        ↓
Script exits → service becomes inactive
        ↓
Ready for next login
```

---

## System Service vs User Service

During development, we discovered that a **system service** (in `/etc/systemd/system/`) cannot reliably display GUI applications after boot. Here is why:

| Aspect | System Service | User Service |
|--------|---------------|--------------|
| **Runs as** | root (or specified user) | The logged-in user |
| **Starts when** | During boot (before login) | After user logs in |
| **Display access** | No direct access to display | Full access to user's display |
| **GPU access** | Needs group permissions | Already has permissions |
| **Config location** | `/etc/systemd/system/` | `~/.config/systemd/user/` |
| **Commands** | `sudo systemctl ...` | `systemctl --user ...` |

### Why We Switched to User Service

When we used a system service, we encountered these problems:
1. **Permission denied** on `/dev/dri/renderD128` — the GPU device was not accessible
2. **Display not found** — the Wayland display was not ready yet
3. **Ordering cycle** — linking to both `graphical.target` and `qtapp.target` created circular dependencies

A user service solves all of these because it runs **after** the user logs in, when the display, GPU, and session are all fully ready.

---

## File Hierarchy

```
/
├── opt/
│   └── splash.gif                              ← The GIF image file
│
├── usr/
│   └── local/
│       └── bin/
│           └── splash-screen.sh                ← Script that shows and closes the GIF
│
└── home/
    └── ayman/
        └── .config/
            └── systemd/
                └── user/
                    └── splash-screen.service    ← User service unit file
```

---

## Files Explained

### 1. Splash GIF Image

**File:** `/opt/splash.gif`

This is the GIF animation that displays on screen at login. It was copied from the Downloads folder:

```bash
sudo cp ~/Downloads/kitty\ kiss\ GIF.gif /opt/splash.gif
```

**Details:**
- Format: GIF 89a
- Resolution: 400 x 225
- Location: `/opt/` (standard location for optional application data)

---

### 2. Splash Screen Script

**File:** `/usr/local/bin/splash-screen.sh`

```bash
#!/bin/bash

export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0
export XDG_RUNTIME_DIR=/run/user/1000

sleep 2

mpv --fullscreen --loop=inf --really-quiet --no-border /opt/splash.gif &
MPV_PID=$!

sleep 7

kill $MPV_PID 2>/dev/null
wait $MPV_PID 2>/dev/null

exit 0
```

#### Line-by-Line Explanation

**`#!/bin/bash`**
Shebang line. Tells the system to execute this script using bash.

**`export DISPLAY=:0`**
Sets the X11 display variable. Even on Wayland, some applications check this as a fallback.

**`export WAYLAND_DISPLAY=wayland-0`**
Sets the Wayland display socket name. mpv uses this to connect to the Wayland compositor (GNOME).

**`export XDG_RUNTIME_DIR=/run/user/1000`**
Sets the runtime directory where Wayland socket files are stored. The number `1000` is the user ID of `ayman`. The Wayland socket file `wayland-0` lives inside this directory.

**`sleep 2`**
Waits 2 seconds before starting mpv. This gives the display server time to fully initialize after login. Without this delay, mpv might fail to connect to the display.

**`mpv --fullscreen --loop=inf --really-quiet --no-border /opt/splash.gif &`**
- `mpv` — The media player that displays the GIF
- `--fullscreen` — Opens the window in fullscreen mode
- `--loop=inf` — Loops the GIF animation infinitely (since GIFs are short)
- `--really-quiet` — Suppresses all terminal output
- `--no-border` — Removes the window border so it looks like a real splash screen
- `/opt/splash.gif` — Path to the GIF file
- `&` — Runs mpv in the background so the script continues to the next line

**`MPV_PID=$!`**
Captures the process ID (PID) of the background mpv process. `$!` is a bash variable that holds the PID of the last background process.

**`sleep 7`**
Waits 7 seconds while the GIF is displayed on screen. This is the splash screen duration. Adjustable to any value.

**`kill $MPV_PID 2>/dev/null`**
Sends a termination signal to mpv using its PID. `2>/dev/null` redirects any error messages to nowhere (in case mpv already exited).

**`wait $MPV_PID 2>/dev/null`**
Waits for the mpv process to fully terminate before continuing. This prevents zombie processes.

**`exit 0`**
Exits the script with code 0 (success). systemd will see this and mark the service as completed successfully.

---

### 3. User Service Unit

**File:** `~/.config/systemd/user/splash-screen.service`

```ini
[Unit]
Description=Splash Screen GIF at boot
After=graphical-session.target

[Service]
Type=oneshot
Environment=DISPLAY=:0
Environment=WAYLAND_DISPLAY=wayland-0
Environment=XDG_RUNTIME_DIR=/run/user/1000
ExecStart=/usr/local/bin/splash-screen.sh
RemainAfterExit=no

[Install]
WantedBy=graphical-session.target
```

#### Section Breakdown

**[Unit]**

`Description=Splash Screen GIF at boot`
Human-readable description shown in status commands.

`After=graphical-session.target`
Wait for the graphical session to be fully ready before starting. `graphical-session.target` is a **user target** that is reached when the desktop environment (GNOME) is fully loaded and the display is ready. This is different from `graphical.target` which is a **system target** that is reached when the display manager (login screen) starts.

**[Service]**

`Type=oneshot`
The service runs a task and exits. It does not stay running as a daemon.

`Environment=DISPLAY=:0`
Sets the X11 display variable for the service process.

`Environment=WAYLAND_DISPLAY=wayland-0`
Sets the Wayland display socket for the service process.

`Environment=XDG_RUNTIME_DIR=/run/user/1000`
Sets the runtime directory for the service process.

`ExecStart=/usr/local/bin/splash-screen.sh`
The script to execute when the service starts.

`RemainAfterExit=no`
After the script finishes, mark the service as **inactive**. This is important because:
- With `RemainAfterExit=yes`: the service stays "active" after exit, and on next boot systemd thinks it's already running and does not start it again
- With `RemainAfterExit=no`: the service goes back to "inactive" after exit, so on next boot it starts fresh

This was one of the key bugs we fixed — the service was not starting on reboot because it still showed as "active" from the previous session.

**[Install]**

`WantedBy=graphical-session.target`
When enabled, a symlink is created so that this service starts automatically whenever `graphical-session.target` is reached (every time the user logs in).

---

## Problems We Faced and How We Solved Them

### Problem 1: --on-top option not found

**Error:**
```
Error parsing option on-top (option not found)
```

**Cause:** The installed version of mpv does not support the `--on-top` flag.

**Solution:** Removed `--on-top` from the mpv command. Used `--fullscreen` and `--no-border` instead.

---

### Problem 2: GIF not visible because WeatherApp covers it

**Symptom:** The splash service ran successfully but only the WeatherApp was visible on screen.

**Cause:** Initially `sleep` was set to 5 seconds which was too short. mpv needs time to open the window and render the first frame.

**Solution:** Increased sleep to 7 seconds and added a 2-second delay before starting mpv to allow the display server to stabilize.

---

### Problem 3: Permission denied on /dev/dri

**Error:**
```
libEGL warning: failed to open /dev/dri/renderD128: Permission denied
```

**Cause:** When running as a system service, the user `ayman` did not have permission to access the GPU device during early boot.

**Solution:** 
1. Added user to render and video groups: `sudo usermod -aG render,video ayman`
2. Switched to a user service which runs after login when permissions are already granted

---

### Problem 4: Service runs before desktop is ready

**Symptom:** The service started during boot before the login screen, so there was no display to show the GIF on.

**Cause:** The system service was configured with `After=graphical.target` which only waits for the display manager (login screen), not for the actual desktop session.

**Solution:** Switched to a user service with `After=graphical-session.target` which waits for the full desktop session to be ready.

---

### Problem 5: Ordering cycle detected

**Error:**
```
Found ordering cycle on qtapp.target/stop
Found dependency on graphical.target/stop
Found dependency on splash-screen.service/stop
```

**Cause:** The splash service had `WantedBy=graphical.target` in Install and `After=graphical.target` in Unit. Combined with `qtapp.target` also being `WantedBy=graphical.target` and depending on splash-screen.service, this created a circular dependency during shutdown.

**Solution:** Moved the splash screen to a user service, completely separating it from the system service dependency chain.

---

### Problem 6: Service does not start after reboot

**Symptom:** The service worked when started manually but did not start automatically after reboot.

**Cause:** `RemainAfterExit=yes` was set, which kept the service in "active (exited)" state. On next boot, systemd saw it as already active and did not start it again.

**Solution:** Changed to `RemainAfterExit=no` so the service returns to "inactive" after completion, allowing it to start fresh on next boot.

---

## Dependency Chain

```
User Login
    ↓
GNOME Session Starts
    ↓
graphical-session.target (user target)
    ↓
splash-screen.service (user service)
    ↓
/usr/local/bin/splash-screen.sh
    ↓
sleep 2 → mpv shows GIF → sleep 7 → kill mpv
    ↓
Service exits (inactive)
```

---

## Setup Instructions

### Step 1: Install mpv
```bash
sudo apt install mpv
```

### Step 2: Copy the GIF
```bash
sudo cp ~/Downloads/kitty\ kiss\ GIF.gif /opt/splash.gif
```

### Step 3: Create the script
```bash
sudo nano /usr/local/bin/splash-screen.sh
# (paste script content)
sudo chmod +x /usr/local/bin/splash-screen.sh
```

### Step 4: Add user to GPU groups
```bash
sudo usermod -aG render ayman
sudo usermod -aG video ayman
```

### Step 5: Create user service directory
```bash
mkdir -p ~/.config/systemd/user/
```

### Step 6: Create the user service
```bash
nano ~/.config/systemd/user/splash-screen.service
# (paste service content)
```

### Step 7: Enable the service
```bash
systemctl --user daemon-reload
systemctl --user enable splash-screen.service
```

### Step 8: Reboot and test
```bash
sudo reboot
```

---

## Verification Commands

### Check service status (user service)
```bash
systemctl --user status splash-screen.service
```

### Check logs (user service)
```bash
journalctl --user -u splash-screen.service --no-pager
```

### Check logs for current boot only
```bash
journalctl --user -b -u splash-screen.service --no-pager
```

### Check if mpv is running (while splash is showing)
```bash
ps aux | grep mpv
```

### Check service timing
```bash
systemctl --user show splash-screen.service --property=ExecMainStartTimestamp,ExecMainExitTimestamp
```

### Manually start the service
```bash
systemctl --user start splash-screen.service
```

### Manually stop the service
```bash
systemctl --user stop splash-screen.service
```

### Disable auto-start
```bash
systemctl --user disable splash-screen.service
```

---

## Integration with Qt Application Stack

The splash screen runs as a **user service** while the Qt application stack runs as **system services**. They are independent:

```
SYSTEM LEVEL:                          USER LEVEL:
─────────────                          ───────────
graphical.target                       graphical-session.target
      ↓                                       ↓
qtapp.target                           splash-screen.service
      ↓                                       ↓
├── mnt-qtdata.mount                   Shows GIF for 7 seconds
├── qtapp-prepare.service
└── qtapp.service (if enabled)
```

The splash screen does not depend on the Qt application stack and the Qt application stack does not depend on the splash screen. They can be enabled or disabled independently.
