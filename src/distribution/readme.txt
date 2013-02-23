Disclaimer: http://sam.zoy.org/wtfpl/COPYING

NOTE: This is the fourth update of this utility.
Still TODO: Add version information!

This package contains three AxeFx II utilities for Windows and Mac.

axebackup:
    This is a backup utility for AxeFx II (tested with firmware 9).
    If you run this without any arguments, a backup will
    be made of banks A, B and C in addition to backing up
    global/system data.  By default all banks are backed up but if you
    only want to back up bank A, run "axebackup -a", only Bank B "axebackup -b",
    Banks B and C "axebackup -b -c" or system data "axebackup -s".

    What makes axebackup different from regular MIDI programs is that
    it recognizes the AxeFx bank dump format, filters away data that does
    not belong to the backup (tempo messages) and verifies the correctness
    of the data as the backup is being created.  If errors are detected,
    you'll know about it straight away instead of when you actually need
    the backup.

axeloader:
    Use it to send preset, backup or IR files to the AxeFx.
    When sending only a single preset, the utility will send the preset
    to the current edit buffer, overriding the embedded preset id, so that
    no presets are ever overwritten.  This is to allow the user to pick a place
    for storing the preset or to preview a preset without saving.  I.e. it's
    great for previewing and adding AxeExchange presets to your AxeFx.

    If you set axeloader up as the default handler for .syx files, it will send
    the data directly to the AxeFx when you double click the .syx file.
    Alternatively, you can drag and drop .syx files onto axeloader to send it
    to the AxeFx.

afx2lg:
    A simple utility to update and generate setup files for
    the Little Giant from Gordius (http://gordius.be).
    You can run the utility without any arguments and it will prompt
    you for input.  If you do prefer to use arguments run "afx2lg -?"
    for details.

    For more, see 'example_setup.lgp' below.

example_setup.lgp:

    This is an example setup file (actually it's what I use in
    conjunction with afx2lg) made with Control Center.
    Open this file up in Control Center, take a look at the patches,
    banks etc just to understand what this file contains.
    There are a few useful things in there such as programming for
    scenes, the looper and support for tap tempo + tuner on the same
    switch. The actual patches though, are just placeholders.  This
    is what afx2lg will read and use to generate new banks.
    This is of course just an example, so feel free to use your own
    setup file.  Also, a generated setup file can be used as input
    to generate another one if you want (you don't always have to work
    with a template setup file).

    In short this is how afx2lg works:
    <axe fx presets> + <LG setup> + afx2lg = <new LG setup>

    So to use, do the following:

    1. Run: axebackup.exe
       This will create .syx backup files of your setup.

    2. Open example_setup.lgp in Control Center and export it
       to a text file via 'File->Export to...->Text...'.

    3. Run: afx2lg.exe
       You will be prompted for the location of a .syx file (step 1)
       and the location of the exported setup file (step 2).
       Additionally you'll need to provide a name for the output file.
       The output will look something like this:

        I need a path to a LittleGiant exported text file: mysetup.txt
        I need a path to a preset file or bank (.syx): BankA.syx
        Enter a preset range or * for all:*
        Enter an output file name:my_new_setup.txt

    4. Open up Control Center again and import the output file (my_new_setup.txt)
       via 'File->Import from...->Text...'.

    5. Voila, you have your new setup file with proper preset names from the AxeFx
       that you can send to your LG.

If you encounter problems, please file bugs here:
http://code.google.com/p/afx2lg/issues/list

Future improvements:

* I plan on adding support for block states to afx2lg, but with fw9 this
  became a tad more complicated due to scenes.  On the bright side, scenes
  make this feature less necessary to have (for me at least) this feature so
  I've been punting. (I'm doing this in my 'free' time, so it's not always easy
  to justify to the family :))
* Provide a way to generate better bank names.
* I've been resisting writing a proper UI mostly because the code is mostly
  cross platform (except for the midi code in the backup utility) and I'd like
  to make the tools available on mac and linux (maybe others?) in the future.
  I've also thought about simply doing the UI via a simple local web server
  and do the UI in a browser and compile the utilities as NACL binaries once
  Chrome gets that ready (and the HTML5 MIDI spec gets implemented).

Enjoy and let me know how it goes! :)
Tommi - tomasgunn@gmail.com
