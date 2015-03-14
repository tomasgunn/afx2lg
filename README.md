# afx2lg
Automatically exported from code.google.com/p/afx2lg

(copy paste of the old readme follows)

# Introduction
<p>The afx2lg project is a set of tools for the AxeFx II from Fractal Audio. It's free (as in beer), open source and a completely use-at-your-own-risk affair.</p>

<p>With the exception of Afx2Lg?, the utilities are command line/terminal utilities which means no fancy UI and some limitations on Mac (associating terminal programs with file extensions requires jumping through hoops).</p>

## Details
<p>The downloadable package contains the following AxeFx II utilities that run on Windows (XP and up) and Mac OS X Lion (10.7) and up.</p>

## axebackup
<p>This is a backup utility for AxeFx II (tested with firmware 7-10.x).</p>

<p>If you run this without any arguments, a backup will be made of banks A, B and C in addition to backing up global/system data. By default all banks are backed up but if you only want to back up bank A, run "axebackup -a", only Bank B "axebackup -b", Banks B and C "axebackup -b -c" or system data "axebackup -s".</p>

<p>What makes axebackup different from regular MIDI programs is that it recognizes the AxeFx bank dump format, filters away data that does not belong to the backup (tempo messages) and verifies the correctness of the data as the backup is being created. If errors are detected, you'll know about it straight away instead of when you actually need the backup.</p>

<p>If you're interested in the actual data, you can additionally run axebackup with a -j, which will create a JSON formatted text file that contains all the presets and their parameters in human readable (and javascript parseable) form.</p>

### "Where are the backup files stored?"
<p>- They are by default stored in the "current" folder. If you're in a terminal window, that's simply the directory you're in, but if you launch the utility directly from Windows Explorer or Mac Finder, then the files will be written into the same folder as axebackup resides in.</p>

## axeloader
<p>axeloader is a tool to send presets, backups, IR files or firmware updates to the AxeFx II. When applied to a single preset file, the utility will send the preset to the current edit buffer, overriding the embedded preset id, so that no presets are ever overwritten and that the preset doesn't go to some unexpected location on the AxeFx. This is so that you, the user, can pick a place where you want to store the preset or if you just want to try it out without saving. I.e. it's great for previewing and adding AxeExchange? presets to your AxeFx.</p>

<p>If you set axeloader up as the default handler for .syx files, it will send the data directly to the AxeFx when you double click the .syx file. Alternatively, you can drag and drop .syx files onto axeloader to send it to the AxeFx.</p>

## afx2lg
<p>This is a utility to update and generate setup files for the Little Giant from Gordius. More details to come but hopefully the UI will be self explanatory enough for it to be useful.</p>

## afx2lg_cmd
<p>This is a simple utility to update and generate setup files for the Little Giant from Gordius. You can run the utility without any arguments and it will prompt you for input. If you do prefer to use arguments run "afx2lg -?" for details.</p>

# example_setup.lgp
<p>This is an example setup file (actually it's what I use in conjunction with afx2lg) made with Control Center (the configuration app for the Little Giant). Open this file up in Control Center, take a look at the patches, banks etc just to understand what this file contains.</p>

<p>There are a few useful things in there such as programming for scenes, the looper and support for tap tempo + tuner on the same switch. The actual patches though, are just placeholders. This is what afx2lg will read and use to generate new banks.</p>

<p>This is just an example, so feel free to use your own setup file. Also, a generated setup file can be used as input to generate another one if you want (you don't always have to work with a template setup file).</p>

<p>In short this is how afx2lg works:</p>

<p>&lt;axe fx presets&gt; + &lt;LG setup&gt; + afx2lg = &lt;new LG setup&gt;</p>
<p>So to use, do the following:<br><br>
<ol>
<li>Run: axebackup. This will create .syx backup files of your setup.</li></li>
<li>Open example_setup.lgp in Control Center and export it to a text file via 'File->Export to...->Text...'.</li>
<li>Run: afx2lg. You will be prompted for the location of a .syx file (step 1) and the location of the exported setup file (step 2). Additionally you'll need to provide a name for the output file.</li>
<li>The output will look something like this:<br>
I need a path to a LittleGiant exported text file: mysetup.txt<br>
I need a path to a preset file or bank (.syx): BankA.syx<br>
Enter a preset range or * for all: *<br>
Enter an output file name: my_new_setup.txt<br>
<li>Open up Control Center again and import the output file (my_new_setup.txt) via 'File->Import from...->Text...'.</li>
</ol>
Voila, you have your new setup file with proper preset names from the AxeFx that you can send to your LG.
If you encounter problems, please file bugs here.
</p>
