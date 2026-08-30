# nvs_manager

`nvs_manager` is a small ESP-IDF component for storing camera and color-sensor calibration data in NVS without maintaining an internal application-level RAM cache.

## Stored data

The component stores three logical data types.

| Data | Identifier | Stored blob |
|---|---|---|
| Camera position list | camera ID + position-list ID | `nvs_manager_pos_list_t` |
| Camera YUV references | camera ID | `nvs_manager_cam_yuv_t` |
| Color-sensor RGBC references | sensor ID | `nvs_manager_color_rgbc_t` |

A position-list contains `NVS_MANAGER_POS_LIST_COUNT` (=12) coordinate points.

## NVS naming

The current implementation builds these names:

```text
cam_<camera_id>
    pos_<pos_list_id>
    yuv

color_<sensor_id>
    rgbc
```

`pos_list_id` does not have to be sequential, so multiple non-contiguous calibration sets can coexist for one camera.

NVS namespace/key length is limited to 15 characters excluding the terminating `\0`; the implementation allocates a 16-byte working buffer for generated names.

## Initialization

Call `nvs_manager_init()` once during application startup, before using the read/write/delete APIs.

```c
ESP_ERROR_CHECK(nvs_manager_init());
```

The initialization is idempotent. If NVS reports `ESP_ERR_NVS_NO_FREE_PAGES` or `ESP_ERR_NVS_NEW_VERSION_FOUND`, the current implementation erases and re-initializes the NVS flash partition.

## Position data

```c
nvs_manager_pos_list_t list = {0};

ESP_ERROR_CHECK(nvs_manager_read_pos_list(0, 10, &list));
ESP_ERROR_CHECK(nvs_manager_write_pos_list(0, 10, &list));
nvs_manager_delete_pos_list(0, 10);
```

Use `nvs_manager_is_pos_list_exist()` to check whether a correctly sized blob exists for the requested camera/list ID.

The read functions clear the caller's output structure with zeroes when the read fails.

## Camera YUV data

`nvs_manager_cam_yuv_t` contains `NVS_MANAGER_CAM_YUV_COLOR_COUNT` YUV entries. The default count in the current header is 8.

Use:

- `nvs_manager_read_cam_yuv()`
- `nvs_manager_write_cam_yuv()`
- `nvs_manager_delete_cam_yuv()`

## Color-sensor RGBC data

`nvs_manager_color_rgbc_t` contains `NVS_MANAGER_COLOR_RGBC_COLOR_COUNT` RGBC entries. The current default is 16.

Use:

- `nvs_manager_read_color_rgbc()`
- `nvs_manager_write_color_rgbc()`
- `nvs_manager_delete_color_rgbc()`

## Cache behavior

This component does not maintain a persistent RAM cache of NVS data. Read APIs copy the selected blob directly into caller-owned structures, and write APIs commit directly to NVS.

Higher-level components may choose to cache these values themselves, as `color_sensor` and `camera_manager` currently do for their own runtime processing.

## Dependencies

- `nvs_flash`

## Public header

- `include/nvs_manager.h`

## Notes

The data structures in `nvs_manager.h` are part of the current binary NVS format. Changing field types, array counts, or structure layouts can make previously stored blobs incompatible with the new firmware.
