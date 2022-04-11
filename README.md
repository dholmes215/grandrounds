# grandrounds

[![ci](https://github.com/dholmes215/grandrounds/actions/workflows/ci.yml/badge.svg)](https://github.com/dholmes215/grandrounds/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/dholmes215/grandrounds/branch/main/graph/badge.svg)](https://codecov.io/gh/dholmes215/grandrounds)
[![CodeQL](https://github.com/dholmes215/grandrounds/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/dholmes215/grandrounds/actions/workflows/codeql-analysis.yml)

## About

This is a nonogram tour of Minneapolis' [Grand Rounds Scenic Byway](https://en.wikipedia.org/wiki/Grand_Rounds_National_Scenic_Byway), a submission for [Jason's C++ Best Practices Game Jam](https://github.com/cpp-best-practices/game_jam/blob/main/README.md).  At least, that's what it's supposed to be, except time constraints mean it shows very little of the Grand Rounds at the moment.

## How To Play

To learn how to play nonograms in general, see: [https://www.youtube.com/watch?v=zisu0Qf4TAI&list=PLH_elo2OIwaAYMF8CAfDnlKcVyyB5UITk]

To play this implementation of the game specifically: click with the left mouse button to fill a cell (color it black).  Click with the right button to clear a cell (color it white).  Click with the middle button to "mark" a cell.  Keyboard controls are presently not supported.

## Requirements

The puzzles are not included in the binary release packages, so if you don't build from source, they must be downloaded from the source release, or from the git repository itself.  The "puzzles" directory must be in either the working directory or some ancestor of the working directory.

You'll need a terminal with mouse support, ideally 24-bit color support, and at least enough font support to display a "▄" character.  I've only tested on Windows, with Windows Terminal, which should work out of the box.

## Bugs

The terminal scrolls a line occasionally, and when it does, the mouse no longer selects the correct line until you scroll it back.  I haven't looked into why.

## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
