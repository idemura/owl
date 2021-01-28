#! /usr/bin/env python3

import argparse
import glob
import json
import os
import sys


NINJA_FILE = "ninja_build"
BUILD_JSON = "BUILD.json"
MAIN_TEST = "main_test.c"
MAIN = "main.c"


cmdline_args = None


class DepCycleError(Exception):
  def __init__(self, name):
    self.name = name


class NoTargetError(Exception):
  def __init__(self, name):
    self.name = name


def obj_name(c_file):
  return os.path.splitext(c_file)[0] + ".o"


class Package:
  def __init__(self, name, src_dir, props):
    self.name = name
    self.src_dir = src_dir
    self.path = os.path.join(src_dir, name)
    self.props = props
    self.MAIN_TEST = os.path.isfile(os.path.join(self.path, MAIN_TEST))
    self.main = os.path.isfile(os.path.join(self.path, MAIN))
    self.deps = props.get('deps', [])
    self.transitive_deps = []
    n = len(os.path.join(src_dir, "_")) - 1
    self.c_files = [f[n:] for f in glob.glob(os.path.join(self.path, "*.c"))]
    self.c_files.sort()


  # Return (main, tests)
  def split_files(self):
    main = []
    test = []
    for f in self.c_files:
      if f.endswith("_test.c") or f == MAIN_TEST:
        test.append(f)
      else:
        main.append(f)

    return (main, test)


  def __repr__(self):
    return "Package({} deps={} {})".format(self.name, self.deps, self.c_files)


class NinjaFile:
    def __init__(self, src_dir):
      self.src_dir = src_dir
      name = os.path.join(self.src_dir, NINJA_FILE)
      if cmdline_args.debug:
        print("Output: {}".format(name))
      self.fo = open(name, 'w')


    def close(self):
      self.fo.close()


    def begin(self, pkg):
      self.fo.write("# package {}\n".format(pkg))


    def build_cc(self, c_file):
      self.fo.write("build {}: cc {}\n".format(obj_name(c_file), c_file))


    def build_ar(self, pkg, c_files):
      objects = [obj_name(f) for f in c_files]
      self.fo.write("build {0}/lib{0}.a: ar {1}\n".format(pkg, ' '.join(objects)))


    def build_ld(self, pkg, is_test, c_files, deps, libs):
      deps.reverse()
      expand_deps = ["{0}/lib{0}.a".format(l) for l in deps]
      bin_name = pkg + "_test" if is_test else pkg
      self.fo.write("build {}/{}: ld {} {}\n".format(
          pkg,
          bin_name,
          ' '.join([obj_name(f) for f in c_files]),
          ' '.join(expand_deps)))
      if len(libs) > 0:
        self.fo.write("  libs = {}\n".format(' '.join(["-l" + l for l in libs])))


    def end(self, pkg):
      self.fo.write("\n")
      pass


def tab(n):
  return "  " * n


# Walks dependency tree and creates transitive dependency lists
def dfs_resolve(packages, discovered, visited, p, depth):
  debug = cmdline_args.debug
  if debug:
    print("{}> {}".format(tab(depth), p.name))

  if p.name in visited:
    if debug:
      print("{}< {} (visited)".format(tab(depth), p.name))
    return

  if p.name in discovered:
    raise DepCycleError(p.name)

  discovered[p.name] = True

  trans = []
  for d in p.deps:
    if d not in packages:
      raise NoTargetError(d)
    trans += dfs_resolve(packages, discovered, visited, packages[d], depth + 1)

  p.transitive_deps = trans + [p.name]
  if debug:
    print("{}{}: {}".format(tab(depth), p.name, p.transitive_deps))

  del discovered[p.name]
  visited[p.name] = True

  if debug:
    print("{}< {}".format(tab(depth), p.name))

  return p.transitive_deps


def resolve_deps(packages):
  visited = {}
  discovered = {}
  for k in packages:
    dfs_resolve(packages, discovered, visited, packages[k], 0)


# Write packge to Ninja file
def write_package(p, nf):
  nf.begin(p.name)

  for c in p.c_files:
    nf.build_cc(c)
  main, test = p.split_files()
  libs = p.props.get("libs", [])
  if p.main:
    nf.build_ld(p.name, False, main, p.transitive_deps, libs)
  else:
    nf.build_ar(p.name, main)

  if p.MAIN_TEST:
    nf.build_ld(p.name, True, test, ["testing"] + p.transitive_deps, libs)

  nf.end(p.name)


# Generates Ninja file
def generate(src_dir):
  packages = {}
  for name in os.listdir(src_dir):
    dirpath = os.path.join(src_dir, name)
    if os.path.isdir(dirpath):
      build_json = os.path.join(dirpath, BUILD_JSON)
      if os.path.exists(build_json):
        with open(build_json) as f:
          props = json.load(f)
      else:
        props = {}
      packages[name] = Package(name, src_dir, props)

  # Resolve and generate transitive dependencies
  resolve_deps(packages)

  # Write ninja targets
  nf = NinjaFile(src_dir)
  for k in packages:
    write_package(packages[k], nf)
  nf.close()


if __name__ == "__main__":
  global args

  parser = argparse.ArgumentParser(description='Generate Ninja file')
  parser.add_argument(
      '-d', '--debug',
      action='store_true',
      help='Debug logs')

  cmdline_args = parser.parse_args()

  base_dir = os.path.dirname(os.path.realpath(__file__))
  try:
    generate(os.path.join(base_dir, "src"))
  except NoTargetError as e:
    print("Target not found: {}".format(e.name))
    sys.exit(1)
  except DepCycleError as e:
    print("Dependency cycle: {}".format(e.name))
    sys.exit(1)

  sys.exit(0)
