#ifndef LVGL_UI_H
#define LVGL_UI_H

#include <lvgl.h>

// -------------------------------------------------------------
// Global UI Nesneleri ve Veriler
// -------------------------------------------------------------
lv_obj_t * arc_target_temp;
lv_obj_t * label_target_temp;
lv_obj_t * label_current_temp;
lv_obj_t * label_humidity;
lv_obj_t * label_pressure;
lv_obj_t * label_gas;
lv_obj_t * label_ble;
lv_obj_t * label_wifi;

// Başlangıç Hedef Sıcaklığı
float target_temp_val = 24.0f;

// Sistem Modu (0: Isıtma, 1: Soğutma, 2: Fan, 3: Kapat)
uint8_t current_system_mode = 0;
bool system_mode_changed = false;
bool settings_need_save = false; // EEPROM'a yazma bayrağı

// Yeni UI Nesneleri (Grafik ve Ayarlar)
lv_obj_t * chart;
lv_chart_series_t * ser_temp;
lv_obj_t * slider_hysteresis;
lv_obj_t * label_hysteresis;

float hysteresis_val = 0.5f;
bool hysteresis_changed = false;

// Wi-Fi Manager Variables
bool wifi_scan_requested = false;
bool wifi_credentials_ready = false;
String selected_ssid = "";
String entered_password = "";

lv_obj_t * wifi_modal = NULL;
lv_obj_t * wifi_list = NULL;
lv_obj_t * wifi_kb = NULL;
lv_obj_t * wifi_ta = NULL;
lv_obj_t * wifi_status_label = NULL;


// Ortak Stiller
static lv_style_t style_bg;
static lv_style_t style_btn;
static lv_style_t style_card;

// -------------------------------------------------------------
// Veri Güncelleme Fonksiyonu (BME680 sensör döngüsünden çağrılır)
// -------------------------------------------------------------
void update_bme680_ui(float temp, float humidity, float pressure, float gas_res) {
    if (label_current_temp) {
        String t = String(temp, 1) + "°C";
        lv_label_set_text(label_current_temp, t.c_str());
    }
    if (label_humidity) {
        String h = String(LV_SYMBOL_TINT) + " " + String(humidity, 1) + "%";
        lv_label_set_text(label_humidity, h.c_str());
    }
    if (label_pressure) {
        String p = String(pressure, 0) + " hPa";
        lv_label_set_text(label_pressure, p.c_str());
    }
    if (label_gas) {
        // Direnci ohm'dan kOhm'a çevirerek göster
        String g = String(gas_res / 1000.0f, 1) + " kOhm";
        lv_label_set_text(label_gas, g.c_str());
    }
}

void update_ble_status_ui(bool connected) {
    if (label_ble) {
        if (connected) {
            lv_obj_set_style_text_color(label_ble, lv_color_hex(0x2196F3), 0); // Mavi (Bağlı)
        } else {
            lv_obj_set_style_text_color(label_ble, lv_color_hex(0x888888), 0); // Gri (Bağlı değil)
        }
    }
}

void update_wifi_status_ui(bool connected) {
    if (label_wifi) {
        if (connected) {
            lv_obj_set_style_text_color(label_wifi, lv_color_hex(0x4CAF50), 0); // Yeşil (Bağlı)
        } else {
            lv_obj_set_style_text_color(label_wifi, lv_color_hex(0x888888), 0); // Gri (Bağlı değil)
        }
    }
}

// -------------------------------------------------------------
// Olay Geri Çağrıları (Event Callbacks)
// -------------------------------------------------------------

// Arc (Kaydırıcı) değiştirildiğinde tetiklenir (Sürekli çalışır)
static void arc_event_cb(lv_event_t * e) {
    lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
    // Range: 150-300 = 15.0 - 30.0 °C
    int value = lv_arc_get_value(arc);
    target_temp_val = value / 10.0f;
    String t = "Hedef: " + String(target_temp_val, 1) + "°C";
    lv_label_set_text(label_target_temp, t.c_str());
}

// Arc bırakıldığında (Flash ömrünü korumak için sadece bırakınca kaydet)
static void arc_release_cb(lv_event_t * e) {
    settings_need_save = true;
}

// Eksi (-) butonuna basıldığında tetiklenir
static void btn_minus_event_cb(lv_event_t * e) {
    if (target_temp_val > 15.0f) {
        target_temp_val -= 0.5f;
        lv_arc_set_value(arc_target_temp, (int)(target_temp_val * 10));
        String t = "Hedef: " + String(target_temp_val, 1) + "°C";
        lv_label_set_text(label_target_temp, t.c_str());
        settings_need_save = true;
    }
}

