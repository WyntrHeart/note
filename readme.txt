A simple cli program to manage plain text notes. Notes are stored in $HOME/documents/notes. The edit subcommand uses the $EDITOR environment variable.

Builds with GCC. Uses dirent.h, so MSVC's standard library won't work.

NOTE: Only help, ls, and edit subcommands are currently implemented, and ls doesn't list subdirs yet.

usage: note NOTE|SUBCOMMAND [ARG1 [ARG2]]
   Display NOTE or do SUBCOMMAND
subcommands:
   help  |  Display this help text
   ls    |  List notes and subdirs in note directory. If ARG1
            is a subdir list notes/subdirs inside it.
   add   |  Add a new note named ARG1.txt
   edit  |  Open ARG1.txt with $EDITOR
   rm    |  Remove ARG1.txt
   cp    |  Copy ARG1.txt to ARG2.txt
   mv    |  Move or rename ARG1.txt to ARG2.txt
   mkdir |  Create subdir named ARG1
   rmdir |  Remove subdir named ARG1

This software is licensed under the GNU GPL version 3
