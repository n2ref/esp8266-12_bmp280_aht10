#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHT10.h>
#include <Wire.h>
#include <math.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "OneButton.h"


#define PIN_RESET_BUTTON   13
#define PIN_PHOTO_RESISTOR A0
#define PIN_SDA            4
#define PIN_SCL            5
#define PIN_LED            2

#define UNIQUE_NUM              "03"
#define WIFI_NAME               "SENSOR_" UNIQUE_NUM
#define WIFI_PASS               NULL
#define RESET_BTN_DELAY         3000
#define SEA_LEVEL_PRESSURE_HPA (1013.25)
#define PHOTO_RESISTOR_MIN      25
#define PHOTO_RESISTOR_MAX      800



ESP8266WebServer  http(80);
WiFiManager       wifiManager;
OneButton         resetBtn(PIN_RESET_BUTTON, true);
Adafruit_BMP280   bmp;
Adafruit_AHT10    aht;





float         temperature       = 0;
float         humidity          = 0;
float         pressure          = 0;
float         pressureMM        = 0;
float         altitude          = 0;
int           illumination      = 0;
int           wifiSignal        = 0;
int           wifiSignalPercent = 0;
unsigned long sensorsMillis     = 0;
unsigned long sensorsInterval   = 10000;
unsigned long ledMillis         = 0;
unsigned long ledInterval       = 5000;
bool          ledState          = false;



/**
 * Инициализация wifi менеджера
 */
void initWifi() {

    Serial.println("initWifi");

    //Set new hostname
    WiFi.hostname(WIFI_NAME);

    //Assign fixed IP
    wifiManager.setAPStaticIPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
    wifiManager.autoConnect(WIFI_NAME, WIFI_PASS);
}


/**
 * Инициализация страниц web сервера
 */
