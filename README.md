# robot_design

## Prerequisites

[CMake](https://cmake.org/download/) >= 3.8
* Ubuntu: The version available through `apt-get` is probably outdated, install from the link above

[LibTorch](https://pytorch.org/get-started/locally/) (select your OS, Package: LibTorch, CUDA: None, cxx11 ABI)
* Ubuntu: Extract the `.zip` file and move the `libtorch` directory to `/usr/local` (suggested location)
* Add `export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/usr/local/libtorch/share/cmake` to `~/.bashrc`
* `source ~/.bashrc`

GLEW
* Ubuntu: `sudo apt-get install libglew-dev`
* Mac OS (Homebrew): `brew install glew`

Python 3 + headers
* Ubuntu: `sudo apt-get install python3 python3-dev`
* Mac OS (Homebrew): `brew install python3`

## Building (Linux, Mac OS)

`git clone https://github.com/allanzhao/robot_design.git`

`cd robot_design`

`git submodule update --init`

`mkdir build; cd build`

`cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPYTHON_EXECUTABLE=/path/to/python3 ..` (replace `/path/to/python3` as appropriate)

`make -j4` (replace 4 with the number of CPU cores available)

## Running Examples

### C++ Examples

Make sure you are in the `build` directory created earlier.

View an example robot design:
`examples/viewer/Viewer ../data/designs/insect.dot 0 1 1 1 2 2 2 3 3 3 5 5 5 4 4 4 -r`

Optimize and view a trajectory for the example robot:
`examples/viewer/Viewer ../data/designs/insect.dot 0 1 1 1 2 2 2 3 3 3 5 5 5 4 4 4 -o -r`

View a rule from the grammar:
`examples/rule_viewer/RuleViewer ../data/designs/insect.dot 0 rhs -r` (views the right-hand side of rule 0)

### Python Examples

Add the `build/examples/python_bindings` directory to your `PYTHONPATH` environment variable, and make sure you are in the `robot_design` directory.

Optimize and view a trajectory for the example robot:
`python3 examples/python_bindings/traj_opt.py`
