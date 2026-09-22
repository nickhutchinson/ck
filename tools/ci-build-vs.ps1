#requires -Version 7.4

param(
    [string]$Compiler,
    [string]$Arch
)

Set-StrictMode -Version Latest

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true

$env:VSCMD_SKIP_SENDTELEMETRY = '1'

if ($env:VSCMD_VER) {
    Write-Warning "Visual Studio developer shell already active"
}

function Run([string]$Executable) {
    Write-Host ('+ ' + ((@($Executable) + $args) -join ' '))
    & $Executable @args
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = Run $vswhere -latest -products '*' -property installationPath
if (-not $vsRoot) { throw 'Could not find Visual Studio' }

$vsArch = switch ($Arch) {
    'x86_64' { 'x64' }
    'i686' { 'x86' }
    'aarch64' { 'arm64' }
    default { throw "Unhandled architecture: $Arch" }
}

# Set up VS environment variables.
& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" `
    -Arch $vsArch `
    -HostArch $env:PROCESSOR_ARCHITECTURE.ToLower() `
    -SkipAutomaticLocation `
    -NoLogo

Write-Output '::group::Compiler version'
Run cl
Run clang-cl --version
Write-Output '::endgroup::'

Write-Output '::group::Configure'
$cmakeArgs = @('-S', '.', '-B', 'out', '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=RelWithDebInfo')
if ($Compiler -eq 'msvc') {
    $cmakeArgs += '-DCMAKE_C_COMPILER=cl'
} else {
    $cmakeArgs += '-DCMAKE_C_COMPILER=clang-cl'
    $cmakeArgs += "-DCMAKE_C_COMPILER_TARGET=${Arch}-pc-windows-msvc"
}
Run cmake @cmakeArgs
Write-Output '::endgroup::'

Write-Output '::group::Build'
Run cmake --build out --verbose
Write-Output '::endgroup::'

Write-Output '::group::Test'
Run cmake --build out --target regressions --verbose
Run ctest --test-dir out -V
Write-Output '::endgroup::'
