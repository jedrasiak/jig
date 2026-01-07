# jig-note

Automate process of creating new note scaffold. Creates a folder with at least one .md file inside.

## synopsis

```
jig note <TITLE> [OPTIONS]
```

## description

- TITLE is mandatory

## options

* -h, --help - display help and exit
* -l, --lang "en,md" - accept a list of languages to use
* -t, --template ./path/to/file - read and use provided template file

## flow

1. Keep the provided title in memory
2. Use slugify() from slugify.h and slugify the provided title. Keep slug in memory
3. Create a folder in current folder using slug
4. If no --lang flag - create single index.md file
5. If --lang flag provided - create a file for each lang in the list using pattern index.LANG.md
6. if no --template flag - put into each file the basic content below
```
---
id: {uuid}
---

# {title}
```
7. if --template flag provided - read the target file and put this content into each file.

BUT - there should ALWAYS be 'id' property generated with uuid() coming from uuid.h

## miscealenous
- follow structure adopted with tree module. create:
    - note folder
    - note.c for code with help()
    - note.h
    - note-cli.c for debugging purposes
    - Makefile that will compile note-cli.c
- include this module into main Makefile
- include this module into main.c orchestrator
- update main.c help() and main README

## template
```
---
id: {{id}}
title: {{title}}
slug: {{slug}}
draft: false
links:
- "[TARGET TITLE](/path?label=parent)"
---

# {{title}}
```