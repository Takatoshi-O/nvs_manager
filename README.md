# nvs_manager

A small ESP-IDF component for calibration data stored in NVS without an internal RAM cache.

## NVS layout

The component uses one namespace per camera or color sensor:

- `camera_<camera_id>`
  - `pos_<pos_list_id>`: one `nvs_manager_pos_list_t` blob (12 x/y points)
  - `yuv`: one `nvs_manager_cam_yuv_t` blob
- `color_<sensor_id>`
  - `rgbc`: one `nvs_manager_color_rgbc_t` blob

`pos_list_id` does not need to be sequential.

## Notes

The YUV and RGBC structures in `nvs_manager.h` use example fixed-size color arrays (`4` colors by default). Adjust:

- `NVS_MANAGER_CAM_YUV_COLOR_COUNT`
- `NVS_MANAGER_COLOR_RGBC_COLOR_COUNT`

and/or the element types to match your project.

The component reads data directly into caller-owned RAM when requested and writes directly to NVS during calibration/save operations.
