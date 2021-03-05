#! /usr/bin/env python3

import argparse
import glob
import json
import os
import sys


NINJA_FILE = "ninja_build"
BUILD_JSON = "BUILD.json"
MAIN_TEST = "main_test.cpp"
MAIN = "main.c"


cmdline_args = None


class DepCycleError(Exception):
    def __init__(self, name):
        self.name = name


class NoTargetError(Exception):
    def __init__(self, name):
        self.name = name


def obj_name(src_file):
    return os.path.splitext(src_file)[0] + ".o"


def lib_name(pkg_name):
    return "{0}/lib{0}.a".format(pkg_name)


class Package:
    def __init__(self, name, src_dir, props):
        self.name = name
        self.src_dir = src_dir
        self.path = os.path.join(src_dir, name)
        self.props = props
        self.is_test = os.path.isfile(os.path.join(self.path, MAIN_TEST))
        self.is_main = (
                os.path.isfile(os.path.join(self.path, "main.c")) or
                os.path.isfile(os.path.join(self.path, "main.cpp")))
        self.deps = props.get('deps', [])
        self.transitive_deps = []

        # Populate sources, split them in main and test parts.
        path_last = len(os.path.join(src_dir, "_")) - 1
        self.src_files = []
        self.src_files += [f[path_last:] for f in glob.glob(os.path.join(self.path, "*.c"))]
        self.src_files += [f[path_last:] for f in glob.glob(os.path.join(self.path, "*.cpp"))]
        self.src_files.sort()

        self.src_main = []
        self.src_test = []
        for f in self.src_files:
            if f.find("_test.c") >= 0:
                self.src_test.append(f)
            else:
                self.src_main.append(f)


    def __repr__(self):
        return "Package({} deps={} {}/{})".format(
                self.name, self.deps, self.src_main, self.src_test)


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


        def compile(self, src_file):
            rule = "compile_c" if src_file.endswith(".c") else "compile_cpp"
            self.fo.write("build {}: {} {}\n".format(obj_name(src_file), rule, src_file))


        def build_ar(self, pkg, src_files):
            objs = [obj_name(f) for f in src_files]
            self.fo.write("build {}: ar {}\n".format(lib_name(pkg), ' '.join(objs)))


        def build_ld(self, pkg, is_test, src_files, deps, libs):
            deps.reverse()
            expand_deps = [lib_name(l) for l in deps]
            bin_name = pkg + "_test" if is_test else pkg
            self.fo.write("build {}/{}: ld {} {}\n".format(
                    pkg,
                    bin_name,
                    ' '.join([obj_name(f) for f in src_files]),
                    ' '.join(expand_deps)))
            if len(libs) > 0:
                self.fo.write("    libs = {}\n".format(
                        ' '.join(["-l" + l for l in libs])))


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
            print("{}< {} (visited): {}".format(tab(depth), p.name, p.transitive_deps))
        return p.transitive_deps + [p.name]

    if p.name in discovered:
        raise DepCycleError(p.name)

    discovered[p.name] = True

    if debug:
        print("{}deps: {}".format(tab(depth), p.deps))

    trans = []
    for d in p.deps:
        if d not in packages:
            raise NoTargetError(d)
        trans += dfs_resolve(packages, discovered, visited, packages[d], depth + 1)

    p.transitive_deps = trans
    if debug:
        print("{}{}: {}".format(tab(depth), p.name, p.transitive_deps))

    del discovered[p.name]
    visited[p.name] = True

    if debug:
        print("{}< {}".format(tab(depth), p.name))

    return p.transitive_deps + [p.name]


def resolve_deps(packages):
    visited = {}
    discovered = {}
    for k in packages:
        dfs_resolve(packages, discovered, visited, packages[k], 0)


# Write packge to Ninja file
def write_package(p, nf):
    nf.begin(p.name)

    for c in p.src_files:
        nf.compile(c)

    libs = p.props.get("libs", [])
    if p.is_main:
        nf.build_ld(p.name, False, p.src_main, p.transitive_deps, libs)
    else:
        nf.build_ar(p.name, p.src_main)

    if p.is_test:
        nf.build_ld(p.name, True, p.src_test, [p.name] + p.transitive_deps, ["gtest"] + libs)

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
