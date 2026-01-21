#ifndef MISTRAL_H
#define MISTRAL_H

#define MISTRAL_API_BASE "https://api.mistral.ai/v1"
#define MISTRAL_MAX_ID 64
#define MISTRAL_MAX_URL 2048

typedef struct {
    char *content;
    size_t size;
} MistralResponse;

typedef struct {
    char file_id[MISTRAL_MAX_ID];
    char signed_url[MISTRAL_MAX_URL];
    char *ocr_result;
    int success;
    char *error;
} MistralOcrResult;

/**
 * Initialize Mistral API (call once at startup)
 */
int mistral_init(void);

/**
 * Cleanup Mistral API (call once at shutdown)
 */
void mistral_cleanup(void);

/**
 * Upload a file to Mistral for OCR processing
 * @param filepath Path to the file to upload
 * @param api_key Mistral API key
 * @param file_id Output buffer for the file ID (must be MISTRAL_MAX_ID bytes)
 * @return 0 on success, -1 on error
 */
int mistral_upload_file(const char *filepath, const char *api_key, char *file_id);

/**
 * Get a signed URL for an uploaded file
 * @param file_id The file ID from upload
 * @param api_key Mistral API key
 * @param expiry_hours URL expiry time in hours
 * @param signed_url Output buffer for the URL (must be MISTRAL_MAX_URL bytes)
 * @return 0 on success, -1 on error
 */
int mistral_get_signed_url(const char *file_id, const char *api_key,
                           int expiry_hours, char *signed_url);

/**
 * Perform OCR on a document using its signed URL
 * @param signed_url The signed URL for the document
 * @param api_key Mistral API key
 * @param result Output buffer for OCR result (allocated, caller must free)
 * @return 0 on success, -1 on error
 */
int mistral_ocr(const char *signed_url, const char *api_key, char **result);

/**
 * Delete an uploaded file from Mistral
 * @param file_id The file ID to delete
 * @param api_key Mistral API key
 * @return 0 on success, -1 on error
 */
int mistral_delete_file(const char *file_id, const char *api_key);

/**
 * Run the full OCR pipeline: upload -> get URL -> OCR -> cleanup
 * @param filepath Path to the file to process
 * @param api_key Mistral API key
 * @param result Output structure with results
 * @return 0 on success, -1 on error
 */
int mistral_process(const char *filepath, const char *api_key, MistralOcrResult *result);

/**
 * Free resources in MistralOcrResult
 */
void mistral_free_result(MistralOcrResult *result);

#endif /* MISTRAL_H */