// Artı (+) butonuna basıldığında tetiklenir
static void btn_plus_event_cb(lv_event_t * e) {
    if (target_temp_val < 30.0f) {
        target_temp_val += 0.5f;
        lv_arc_set_value(arc_target_temp, (int)(target_temp_val * 10));
        String t = "Hedef: " + String(target_temp_val, 1) + "°C";
        lv_label_set_text(label_target_temp, t.c_str());
        settings_need_save = true;
    }
}

// Mod butonlarına tıklandığında tetiklenir
static void btn_mode_event_cb(lv_event_t * e) {
    lv_obj_t * clicked_btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * parent = lv_obj_get_parent(clicked_btn);
    
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(parent, i);
        if(child == clicked_btn) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFF5722), 0);
            if(current_system_mode != i) {
                current_system_mode = i;
                system_mode_changed = true;
                settings_need_save = true;
            }
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2A2A2A), 0);
        }
    }
}

// Histerezis Slider Callback
static void slider_hysteresis_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    hysteresis_val = value / 10.0f;
    String t = "Tolerans: +-" + String(hysteresis_val, 1) + "°C";
    lv_label_set_text(label_hysteresis, t.c_str());
}

static void slider_release_cb(lv_event_t * e) {
    hysteresis_changed = true;
    settings_need_save = true;
}

static void wifi_cancel_cb(lv_event_t * e) {
    if (wifi_modal) {
        lv_obj_del(wifi_modal);
        wifi_modal = NULL;
    }
}

static void wifi_kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_READY) {
        entered_password = String(lv_textarea_get_text(wifi_ta));
        wifi_credentials_ready = true;
        if (wifi_modal) {
            lv_obj_del(wifi_modal);
            wifi_modal = NULL;
        }
    } else if(code == LV_EVENT_CANCEL) {
        wifi_cancel_cb(e);
    }
}

static void wifi_list_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        selected_ssid = String(lv_list_get_btn_text(wifi_list, obj));
        // Listeyi gizle, klavye ve metin kutusunu göster
        lv_obj_add_flag(wifi_list, LV_OBJ_FLAG_HIDDEN);
        
        wifi_ta = lv_textarea_create(wifi_modal);
        lv_obj_set_size(wifi_ta, 400, 60);
        lv_obj_align(wifi_ta, LV_ALIGN_TOP_MID, 0, 100);
        lv_textarea_set_placeholder_text(wifi_ta, "Sifre girin...");
        lv_textarea_set_password_mode(wifi_ta, false);
        lv_textarea_set_one_line(wifi_ta, true);
        
        wifi_kb = lv_keyboard_create(wifi_modal);
        lv_obj_set_size(wifi_kb, 480, 400);
        lv_obj_align(wifi_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_keyboard_set_textarea(wifi_kb, wifi_ta);
        lv_obj_add_event_cb(wifi_kb, wifi_kb_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_add_event_cb(wifi_kb, wifi_kb_event_cb, LV_EVENT_READY, NULL);
        lv_obj_add_event_cb(wifi_kb, wifi_kb_event_cb, LV_EVENT_CANCEL, NULL);
    }
}

void show_wifi_scan_modal() {
    wifi_modal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wifi_modal, 480, 800);
    lv_obj_set_style_bg_color(wifi_modal, lv_color_hex(0x121212), 0);
    lv_obj_set_style_bg_opa(wifi_modal, LV_OPA_90, 0);
    
    wifi_status_label = lv_label_create(wifi_modal);
    lv_label_set_text(wifi_status_label, "Aglari Taraniyor...");
    lv_obj_center(wifi_status_label);
}

static void btn_wifi_scan_cb(lv_event_t * e) {
    wifi_scan_requested = true;
    show_wifi_scan_modal();
}

