# ESP NTP 2 serial

## All infos on: <http://weigu.lu/microcontroller/tips_tricks/esp_NTP_tips_tricks/index.html>

## Some infos

Often we connect our ESP's over WiFi with a network. This saves us an `RTC` (real time clock) module if the network provides a `NTP` ([NetWork Time Protocol](https://en.wikipedia.org/wiki/Network_Time_Protocol)) time server.

`NTP` is included in the ESP Core beginning with version 2.6.0 (2019). So we don't need an extra library any more. Everything is included in the `time.h` core library, even time zones (`TZ`) and daylight saving time (`DST`, summer/winter time).
