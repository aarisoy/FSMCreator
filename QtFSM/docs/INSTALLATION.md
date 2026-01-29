# QtFSM Installation Guide

This guide will walk you through installing all required tools to build and run QtFSM.

## Choose Your Development Environment

You have two options:

1. **WSL2 (Ubuntu)** - Recommended for best compatibility with CI/CD
2. **Native Windows** - Good if you prefer Windows-native development

---

## Option 1: WSL2 Setup (Recommended)

### Step 1: Install WSL2

If you don't have WSL2 installed yet:

```powershell
# Run in PowerShell as Administrator
wsl --install -d Ubuntu-22.04
# Restart your computer when prompted
```

### Step 2: Install Required Packages

Open WSL2 terminal and run:

```bash
# Update package lists
sudo apt update

# Install build essentials
sudo apt install -y build-essential cmake git ninja-build

# Install Qt6
sudo apt install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools

# Install OpenGL libraries (required for Qt)
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev

# Optional: Install development tools
sudo apt install -y clang-format clang-tidy gdb valgrind
```

### Step 3: Verify Installation

```bash
cmake --version      # Should show 3.22 or higher
qmake6 --version     # Should show Qt 6.x
g++ --version        # Should show gcc 11.x
```

### Step 4: Set Up X11 (to run GUI apps from WSL)

**Install an X Server on Windows:**
- Download and install [VcXsrv](https://sourceforge.net/projects/vcxsrv/) or [Xming](http://www.straightrunning.com/XmingNotes/)
- Launch XLaunch, select "Multiple windows", Display number 0

**Configure WSL to use it:**
```bash
# Add to ~/.bashrc
echo 'export DISPLAY=:0' >> ~/.bashrc
source ~/.bashrc
```

### Step 5: Build the Project

```bash
cd /mnt/c/Users/Pc/Desktop/QtFSM
cmake --preset dev
cmake --build build
./build/QtFSM
```

---

## Option 2: Native Windows Setup

### Step 1: Install Qt

1. **Download Qt Online Installer**
   - Go to https://www.qt.io/download-qt-installer
   - Click "Download the Qt Online Installer"
   - Run the installer

2. **During Installation, Select:**
   - Qt 6.5.3 (or latest 6.x)
   - Under Qt 6.5.3, check:
     - ✅ MSVC 2019 64-bit (or MinGW if you don't have Visual Studio)
     - ✅ Qt Creator
     - ✅ CMake
     - ✅ Ninja

3. **Installation Path:** Note where Qt is installed (e.g., `C:\Qt\6.5.3`)

### Step 2: Install CMake (if not installed with Qt)

**Option A: Using winget (Windows 11)**
```powershell
winget install Kitware.CMake
```

**Option B: Manual Download**
- Go to https://cmake.org/download/
- Download Windows x64 Installer
- Install with "Add CMake to PATH" option checked

### Step 3: Install Git

**Option A: Using winget**
```powershell
winget install Git.Git
```

**Option B: Manual Download**
- Go to https://git-scm.com/download/win
- Download and install

### Step 4: Install Visual Studio Build Tools (if using MSVC)

**Option A: Visual Studio Community (Full IDE)**
- Download from https://visualstudio.microsoft.com/
- Install "Desktop development with C++" workload

**Option B: Build Tools Only (Smaller)**
- Download Visual Studio Build Tools
- Select "C++ build tools" workload

**Option C: Use MinGW (Alternative to MSVC)**
- MinGW can be installed with Qt installer (step 1)
- Smaller footprint, easier setup

### Step 5: Verify Installation

Open a new PowerShell/Command Prompt:
```powershell
cmake --version      # Should show 3.16 or higher
git --version        # Should show git version
qmake --version      # Should show Qt 6.x
```

### Step 6: Set Up Environment (if Qt not in PATH)

If `qmake` is not found, add Qt to PATH:

```powershell
# Add to System PATH (replace with your Qt installation path)
$env:Path += ";C:\Qt\6.5.3\msvc2019_64\bin"
# Or for MinGW:
$env:Path += ";C:\Qt\6.5.3\mingw_64\bin"
```

### Step 7: Build the Project

**Option A: Using Qt Creator (Easiest)**
1. Launch Qt Creator
2. File → Open File or Project
3. Navigate to `C:\Users\Pc\Desktop\QtFSM\CMakeLists.txt`
4. Select your Qt kit (Desktop Qt 6.5.3 MSVC2019 64bit)
5. Click "Configure Project"
6. Click the green "Run" button (▶)

**Option B: Using Command Line**
```powershell
cd C:\Users\Pc\Desktop\QtFSM

# Configure
cmake --preset dev

# Build
cmake --build build --config Debug

# Run (you may need to add Qt DLLs to PATH first)
.\build\Debug\QtFSM.exe
```

---

## Quick Installation Check

Run the verification script to check what's installed:

### Windows:
```powershell
cd C:\Users\Pc\Desktop\QtFSM
.\scripts\setup_dev_env.ps1
```

### WSL2:
```bash
cd /mnt/c/Users/Pc/Desktop/QtFSM
chmod +x scripts/setup_dev_env.sh
./scripts/setup_dev_env.sh
```

---

## Troubleshooting

### Issue: CMake can't find Qt

**Solution:** Set Qt path explicitly:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.5.3/gcc_64 -B build
# Windows example: -DCMAKE_PREFIX_PATH=C:/Qt/6.5.3/msvc2019_64
```

### Issue: Qt Creator doesn't show any kits

**Solution:**
1. Tools → Options → Kits
2. Click "Auto-detect" to scan for Qt installations
3. Manually add Qt version if needed

### Issue: Missing OpenGL libraries (Linux/WSL)

**Solution:**
```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

### Issue: "VCRUNTIME140.dll missing" on Windows

**Solution:** Install Visual C++ Redistributable
- Download from https://aka.ms/vs/17/release/vc_redist.x64.exe

---

## What Gets Installed?

### Essential (Required)
| Tool | Purpose | Size |
|------|---------|------|
| Qt 6.5+ | GUI framework | ~2-3 GB |
| CMake | Build system | ~50 MB |
| C++ Compiler | Code compilation | 500 MB - 5 GB |
| Git | Version control | ~50 MB |

### Optional (Recommended)
| Tool | Purpose | Size |
|------|---------|------|
| Qt Creator | IDE | ~200 MB |
| Ninja | Fast builds | ~5 MB |
| clang-format | Code formatting | ~50 MB |
| clang-tidy | Static analysis | ~100 MB |

**Total Disk Space Needed:** 3-8 GB depending on choices

---

## Next Steps After Installation

Once everything is installed:

1. **Verify the build:**
   ```bash
   cmake --preset dev
   cmake --build build
   ```

2. **Run the application:**
   - WSL: `./build/QtFSM`
   - Windows: `.\build\Debug\QtFSM.exe`

3. **You should see:**
   - Main window with menus (File, Edit, View, Tools, Help)
   - Central diagram editor (empty gray canvas)
   - Properties panel on the right

4. **Ready to continue!** We'll implement the diagram editor next.

---

## Need Help?

If you run into issues:
1. Check the error message
2. Verify installations with the verification script
3. Check the [dependencies.md](docs/dependencies.md) for detailed version requirements
4. Let me know and we'll solve it together! 🚀
