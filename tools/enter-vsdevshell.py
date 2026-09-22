#!/usr/bin/env python3
import argparse
import os
import re
import subprocess
import sys


def resolve_vs_install_root() -> str:
    """Return the latest Visual Studio installation root."""
    vswhere = os.path.join(
        os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
        r"Microsoft Visual Studio\Installer\vswhere.exe",
    )
    cmd = [vswhere, "-latest", "-products", "*", "-property", "installationPath"]

    print(f"+ {subprocess.list2cmdline(cmd)}", file=sys.stderr)
    result = subprocess.run(cmd, check=True, capture_output=True, text=True)
    if not (vs_install_root := result.stdout.strip()):
        raise RuntimeError("vswhere could not find a Visual Studio instance")
    return vs_install_root


def resolve_vc_toolset_version(vs_install_root: str, toolset_name: str | None) -> str:
    """Resolve a VC toolset name to the full version number."""
    vc_toolset_basename = (
        "Microsoft.VCToolsVersion.default.txt"
        if toolset_name is None
        else f"Microsoft.VCToolsVersion.{toolset_name}.default.txt"
    )
    vc_toolset_path = rf"{vs_install_root}\VC\Auxiliary\Build\{vc_toolset_basename}"

    try:
        with open(vc_toolset_path) as f:
            return f.read().strip()
    except FileNotFoundError as e:
        raise RuntimeError(f"Failed to resolve VC toolset version") from e


def run_vcvarsall(
    vs_install_root: str,
    vc_toolset_version: str,
    arch: str,
) -> dict[str, str]:
    """Invoke vcvarsall and return the environment variables it created."""
    vcvarsall = rf"{vs_install_root}\VC\Auxiliary\Build\vcvarsall.bat"

    cmd = f'"{vcvarsall}" {arch} -vcvars_ver={vc_toolset_version} 1>&2 & set'
    print(f"+ {cmd}", file=sys.stderr)

    result = subprocess.run(
        cmd,
        shell=True,
        env={**os.environ, "VSCMD_SKIP_SENDTELEMETRY": "1"},
        stdout=subprocess.PIPE,
        text=True,
    )

    env = dict(
        line.split("=", 1)
        for line in result.stdout.splitlines()
        if "=" in line and not line.startswith("=")
    )

    # Validate vcvarsall actually left us with a useable cl.exe.
    try:
        subprocess.run("cl.exe", shell=True, env=env, check=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("vcvarsall.bat failed to provide a useable cl.exe") from e

    return env


def main() -> int:
    """Configure and print a Visual Studio developer environment."""

    def validate_toolset_name(v: str) -> str:
        if not re.fullmatch(r"v\w+", v):
            raise argparse.ArgumentTypeError("must be a VC toolset name such as 'v145'")
        return v

    def validate_arch(v: str) -> str:
        if not re.fullmatch(r"\w+", v):
            raise argparse.ArgumentTypeError("invalid architecture")
        return v

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--toolset",
        type=validate_toolset_name,
        help="a VC toolset name, such as v143",
    )
    parser.add_argument(
        "--full-env",
        action=argparse.BooleanOptionalAction,
        help="dump the full environment instead of only changes",
    )
    parser.add_argument(
        "arch",
        type=validate_arch,
        help="vcvarsall architecture name, such as x64 or x64_x86",
    )

    args = parser.parse_args()

    vs_install_root = resolve_vs_install_root()
    vc_toolset_version = resolve_vc_toolset_version(vs_install_root, args.toolset)

    env = run_vcvarsall(vs_install_root, vc_toolset_version, args.arch)

    for k, v in env.items():
        if args.full_env or os.environ.get(k) != v:
            print(f"{k}={v}")

    return 0


if __name__ == "__main__":
    exit(main())
