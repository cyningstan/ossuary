# Ossuary

Ossuary is a very simplistic Roguelike for DOS, designed to run on any PC with an 8088 or better processor, and CGA or compatible graphics. DOS binaries with packaged documentation are available on [Itch](https://cyningstan.itch.io/ossuary) and at the game's [home page](http://damian.cyningstan.org.uk/download/176/ossuary-for-dos).

The gameplay is extremely simplistic and was originally designed for a 16K ZX Spectrum computer. If you want a deep, detailed roguelike experience then this is probably not the game for you. If you are happy with a half-hour dungeon crawl involving just a handful of different monsters and weapons then this might be your cup of tea.

## Getting Started

If you just want to play the game, then I recommend downloading the binary package from one of the sources above. This repo is for those who want to examine the source code, and perhaps to modify it or fork their own version of the game.

The source files are developed for OpenWatcom C 2.0. Compilation has been tested both under Linux and DOS. It may work under Windows too, but if your system cannot compile the source natively then it should work under DOSBox.

The game uses a modified version of the CGALIB library, the original of which may or may not be available on GitHub by the time you read this (at the time of writing, it isn't). But since this is a modified version of the library, it is included with this repo.

## Building on Linux

To build for Linux, you need OpenWatcom 2.0 to be set up properly for compilation, with its environment variables and path set as per the compiler's documentation. You will also need DOSBox to be set up and accessible as it is called as part of the build process.

In case you're new to git or github, the first thing you need to do (after installing git and getting a github account) is to clone the repo:

```
$ git clone https://github.com/cyningstan/ossuary
$ cd ossuary
```

Then, assuming that you have your OpenWatcom set up for compilation under Linux, you can just run OpenWatcom's make utility:

```
$ wmake
```

The program that generates the game's asset file is a DOS program; you will briefly see DOSBox appear when that program is run. After successful building, there will be a second `ossuary` directory within the first one; this inner directory is the DOS distribution and can be zipped for distribution, moved to your DOSBox's C drive or transferred to whatever other environment you want to run the game on.

## Building for DOS

First you need to clone the repo using whatever facility your host operating system offers. For the sake of this guide I'll assume that you're using Linux and you're going to clone ossuary into /home/me/ossuary. That done, launch DOSBox. Choose a free drive letter; I'll assume D, and mount the repo into a DOS drive:

```
Z:\> mount d /home/me/ossuary
Z:\> d:
```

From here, run the wmake utility as per the Linux example:

```
D:\> wmake
```

The program that generates the assets briefly puts them on the screen, so you may see a graphical display pop up for a fraction of a second. Once the compilation is done, the `D:\OSSUARY` directory will contain the compiled, installed game for you to run or to transfer elsewhere.

## Licence

CGALIB is public domain. If you'd like to use it, I'd recommend grabbing the original distribution from [here](http://damian.cyningstan.org.uk/download/163/cgalib-a-graphics-library-for-cga-games) instead of working with the customised version used by Ossuary. I intend to give it its own GitHub repo eventually.

The rest of the game is released under the GPL. If you'd like to make a port, or use Ossuary's code in your own project, the LICENSE file tells you what you have to do (basically distribute your own modifications under the GPL too).