void populate_wifi_list(int count, String* ssids) {
    if(!wifi_modal) return;
    
    lv_obj_del(wifi_status_label);
    
    wifi_list = lv_list_create(wifi_modal);
    lv_obj_set_size(wifi_list, 400, 600);
    lv_obj_center(wifi_list);
    
    for(int i = 0; i < count; i++) {
        lv_obj_t * btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, ssids[i].c_str());
        lv_obj_add_event_cb(btn, wifi_list_event_cb, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_t * btn_close = lv_list_add_btn(wifi_list, LV_SYMBOL_CLOSE, "Iptal");
    lv_obj_add_event_cb(btn_close, wifi_cancel_cb, LV_EVENT_CLICKED, NULL);
}

// -------------------------------------------------------------
// Ana UI Oluşturma Fonksiyonu
// -------------------------------------------------------------
void create_thermostat_ui(void) {
    // 1. Tema ve Arka Plan Ayarları (Tam Siyah #121212)
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_hex(0x121212));
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    lv_style_set_text_color(&style_bg, lv_color_hex(0xFFFFFF));
    lv_style_set_border_width(&style_bg, 0);
    
    lv_obj_t * scr = lv_scr_act();
    lv_obj_add_style(scr, &style_bg, 0);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    // 2. Üst Bar (Header - Y: 0-60px)
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, 480, 60);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1E1E1E), 0); 
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
    
    // Header elemanları
    lv_obj_t * label_room = lv_label_create(header);
    lv_label_set_text(label_room, "Oturma Odasi");
    lv_obj_align(label_room, LV_ALIGN_LEFT_MID, 10, 0);
    
    lv_obj_t * label_time = lv_label_create(header);
    lv_label_set_text(label_time, "<   > Meni"); // Kaydırma ipucu
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
    
    label_ble = lv_label_create(header);
    lv_label_set_text(label_ble, LV_SYMBOL_BLUETOOTH);
    lv_obj_align(label_ble, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_color(label_ble, lv_color_hex(0x888888), 0); // Başlangıçta gri
    
    label_wifi = lv_label_create(header);
    lv_label_set_text(label_wifi, LV_SYMBOL_WIFI);
    lv_obj_align(label_wifi, LV_ALIGN_RIGHT_MID, -45, 0);
    lv_obj_set_style_text_color(label_wifi, lv_color_hex(0x888888), 0); // Başlangıçta gri

    // ==========================================
    // TILEVIEW OLUŞTURMA
    // ==========================================
    lv_obj_t * tv = lv_tileview_create(scr);
    lv_obj_set_size(tv, 480, 740);
    lv_obj_align(tv, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(tv, lv_color_hex(0x121212), 0);
    lv_obj_set_style_text_color(tv, lv_color_hex(0xFFFFFF), 0); // Varsayılan yazı rengi beyaz olsun

    lv_obj_t * tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT); // Sola kaydırılabilir
    lv_obj_t * tile2 = lv_tileview_add_tile(tv, 1, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    lv_obj_t * tile3 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT);

    // ==========================================
    // TILE 1: ANA EKRAN
    // ==========================================
    // 3. Ana Merkez (Sıcaklık Kadranı - Arc)
    arc_target_temp = lv_arc_create(tile1);
    lv_obj_set_size(arc_target_temp, 340, 340);
    // Ekranın üst-orta kısmına hizala (Tile 1 için Y=30 uygun)
    lv_obj_align(arc_target_temp, LV_ALIGN_TOP_MID, 0, 30); 
    
    lv_arc_set_range(arc_target_temp, 150, 300); // 15.0 ile 30.0 °C
    lv_arc_set_value(arc_target_temp, (int)(target_temp_val * 10));
    
    lv_obj_set_style_arc_width(arc_target_temp, 22, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_target_temp, lv_color_hex(0x333333), LV_PART_MAIN);
    
    lv_obj_set_style_arc_width(arc_target_temp, 22, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_target_temp, lv_color_hex(0xFF5722), LV_PART_INDICATOR); // Turuncu
    
    lv_obj_set_style_bg_color(arc_target_temp, lv_color_hex(0xFF5722), LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc_target_temp, 12, LV_PART_KNOB);
    
    // LVGL Mavi focus çizgisini (outline) gizle
    lv_obj_set_style_outline_width(arc_target_temp, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(arc_target_temp, 0, LV_PART_INDICATOR | LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(arc_target_temp, 0, LV_PART_KNOB | LV_STATE_FOCUSED);

    lv_obj_add_event_cb(arc_target_temp, arc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(arc_target_temp, arc_release_cb, LV_EVENT_RELEASED, NULL);

    // Merkezdeki ANLIK GERÇEK Sıcaklık Yazısı (Büyük Font)
    label_current_temp = lv_label_create(tile1);
    lv_label_set_text(label_current_temp, "--.-°C"); // İlk değer (Sensör gelene kadar)
    lv_obj_align_to(label_current_temp, arc_target_temp, LV_ALIGN_CENTER, 0, -15);
    
    // Merkezdeki Hedef Sıcaklık Yazısı
    label_target_temp = lv_label_create(tile1);
    lv_obj_set_style_text_color(label_target_temp, lv_color_hex(0xFF5722), 0); // Turuncu vurgu
    String t_hedef = "Hedef: " + String(target_temp_val, 1) + "°C";
    lv_label_set_text(label_target_temp, t_hedef.c_str());
    lv_obj_align_to(label_target_temp, arc_target_temp, LV_ALIGN_CENTER, 0, 35);

    // 4. İnce Ayar Butonları (- ve +)
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x2A2A2A));
    lv_style_set_radius(&style_btn, LV_RADIUS_CIRCLE);
    lv_style_set_text_color(&style_btn, lv_color_hex(0xFFFFFF));
    lv_style_set_outline_width(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);

    lv_obj_t * btn_minus = lv_btn_create(tile1);
    lv_obj_set_size(btn_minus, 70, 70);
    lv_obj_add_style(btn_minus, &style_btn, 0);
    // Arc'ın altına hizala
    lv_obj_align_to(btn_minus, arc_target_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 30);
    lv_obj_t * label_minus = lv_label_create(btn_minus);
    lv_label_set_text(label_minus, LV_SYMBOL_MINUS);
    lv_obj_center(label_minus);
    lv_obj_add_event_cb(btn_minus, btn_minus_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_plus = lv_btn_create(tile1);
    lv_obj_set_size(btn_plus, 70, 70);
    lv_obj_add_style(btn_plus, &style_btn, 0);
    lv_obj_align_to(btn_plus, arc_target_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, -50, 30);
    lv_obj_t * label_plus = lv_label_create(btn_plus);
    lv_label_set_text(label_plus, LV_SYMBOL_PLUS);
    lv_obj_center(label_plus);
    lv_obj_add_event_cb(btn_plus, btn_plus_event_cb, LV_EVENT_CLICKED, NULL);

    // 5. BME680 Sensör Paneli (Dashboard)
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(0x1A1A1A)); // Hafif aydınlık siyah
    lv_style_set_text_color(&style_card, lv_color_hex(0xFFFFFF)); // Yazı rengi beyaz olsun
    lv_style_set_radius(&style_card, 15);
    lv_style_set_border_width(&style_card, 0);
    lv_style_set_pad_all(&style_card, 10);
    
    lv_obj_t * dashboard = lv_obj_create(tile1);
    lv_obj_set_size(dashboard, 460, 100);
    lv_obj_align(dashboard, LV_ALIGN_TOP_MID, 0, 500); // Tile içinde biraz yukarı kaydırdık
    lv_obj_set_style_bg_color(dashboard, lv_color_hex(0x121212), 0); // Kapsayıcı arka planı görünmez gibi
    lv_obj_set_style_border_width(dashboard, 0, 0);
    lv_obj_set_scrollbar_mode(dashboard, LV_SCROLLBAR_MODE_OFF);
    
    // Grid veya Flex... Flex daha kolay ve responsive
    lv_obj_set_layout(dashboard, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dashboard, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dashboard, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Nem Kartı
    lv_obj_t * card_hum = lv_obj_create(dashboard);
    lv_obj_set_size(card_hum, 135, 75);
    lv_obj_add_style(card_hum, &style_card, 0);
    lv_obj_set_scrollbar_mode(card_hum, LV_SCROLLBAR_MODE_OFF);
    label_humidity = lv_label_create(card_hum);
    lv_label_set_text(label_humidity, LV_SYMBOL_TINT " -- %");
    lv_obj_center(label_humidity);

    // Basınç Kartı
    lv_obj_t * card_press = lv_obj_create(dashboard);
    lv_obj_set_size(card_press, 135, 75);
    lv_obj_add_style(card_press, &style_card, 0);
    lv_obj_set_scrollbar_mode(card_press, LV_SCROLLBAR_MODE_OFF);
    label_pressure = lv_label_create(card_press);
    lv_label_set_text(label_pressure, "-- hPa");
    lv_obj_center(label_pressure);

    // Gaz (IAQ) Kartı
    lv_obj_t * card_gas = lv_obj_create(dashboard);
    lv_obj_set_size(card_gas, 135, 75);
    lv_obj_add_style(card_gas, &style_card, 0);
    lv_obj_set_scrollbar_mode(card_gas, LV_SCROLLBAR_MODE_OFF);
    label_gas = lv_label_create(card_gas);
    lv_label_set_text(label_gas, "-- kOhm");
    lv_obj_center(label_gas);

    // 6. Alt Kontrol Paneli (Modlar)
    lv_obj_t * bottom_panel = lv_obj_create(tile1);
    lv_obj_set_size(bottom_panel, 480, 100);
    lv_obj_align(bottom_panel, LV_ALIGN_BOTTOM_MID, 0, 0); // En alta hizala
    lv_obj_set_style_bg_color(bottom_panel, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_border_width(bottom_panel, 0, 0);
    lv_obj_set_style_radius(bottom_panel, 0, 0);
    lv_obj_set_scrollbar_mode(bottom_panel, LV_SCROLLBAR_MODE_OFF);
    
    // Flex Layout ile butonları diz
    lv_obj_set_layout(bottom_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bottom_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_panel, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const char * modes[] = {"Isitma", "Sogutma", "Fan", "Kapat"};
    for(int i = 0; i < 4; i++) {
        lv_obj_t * btn = lv_btn_create(bottom_panel);
        lv_obj_set_size(btn, 100, 60);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_set_style_outline_width(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        
        // Isıtma modunu aktif göster
        if(i == current_system_mode) { 
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5722), 0);
        }

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, modes[i]);
        lv_obj_center(label);
        
        lv_obj_add_event_cb(btn, btn_mode_event_cb, LV_EVENT_CLICKED, NULL);
    }

    // ==========================================
    // TILE 2: GRAFİK EKRANI
    // ==========================================
    lv_obj_t * label_chart_title = lv_label_create(tile2);
    lv_label_set_text(label_chart_title, "Son Saatlerdeki Sicaklik");
    lv_obj_align(label_chart_title, LV_ALIGN_TOP_MID, 0, 20);

    chart = lv_chart_create(tile2);
    lv_obj_set_size(chart, 440, 500);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, -30);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   
    lv_chart_set_point_count(chart, 20);            
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 15, 35); 

    ser_temp = lv_chart_add_series(chart, lv_color_hex(0xFF5722), LV_CHART_AXIS_PRIMARY_Y);
    // Örnek birkaç değer atayalım (Başlangıçta boş durmasın)
    for(int i=0; i<20; i++) lv_chart_set_next_value(chart, ser_temp, 24);

    // ==========================================
    // TILE 3: AYARLAR EKRANI
    // ==========================================
    lv_obj_t * label_settings_title = lv_label_create(tile3);
    lv_label_set_text(label_settings_title, "Sistem Ayarlari");
    lv_obj_align(label_settings_title, LV_ALIGN_TOP_MID, 0, 20);

    slider_hysteresis = lv_slider_create(tile3);
    lv_obj_set_width(slider_hysteresis, 350);
    lv_obj_align(slider_hysteresis, LV_ALIGN_TOP_MID, 0, 150);
    lv_slider_set_range(slider_hysteresis, 2, 20); // 0.2 C to 2.0 C
    lv_slider_set_value(slider_hysteresis, (int)(hysteresis_val * 10), LV_ANIM_OFF);

    lv_obj_set_style_bg_color(slider_hysteresis, lv_color_hex(0xFF5722), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_hysteresis, lv_color_hex(0xFF5722), LV_PART_KNOB);

    label_hysteresis = lv_label_create(tile3);
    String th = "Tolerans (Histerezis): +-" + String(hysteresis_val, 1) + "°C";
    lv_label_set_text(label_hysteresis, th.c_str());
    lv_obj_align_to(label_hysteresis, slider_hysteresis, LV_ALIGN_OUT_TOP_MID, 0, -20);

    lv_obj_add_event_cb(slider_hysteresis, slider_hysteresis_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(slider_hysteresis, slider_release_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_t * btn_wifi = lv_btn_create(tile3);
    lv_obj_set_size(btn_wifi, 350, 60);
    lv_obj_align(btn_wifi, LV_ALIGN_TOP_MID, 0, 250);
    lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x2196F3), 0);
    
    lv_obj_t * label_wifi_btn = lv_label_create(btn_wifi);
    lv_label_set_text(label_wifi_btn, "Wi-Fi Agi Ekle");
    lv_obj_center(label_wifi_btn);
    
    lv_obj_add_event_cb(btn_wifi, btn_wifi_scan_cb, LV_EVENT_CLICKED, NULL);
}

#endif // LVGL_UI_H
