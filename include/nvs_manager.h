#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 1つのpos_listに含まれる座標データの個数です。 */
#define NVS_MANAGER_POS_LIST_COUNT 12

/** 公開APIで指定できるカメラID・センサーIDの最大値です（0～255）。 */
#define NVS_MANAGER_ID_MAX UINT8_MAX

/**
 * @brief カメラの位置キャリブレーション用の1点分の座標データです。
 *
 * 1つのpos_listにはNVS_MANAGER_POS_LIST_COUNT個のデータが含まれます。
 */
typedef struct {
    int16_t x;
    int16_t y;
} nvs_manager_pos_t;

/**
 * @brief カメラの位置キャリブレーションデータ一式です。
 */
typedef struct {
    nvs_manager_pos_t pos[NVS_MANAGER_POS_LIST_COUNT];
} nvs_manager_pos_list_t;

/**
 * @brief カメラのYUVキャリブレーション用のデータです。
 *
 * 色の種類数や各データの型は、実際のアプリケーションに合わせて変更してください。
 * この構造体全体を、カメラごとに1つのNVS BLOBとして保存します。
 */
#ifndef NVS_MANAGER_CAM_YUV_COLOR_COUNT
#define NVS_MANAGER_CAM_YUV_COLOR_COUNT 8
#endif

typedef struct {
    uint8_t y;
    uint8_t u;
    uint8_t v;
} nvs_manager_yuv_t;

typedef struct {
    nvs_manager_yuv_t color[NVS_MANAGER_CAM_YUV_COLOR_COUNT];
} nvs_manager_cam_yuv_t;

/**
 * @brief カラーセンサーのRGBCキャリブレーション用のデータです。
 *
 * 色の種類数や各データの型は、実際のアプリケーションに合わせて変更してください。
 * この構造体全体を、カラーセンサーごとに1つのNVS BLOBとして保存します。
 */
#ifndef NVS_MANAGER_COLOR_RGBC_COLOR_COUNT
#define NVS_MANAGER_COLOR_RGBC_COLOR_COUNT 16
#endif

typedef struct {
    uint32_t r;
    uint32_t g;
    uint32_t b;
    uint32_t c;
} nvs_manager_rgbc_t;

typedef struct {
    nvs_manager_rgbc_t color[NVS_MANAGER_COLOR_RGBC_COLOR_COUNT];
} nvs_manager_color_rgbc_t;

/**
 * @brief NVSマネージャーを初期化します。
 *
 * デフォルトのNVSパーティションを初期化し、このコンポーネントを使用できる状態にします。
 * アプリケーション起動時に、他のnvs_manager_*関数より先に1回呼び出してください。
 *
 * @return 成功した場合はESP_OK、それ以外の場合はESP-IDFのNVSエラーコードを返します。
 */
esp_err_t nvs_manager_init(void);

/**
 * @brief NVSから指定したpos_listを読み込みます。
 *
 * カメラIDとpos_list IDで指定された12個の座標データを、呼び出し側が用意した構造体へ読み込みます。
 * このコンポーネント内部ではRAMキャッシュを保持しません。
 *
 * NVS namespace: "camera_<camera_id>"
 * NVS key:       "pos_<pos_list_id>"
 *
 * @param camera_id カメラを識別するIDです。
 * @param pos_list_id pos_listを識別するIDです。連番である必要はありません。
 * @param data 12個の座標データを格納する読み込み先です。
 * @return 成功した場合はESP_OK、指定データが存在しない場合はESP_ERR_NVS_NOT_FOUND、
 *         その他のエラーが発生した場合は対応するESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_read_pos_list(
    uint8_t camera_id,
    uint16_t pos_list_id,
    nvs_manager_pos_list_t *data
);

/**
 * @brief 指定したpos_listをNVSへ書き込みます。
 *
 * 12個の座標データを1つのNVS BLOBとして保存します。
 *
 * NVS namespace: "camera_<camera_id>"
 * NVS key:       "pos_<pos_list_id>"
 *
 * @param camera_id カメラを識別するIDです。
 * @param pos_list_id pos_listを識別するIDです。連番である必要はありません。
 * @param data 保存する12個の座標データです。
 * @return 成功した場合はESP_OK、それ以外の場合はESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_write_pos_list(
    uint8_t camera_id,
    uint16_t pos_list_id,
    const nvs_manager_pos_list_t *data
);

/**
 * @brief 指定したカメラのpos_listを削除する
 *
 * @param camera_id カメラID
 * @param pos_list_id pos_listのID
 * @return ESP_OK: 削除成功
 * @return ESP_ERR_NVS_NOT_FOUND: 指定したデータが存在しない
 * @return その他: ESP-IDFのエラーコード
 */
