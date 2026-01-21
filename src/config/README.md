# jig-config(1)

## NAME

**jig-config** - parse and display configuration from jig.conf

## SYNOPSIS

```
jig config [OPTIONS]
jig-config [OPTIONS]
```

## DESCRIPTION

**jig-config** parses the INI-style configuration file (`jig.conf`) from the current directory and displays the loaded settings. It extracts provider configurations for API integrations.

The configuration file uses INI format with sections:
- `[provider.<name>]` - API provider configurations (key, endpoint)
- `[meta]` - Project metadata
- `[task.<name>]` - Task-specific settings

## OPTIONS

- **-h, --help** - Display help information and exit

## CONFIGURATION FILE

The `jig.conf` file uses INI format:

```ini
[meta]
version = 1

[provider.mistral]
key = your-api-key
endpoint = https://api.mistral.ai/v1

[task.ocr]
provider = mistral
model = mistral-ocr-latest
```

### Provider Section

Each `[provider.<name>]` section defines an API provider:

- **key** - API authentication key
- **endpoint** - Base URL for API requests

## EXIT STATUS

- **0** - Success
- **1** - Error (configuration file not found or parse failure)

## EXAMPLES

Display current configuration:
```bash
$ jig config
[provider.mistral]
key:      api-key-here
endpoint: https://api.mistral.ai/v1
```

Show help:
```bash
$ jig config -h
Usage jig config [OPTIONS]
```

## FILES

- **./jig.conf** - Configuration file (must exist in current directory)

## INVOCATION MODES

**jig-config** can be invoked in two ways:

1. **Subcommand mode**: `jig config <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-config <args>` - called as a standalone executable

Both modes provide identical functionality.
