# jig-ocr(1)

## NAME

**jig-ocr** - perform OCR on PDF documents using cloud providers

## SYNOPSIS

```
jig ocr -f FILE [OPTIONS]
jig-ocr -f FILE [OPTIONS]
```

## DESCRIPTION

**jig-ocr** extracts text and images from PDF documents using optical character recognition (OCR). It processes PDF files through cloud OCR providers (currently Mistral) and outputs clean markdown with extracted images saved as separate files.

The tool automatically:
- Uploads the PDF to the OCR provider
- Processes all pages and merges content into a single markdown file
- Extracts embedded images and saves them as separate files
- Replaces table references with inline markdown tables
- Cleans up uploaded files from the provider after processing

## OPTIONS

- **-f, --file FILE** - Specify the PDF file to process (required)
- **-o, --output FILE** - Save output to file (default: index.md in PDF directory)
- **-p, --provider NAME** - Specify OCR provider to use (default: mistral)
- **-h, --help** - Display help information and exit

## CONFIGURATION

The OCR provider must be configured in `jig.conf` with an API key:

```
[provider:mistral]
key = YOUR_API_KEY
endpoint = https://api.mistral.ai
```

Initialize configuration with `jig init` if not already present.

## USAGE

### Process a PDF file with default output
```bash
jig ocr -f document.pdf
# Output: document directory/index.md + extracted images
```

### Specify custom output location
```bash
jig ocr -f document.pdf -o /output/content.md
# Output: /output/content.md + images saved to /output/
```

### Use a specific provider
```bash
jig ocr -f document.pdf -p mistral
```

## EXIT STATUS

- **0** - Success (OCR completed and output saved)
- **1** - Error (file not found, invalid format, API error, or configuration issue)

## EXAMPLES

Process a PDF in the current directory:
```bash
$ jig ocr -f ./book.pdf
Uploading ./book.pdf...
File uploaded: abc123
Getting signed URL...
Signed URL obtained
Running OCR...
OCR complete
Saved image: ./img-0.jpeg
Cleaning up...
Cleanup complete
Output saved to: ./index.md
```

Process with custom output:
```bash
$ jig ocr -f /docs/report.pdf -o /notes/report.md
# Creates /notes/report.md and saves images to /notes/
```

## OUTPUT FORMAT

The output is a markdown file containing:
- Merged text from all PDF pages
- Image references as `![img-N.jpeg](img-N.jpeg)`
- Tables rendered as markdown tables

Images are saved alongside the output file with their original identifiers (e.g., `img-0.jpeg`, `img-1.jpeg`).

## SUPPORTED FILES

- **pdf** - Portable Document Format

## SUPPORTED PROVIDERS

- **mistral** - Mistral AI OCR API (default)

## PREPROCESSING WITH QPDF

For large PDFs, consider splitting into smaller files before OCR processing using `qpdf`:

### Installation
```bash
sudo apt update && sudo apt install qpdf
```

### Split PDF by page range
```bash
# Extract pages 1-10
qpdf input.pdf --pages . 1-10 -- output.pdf

# Extract single page
qpdf input.pdf --pages . 5 -- page5.pdf

# Extract multiple ranges
qpdf input.pdf --pages . 1-5,10-15 -- selected.pdf
```

### Workflow example
```bash
# Split a large book into chapters
qpdf book.pdf --pages . 1-20 -- chapter1.pdf
qpdf book.pdf --pages . 21-45 -- chapter2.pdf

# OCR each chapter
jig ocr -f chapter1.pdf -o chapter1/index.md
jig ocr -f chapter2.pdf -o chapter2/index.md
```

## INVOCATION MODES

**jig-ocr** can be invoked in two ways:

1. **Subcommand mode**: `jig ocr <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-ocr <args>` - called as a standalone executable

Both modes provide identical functionality.

## DEPENDENCIES

- **libcurl** - HTTP client library for API requests
- **cJSON** - JSON parsing (vendored)

Build with:
```bash
sudo apt install libcurl4-openssl-dev
make
```