void initWebServer() {

    Serial.println("initWebServer");

    http.on("/", HTTP_GET, []() {

        // HTML & CSS contents which display on web server
        http.send(200, "text/html",
                  "<!DOCTYPE html>" +
                  (String)"<html class=\"h-100\">" +
                  "<head>" +
                  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">" +
                  "<meta content=\"width=device-width, initial-scale=1\" name=\"viewport\">" +
                  "<meta name=\"theme-color\" content=\"#212529\">" +
                  "<title>Датчик " + (String)UNIQUE_NUM + "</title>" +
                  "<style>:root{--bs-blue:#0d6efd;--bs-indigo:#6610f2;--bs-purple:#6f42c1;--bs-pink:#d63384;--bs-red:#dc3545;--bs-orange:#fd7e14;--bs-yellow:#ffc107;--bs-green:#198754;--bs-teal:#20c997;--bs-cyan:#0dcaf0;--bs-white:#fff;--bs-gray:#6c757d;--bs-gray-dark:#343a40;--bs-primary:#0d6efd;--bs-secondary:#6c757d;--bs-success:#198754;--bs-info:#0dcaf0;--bs-warning:#ffc107;--bs-danger:#dc3545;--bs-light:#f8f9fa;--bs-dark:#212529;--bs-font-sans-serif:system-ui,-apple-system,\"Segoe UI\",Roboto,\"Helvetica Neue\",Arial,\"Noto Sans\",\"Liberation Sans\",sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\",\"Noto Color Emoji\";--bs-font-monospace:SFMono-Regular,Menlo,Monaco,Consolas,\"Liberation Mono\",\"Courier New\",monospace;--bs-gradient:linear-gradient(180deg, rgba(255, 255, 255, 0.15), rgba(255, 255, 255, 0))}*,::after,::before{box-sizing:border-box}@media (prefers-reduced-motion:no-preference){:root{scroll-behavior:smooth}}body{margin:0;font-family:var(--bs-font-sans-serif);font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#fff;-webkit-text-size-adjust:100%;-webkit-tap-highlight-color:transparent}.h1,.h2,.h3,.h4,.h5,.h6,h1,h2,h3,h4,h5,h6{margin-top:0;margin-bottom:.5rem;font-weight:500;line-height:1.2}.h1,h1{font-size:calc(1.375rem + 1.5vw)}@media (min-width:1200px){.h1,h1{font-size:2.5rem}}.h2,h2{font-size:calc(1.325rem + .9vw)}@media (min-width:1200px){.h2,h2{font-size:2rem}}.h3,h3{font-size:calc(1.3rem + .6vw)}@media (min-width:1200px){.h3,h3{font-size:1.75rem}}.h4,h4{font-size:calc(1.275rem + .3vw)}@media (min-width:1200px){.h4,h4{font-size:1.5rem}}.h5,h5{font-size:1.25rem}.h6,h6{font-size:1rem}p{margin-top:0;margin-bottom:1rem}ol,ul{padding-left:2rem}dl,ol,ul{margin-top:0;margin-bottom:1rem}ol ol,ol ul,ul ol,ul ul{margin-bottom:0}b,strong{font-weight:bolder}.small,small{font-size:.875em}a{color:#0d6efd;text-decoration:underline}a:hover{color:#0a58ca}a:not([href]):not([class]),a:not([href]):not([class]):hover{color:inherit;text-decoration:none}::-moz-focus-inner{padding:0;border-style:none}iframe{border:0}[hidden]{display:none!important}.display-1{font-size:calc(1.625rem + 4.5vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-1{font-size:5rem}}.display-2{font-size:calc(1.575rem + 3.9vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-2{font-size:4.5rem}}.display-3{font-size:calc(1.525rem + 3.3vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-3{font-size:4rem}}.display-4{font-size:calc(1.475rem + 2.7vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-4{font-size:3.5rem}}.display-5{font-size:calc(1.425rem + 2.1vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-5{font-size:3rem}}.display-6{font-size:calc(1.375rem + 1.5vw);font-weight:300;line-height:1.2}@media (min-width:1200px){.display-6{font-size:2.5rem}}.container,.container-fluid,.container-lg,.container-md,.container-sm,.container-xl,.container-xxl{width:100%;padding-right:var(--bs-gutter-x,.75rem);padding-left:var(--bs-gutter-x,.75rem);margin-right:auto;margin-left:auto}@media (min-width:576px){.container,.container-sm{max-width:540px}}@media (min-width:768px){.container,.container-md,.container-sm{max-width:720px}}@media (min-width:992px){.container,.container-lg,.container-md,.container-sm{max-width:960px}}@media (min-width:1200px){.container,.container-lg,.container-md,.container-sm,.container-xl{max-width:1140px}}@media (min-width:1400px){.container,.container-lg,.container-md,.container-sm,.container-xl,.container-xxl{max-width:1320px}}.row{--bs-gutter-x:1.5rem;--bs-gutter-y:0;display:flex;flex-wrap:wrap;margin-top:calc(var(--bs-gutter-y) * -1);margin-right:calc(var(--bs-gutter-x) * -.5);margin-left:calc(var(--bs-gutter-x) * -.5)}.row>*{flex-shrink:0;width:100%;max-width:100%;padding-right:calc(var(--bs-gutter-x) * .5);padding-left:calc(var(--bs-gutter-x) * .5);margin-top:var(--bs-gutter-y)}.col{flex:1 0 0%}.d-inline{display:inline!important}.d-inline-block{display:inline-block!important}.d-block{display:block!important}.d-grid{display:grid!important}.d-table{display:table!important}.d-table-row{display:table-row!important}.d-table-cell{display:table-cell!important}.d-flex{display:flex!important}.d-inline-flex{display:inline-flex!important}.d-none{display:none!important}@media (min-width:576px){.col-sm{flex:1 0 0%}}@media (min-width:768px){.col-md{flex:1 0 0%}}@media (min-width:992px){.col-lg{flex:1 0 0%}}@media (min-width:1200px){.col-xl{flex:1 0 0%}}@media (min-width:1400px){.col-xxl{flex:1 0 0%}}.col-auto{flex:0 0 auto;width:auto}.col-1{flex:0 0 auto;width:8.33333333%}.col-2{flex:0 0 auto;width:16.66666667%}.col-3{flex:0 0 auto;width:25%}.col-4{flex:0 0 auto;width:33.33333333%}.col-5{flex:0 0 auto;width:41.66666667%}.col-6{flex:0 0 auto;width:50%}.col-7{flex:0 0 auto;width:58.33333333%}.col-8{flex:0 0 auto;width:66.66666667%}.col-9{flex:0 0 auto;width:75%}.col-10{flex:0 0 auto;width:83.33333333%}.col-11{flex:0 0 auto;width:91.66666667%}.col-12{flex:0 0 auto;width:100%}@media (min-width:576px){.col-sm-auto{flex:0 0 auto;width:auto}.col-sm-1{flex:0 0 auto;width:8.33333333%}.col-sm-2{flex:0 0 auto;width:16.66666667%}.col-sm-3{flex:0 0 auto;width:25%}.col-sm-4{flex:0 0 auto;width:33.33333333%}.col-sm-5{flex:0 0 auto;width:41.66666667%}.col-sm-6{flex:0 0 auto;width:50%}.col-sm-7{flex:0 0 auto;width:58.33333333%}.col-sm-8{flex:0 0 auto;width:66.66666667%}.col-sm-9{flex:0 0 auto;width:75%}.col-sm-10{flex:0 0 auto;width:83.33333333%}.col-sm-11{flex:0 0 auto;width:91.66666667%}.col-sm-12{flex:0 0 auto;width:100%}}@media (min-width:768px){.col-md-auto{flex:0 0 auto;width:auto}.col-md-1{flex:0 0 auto;width:8.33333333%}.col-md-2{flex:0 0 auto;width:16.66666667%}.col-md-3{flex:0 0 auto;width:25%}.col-md-4{flex:0 0 auto;width:33.33333333%}.col-md-5{flex:0 0 auto;width:41.66666667%}.col-md-6{flex:0 0 auto;width:50%}.col-md-7{flex:0 0 auto;width:58.33333333%}.col-md-8{flex:0 0 auto;width:66.66666667%}.col-md-9{flex:0 0 auto;width:75%}.col-md-10{flex:0 0 auto;width:83.33333333%}.col-md-11{flex:0 0 auto;width:91.66666667%}.col-md-12{flex:0 0 auto;width:100%}}.nav{display:flex;flex-wrap:wrap;padding-left:0;margin-bottom:0;list-style:none}.nav-fill .nav-item,.nav-fill>.nav-link{flex:1 1 auto;text-align:center}.nav-justified .nav-item,.nav-justified>.nav-link{flex-basis:0;flex-grow:1;text-align:center}.nav-fill .nav-item .nav-link,.nav-justified .nav-item .nav-link{width:100%}.navbar{position:relative;display:flex;flex-wrap:wrap;align-items:center;justify-content:space-between;padding-top:.5rem;padding-bottom:.5rem}.navbar>.container,.navbar>.container-fluid,.navbar>.container-lg,.navbar>.container-md,.navbar>.container-sm,.navbar>.container-xl,.navbar>.container-xxl{display:flex;flex-wrap:inherit;align-items:center;justify-content:space-between}.navbar-brand{padding-top:.3125rem;padding-bottom:.3125rem;margin-right:1rem;font-size:1.25rem;text-decoration:none;white-space:nowrap}.navbar-nav{display:flex;flex-direction:column;padding-left:0;margin-bottom:0;list-style:none}.navbar-nav .nav-link{padding-right:0;padding-left:0}.navbar-nav .dropdown-menu{position:static}.navbar-text{padding-top:.5rem;padding-bottom:.5rem}.navbar-dark .navbar-brand{color:#fff}.navbar-dark .navbar-brand:focus,.navbar-dark .navbar-brand:hover{color:#fff}.navbar-dark .navbar-nav .nav-link{color:rgba(255,255,255,.55)}.navbar-dark .navbar-nav .nav-link:focus,.navbar-dark .navbar-nav .nav-link:hover{color:rgba(255,255,255,.75)}.navbar-dark .navbar-nav .nav-link.disabled{color:rgba(255,255,255,.25)}.navbar-dark .navbar-nav .nav-link.active,.navbar-dark .navbar-nav .show>.nav-link{color:#fff}.navbar-dark .navbar-text{color:rgba(255,255,255,.55)}.navbar-dark .navbar-text a,.navbar-dark .navbar-text a:focus,.navbar-dark .navbar-text a:hover{color:#fff}.card{position:relative;display:flex;flex-direction:column;min-width:0;word-wrap:break-word;background-color:#fff;background-clip:border-box;border:1px solid rgba(0,0,0,.125);border-radius:.25rem}.card-body{flex:1 1 auto;padding:1rem 1rem}.card-text:last-child{margin-bottom:0}.clearfix::after{display:block;clear:both;content:\"\"}.w-25{width:25%!important}.w-50{width:50%!important}.w-75{width:75%!important}.w-100{width:100%!important}.w-auto{width:auto!important}.mw-100{max-width:100%!important}.vw-100{width:100vw!important}.min-vw-100{min-width:100vw!important}.h-25{height:25%!important}.h-50{height:50%!important}.h-75{height:75%!important}.h-100{height:100%!important}.h-auto{height:auto!important}.mh-100{max-height:100%!important}.vh-100{height:100vh!important}.min-vh-100{min-height:100vh!important}.flex-column{flex-direction:column!important}.flex-column-reverse{flex-direction:column-reverse!important}.flex-shrink-0{flex-shrink:0!important}.flex-shrink-1{flex-shrink:1!important}.justify-content-center{justify-content:center!important}.mx-0{margin-right:0!important;margin-left:0!important}.mx-1{margin-right:.25rem!important;margin-left:.25rem!important}.mx-2{margin-right:.5rem!important;margin-left:.5rem!important}.mx-3{margin-right:1rem!important;margin-left:1rem!important}.mx-4{margin-right:1.5rem!important;margin-left:1.5rem!important}.mx-5{margin-right:3rem!important;margin-left:3rem!important}.mx-auto{margin-right:auto!important;margin-left:auto!important}.my-0{margin-top:0!important;margin-bottom:0!important}.my-1{margin-top:.25rem!important;margin-bottom:.25rem!important}.my-2{margin-top:.5rem!important;margin-bottom:.5rem!important}.my-3{margin-top:1rem!important;margin-bottom:1rem!important}.my-4{margin-top:1.5rem!important;margin-bottom:1.5rem!important}.my-5{margin-top:3rem!important;margin-bottom:3rem!important}.my-auto{margin-top:auto!important;margin-bottom:auto!important}.mt-0{margin-top:0!important}.mt-1{margin-top:.25rem!important}.mt-2{margin-top:.5rem!important}.mt-3{margin-top:1rem!important}.mt-4{margin-top:1.5rem!important}.mt-5{margin-top:3rem!important}.mt-auto{margin-top:auto!important}.me-0{margin-right:0!important}.me-1{margin-right:.25rem!important}.me-2{margin-right:.5rem!important}.me-3{margin-right:1rem!important}.me-4{margin-right:1.5rem!important}.me-5{margin-right:3rem!important}.me-auto{margin-right:auto!important}.mb-0{margin-bottom:0!important}.mb-1{margin-bottom:.25rem!important}.mb-2{margin-bottom:.5rem!important}.mb-3{margin-bottom:1rem!important}.mb-4{margin-bottom:1.5rem!important}.mb-5{margin-bottom:3rem!important}.mb-auto{margin-bottom:auto!important}.ms-0{margin-left:0!important}.ms-1{margin-left:.25rem!important}.ms-2{margin-left:.5rem!important}.ms-3{margin-left:1rem!important}.ms-4{margin-left:1.5rem!important}.ms-5{margin-left:3rem!important}.ms-auto{margin-left:auto!important}.py-0{padding-top:0!important;padding-bottom:0!important}.py-1{padding-top:.25rem!important;padding-bottom:.25rem!important}.py-2{padding-top:.5rem!important;padding-bottom:.5rem!important}.py-3{padding-top:1rem!important;padding-bottom:1rem!important}.py-4{padding-top:1.5rem!important;padding-bottom:1.5rem!important}.py-5{padding-top:3rem!important;padding-bottom:3rem!important}.pt-0{padding-top:0!important}.pt-1{padding-top:.25rem!important}.pt-2{padding-top:.5rem!important}.pt-3{padding-top:1rem!important}.pt-4{padding-top:1.5rem!important}.pt-5{padding-top:3rem!important}.pe-0{padding-right:0!important}.pe-1{padding-right:.25rem!important}.pe-2{padding-right:.5rem!important}.pe-3{padding-right:1rem!important}.pe-4{padding-right:1.5rem!important}.pe-5{padding-right:3rem!important}.pb-0{padding-bottom:0!important}.pb-1{padding-bottom:.25rem!important}.pb-2{padding-bottom:.5rem!important}.pb-3{padding-bottom:1rem!important}.pb-4{padding-bottom:1.5rem!important}.pb-5{padding-bottom:3rem!important}.ps-0{padding-left:0!important}.ps-1{padding-left:.25rem!important}.ps-2{padding-left:.5rem!important}.ps-3{padding-left:1rem!important}.ps-4{padding-left:1.5rem!important}.ps-5{padding-left:3rem!important}.text-muted{color:#6c757d!important}.bg-light{background-color:#f8f9fa!important}.bg-dark{background-color:#212529!important}</style>" +
                  "<style>main>.container{padding:60px 15px 0}</style>" +
                  "<script>function httpGetAsync(e,t){var n=new XMLHttpRequest;n.onreadystatechange=function(){4===n.readyState&&200===n.status&&t(n.responseText)},n.open(\"GET\",e,!0),n.send(null)}setInterval(function(){httpGetAsync(\"/metrics.json\",function(e){try{var t=JSON.parse(e),n=document.querySelector(\"#sensor-temperature .number\"),r=document.querySelector(\"#sensor-humidity .number\"),o=document.querySelector(\"#sensor-pressure-mm .number\"),u=document.querySelector(\"#sensor-pressure .number\"),s=document.querySelector(\"#sensor-altitude .number\"),c=document.querySelector(\"#sensor-illumination .number\"),i=document.querySelector(\"#sensor-wifi-signal .number\"),m=document.querySelector(\"#sensor-wifi-signal-percent .number\");n.textContent=t.temperature,r.textContent=t.humidity,o.textContent=t.pressure_mm,u.textContent=t.pressure,s.textContent=t.altitude,c.textContent=t.illumination,i.textContent=t.wifi_signal,m.textContent=t.wifi_signal_percent}catch(e){console.log(e)}})},3e3);</script>" +
                  "</head>" +
                  "<body class=\"d-flex flex-column h-100\">" +
                  "<header><nav class=\"navbar navbar-expand-md navbar-dark bg-dark\"><div class=\"container justify-content-center\"><a class=\"navbar-brand\" href=\"/\">Датчик " + (String)UNIQUE_NUM + "</a></div></nav></header>" +
                  "<main class=\"flex-shrink-0\"><div class=\"container\"><div class=\"row\"><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-temperature\"><div class=\"card-body\"><h5 class=\"card-title\">&#127777; Температура</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">градусы</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-humidity\"><div class=\"card-body\"><h5 class=\"card-title\">&#128167; Влажность</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">%</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-pressure-mm\"><div class=\"card-body\"><h5 class=\"card-title\">&#128173; Атмосферное давление</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">миллиметры ртутного столба</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-pressure\"><div class=\"card-body\"><h5 class=\"card-title\">&#128173; Атмосферное давление</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">гПа, гектопаскали</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-altitude\"><div class=\"card-body\"><h5 class=\"card-title\">&#128507; Высота над уровнем моря</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">Метры</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-illumination\"><div class=\"card-body\"><h5 class=\"card-title\">&#9728; Освещение</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">%</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-wifi-signal\"><div class=\"card-body\"><h5 class=\"card-title\">&#128246; Wifi сигнал</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">дБм, децибел относительно 1 милливатта</small></p></div></div></div><div class=\"col-sm-6 col-md-4 mb-4\"><div class=\"card\" id=\"sensor-wifi-signal-percent\"><div class=\"card-body\"><h5 class=\"card-title\">&#128246; Wifi сигнал</h5><p class=\"card-text\"><span class=\"number display-6\">0</span> <small class=\"text-muted\">%</small></p></div></div></div></div></div></main><footer class=\"footer mt-auto py-3 bg-light\"><div class=\"container\"><a href=\"/metrics\" class=\"text-muted\">/metrics</a> <a href=\"/metrics.json\" class=\"text-muted\">/metrics.json</a></div></footer>" +
                  "</body>" +
                  "</html>");
    });


    http.on("/metrics", HTTP_GET, []() {

        String html =
                (String)"# HELP sensor_temperature Temperature\n" +
                "# TYPE sensor_temperature counter\n" +
                "sensor_temperature{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)temperature + "\n" +
                "# HELP sensor_humidity Humidity\n" +
                "# TYPE sensor_humidity counter\n" +
                "sensor_humidity{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)humidity + "\n"+
                "# HELP sensor_pressure Pressure\n" +
                "# TYPE sensor_pressure counter\n" +
                "sensor_pressure{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)pressure + "\n"+
                "# HELP sensor_pressure_mm Pressure Millimeter\n" +
                "# TYPE sensor_pressure_mm counter\n" +
                "sensor_pressure_mm{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)pressureMM + "\n"+
                "# HELP sensor_altitude Altitude\n" +
                "# TYPE sensor_altitude counter\n" +
                "sensor_altitude{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)altitude + "\n"+
                "# HELP sensor_illumination Illumination\n" +
                "# TYPE sensor_illumination counter\n" +
                "sensor_illumination{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)illumination + "\n"+
                "# HELP sensor_wifi_signal Wifi Signal\n" +
                "# TYPE sensor_wifi_signal counter\n" +
                "sensor_wifi_signal{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)wifiSignal + "\n"+
                "# HELP sensor_wifi_signal_percent Wifi Signal Percent\n" +
                "# TYPE sensor_wifi_signal_percent counter\n" +
                "sensor_wifi_signal_percent{name=\"" + (String)UNIQUE_NUM + "\"} " + (String)wifiSignalPercent;

        http.send(200, "text/plain", html);
    });


    http.on("/metrics.json", HTTP_GET, []() {

        String html =
                (String)"{" +
                "\"name\": \"" + (String)UNIQUE_NUM + "\"," +
                "\"temperature\": \"" + (String)temperature + "\"," +
                "\"humidity\": \"" + (String)humidity + "\"," +
                "\"pressure\": \"" + (String)pressure + "\"," +
                "\"pressure_mm\": \"" + (String)pressureMM + "\"," +
                "\"altitude\": \"" + (String)altitude + "\"," +
                "\"illumination\": \"" + (String)illumination + "\"," +
                "\"wifi_signal\": \"" + (String)wifiSignal + "\"," +
                "\"wifi_signal_percent\": \"" + (String)wifiSignalPercent + "\"" +
                "}";

        http.send(200, "application/json", html);
    });

    http.begin();
}


