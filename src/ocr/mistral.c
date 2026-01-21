#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "mistral.h"
#include "../../vendor/cjson/cJSON.h"

/* --- Response buffer handling --- */

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MistralResponse *resp = (MistralResponse *)userp;

    char *ptr = realloc(resp->content, resp->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }

    resp->content = ptr;
    memcpy(&(resp->content[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->content[resp->size] = '\0';

    return realsize;
}

static void init_response(MistralResponse *resp) {
    resp->content = malloc(1);
    resp->content[0] = '\0';
    resp->size = 0;
}

static void free_response(MistralResponse *resp) {
    if (resp->content) {
        free(resp->content);
        resp->content = NULL;
    }
    resp->size = 0;
}

/* --- Public API --- */

int mistral_init(void) {
    return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK ? 0 : -1;
}

void mistral_cleanup(void) {
    curl_global_cleanup();
}

int mistral_upload_file(const char *filepath, const char *api_key, char *file_id) {
    CURL *curl;
    CURLcode res;
    MistralResponse response;
    struct curl_slist *headers = NULL;
    curl_mime *mime = NULL;
    curl_mimepart *part = NULL;
    char auth_header[256];
    int ret = -1;

    init_response(&response);
    file_id[0] = '\0';

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        return -1;
    }

    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    mime = curl_mime_init(curl);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "purpose");
    curl_mime_data(part, "ocr", CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, filepath);

    curl_easy_setopt(curl, CURLOPT_URL, MISTRAL_API_BASE "/files");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: Upload failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200 && http_code != 201) {
        fprintf(stderr, "Error: Upload failed with HTTP %ld: %s\n", http_code, response.content);
        goto cleanup;
    }

    cJSON *json = cJSON_Parse(response.content);
    if (!json) {
        fprintf(stderr, "Error: Failed to parse upload response\n");
        goto cleanup;
    }

    cJSON *id = cJSON_GetObjectItem(json, "id");
    if (!cJSON_IsString(id) || id->valuestring == NULL) {
        fprintf(stderr, "Error: No file ID in response\n");
        cJSON_Delete(json);
        goto cleanup;
    }

    strncpy(file_id, id->valuestring, MISTRAL_MAX_ID - 1);
    file_id[MISTRAL_MAX_ID - 1] = '\0';
    cJSON_Delete(json);
    ret = 0;

cleanup:
    curl_mime_free(mime);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free_response(&response);
    return ret;
}

int mistral_get_signed_url(const char *file_id, const char *api_key,
                           int expiry_hours, char *signed_url) {
    CURL *curl;
    CURLcode res;
    MistralResponse response;
    struct curl_slist *headers = NULL;
    char url[512];
    char auth_header[256];
    int ret = -1;

    init_response(&response);
    signed_url[0] = '\0';

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        return -1;
    }

    snprintf(url, sizeof(url), "%s/files/%s/url?expiry=%d",
             MISTRAL_API_BASE, file_id, expiry_hours);
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);

    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: Get URL failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "Error: Get URL failed with HTTP %ld: %s\n", http_code, response.content);
        goto cleanup;
    }

    cJSON *json = cJSON_Parse(response.content);
    if (!json) {
        fprintf(stderr, "Error: Failed to parse URL response\n");
        goto cleanup;
    }

    cJSON *url_field = cJSON_GetObjectItem(json, "url");
    if (!cJSON_IsString(url_field) || url_field->valuestring == NULL) {
        fprintf(stderr, "Error: No URL in response\n");
        cJSON_Delete(json);
        goto cleanup;
    }

    strncpy(signed_url, url_field->valuestring, MISTRAL_MAX_URL - 1);
    signed_url[MISTRAL_MAX_URL - 1] = '\0';
    cJSON_Delete(json);
    ret = 0;

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free_response(&response);
    return ret;
}

int mistral_ocr(const char *signed_url, const char *api_key, char **result) {
    CURL *curl;
    CURLcode res;
    MistralResponse response;
    struct curl_slist *headers = NULL;
    char auth_header[256];
    char *post_data = NULL;
    int ret = -1;

    init_response(&response);
    *result = NULL;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        return -1;
    }

    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "model", "mistral-ocr-latest");

    cJSON *document = cJSON_CreateObject();
    cJSON_AddStringToObject(document, "type", "document_url");
    cJSON_AddStringToObject(document, "document_url", signed_url);
    cJSON_AddItemToObject(body, "document", document);

    cJSON_AddStringToObject(body, "table_format", "markdown");
    cJSON_AddBoolToObject(body, "include_image_base64", cJSON_True);

    post_data = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    if (!post_data) {
        fprintf(stderr, "Error: Failed to create request body\n");
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_URL, MISTRAL_API_BASE "/ocr");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: OCR request failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "Error: OCR failed with HTTP %ld: %s\n", http_code, response.content);
        goto cleanup;
    }

    *result = response.content;
    response.content = NULL;
    ret = 0;

cleanup:
    if (post_data) free(post_data);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free_response(&response);
    return ret;
}

int mistral_delete_file(const char *file_id, const char *api_key) {
    CURL *curl;
    CURLcode res;
    MistralResponse response;
    struct curl_slist *headers = NULL;
    char url[512];
    char auth_header[256];
    int ret = -1;

    init_response(&response);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        return -1;
    }

    snprintf(url, sizeof(url), "%s/files/%s", MISTRAL_API_BASE, file_id);
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: Delete failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200 && http_code != 204) {
        fprintf(stderr, "Error: Delete failed with HTTP %ld: %s\n", http_code, response.content);
        goto cleanup;
    }

    ret = 0;

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free_response(&response);
    return ret;
}

int mistral_process(const char *filepath, const char *api_key, MistralOcrResult *result) {
    memset(result, 0, sizeof(MistralOcrResult));
    result->success = 0;

    fprintf(stderr, "Uploading %s...\n", filepath);
    if (mistral_upload_file(filepath, api_key, result->file_id) != 0) {
        result->error = strdup("Failed to upload file");
        return -1;
    }
    fprintf(stderr, "File uploaded: %s\n", result->file_id);

    fprintf(stderr, "Getting signed URL...\n");
    if (mistral_get_signed_url(result->file_id, api_key, 1, result->signed_url) != 0) {
        result->error = strdup("Failed to get signed URL");
        mistral_delete_file(result->file_id, api_key);
        return -1;
    }
    fprintf(stderr, "Signed URL obtained\n");

    fprintf(stderr, "Running OCR...\n");
    if (mistral_ocr(result->signed_url, api_key, &result->ocr_result) != 0) {
        result->error = strdup("Failed to perform OCR");
        mistral_delete_file(result->file_id, api_key);
        return -1;
    }
    fprintf(stderr, "OCR complete\n");

    fprintf(stderr, "Cleaning up...\n");
    mistral_delete_file(result->file_id, api_key);
    fprintf(stderr, "Cleanup complete\n");

    result->success = 1;
    return 0;
}

void mistral_free_result(MistralOcrResult *result) {
    if (result->ocr_result) {
        free(result->ocr_result);
        result->ocr_result = NULL;
    }
    if (result->error) {
        free(result->error);
        result->error = NULL;
    }
}
