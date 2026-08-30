#include "nvs_manager.h"

#include <stdio.h>
#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"

#define NVS_MANAGER_CAMERA_NAMESPACE_FORMAT "cam_%u"
#define NVS_MANAGER_COLOR_NAMESPACE_FORMAT  "color_%u"
#define NVS_MANAGER_POS_KEY_FORMAT           "pos_%u"
#define NVS_MANAGER_YUV_KEY                  "yuv"
#define NVS_MANAGER_RGBC_KEY                 "rgbc"

/* NVS key and namespace names are limited to 15 characters excluding '\0'. */
#define NVS_MANAGER_NAME_BUFFER_SIZE 16

static esp_err_t open_namespace_for_read(const char *namespace_name, nvs_handle_t *handle)
{
    if (namespace_name == NULL || handle == NULL) return ESP_ERR_INVALID_ARG;

    return nvs_open(namespace_name, NVS_READONLY, handle);
}

static esp_err_t open_namespace_for_write(const char *namespace_name, nvs_handle_t *handle)
{
    if (namespace_name == NULL || handle == NULL) return ESP_ERR_INVALID_ARG;

    return nvs_open(namespace_name, NVS_READWRITE, handle);
}

static esp_err_t read_blob(const char *namespace_name, const char *key, void *data, size_t expected_size)
{
    if (namespace_name == NULL || key == NULL || data == NULL || expected_size == 0U)
        return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = open_namespace_for_read(namespace_name, &handle);
    if (err != ESP_OK) return err;

    size_t size = expected_size;
    err = nvs_get_blob(handle, key, data, &size);
    if (err == ESP_OK && size != expected_size) err = ESP_ERR_INVALID_SIZE;

    nvs_close(handle);
    return err;
}

static esp_err_t write_blob(const char *namespace_name, const char *key, const void *data, size_t size)
{
    if (namespace_name == NULL || key == NULL || data == NULL || size == 0U)
        return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = open_namespace_for_write(namespace_name, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, key, data, size);
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}

static bool blob_exists(const char *namespace_name, const char *key, size_t expected_size)
{
    if (namespace_name == NULL || key == NULL || expected_size == 0U) return false;

    nvs_handle_t handle;
    if (open_namespace_for_read(namespace_name, &handle) != ESP_OK) return false;

    size_t size = expected_size;
    esp_err_t err = nvs_get_blob(handle, key, NULL, &size);
    nvs_close(handle);

    return (err == ESP_OK) && (size == expected_size);
}

static esp_err_t make_camera_namespace(uint8_t camera_id, char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE])
{
    int written = snprintf(namespace_name, NVS_MANAGER_NAME_BUFFER_SIZE,
                            NVS_MANAGER_CAMERA_NAMESPACE_FORMAT,
                            (unsigned int)camera_id);
    if (written < 0 || written >= NVS_MANAGER_NAME_BUFFER_SIZE) return ESP_ERR_INVALID_SIZE;

    return ESP_OK;
}

static esp_err_t make_color_namespace(uint8_t sensor_id, char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE])
{
    int written = snprintf(namespace_name, NVS_MANAGER_NAME_BUFFER_SIZE,
                            NVS_MANAGER_COLOR_NAMESPACE_FORMAT,
                            (unsigned int)sensor_id);
    if (written < 0 || written >= NVS_MANAGER_NAME_BUFFER_SIZE) return ESP_ERR_INVALID_SIZE;

    return ESP_OK;
}

static esp_err_t make_pos_key(uint16_t pos_list_id, char key[NVS_MANAGER_NAME_BUFFER_SIZE])
{
    int written = snprintf(key, NVS_MANAGER_NAME_BUFFER_SIZE,
                            NVS_MANAGER_POS_KEY_FORMAT,
                            (unsigned int)pos_list_id);
    if (written < 0 || written >= NVS_MANAGER_NAME_BUFFER_SIZE) return ESP_ERR_INVALID_SIZE;

    return ESP_OK;
}

static bool s_initialized = false;

