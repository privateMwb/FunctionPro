#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import sys

parser = argparse.ArgumentParser()

parser.add_argument("--package", required=True)
parser.add_argument("--repo", required=True)
parser.add_argument("--version", required=True)
parser.add_argument("--archive", required=True)
parser.add_argument("--root-dir", required=True)
parser.add_argument("--conan-dir", required=True)
parser.add_argument("--vcpkg-dir", required=True)

args = parser.parse_args()

package = args.package
repo = args.repo
version = args.version

archive = pathlib.Path(args.archive)

if not archive.exists():
    sys.exit(f"Archive not found: {archive}")

data = archive.read_bytes()

sha256 = hashlib.sha256(data).hexdigest()
sha512 = hashlib.sha512(data).hexdigest()

print(f"Version : {version}")
print(f"SHA256  : {sha256}")
print(f"SHA512  : {sha512}")

# ---------------------------------------------------------------------
# Root CMakeLists.txt
# ---------------------------------------------------------------------

root_cmakelists = pathlib.Path(args.root_dir) / "CMakeLists.txt"

text = root_cmakelists.read_text(encoding="utf-8")


def _bump_project_version(match: re.Match) -> str:
    # Substitute only within the matched project(...) call, so this
    # can't touch an unrelated VERSION elsewhere in the file (e.g.
    # cmake_minimum_required(VERSION ...)).
    return re.sub(
        r"VERSION\s+[0-9A-Za-z.\-_]+",
        f"VERSION {version}",
        match.group(0),
        count=1,
    )


new_text, count = re.subn(
    r"project\s*\([^)]*\)",
    _bump_project_version,
    text,
    count=1,
    flags=re.DOTALL,
)

if count == 0:
    sys.exit(f"No project() call found in {root_cmakelists}")

root_cmakelists.write_text(new_text, encoding="utf-8")

print("✓ Updated CMakeLists.txt")

# ---------------------------------------------------------------------
# Conan
# ---------------------------------------------------------------------

recipe_dir = pathlib.Path(args.conan_dir)

# conanfile.py

conanfile = recipe_dir / "conanfile.py"

text = conanfile.read_text(encoding="utf-8")

text = re.sub(
    r'version\s*=\s*"[^"]+"',
    f'version = "{version}"',
    text,
)

conanfile.write_text(text, encoding="utf-8")

print("✓ Updated conanfile.py")

# conandata.yml

conandata = recipe_dir / "conandata.yml"

conandata.write_text(
f'''sources:
  "{version}":
    url: "https://github.com/{repo}/archive/refs/tags/v{version}.tar.gz"
    sha256: "{sha256}"
''',
encoding="utf-8"
)

print("✓ Updated conandata.yml")

# ---------------------------------------------------------------------
# vcpkg
# ---------------------------------------------------------------------

port_dir = pathlib.Path(args.vcpkg_dir)

# portfile.cmake

portfile = port_dir / "portfile.cmake"

text = portfile.read_text(encoding="utf-8")

text = re.sub(
    r"REF\s+v[0-9A-Za-z.\-_]+",
    f"REF v{version}",
    text,
)

text = re.sub(
    r"SHA512\s+[0-9a-fA-F]+",
    f"SHA512 {sha512}",
    text,
)

portfile.write_text(text, encoding="utf-8")

print("✓ Updated portfile.cmake")

# vcpkg.json

vcpkg_json = port_dir / "vcpkg.json"

data = json.loads(vcpkg_json.read_text(encoding="utf-8"))

data["version"] = version

vcpkg_json.write_text(
    json.dumps(data, indent=2) + "\n",
    encoding="utf-8",
)

print("✓ Updated vcpkg.json")