

# Introduction #

The afx2lg project is a set of tools for the AxeFx II from Fractal Audio.  It's free (as in beer), open source and a completely use-at-your-own-risk affair.

All the utilities are currently command line/terminal utilities which means no fancy UI and some limitations on Mac (associating terminal programs with file extensions requires jumping through hoops).  The plan is to release a utility one of these days that has some sort of UI.

# Details #

The downloadable [package](https://code.google.com/p/afx2lg/downloads/list) contains the following AxeFx II utilities that run on Windows (XP and up) and Mac OS X Lion (10.7) and up.

## axebackup ##

This is a backup utility for AxeFx II (tested with firmware 9.x).

If you run this without any arguments, a backup will be made of banks A, B and C in addition to backing up global/system data.  By default all banks are backed up but if you only want to back up bank A, run "axebackup -a", only Bank B "axebackup -b", Banks B and C "axebackup -b -c" or system data "axebackup -s".

What makes axebackup different from regular MIDI programs is that it recognizes the AxeFx bank dump format, filters away data that does not belong to the backup (tempo messages) and verifies the correctness of the data as the backup is being created.  If errors are detected, you'll know about it straight away instead of when you actually need the backup.

If you're interested in the actual data, you can additionally run axebackup with a -j, which will create a JSON formatted text file that contains all the presets and their parameters in human readable (and javascript parseable) form.

## axeloader ##

**axeloader** is a tool to send presets, backups or IR files to the AxeFx II.  When applied to a single preset file, the utility will send the preset to the current edit buffer, overriding the embedded preset id, so that no presets are ever overwritten and that the preset doesn't go to some unexpected location on the AxeFx.  This is so that you, the user, can pick a place where you want to store the preset or if you just want to try it out without saving.  I.e. it's great for previewing and adding AxeExchange presets to your AxeFx.

If you set axeloader up as the default handler for .syx files, it will send the data directly to the AxeFx when you double click the .syx file.  Alternatively, you can drag and drop .syx files onto axeloader to send it to the AxeFx.

## afx2lg ##

This is a simple utility to update and generate setup files for the Little Giant from [Gordius](http://gordius.be).  You can run the utility without any arguments and it will prompt you for input.  If you do prefer to use arguments run "afx2lg -?" for details.

### example\_setup.lgp ###

This is an example setup file (actually it's what I use in conjunction with **afx2lg**) made with Control Center (the configuration app for the Little Giant).  Open this file up in Control Center, take a look at the patches, banks etc just to understand what this file contains.

There are a few useful things in there such as programming for scenes, the looper and support for tap tempo + tuner on the same switch. The actual patches though, are just placeholders.  This is what afx2lg will read and use to generate new banks.

This is just an example, so feel free to use your own setup file.  Also, a generated setup file can be used as input to generate another one if you want (you don't always have to work with a template setup file).

In short this is how afx2lg works:

> _<axe fx presets> + <LG setup> + **afx2lg** = <new LG setup>_

So to use, do the following:

  1. Run: **axebackup**.  This will create .syx backup files of your setup.
  1. Open example\_setup.lgp in Control Center and export it to a text file via 'File->Export to...->Text...'.
  1. Run: **afx2lg**.  You will be prompted for the location of a .syx file (step 1) and the location of the exported setup file (step 2).  Additionally you'll need to provide a name for the output file.
> > The output will look something like this:
> > > <pre>I need a path to a LittleGiant exported text file: mysetup.txt</pre>
> > > <pre>I need a path to a preset file or bank (.syx): BankA.syx</pre>
> > > <pre>Enter a preset range or * for all: *</pre>
> > > <pre>Enter an output file name: my_new_setup.txt</pre>
  1. Open up Control Center again and import the output file (my\_new\_setup.txt) via 'File->Import from...->Text...'.
  1. Voila, you have your new setup file with proper preset names from the AxeFx that you can send to your LG.

If you encounter problems, please file bugs [here](http://code.google.com/p/afx2lg/issues/list).