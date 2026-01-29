Write-Host "Building QtFSM..." -ForegroundColor Cyan

# Ensure build directory exists
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
}

# Run build in WSL
# We leverage wslpath to correctly map the current Windows path to WSL
wsl bash -c "cd `"`$(wslpath '$PWD')`" && cd build && cmake .. && make -j`$(nproc)"

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild SUCCESSFUL!" -ForegroundColor Green
    Write-Host "Binary is located at: build/QtFSM" -ForegroundColor Gray
} else {
    Write-Host "`nBuild FAILED!" -ForegroundColor Red
    exit $LASTEXITCODE
}
