/*
    Kiwi board -> phone audio.

    The ESP32-S3 has no Bluetooth Classic, so it can't play on headphones
    directly. Instead it creates a Wi-Fi network and serves a page; the phone
    opens it and every note the board plays arrives over a WebSocket as
    "<freq_hz> <ms>". The browser synthesizes it, so it comes out wherever the
    phone's audio goes (e.g. its Bluetooth headphones).

    KEY1..KEY4 play a note. The page's button plays a tune.
*/

#include <stdio.h>
#include <string.h>

#include "display.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "move_square_game.h"
#include "move_cpn_game.h"
#include "move_scream_game.h"
#define WIFI_SSID   "kiwi-audio"
#define WIFI_PASS   "kiwikiwi"

static const char *TAG = "kiwi";
static httpd_handle_t s_server;
static const gpio_num_t s_keys[] = {GPIO_NUM_14, GPIO_NUM_21, GPIO_NUM_47, GPIO_NUM_48};
static volatile int file_requested;
bool display_ok;
typedef enum{
    FILE_NONE = 0,
    FILE_1,
    FILE_2,
    FILE_3
} files;


static const char s_page[] =
    "<!doctype html><html><head><meta charset=utf-8>"
    "<meta name=viewport content='width=device-width,initial-scale=1'><title>Kiwi</title>"
    "<style>body{font-family:sans-serif;text-align:center;padding-top:15vh}"
    "button{font-size:1.6em;padding:.5em 1em;margin:.4em}</style></head><body>"
    "<h1>NTV Games</h1>"
    "<script>window.onerror=function(m,u,l){alert('JS: '+m+' (linea '+l+')')}</script>"
    "<button id=f1>Juego 1</button><br>"
    "<button id=f2>Juego 2</button><br>"
    "<button id=f3>Juego 3</button>"
    "<p id=st>conectando...</p><script>"
    "let ws;"
    "const en=b=>document.querySelectorAll('button').forEach(x=>x.disabled=!b);"
    "const send=m=>{if(ws&&ws.readyState==1)ws.send(m)};"
    "f1.onclick=()=>send('file_1');"
    "f2.onclick=()=>send('file_2');"
    "f3.onclick=()=>send('file_3');"
    "function connect(){if(ws&&ws.readyState<2)return;"
    "ws=new WebSocket('ws://'+location.host+'/ws');"
    "ws.onopen=()=>{st.textContent='connected';en(true)};"
    "ws.onclose=()=>{en(false);st.textContent='disconnected, retrying...';setTimeout(connect,1000)}}"
    "en(false);connect();"
    "</script></body></html>";

static esp_err_t page_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "pagina pedida");
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, s_page, sizeof(s_page) - 1);
}

static esp_err_t ws_handler(httpd_req_t *req)
{

    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "phone connected");
        return ESP_OK;
    }
    uint8_t buf[16] = {0};
    httpd_ws_frame_t frame = {.payload = buf};
    if (httpd_ws_recv_frame(req, &frame, sizeof(buf) - 1) != ESP_OK) {
        return ESP_FAIL;
    }
    if (frame.type == HTTPD_WS_TYPE_TEXT) {
        ESP_LOGI(TAG, "ws recibido: %s", (char *)buf);
        if(strcmp((char *)buf, "file_1") == 0){
            file_requested = FILE_1;
        } else if (strcmp((char *)buf, "file_2") == 0)
        {
            file_requested = FILE_2;
        } else if (strcmp((char *)buf, "file_3") == 0)
        {
            file_requested = FILE_3;
        }
        
    }
    return ESP_OK;
}


static void wifi_ap_start(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));
    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    // No power save: it adds latency to every note.
    esp_wifi_set_ps(WIFI_PS_NONE);
}

void display_waiting_animation(int x)
{
        display_text(x, 24, ".  ", false);
        display_flush();       
        vTaskDelay(pdMS_TO_TICKS(250));
        display_text(x, 24, ".. ", false);
        display_flush();
        vTaskDelay(pdMS_TO_TICKS(250));
        display_text(x, 24, "...", false);
        display_flush();
        vTaskDelay(pdMS_TO_TICKS(500));
        display_flush();
}

void init(void){
    display_ok = display_init() == ESP_OK;
    ESP_ERROR_CHECK(buttons_init());
    if (!display_ok) {
        ESP_LOGE(TAG, "display init failed - continuing without screen");
    }
        
}

bool initial_audio_check() {

    bool audio_ok = audio_init() == ESP_OK;
    if (audio_ok) {
        const int notes[] = {523, 659, 784};
        for (int i = 0; i < 3; i++) {
            audio_play_tone(notes[i], 120);
        }
    } else {
        ESP_LOGE(TAG, "Error con el audio. Se continuará sin él");
    }

    return audio_ok;

}

void app_main(void)
{
    wifi_ap_start();
    init();
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(httpd_start(&s_server, &cfg));
    const httpd_uri_t page = {.uri = "/", .method = HTTP_GET, .handler = page_handler};
    const httpd_uri_t ws = {.uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .is_websocket = true};
    ESP_ERROR_CHECK(httpd_register_uri_handler(s_server, &page));
    ESP_ERROR_CHECK(httpd_register_uri_handler(s_server, &ws));
    ESP_LOGI(TAG, "join Wi-Fi '%s' (password '%s') and open http://192.168.4.1", WIFI_SSID, WIFI_PASS);

    for (int i = 0; i < 4; i++) {
        gpio_set_direction(s_keys[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(s_keys[i], GPIO_PULLUP_ONLY);
    }

    bool audio_ok = initial_audio_check();

    int last = -1; 
    while (1) {
        int req = file_requested;
        if (req != last) { //Solo redibuja si hay un cambio en el archivo solicitado
            last = req;
            display_clear();
            switch (req) {
                case FILE_1: init_square_game(); break;
                case FILE_2: init_scream_game(audio_ok); break; 
                case FILE_3: init_cpn_game(); break;
                default:     display_text(10, 8, "Esperando archivo!", false); break;
            }
        display_flush();
        }

        switch (req) {
            case FILE_1: update_square_game_state(display_ok); break;
            case FILE_2: update_scream_game_state(display_ok); break;
            case FILE_3: update_cpn_game_state(display_ok); break;
            default: display_waiting_animation(10); break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
