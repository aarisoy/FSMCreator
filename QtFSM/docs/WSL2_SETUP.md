# WSL2 Setup Guide for QtFSM

Quick setup guide for running QtFSM in WSL2.

## Prerequisites

- Windows 10 version 2004+ or Windows 11
- WSL2 installed with Ubuntu 22.04

## Step 1: Install WSL2 (if not already installed)

Open PowerShell as Administrator:

```powershell
wsl --install -d Ubuntu-22.04
```

Restart your computer, then open Ubuntu from Start menu.

## Step 2: Run Automated Setup Script

In your WSL2 terminal:

```bash
cd /mnt/c/Users/Pc/Desktop/QtFSM
chmod +x scripts/setup_dev_env.sh
./scripts/setup_dev_env.sh
```

This installs:
- ✅ Build tools (gcc, cmake, git, ninja)
- ✅ Qt6 framework
- ✅ OpenGL libraries
- ✅ Development tools (clang-format, gdb, valgrind)

**Time:** ~5-10 minutes depending on internet speed

## Step 3: Install X Server on Windows

To run GUI apps from WSL, you need an X server on Windows:

### Option A: VcXsrv (Recommended)

1. Download: https://sourceforge.net/projects/vcxsrv/
2. Install and run XLaunch
3. Select:
   - ✅ Multiple windows
   - ✅ Display number: 0
   - ✅ Start no client
   - ✅ Disable access control (for local testing)
4. Click Finish

### Option B: X410 (Paid, but nicer)

Available in Microsoft Store

## Step 4: Build QtFSM

```bash
# Navigate to project
cd /mnt/c/Users/Pc/Desktop/QtFSM

# Configure
cmake --preset dev

# Build
cmake --build build

# Run
./build/QtFSM
```

## Troubleshooting

### GUI doesn't appear

**Check DISPLAY variable:**
```bash
echo $DISPLAY  # Should show :0
```

**If empty, set it:**
```bash
export DISPLAY=:0
```

**Make it permanent:**
```bash
echo 'export DISPLAY=:0' >> ~/.bashrc
source ~/.bashrc
```

### "cannot open display" error

- Make sure X server (VcXsrv) is running on Windows
- Check Windows Firewall isn't blocking it
- Try: `export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0`

### Qt libraries not found

```bash
sudo apt install --reinstall qt6-base-dev qt6-tools-dev
```

### Build errors

```bash
# Clean and rebuild
rm -rf build
cmake --preset dev
cmake --build build
```

## Quick Reference

```bash
# Development workflow
cd /mnt/c/Users/Pc/Desktop/QtFSM

# Clean build
rm -rf build && cmake --preset dev && cmake --build build

# Run
./build/QtFSM

# Debug mode
gdb ./build/QtFSM
```

## What You'll See

When QtFSM launches:
- Main window with menu bar (File, Edit, View, Tools, Help)
- Toolbar with action buttons
- Central diagram editor (gray canvas)
- Properties panel on the right

## Next: Development

Once the app runs, we'll implement:
1. Interactive state creation
2. Transition drawing
3. Property editing
4. Code generation
5. Code parsing

Ready to code! 🚀
