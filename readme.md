# note
A simple cli program to manage plain text notes. By default notes are stored in `$HOME/documents/notes` (or `%USERPROFILE%\Documents\notes` on Windows if `%HOME%` isn't set). You can set `$NOTE_DIR` to override the default directory. The edit subcommand uses the `$EDITOR` environment variable. Tested on Windows 10, Android 9 and 13, and Ubuntu 24.04.
Builds with GCC. *MSVC does not work.*

**NOTICE**: I am a hobby programmer with no proper education. Please use this software with caution. I am not responsible if it deletes all your bank info, spontaneously combusts your computer, or if reading my awful code causes you to have an aneurysm. Feedback from more knowledgeable programmers would be great.

```
usage: note NOTE|SUBCOMMAND [ARG1 [ARG2]]
    Display NOTE or do SUBCOMMAND
subcommands:
    help   Display this help text
    ls     List notes and subdirs in note directory. If ARG1 is a
           subdir list notes/subdirs inside it.
    add    Append the string ARG2 to a new line at the end of ARG1.txt
    edit   Open ARG1.txt with $EDITOR
    rm     Remove ARG1.txt
    cp     Copy ARG1.txt to ARG2.txt
    mv     Move or rename ARG1.txt to ARG2.txt
    mkdir  Create subdir named ARG1
    rmdir  Remove subdir named ARG1
```

This software is licensed under the GNU GPL version 3
