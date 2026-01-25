# The Ludwig Editor

```text
{**********************************************************************}
{                                                                      }
{            L      U   U   DDDD   W      W  IIIII   GGGG              }
{            L      U   U   D   D   W    W     I    G                  }
{            L      U   U   D   D   W ww W     I    G   GG             }
{            L      U   U   D   D    W  W      I    G    G             }
{            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              }
{                                                                      }
{**********************************************************************}
```

## About

Ludwig is a text editor developed at the University of Adelaide.
It is an interactive, screen-oriented text editor.
It may be used to create and modify computer programs, documents
or any other text which consists only of printable characters.

Ludwig may also be used on hardcopy terminals or non-interactively,
but it is primarily an interactive screen editor.

This is a C++ port of the Ludwig code. The original Pascal code is
available here: [cjbarter/ludwig](https://github.com/cjbarter/ludwig).

## Building

Either clang++ or g++ can build Ludwig-C.

```sh
mkdir build
cd build
cmake ..
make
```

Despite using CMake, I think it is highly unlikely that this will work on
Microsoft Windows, although you may have some luck using WSL.

This will produce `ludwig` which can be copied to your preferred directory for
local binaries, eg `/usr/local/bin`.

Note that two help files are also built, `ludwighlp.idx` and `ludwignewhlp.idx`
for the old and new command sets respectively.  Ludwig is hardcoded to find
these files in `/usr/local/help`, or alternatively in a location pointed to by
the environment variables `LUD_HELPFILE` and `LUD_NEWHELPFILE`.

## Coverage

Unit test coverage is very low right now.  This is being worked on as
refactoring and modernisation continues.

```sh
mkdir build-coverage
cd build-coverage
cmake -DENABLE_COVERAGE=ON ..
make
make coverage
open coverage/index.html
```

## System Tests

There is reasonable system test coverage.  The system tests leverage 
Ludwig's batch mode, where a command string is provided on stdin.  The
general approach is:

- The test provides a selection of initial filenames and contents, together with expected output files and contents and a command string
- The test framework creates a temporary directory and populates it with the supplied files
- The command string is piped into a Ludwig process running in the temporary directory
- Once the process completes, the files in the temporary directory are collected and compared against expectations

You can clone the system tests [here](https://github.com/clstrfsck/ludwig-system-test)
using:

```sh
git clone https://github.com/clstrfsck/ludwig-system-test system-test
# Assuming you have python and pytest installed
./system-test/run-system-tests.sh
```

You should see a bunch of dots, followed by something like:

```text
326 passed, 3 skipped in 1.90s
```

Two of the three skipped tests are cases where regular expression patterns
don't match candidate strings in the way I think they should.  The third is
a window related command that is not implemented nor appropriate for batch
mode.

I have checked that the system tests run as expected on both the original
implementation as well as this port.

## Usage

Open/create a file with name `file-name`:

```sh
ludwig file-name
```

The file `.ludwigrc` in your home directory will be loaded whenever you start
ludwig.

Or with some additional initialisation parameters:

```sh
ludwig -O -i initialisation-file-name file-name
```

- `-O` invokes Version 5 command names
- `-i` initialisation file (optional) executed after .ludwigrc

## Help

There are two help files

- old commands help files: `ludwighlp.idx`
- new commands help files: `ludwignewhlp.idx`

Copy these into `/usr/local/help`

```sh
mkdir -p /usr/local/help
cp *.idx /usr/local/help
```

A couple of useful commands (`-O` version) to get you started are:

```ludwig
km/home/<ac/
km/end/>eol [<ac] >ac/
```

This will make your home key move the cursor to the start of the line, and
the end key move the cursor to the end of the current line.

They can be put into an initialisation file or `.ludwigrc`.

Ludwig command `\h` will give you the help pages on Ludwig
commands and `\q` will exit the editing session.