esp_err_t nvs_manager_init(void)
{
    if (s_initialized) return ESP_OK;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        if ((err = nvs_flash_erase()) == ESP_OK) err = nvs_flash_init();
    }

    if (err == ESP_OK) s_initialized = true;
    return err;
}

esp_err_t nvs_manager_read_pos_list(uint8_t camera_id, uint16_t pos_list_id, nvs_manager_pos_list_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    char key[NVS_MANAGER_NAME_BUFFER_SIZE];

    esp_err_t err = make_camera_namespace(camera_id, namespace_name);
    if (err != ESP_OK) return err;

    err = make_pos_key(pos_list_id, key);
    if (err != ESP_OK) return err;

    err = read_blob(namespace_name, key, data, sizeof(*data));
    if (err != ESP_OK) memset(data, 0, sizeof(*data));

    return err;
}

esp_err_t nvs_manager_write_pos_list(uint8_t camera_id, uint16_t pos_list_id, const nvs_manager_pos_list_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    char key[NVS_MANAGER_NAME_BUFFER_SIZE];

    esp_err_t err = make_camera_namespace(camera_id, namespace_name);
    if (err != ESP_OK) return err;

    err = make_pos_key(pos_list_id, key);
    if (err != ESP_OK) return err;

    return write_blob(namespace_name, key, data, sizeof(*data));
}

esp_err_t nvs_manager_delete_pos_list(uint8_t camera_id, uint16_t pos_list_id)
{
    char namespace[16], key[16];
    snprintf(namespace, sizeof(namespace), "camera_%u", camera_id);
    snprintf(key, sizeof(key), "pos_%u", pos_list_id);

    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, key);
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}

bool nvs_manager_is_pos_list_exist(uint8_t camera_id, uint16_t pos_list_id)
{
    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    char key[NVS_MANAGER_NAME_BUFFER_SIZE];

    if (make_camera_namespace(camera_id, namespace_name) != ESP_OK) return false;

    if (make_pos_key(pos_list_id, key) != ESP_OK) return false;

    return blob_exists(namespace_name, key, sizeof(nvs_manager_pos_list_t));
}

esp_err_t nvs_manager_read_cam_yuv(uint8_t camera_id, nvs_manager_cam_yuv_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    esp_err_t err = make_camera_namespace(camera_id, namespace_name);
    if (err != ESP_OK) return err;

    err = read_blob(namespace_name, NVS_MANAGER_YUV_KEY, data, sizeof(*data));
    if (err != ESP_OK) memset(data, 0, sizeof(*data));
    
    return err;
}

esp_err_t nvs_manager_write_cam_yuv(uint8_t camera_id, const nvs_manager_cam_yuv_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    esp_err_t err = make_camera_namespace(camera_id, namespace_name);
    if (err != ESP_OK) return err;

    return write_blob(namespace_name, NVS_MANAGER_YUV_KEY, data, sizeof(*data));
}

esp_err_t nvs_manager_delete_cam_yuv(uint8_t camera_id)
{
    char namespace[16];
    snprintf(namespace, sizeof(namespace), "camera_%u", camera_id);

    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, "yuv");
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}

esp_err_t nvs_manager_read_color_rgbc(uint8_t sensor_id, nvs_manager_color_rgbc_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    esp_err_t err = make_color_namespace(sensor_id, namespace_name);
    if (err != ESP_OK) return err;

    err = read_blob(namespace_name, NVS_MANAGER_RGBC_KEY, data, sizeof(*data));
    if (err != ESP_OK) memset(data, 0, sizeof(*data));
    
    return err;
}

esp_err_t nvs_manager_write_color_rgbc(uint8_t sensor_id, const nvs_manager_color_rgbc_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;

    char namespace_name[NVS_MANAGER_NAME_BUFFER_SIZE];
    esp_err_t err = make_color_namespace(sensor_id, namespace_name);
    if (err != ESP_OK) return err;

    return write_blob(namespace_name, NVS_MANAGER_RGBC_KEY, data, sizeof(*data));
}

esp_err_t nvs_manager_delete_color_rgbc(uint8_t sensor_id)
{
    char namespace[16];
    snprintf(namespace, sizeof(namespace), "color_%u", sensor_id);

    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, "rgbc");
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}