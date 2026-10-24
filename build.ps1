$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    & cmake -S . -B build-cmake
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
    & cmake --build build-cmake --config Release
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed' }
    & ctest --test-dir build-cmake -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
    Write-Output 'Built successfully. Run Slide.exe in build-cmake/Release (Visual Studio) or build-cmake (MinGW). Its assets folder is beside it.'
} finally {
    Pop-Location
}