esp_err_t nvs_manager_delete_pos_list(
    uint8_t camera_id,
    uint16_t pos_list_id
);

/**
 * @brief 指定したpos_listがNVSに存在するか確認します。
 *
 * 指定したカメラのnamespaceとpos_listのkeyを確認します。
 * RAMキャッシュの確保や管理は行いません。
 *
 * @param camera_id カメラを識別するIDです。
 * @param pos_list_id pos_listを識別するIDです。
 * @return データが存在する場合はtrue、存在しない場合はfalseを返します。
 */
bool nvs_manager_is_pos_list_exist(
    uint8_t camera_id,
    uint16_t pos_list_id
);

/**
 * @brief 指定したカメラのYUVキャリブレーションデータをNVSから読み込みます。
 *
 * 指定したカメラに保存されているYUVキャリブレーションデータ一式を読み込みます。
 *
 * NVS namespace: "camera_<camera_id>"
 * NVS key:       "yuv"
 *
 * @param camera_id カメラを識別するIDです。
 * @param data YUVキャリブレーションデータの読み込み先です。
 * @return 成功した場合はESP_OK、指定データが存在しない場合はESP_ERR_NVS_NOT_FOUND、
 *         その他のエラーが発生した場合は対応するESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_read_cam_yuv(
    uint8_t camera_id,
    nvs_manager_cam_yuv_t *data
);

/**
 * @brief 指定したカメラのYUVキャリブレーションデータをNVSへ書き込みます。
 *
 * YUVキャリブレーションデータ一式を1つのNVS BLOBとして保存します。
 *
 * NVS namespace: "camera_<camera_id>"
 * NVS key:       "yuv"
 *
 * @param camera_id カメラを識別するIDです。
 * @param data 保存するカメラのYUVキャリブレーションデータです。
 * @return 成功した場合はESP_OK、それ以外の場合はESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_write_cam_yuv(
    uint8_t camera_id,
    const nvs_manager_cam_yuv_t *data
);

/**
 * @brief 指定したカメラのYUVキャリブレーションデータを削除する
 *
 * @param camera_id カメラID
 * @return ESP_OK: 削除成功
 * @return ESP_ERR_NVS_NOT_FOUND: 指定したデータが存在しない
 * @return その他: ESP-IDFのエラーコード
 */
esp_err_t nvs_manager_delete_cam_yuv(
    uint8_t camera_id
);


/**
 * @brief 指定したカラーセンサーのRGBCキャリブレーションデータをNVSから読み込みます。
 *
 * 指定したカラーセンサーに保存されているRGBCキャリブレーションデータ一式を読み込みます。
 *
 * NVS namespace: "color_<sensor_id>"
 * NVS key:       "rgbc"
 *
 * @param sensor_id カラーセンサーを識別するIDです。
 * @param data RGBCキャリブレーションデータの読み込み先です。
 * @return 成功した場合はESP_OK、指定データが存在しない場合はESP_ERR_NVS_NOT_FOUND、
 *         その他のエラーが発生した場合は対応するESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_read_color_rgbc(
    uint8_t sensor_id,
    nvs_manager_color_rgbc_t *data
);

/**
 * @brief 指定したカラーセンサーのRGBCキャリブレーションデータをNVSへ書き込みます。
 *
 * RGBCキャリブレーションデータ一式を1つのNVS BLOBとして保存します。
 *
 * NVS namespace: "color_<sensor_id>"
 * NVS key:       "rgbc"
 *
 * @param sensor_id カラーセンサーを識別するIDです。
 * @param data 保存するカラーセンサーのRGBCキャリブレーションデータです。
 * @return 成功した場合はESP_OK、それ以外の場合はESP-IDFエラーコードを返します。
 */
esp_err_t nvs_manager_write_color_rgbc(
    uint8_t sensor_id,
    const nvs_manager_color_rgbc_t *data
);

/**
 * @brief 指定したカラーセンサーのRGBCキャリブレーションデータを削除する
 *
 * @param sensor_id カラーセンサーID
 * @return ESP_OK: 削除成功
 * @return ESP_ERR_NVS_NOT_FOUND: 指定したデータが存在しない
 * @return その他: ESP-IDFのエラーコード
 */
esp_err_t nvs_manager_delete_color_rgbc(
    uint8_t sensor_id
);

#ifdef __cplusplus
}
#endif

#endif /* NVS_MANAGER_H */
