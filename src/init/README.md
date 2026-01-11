# jig-init(1)

## NAME

**jig-init** - initialize a jig configuration file

## SYNOPSIS

```
jig init [OPTIONS]
jig-init [OPTIONS]
```

## DESCRIPTION

**jig-init** creates a new `jig.conf` configuration file in the current directory with default provider settings.

If a configuration file already exists, the command exits with an error to prevent overwriting existing configuration.

## OPTIONS

- **-h, --help** - Display help information and exit

## EXIT STATUS

- **0** - Success (configuration file created)
- **1** - Error (configuration file already exists or write failure)

## EXAMPLES

Initialize a new project:
```bash
$ jig init
$ cat jig.conf
[provider.mistral]
key =
endpoint = https://api.mistral.ai/v1
```

Attempting to initialize when config exists:
```bash
$ jig init
Error: ./jig.conf already exists.
```

Show help:
```bash
$ jig init -h
Usage: jig init [OPTIONS]

Initialize a new jig configuration file in the current directory.

OPTIONS:
  -h, --help    Display this help message
```

## FILES

- **./jig.conf** - Configuration file created by this command

## INVOCATION MODES

**jig-init** can be invoked in two ways:

1. **Subcommand mode**: `jig init <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-init <args>` - called as a standalone executable

Both modes provide identical functionality.

## SEE ALSO

jig-config(1) - parse and display configuration
