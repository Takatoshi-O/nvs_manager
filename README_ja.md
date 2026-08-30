# nvs_manager

`nvs_manager` は、カメラやカラーセンサーのキャリブレーションデータをNVSへ保存するための小規模なESP-IDFコンポーネントです。コンポーネント自身は永続的なRAMキャッシュを保持しません。

## 保存データ

論理的には3種類のデータを保存します。

| データ | 識別方法 | 保存するBLOB |
|---|---|---|
| カメラ位置リスト | カメラID + pos_list ID | `nvs_manager_pos_list_t` |
| カメラYUV基準値 | カメラID | `nvs_manager_cam_yuv_t` |
| カラーセンサーRGBC基準値 | センサーID | `nvs_manager_color_rgbc_t` |

位置リストには `NVS_MANAGER_POS_LIST_COUNT` (=12) 個の座標点が含まれます。

## NVSの名前

現在の実装では次の名前を生成します。

```text
cam_<camera_id>
    pos_<pos_list_id>
    yuv

color_<sensor_id>
    rgbc
```

`pos_list_id` は連番である必要がないため、1台のカメラについて番号が飛んだ複数のキャリブレーションセットを保存できます。

NVSのnamespace/key名は終端の `\0` を除いて15文字以内という制約があるため、実装では生成用に16バイトのバッファを使用しています。

## 初期化

アプリケーション起動時に `nvs_manager_init()` を1回呼び出し、その後にread/write/delete APIを利用します。

```c
ESP_ERROR_CHECK(nvs_manager_init());
```

初期化は冪等です。NVSから `ESP_ERR_NVS_NO_FREE_PAGES` または `ESP_ERR_NVS_NEW_VERSION_FOUND` が返った場合、現在の実装ではNVSフラッシュ領域を消去して再初期化します。

## 位置データ

```c
nvs_manager_pos_list_t list = {0};

ESP_ERROR_CHECK(nvs_manager_read_pos_list(0, 10, &list));
ESP_ERROR_CHECK(nvs_manager_write_pos_list(0, 10, &list));
nvs_manager_delete_pos_list(0, 10);
```

指定したカメラID・リストIDについて、正しいサイズのBLOBが存在するかは `nvs_manager_is_pos_list_exist()` で確認できます。

read APIが失敗した場合、現在の実装では呼び出し側の出力構造体を0クリアします。

## カメラYUVデータ

`nvs_manager_cam_yuv_t` は `NVS_MANAGER_CAM_YUV_COLOR_COUNT` 個のYUVデータを保持します。現在のヘッダーのデフォルト値は8です。

使用するAPIは次のとおりです。

- `nvs_manager_read_cam_yuv()`
- `nvs_manager_write_cam_yuv()`
- `nvs_manager_delete_cam_yuv()`

## カラーセンサーRGBCデータ

`nvs_manager_color_rgbc_t` は `NVS_MANAGER_COLOR_RGBC_COLOR_COUNT` 個のRGBCデータを保持します。現在のデフォルト値は16です。

使用するAPIは次のとおりです。

- `nvs_manager_read_color_rgbc()`
- `nvs_manager_write_color_rgbc()`
- `nvs_manager_delete_color_rgbc()`

## キャッシュ方針

このコンポーネント自身はNVSデータの永続的なRAMキャッシュを持ちません。read APIは指定されたBLOBを呼び出し側の構造体へ直接読み込み、write APIはNVSへ直接保存してcommitします。

上位コンポーネント側で必要に応じてRAMへキャッシュできます。現在の `color_sensor` と `camera_manager` も実行時処理のために独自のキャッシュを持っています。

## 依存コンポーネント

- `nvs_flash`

## 公開ヘッダー

- `include/nvs_manager.h`

## 注意事項

`nvs_manager.h` の構造体は現在のNVS BLOB形式の一部です。メンバー型、配列数、構造体レイアウトを変更すると、以前のファームウェアで保存したBLOBを新しいファームウェアで読めなくなる可能性があります。
