#requires -Version 7.4

param(
    [string]$Compiler,
    [string]$TargetArch,
    [string]$VsArch
)

Set-StrictMode -Version Latest

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true
$env:VSCMD_SKIP_SENDTELEMETRY = '1'

if ($env:VSCMD_VER) {
    Write-Warning "Visual Studio developer shell already active (VSCMD_VER=$env:VSCMD_VER); reinitializing"
}

# Keep native argument syntax intact (notably vswhere's literal '*').
function Run([string]$Display, [scriptblock]$Command) {
    Write-Host "+ $Display"
    & $Command
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = Run "'$vswhere' -latest -products '*' -property installationPath" {
    & $vswhere -latest -products '*' -property installationPath
}
if (-not $vsRoot) { throw 'Could not find Visual Studio' }

# OSArchitecture reflects the host even if PowerShell runs under emulation.
$hostArch = switch ([System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture) {
    'X64' { 'amd64' }
    'Arm64' { 'arm64' }
    default { throw 'Unsupported Visual Studio host architecture' }
}

& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" `
    -Arch $VsArch -HostArch $hostArch -SkipAutomaticLocation -NoLogo

Write-Output '::group::Compiler version'
Run 'cl' { cl }
Run 'clang-cl --version' { clang-cl --version }
Write-Output '::endgroup::'

Write-Output '::group::Configure'
$cmakeArgs = @('-S', '.', '-B', 'out', '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=RelWithDebInfo')
if ($Compiler -eq 'msvc') {
    $cmakeArgs += '-DCMAKE_C_COMPILER=cl'
} else {
    $cmakeArgs += '-DCMAKE_C_COMPILER=clang-cl'
    $cmakeArgs += "-DCMAKE_C_COMPILER_TARGET=${TargetArch}-pc-windows-msvc"
}
Run "cmake $($cmakeArgs -join ' ')" { cmake @cmakeArgs }
Write-Output '::endgroup::'

Write-Output '::group::Build'
Run 'cmake --build out --verbose' { cmake --build out --verbose }
Write-Output '::endgroup::'

Write-Output '::group::Test'
Run 'cmake --build out --target regressions --verbose' { cmake --build out --target regressions --verbose }
Run 'ctest --test-dir out -V' { ctest --test-dir out -V }
Write-Output '::endgroup::'