/**
 * Инициализация кнопки сброса настроек wifi
 */
void initResetWifi() {

    Serial.println("initResetWifi");

    resetBtn.setPressTicks(RESET_BTN_DELAY);
    resetBtn.attachLongPressStop([](){

        wifiManager.resetSettings();
        ESP.restart();
    });

    // resetBtn.attachClick(ClickFunction);
    // resetBtn.attachDoubleClick(DoubleclickFunction);
    // resetBtn.attachLongPressStart(LongPressStartFunction);
    // resetBtn.attachDuringLongPress(LongPressFunction);
}


/**
 *  Инициализация лампочек
 */
void initLed() {

    Serial.println("initLed");

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
}


/**
 * Получение качества Wifi в процентах
 * @param RSSI
 * @return int
 */
int getRSSIasQuality(int RSSI) {
    int quality = 0;

    if (RSSI <= -100) {
        quality = 0;
    } else if (RSSI >= -50) {
        quality = 100;
    } else {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}


/**
 * Инициализация BMP280 датчика
 */
void initBMP() {

    Serial.println("initBMP");
    Wire.begin(PIN_SDA, PIN_SCL);

    if ( ! bmp.begin(0x76)) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    } else {
        Serial.println("Found BMP280 sensor");

        // Настройка режима работы датчика BMP280
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                        Adafruit_BMP280::SAMPLING_X16,
                        Adafruit_BMP280::SAMPLING_X16,
                        Adafruit_BMP280::FILTER_X16,
                        Adafruit_BMP280::STANDBY_MS_1000);
    }
    // bmp.begin(0x76);
    //bmp.begin(PIN_SDA, PIN_SCL);
}


