param(
    [string]$Compiler,
    [string]$TargetArch,
    [string]$VsArch,
    [string]$VsHostArch
)

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
Write-Host "+ '$vswhere' -latest -products '*' -property installationPath"
$vsRoot = & $vswhere -latest -products '*' -property installationPath
if (-not $vsRoot) { throw 'Could not find Visual Studio' }

& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" `
    -Arch $VsArch -HostArch $VsHostArch -SkipAutomaticLocation -NoLogo

Write-Output '::group::Compiler version'
Write-Host '+ cl'
cl
Write-Host '+ clang-cl --version'
clang-cl --version
Write-Output '::endgroup::'

Write-Output '::group::Configure'
$cmakeArgs = @('-S', '.', '-B', 'out', '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=RelWithDebInfo')
if ($Compiler -eq 'msvc') {
    $cmakeArgs += '-DCMAKE_C_COMPILER=cl'
} else {
    $cmakeArgs += '-DCMAKE_C_COMPILER=clang-cl'
    $cmakeArgs += "-DCMAKE_C_COMPILER_TARGET=${TargetArch}-pc-windows-msvc"
}
Write-Host "+ cmake $($cmakeArgs -join ' ')"
cmake @cmakeArgs
Write-Output '::endgroup::'

Write-Output '::group::Build'
Write-Host '+ cmake --build out --verbose'
cmake --build out --verbose
Write-Output '::endgroup::'

Write-Output '::group::Test'
Write-Host '+ cmake --build out --target regressions --verbose'
cmake --build out --target regressions --verbose
Write-Host '+ ctest --test-dir out -V'
ctest --test-dir out -V
Write-Output '::endgroup::'