/**
 * Инициализация ATH10 датчика
 */
void initATH10() {

    Serial.println("initATH10");
    if ( ! aht.begin()) {
        Serial.println("Could not find a valid ATH10 sensor, check wiring!");
    } else {
        Serial.println("Found ATH10 sensor");
    }
}


/**
 * Получение данных с датчиков
 */
void runSensors() {

    unsigned long currentMillis = millis();

    if (currentMillis - sensorsMillis >= sensorsInterval) {
        sensorsMillis = currentMillis;

        Serial.println("Update sensors");

        // ATH sensor
        sensors_event_t ahtHumidity, ahtTemp;
        aht.getEvent(&ahtHumidity, &ahtTemp);


        // BMP sensor
        float temperatureValue       = bmp.readTemperature();
        //float humidityValue          = bmp.readHumidity();
        float pressureValue          = bmp.readPressure() / 100.0F;
        float pressureMMValue        = pressureValue * 0.75;
        float altitudeValue          = pressureValue > 0 ? bmp.readAltitude(SEA_LEVEL_PRESSURE_HPA) : 0;
        int   illuminationValue      = analogRead(PIN_PHOTO_RESISTOR);
        int   wifiSignalValue        = WiFi.RSSI();
        int   wifiSignalPercentValue = getRSSIasQuality(WiFi.RSSI());


        float humidityValue = ahtHumidity.relative_humidity;

        if (temperatureValue == 0 && pressureValue == 0) {
            temperatureValue = ahtTemp.temperature;
        }


        // Save
        if ( ! isnan(temperatureValue)) {
            temperature = temperatureValue;
        }
        if ( ! isnan(humidityValue)) {
            humidity = humidityValue;
        }
        if ( ! isnan(pressureValue)) {
            pressure = pressureValue;
        }
        if ( ! isnan(pressureMMValue)) {
            pressureMM = pressureMMValue;
        }
        if ( ! isnan(altitudeValue)) {
            altitude = altitudeValue;
        }
        if ( ! isnan(illuminationValue)) {
            // illumination = 100 - ((illuminationValue - MIN_VALUE) * 100 / (MAX_VALUE - MIN_VALUE));
            illumination = map(illuminationValue, 0, 1023, 0, 100);
        }
        if ( ! isnan(wifiSignalValue)) {
            wifiSignal = wifiSignalValue;
        }
        if ( ! isnan(wifiSignalPercentValue)) {
            wifiSignalPercent = wifiSignalPercentValue;
        }
    }
}


/**
 * Мигание лампочкой
 */
void runLed() {

    unsigned long currentMillis = millis();

    if (currentMillis - ledMillis >= ledInterval) {
        ledMillis = currentMillis;

        if (ledState) {
            digitalWrite(PIN_LED, LOW);
            ledState = false;
        } else {
            digitalWrite(PIN_LED, HIGH);
            ledState = true;
        }
    }
}


/**
 * 
 */
void setup() {

    Serial.begin(115200);

    initWifi();
    initWebServer();
    initBMP();
    initATH10();
    initResetWifi();
    initLed();

    Serial.println("setup done");
    delay(100);
}


/**
 * 
 */
void loop() {

    runSensors();
    runLed();

    http.handleClient();
    resetBtn.tick();
    delay(20);
}
