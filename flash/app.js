(() => {
  // src/js/core/strings.js
  var STRINGS = {
    // ---- page ----
    "app.title": { en: "NightStand Nano \u2013 flasher & hardware setup", "zh-TW": "NightStand Nano \u2013 \u71D2\u9304\u8207\u786C\u9AD4\u8A2D\u5B9A", "zh-CN": "NightStand Nano \u2013 \u70E7\u5F55\u4E0E\u786C\u4EF6\u8BBE\u7F6E" },
    "top.what": {
      en: "Put the firmware on a NightStand Nano box and set its hardware pins over a USB-serial cable, from this page. Nothing is uploaded anywhere: the firmware file you pick is read inside your browser.",
      "zh-TW": "\u7528\u9019\u500B\u9801\u9762\u900F\u904E USB \u5E8F\u5217\u7DDA\u628A\u97CC\u9AD4\u71D2\u9032 NightStand Nano\uFF0C\u4E26\u8A2D\u5B9A\u5B83\u7684\u786C\u9AD4\u8173\u4F4D\u3002\u4E0D\u6703\u4E0A\u50B3\u4EFB\u4F55\u6771\u897F\uFF1A\u4F60\u9078\u7684\u97CC\u9AD4\u6A94\u53EA\u5728\u700F\u89BD\u5668\u88E1\u8B80\u53D6\u3002",
      "zh-CN": "\u7528\u8FD9\u4E2A\u9875\u9762\u901A\u8FC7 USB \u4E32\u53E3\u7EBF\u628A\u56FA\u4EF6\u70E7\u8FDB NightStand Nano\uFF0C\u5E76\u8BBE\u7F6E\u5B83\u7684\u786C\u4EF6\u5F15\u811A\u3002\u4E0D\u4F1A\u4E0A\u4F20\u4EFB\u4F55\u4E1C\u897F\uFF1A\u4F60\u9009\u7684\u56FA\u4EF6\u6587\u4EF6\u53EA\u5728\u6D4F\u89C8\u5668\u91CC\u8BFB\u53D6\u3002"
    },
    "top.releases": { en: "Download the firmware (Releases)", "zh-TW": "\u4E0B\u8F09\u97CC\u9AD4\uFF08Releases\uFF09", "zh-CN": "\u4E0B\u8F7D\u56FA\u4EF6\uFF08Releases\uFF09" },
    "top.control": { en: "Control page", "zh-TW": "\u63A7\u5236\u9801", "zh-CN": "\u63A7\u5236\u9875" },
    "top.readme": { en: "README on GitHub", "zh-TW": "GitHub \u4E0A\u7684 README", "zh-CN": "GitHub \u4E0A\u7684 README" },
    "lang.label": { en: "Language", "zh-TW": "\u8A9E\u8A00", "zh-CN": "\u8BED\u8A00" },
    "lang.auto": { en: "Automatic", "zh-TW": "\u81EA\u52D5", "zh-CN": "\u81EA\u52A8" },
    "warn.noserial": {
      en: "This browser has no Web Serial, so it cannot flash or talk to the box. Open this page in Chrome or Edge on a desktop computer (Windows, macOS, Linux, ChromeOS). Firefox, Safari and phones do not work.",
      "zh-TW": "\u9019\u500B\u700F\u89BD\u5668\u6C92\u6709 Web Serial\uFF0C\u7121\u6CD5\u71D2\u9304\u4E5F\u7121\u6CD5\u548C\u6A5F\u5668\u901A\u8A0A\u3002\u8ACB\u5728\u684C\u4E0A\u578B\u96FB\u8166\uFF08Windows\u3001macOS\u3001Linux\u3001ChromeOS\uFF09\u7528 Chrome \u6216 Edge \u958B\u555F\u672C\u9801\u3002Firefox\u3001Safari \u548C\u624B\u6A5F\u90FD\u4E0D\u884C\u3002",
      "zh-CN": "\u8FD9\u4E2A\u6D4F\u89C8\u5668\u6CA1\u6709 Web Serial\uFF0C\u65E0\u6CD5\u70E7\u5F55\u4E5F\u65E0\u6CD5\u548C\u673A\u5668\u901A\u4FE1\u3002\u8BF7\u5728\u53F0\u5F0F\u7535\u8111\uFF08Windows\u3001macOS\u3001Linux\u3001ChromeOS\uFF09\u4E0A\u7528 Chrome \u6216 Edge \u6253\u5F00\u672C\u9875\u3002Firefox\u3001Safari \u548C\u624B\u673A\u90FD\u4E0D\u884C\u3002"
    },
    "warn.demo": { en: "Demo mode: a pretend board answers, nothing is written to any hardware.", "zh-TW": "\u793A\u7BC4\u6A21\u5F0F\uFF1A\u56DE\u7B54\u7684\u662F\u5047\u7684\u6A5F\u5668\uFF0C\u4E0D\u6703\u5BEB\u5165\u4EFB\u4F55\u786C\u9AD4\u3002", "zh-CN": "\u6F14\u793A\u6A21\u5F0F\uFF1A\u56DE\u7B54\u7684\u662F\u5047\u7684\u673A\u5668\uFF0C\u4E0D\u4F1A\u5199\u5165\u4EFB\u4F55\u786C\u4EF6\u3002" },
    // ---- connection ----
    "conn.title": { en: "Connection", "zh-TW": "\u9023\u7DDA", "zh-CN": "\u8FDE\u63A5" },
    "conn.off": { en: "Not connected", "zh-TW": "\u672A\u9023\u7DDA", "zh-CN": "\u672A\u8FDE\u63A5" },
    "conn.on": { en: "Connected: {port}", "zh-TW": "\u5DF2\u9023\u7DDA\uFF1A{port}", "zh-CN": "\u5DF2\u8FDE\u63A5\uFF1A{port}" },
    "conn.flashing": { en: "Flashing: the console is paused", "zh-TW": "\u71D2\u9304\u4E2D\uFF1A\u4E3B\u63A7\u53F0\u5DF2\u66AB\u505C", "zh-CN": "\u70E7\u5F55\u4E2D\uFF1A\u63A7\u5236\u53F0\u5DF2\u6682\u505C" },
    "btn.connect": { en: "Connect", "zh-TW": "\u9023\u7DDA", "zh-CN": "\u8FDE\u63A5" },
    "btn.disconnect": { en: "Disconnect", "zh-TW": "\u4E2D\u65B7\u9023\u7DDA", "zh-CN": "\u65AD\u5F00\u8FDE\u63A5" },
    "btn.anyport": { en: "Port not listed? Show all ports", "zh-TW": "\u627E\u4E0D\u5230\u57E0\uFF1F\u986F\u793A\u6240\u6709\u5E8F\u5217\u57E0", "zh-CN": "\u627E\u4E0D\u5230\u7AEF\u53E3\uFF1F\u663E\u793A\u6240\u6709\u4E32\u53E3" },
    "conn.hint": {
      en: "Plug the box's UART port (the USB-serial bridge, usually a CH340) into this computer, not the USB port of the USB DAC. Connecting restarts the board, like any serial monitor. Close other serial programs first.",
      "zh-TW": "\u628A\u6A5F\u5668\u7684 UART \u57E0\uFF08USB \u5E8F\u5217\u6A4B\u63A5\u6676\u7247\uFF0C\u901A\u5E38\u662F CH340\uFF09\u63A5\u5230\u9019\u53F0\u96FB\u8166\uFF0C\u4E0D\u662F USB DAC \u7684\u90A3\u500B USB \u57E0\u3002\u9023\u7DDA\u6703\u8B93\u6A5F\u5668\u91CD\u65B0\u555F\u52D5\uFF0C\u548C\u4E00\u822C\u5E8F\u5217\u76E3\u8996\u5668\u4E00\u6A23\u3002\u8ACB\u5148\u95DC\u6389\u5176\u4ED6\u5E8F\u5217\u57E0\u7A0B\u5F0F\u3002",
      "zh-CN": "\u628A\u673A\u5668\u7684 UART \u53E3\uFF08USB \u4E32\u53E3\u6865\u63A5\u82AF\u7247\uFF0C\u901A\u5E38\u662F CH340\uFF09\u63A5\u5230\u8FD9\u53F0\u7535\u8111\uFF0C\u4E0D\u662F USB DAC \u7684\u90A3\u4E2A USB \u53E3\u3002\u8FDE\u63A5\u4F1A\u8BA9\u673A\u5668\u91CD\u542F\uFF0C\u548C\u4E00\u822C\u4E32\u53E3\u76D1\u89C6\u5668\u4E00\u6837\u3002\u8BF7\u5148\u5173\u6389\u5176\u4ED6\u4E32\u53E3\u7A0B\u5E8F\u3002"
    },
    "conn.err.open": { en: "Could not open the port: {msg}. Is another program (a serial monitor, idf.py, esptool) using it?", "zh-TW": "\u7121\u6CD5\u958B\u555F\u5E8F\u5217\u57E0\uFF1A{msg}\u3002\u662F\u4E0D\u662F\u6709\u5176\u4ED6\u7A0B\u5F0F\uFF08\u5E8F\u5217\u76E3\u8996\u5668\u3001idf.py\u3001esptool\uFF09\u6B63\u5728\u4F7F\u7528\uFF1F", "zh-CN": "\u65E0\u6CD5\u6253\u5F00\u4E32\u53E3\uFF1A{msg}\u3002\u662F\u4E0D\u662F\u6709\u5176\u4ED6\u7A0B\u5E8F\uFF08\u4E32\u53E3\u76D1\u89C6\u5668\u3001idf.py\u3001esptool\uFF09\u6B63\u5728\u4F7F\u7528\uFF1F" },
    "conn.lost": { en: "The connection was lost (cable pulled?). Click Connect again.", "zh-TW": "\u9023\u7DDA\u4E2D\u65B7\u4E86\uFF08\u7DDA\u88AB\u62D4\u6389\uFF1F\uFF09\u3002\u8ACB\u518D\u6309\u4E00\u6B21\u300C\u9023\u7DDA\u300D\u3002", "zh-CN": "\u8FDE\u63A5\u65AD\u5F00\u4E86\uFF08\u7EBF\u88AB\u62D4\u6389\uFF1F\uFF09\u3002\u8BF7\u518D\u70B9\u4E00\u6B21\u201C\u8FDE\u63A5\u201D\u3002" },
    "conn.fw": { en: "Firmware {version}{detail} \xB7 ESP-IDF {idf} \xB7 build {built}", "zh-TW": "\u97CC\u9AD4 {version}{detail} \xB7 ESP-IDF {idf} \xB7 \u5EFA\u7F6E {built}", "zh-CN": "\u56FA\u4EF6 {version}{detail} \xB7 ESP-IDF {idf} \xB7 \u6784\u5EFA {built}" },
    "conn.fw.commit": { en: " ({commit})", "zh-TW": "\uFF08{commit}\uFF09", "zh-CN": "\uFF08{commit}\uFF09" },
    "conn.fw.dirty": { en: " ({commit}, with uncommitted changes)", "zh-TW": "\uFF08{commit}\uFF0C\u542B\u672A\u63D0\u4EA4\u7684\u4FEE\u6539\uFF09", "zh-CN": "\uFF08{commit}\uFF0C\u542B\u672A\u63D0\u4EA4\u7684\u4FEE\u6539\uFF09" },
    "conn.fw.dirty0": { en: " (with uncommitted changes)", "zh-TW": "\uFF08\u542B\u672A\u63D0\u4EA4\u7684\u4FEE\u6539\uFF09", "zh-CN": "\uFF08\u542B\u672A\u63D0\u4EA4\u7684\u4FEE\u6539\uFF09" },
    "conn.waitver": { en: "Waiting for the board to start\u2026", "zh-TW": "\u7B49\u5F85\u6A5F\u5668\u958B\u6A5F\u2026", "zh-CN": "\u7B49\u5F85\u673A\u5668\u542F\u52A8\u2026" },
    "conn.nover": {
      en: "The board does not answer. A new board has no firmware yet (flash it below); otherwise check the cable and that it is the UART port.",
      "zh-TW": "\u6A5F\u5668\u6C92\u6709\u56DE\u61C9\u3002\u5168\u65B0\u7684\u6A5F\u5668\u9084\u6C92\u6709\u97CC\u9AD4\uFF08\u8ACB\u5728\u4E0B\u9762\u71D2\u9304\uFF09\uFF1B\u5426\u5247\u8ACB\u6AA2\u67E5\u7DDA\u6750\uFF0C\u4E26\u78BA\u8A8D\u63A5\u7684\u662F UART \u57E0\u3002",
      "zh-CN": "\u673A\u5668\u6CA1\u6709\u54CD\u5E94\u3002\u5168\u65B0\u7684\u673A\u5668\u8FD8\u6CA1\u6709\u56FA\u4EF6\uFF08\u8BF7\u5728\u4E0B\u9762\u70E7\u5F55\uFF09\uFF1B\u5426\u5219\u8BF7\u68C0\u67E5\u7EBF\u6750\uFF0C\u5E76\u786E\u8BA4\u63A5\u7684\u662F UART \u53E3\u3002"
    },
    "tab.flash": { en: "Flash firmware", "zh-TW": "\u71D2\u9304\u97CC\u9AD4", "zh-CN": "\u70E7\u5F55\u56FA\u4EF6" },
    "tab.hw": { en: "Hardware setup", "zh-TW": "\u786C\u9AD4\u8A2D\u5B9A", "zh-CN": "\u786C\u4EF6\u8BBE\u7F6E" },
    // ---- flash tab ----
    "fl.intro": {
      en: "Download the release file(s) from the Releases page, pick them here, then connect and write. An update keeps your settings (Wi-Fi, Bluetooth pairings, volume, pins) unless you tick the erase box.",
      "zh-TW": "\u5230 Releases \u9801\u9762\u4E0B\u8F09\u767C\u4F48\u6A94\uFF0C\u5728\u9019\u88E1\u9078\u53D6\uFF0C\u518D\u9023\u7DDA\u4E26\u5BEB\u5165\u3002\u66F4\u65B0\u6703\u4FDD\u7559\u4F60\u7684\u8A2D\u5B9A\uFF08Wi-Fi\u3001\u85CD\u7259\u914D\u5C0D\u3001\u97F3\u91CF\u3001\u8173\u4F4D\uFF09\uFF0C\u9664\u975E\u4F60\u52FE\u9078\u6E05\u9664\u8A2D\u5B9A\u3002",
      "zh-CN": "\u5230 Releases \u9875\u9762\u4E0B\u8F7D\u53D1\u5E03\u6587\u4EF6\uFF0C\u5728\u8FD9\u91CC\u9009\u62E9\uFF0C\u518D\u8FDE\u63A5\u5E76\u5199\u5165\u3002\u66F4\u65B0\u4F1A\u4FDD\u7559\u4F60\u7684\u8BBE\u7F6E\uFF08Wi-Fi\u3001\u84DD\u7259\u914D\u5BF9\u3001\u97F3\u91CF\u3001\u5F15\u811A\uFF09\uFF0C\u9664\u975E\u4F60\u52FE\u9009\u6E05\u9664\u8BBE\u7F6E\u3002"
    },
    "fl.files.title": { en: "Which files?", "zh-TW": "\u8981\u9078\u54EA\u4E9B\u6A94\u6848\uFF1F", "zh-CN": "\u8981\u9009\u54EA\u4E9B\u6587\u4EF6\uFF1F" },
    "fl.files.body": {
      en: "Either ONE merged image, NightStandNano-<version>-full.bin, or THREE files: NightStandNano-<version>-bootloader.bin, NightStandNano-<version>-partition-table.bin and NightStandNano-<version>-app.bin (the raw build names bootloader.bin, partition-table.bin and nightstand_nano.bin work too). The page checks the content, not the name.",
      "zh-TW": "\u9078\u4E00\u500B\u5408\u4F75\u6620\u50CF NightStandNano-<\u7248\u672C>-full.bin\uFF0C\u6216\u4E09\u500B\u6A94\u6848\uFF1ANightStandNano-<\u7248\u672C>-bootloader.bin\u3001NightStandNano-<\u7248\u672C>-partition-table.bin\u3001NightStandNano-<\u7248\u672C>-app.bin\uFF08\u5EFA\u7F6E\u539F\u540D bootloader.bin\u3001partition-table.bin\u3001nightstand_nano.bin \u4E5F\u53EF\u4EE5\uFF09\u3002\u9801\u9762\u6AA2\u67E5\u7684\u662F\u5167\u5BB9\uFF0C\u4E0D\u662F\u6A94\u540D\u3002",
      "zh-CN": "\u9009\u4E00\u4E2A\u5408\u5E76\u6620\u50CF NightStandNano-<\u7248\u672C>-full.bin\uFF0C\u6216\u4E09\u4E2A\u6587\u4EF6\uFF1ANightStandNano-<\u7248\u672C>-bootloader.bin\u3001NightStandNano-<\u7248\u672C>-partition-table.bin\u3001NightStandNano-<\u7248\u672C>-app.bin\uFF08\u6784\u5EFA\u539F\u540D bootloader.bin\u3001partition-table.bin\u3001nightstand_nano.bin \u4E5F\u53EF\u4EE5\uFF09\u3002\u9875\u9762\u68C0\u67E5\u7684\u662F\u5185\u5BB9\uFF0C\u4E0D\u662F\u6587\u4EF6\u540D\u3002"
    },
    "fl.pick": { en: "Choose firmware file(s)\u2026", "zh-TW": "\u9078\u64C7\u97CC\u9AD4\u6A94\u2026", "zh-CN": "\u9009\u62E9\u56FA\u4EF6\u6587\u4EF6\u2026" },
    "fl.drop": { en: "or drop the file(s) here", "zh-TW": "\u6216\u628A\u6A94\u6848\u62D6\u5230\u9019\u88E1", "zh-CN": "\u6216\u628A\u6587\u4EF6\u62D6\u5230\u8FD9\u91CC" },
    "fl.kind.merged": { en: "One merged image (flash offset 0x0). Only the three regions below are written; the settings area in between is not touched.", "zh-TW": "\u4E00\u500B\u5408\u4F75\u6620\u50CF\uFF08\u71D2\u9304\u4F4D\u5740 0x0\uFF09\u3002\u53EA\u5BEB\u5165\u4E0B\u9762\u4E09\u500B\u5340\u6BB5\uFF0C\u4E2D\u9593\u7684\u8A2D\u5B9A\u5340\u4E0D\u6703\u52D5\u5230\u3002", "zh-CN": "\u4E00\u4E2A\u5408\u5E76\u6620\u50CF\uFF08\u70E7\u5F55\u5730\u5740 0x0\uFF09\u3002\u53EA\u5199\u5165\u4E0B\u9762\u4E09\u4E2A\u533A\u6BB5\uFF0C\u4E2D\u95F4\u7684\u8BBE\u7F6E\u533A\u4E0D\u4F1A\u52A8\u5230\u3002" },
    "fl.kind.three": { en: "Three separate files.", "zh-TW": "\u4E09\u500B\u5206\u958B\u7684\u6A94\u6848\u3002", "zh-CN": "\u4E09\u4E2A\u5206\u5F00\u7684\u6587\u4EF6\u3002" },
    "fl.col.part": { en: "Part", "zh-TW": "\u5340\u6BB5", "zh-CN": "\u533A\u6BB5" },
    "fl.col.file": { en: "File", "zh-TW": "\u6A94\u6848", "zh-CN": "\u6587\u4EF6" },
    "fl.col.offset": { en: "Written at", "zh-TW": "\u5BEB\u5165\u4F4D\u5740", "zh-CN": "\u5199\u5165\u5730\u5740" },
    "fl.col.size": { en: "Size", "zh-TW": "\u5927\u5C0F", "zh-CN": "\u5927\u5C0F" },
    "fl.part.bootloader": { en: "bootloader", "zh-TW": "\u555F\u52D5\u8F09\u5165\u5668", "zh-CN": "\u5F15\u5BFC\u52A0\u8F7D\u5668" },
    "fl.part.partition-table": { en: "partition table", "zh-TW": "\u5206\u5272\u8868", "zh-CN": "\u5206\u533A\u8868" },
    "fl.part.app": { en: "application", "zh-TW": "\u61C9\u7528\u7A0B\u5F0F", "zh-CN": "\u5E94\u7528\u7A0B\u5E8F" },
    "fl.part.nvs": { en: "settings (NVS)", "zh-TW": "\u8A2D\u5B9A\u5340\uFF08NVS\uFF09", "zh-CN": "\u8BBE\u7F6E\u533A\uFF08NVS\uFF09" },
    "fl.ver": { en: "Firmware {version}, built {built}, ESP-IDF {idf}", "zh-TW": "\u97CC\u9AD4 {version}\uFF0C\u5EFA\u7F6E\u65BC {built}\uFF0CESP-IDF {idf}", "zh-CN": "\u56FA\u4EF6 {version}\uFF0C\u6784\u5EFA\u4E8E {built}\uFF0CESP-IDF {idf}" },
    "fl.flashparams": { en: "Flash settings in the image: {mode}, {freq}, {size}", "zh-TW": "\u6620\u50CF\u88E1\u7684\u5FEB\u9583\u8A2D\u5B9A\uFF1A{mode}\u3001{freq}\u3001{size}", "zh-CN": "\u6620\u50CF\u91CC\u7684\u95EA\u5B58\u8BBE\u7F6E\uFF1A{mode}\u3001{freq}\u3001{size}" },
    "fl.flashparams.diff": { en: "This board is flashed with dio, 80 MHz and 16 MB. A different header may not boot: use the files from the Releases page.", "zh-TW": "\u9019\u584A\u677F\u5B50\u7684\u8A2D\u5B9A\u662F dio\u300180 MHz\u300116 MB\u3002\u6A19\u982D\u4E0D\u540C\u53EF\u80FD\u7121\u6CD5\u958B\u6A5F\uFF1A\u8ACB\u7528 Releases \u9801\u9762\u7684\u6A94\u6848\u3002", "zh-CN": "\u8FD9\u5757\u677F\u5B50\u7684\u8BBE\u7F6E\u662F dio\u300180 MHz\u300116 MB\u3002\u5934\u90E8\u4E0D\u540C\u53EF\u80FD\u65E0\u6CD5\u542F\u52A8\uFF1A\u8BF7\u7528 Releases \u9875\u9762\u7684\u6587\u4EF6\u3002" },
    "fl.project.diff": { en: "The image is called \u201C{name}\u201D, not nightstand_nano: it may be another firmware.", "zh-TW": "\u9019\u500B\u6620\u50CF\u53EB\u300C{name}\u300D\uFF0C\u4E0D\u662F nightstand_nano\uFF1A\u53EF\u80FD\u662F\u5225\u7684\u97CC\u9AD4\u3002", "zh-CN": "\u8FD9\u4E2A\u6620\u50CF\u53EB\u201C{name}\u201D\uFF0C\u4E0D\u662F nightstand_nano\uFF1A\u53EF\u80FD\u662F\u522B\u7684\u56FA\u4EF6\u3002" },
    "fl.ok": { en: "The files look right.", "zh-TW": "\u6A94\u6848\u770B\u8D77\u4F86\u6B63\u78BA\u3002", "zh-CN": "\u6587\u4EF6\u770B\u8D77\u6765\u6B63\u786E\u3002" },
    "fl.erase.label": { en: "Also erase the settings (NVS)", "zh-TW": "\u540C\u6642\u6E05\u9664\u8A2D\u5B9A\uFF08NVS\uFF09", "zh-CN": "\u540C\u65F6\u6E05\u9664\u8BBE\u7F6E\uFF08NVS\uFF09" },
    "fl.erase.warn": {
      en: "Erases the settings area ({off}, {size}). Wi-Fi settings, Bluetooth pairings, the volume and the hardware pin settings are lost and the box starts like a new one. Leave this off for an ordinary update.",
      "zh-TW": "\u6703\u6E05\u9664\u8A2D\u5B9A\u5340\uFF08{off}\uFF0C{size}\uFF09\u3002Wi-Fi \u8A2D\u5B9A\u3001\u85CD\u7259\u914D\u5C0D\u3001\u97F3\u91CF\u548C\u786C\u9AD4\u8173\u4F4D\u8A2D\u5B9A\u90FD\u6703\u6D88\u5931\uFF0C\u6A5F\u5668\u6703\u50CF\u5168\u65B0\u7684\u4E00\u6A23\u958B\u6A5F\u3002\u4E00\u822C\u66F4\u65B0\u8ACB\u4E0D\u8981\u52FE\u9078\u3002",
      "zh-CN": "\u4F1A\u6E05\u9664\u8BBE\u7F6E\u533A\uFF08{off}\uFF0C{size}\uFF09\u3002Wi-Fi \u8BBE\u7F6E\u3001\u84DD\u7259\u914D\u5BF9\u3001\u97F3\u91CF\u548C\u786C\u4EF6\u5F15\u811A\u8BBE\u7F6E\u90FD\u4F1A\u6D88\u5931\uFF0C\u673A\u5668\u4F1A\u50CF\u5168\u65B0\u7684\u4E00\u6837\u542F\u52A8\u3002\u4E00\u822C\u66F4\u65B0\u8BF7\u4E0D\u8981\u52FE\u9009\u3002"
    },
    "fl.plan.title": { en: "What will be written", "zh-TW": "\u5C07\u8981\u5BEB\u5165\u7684\u5167\u5BB9", "zh-CN": "\u5C06\u8981\u5199\u5165\u7684\u5185\u5BB9" },
    "fl.plan.kept": { en: "Left alone (settings survive): {list}", "zh-TW": "\u4E0D\u6703\u52D5\uFF08\u8A2D\u5B9A\u4FDD\u7559\uFF09\uFF1A{list}", "zh-CN": "\u4E0D\u4F1A\u52A8\uFF08\u8BBE\u7F6E\u4FDD\u7559\uFF09\uFF1A{list}" },
    "fl.plan.erased": { en: "Erased: {list}", "zh-TW": "\u6703\u6E05\u9664\uFF1A{list}", "zh-CN": "\u4F1A\u6E05\u9664\uFF1A{list}" },
    "btn.fl.check": { en: "Connect and check the board", "zh-TW": "\u9023\u7DDA\u4E26\u6AA2\u67E5\u6A5F\u5668", "zh-CN": "\u8FDE\u63A5\u5E76\u68C0\u67E5\u673A\u5668" },
    "btn.fl.write": { en: "Write to the board", "zh-TW": "\u5BEB\u5165\u6A5F\u5668", "zh-CN": "\u5199\u5165\u673A\u5668" },
    "btn.fl.cancel": { en: "Cancel", "zh-TW": "\u53D6\u6D88", "zh-CN": "\u53D6\u6D88" },
    "fl.checking": { en: "Putting the board into its bootloader (DTR/RTS reset)\u2026", "zh-TW": "\u6B63\u5728\u8B93\u6A5F\u5668\u9032\u5165\u555F\u52D5\u8F09\u5165\u5668\uFF08DTR/RTS \u91CD\u8A2D\uFF09\u2026", "zh-CN": "\u6B63\u5728\u8BA9\u673A\u5668\u8FDB\u5165\u5F15\u5BFC\u52A0\u8F7D\u5668\uFF08DTR/RTS \u590D\u4F4D\uFF09\u2026" },
    "fl.found": { en: "Found {chip}, flash {size}, link {baud} baud.", "zh-TW": "\u627E\u5230 {chip}\uFF0C\u5FEB\u9583 {size}\uFF0C\u9023\u7DDA\u901F\u7387 {baud} baud\u3002", "zh-CN": "\u627E\u5230 {chip}\uFF0C\u95EA\u5B58 {size}\uFF0C\u8FDE\u63A5\u901F\u7387 {baud} baud\u3002" },
    "fl.ready": { en: "The board and the files fit. Ready to write.", "zh-TW": "\u6A5F\u5668\u8207\u6A94\u6848\u76F8\u7B26\uFF0C\u53EF\u4EE5\u5BEB\u5165\u3002", "zh-CN": "\u673A\u5668\u4E0E\u6587\u4EF6\u76F8\u7B26\uFF0C\u53EF\u4EE5\u5199\u5165\u3002" },
    "fl.confirm.title": { en: "Really erase the settings too?", "zh-TW": "\u771F\u7684\u8981\u4E00\u4F75\u6E05\u9664\u8A2D\u5B9A\u55CE\uFF1F", "zh-CN": "\u771F\u7684\u8981\u4E00\u5E76\u6E05\u9664\u8BBE\u7F6E\u5417\uFF1F" },
    "btn.fl.confirm": { en: "Yes, erase settings and write", "zh-TW": "\u662F\uFF0C\u6E05\u9664\u8A2D\u5B9A\u4E26\u5BEB\u5165", "zh-CN": "\u662F\uFF0C\u6E05\u9664\u8BBE\u7F6E\u5E76\u5199\u5165" },
    "fl.writing": { en: "Writing {name}\u2026 {pct}%", "zh-TW": "\u6B63\u5728\u5BEB\u5165{name}\u2026 {pct}%", "zh-CN": "\u6B63\u5728\u5199\u5165{name}\u2026 {pct}%" },
    "fl.verifying": { en: "Verifying {name}\u2026", "zh-TW": "\u6B63\u5728\u9A57\u8B49{name}\u2026", "zh-CN": "\u6B63\u5728\u9A8C\u8BC1{name}\u2026" },
    "fl.finishing": { en: "Restarting the board\u2026", "zh-TW": "\u6B63\u5728\u91CD\u65B0\u555F\u52D5\u6A5F\u5668\u2026", "zh-CN": "\u6B63\u5728\u91CD\u542F\u673A\u5668\u2026" },
    "fl.done.title": { en: "Flashed and verified. The box is restarting.", "zh-TW": "\u71D2\u9304\u5B8C\u6210\u4E26\u5DF2\u9A57\u8B49\u3002\u6A5F\u5668\u6B63\u5728\u91CD\u65B0\u555F\u52D5\u3002", "zh-CN": "\u70E7\u5F55\u5B8C\u6210\u5E76\u5DF2\u9A8C\u8BC1\u3002\u673A\u5668\u6B63\u5728\u91CD\u542F\u3002" },
    "fl.done.next": { en: "Next steps", "zh-TW": "\u63A5\u4E0B\u4F86", "zh-CN": "\u63A5\u4E0B\u6765" },
    "fl.done.s1": { en: "Open the control page and connect to the box over Bluetooth.", "zh-TW": "\u6253\u958B\u63A7\u5236\u9801\uFF0C\u7528\u85CD\u7259\u9023\u4E0A\u6A5F\u5668\u3002", "zh-CN": "\u6253\u5F00\u63A7\u5236\u9875\uFF0C\u7528\u84DD\u7259\u8FDE\u4E0A\u673A\u5668\u3002" },
    "fl.done.s2": { en: "Set up Wi-Fi there (Settings, then Wi-Fi).", "zh-TW": "\u5728\u90A3\u88E1\u8A2D\u5B9A Wi-Fi\uFF08\u8A2D\u5B9A\uFF0C\u518D\u9078 Wi-Fi\uFF09\u3002", "zh-CN": "\u5728\u90A3\u91CC\u8BBE\u7F6E Wi-Fi\uFF08\u8BBE\u7F6E\uFF0C\u518D\u9009 Wi-Fi\uFF09\u3002" },
    "fl.done.s3": { en: "Only if your board differs from the defaults: set the SD card and LED pins on the Hardware setup tab of this page.", "zh-TW": "\u53EA\u6709\u677F\u5B50\u548C\u9810\u8A2D\u4E0D\u540C\u6642\uFF1A\u5230\u672C\u9801\u7684\u300C\u786C\u9AD4\u8A2D\u5B9A\u300D\u5206\u9801\u8A2D\u5B9A SD \u5361\u8207 LED \u8173\u4F4D\u3002", "zh-CN": "\u53EA\u6709\u677F\u5B50\u548C\u9ED8\u8BA4\u4E0D\u540C\u65F6\uFF1A\u5230\u672C\u9875\u7684\u201C\u786C\u4EF6\u8BBE\u7F6E\u201D\u5206\u9875\u8BBE\u7F6E SD \u5361\u4E0E LED \u5F15\u811A\u3002" },
    "btn.openControl": { en: "Open the control page", "zh-TW": "\u6253\u958B\u63A7\u5236\u9801", "zh-CN": "\u6253\u5F00\u63A7\u5236\u9875" },
    "fl.fail.write": {
      en: "Writing failed: {msg}. The flash may hold a half-written image. Do not unplug: click \u201CConnect and check the board\u201D and write again. If the box still does not start, hold BOOT, tap RESET and try again.",
      "zh-TW": "\u5BEB\u5165\u5931\u6557\uFF1A{msg}\u3002\u5FEB\u9583\u88E1\u53EF\u80FD\u662F\u5BEB\u5230\u4E00\u534A\u7684\u6620\u50CF\u3002\u8ACB\u4E0D\u8981\u62D4\u7DDA\uFF1A\u6309\u300C\u9023\u7DDA\u4E26\u6AA2\u67E5\u6A5F\u5668\u300D\u518D\u5BEB\u4E00\u6B21\u3002\u5982\u679C\u6A5F\u5668\u9084\u662F\u7121\u6CD5\u958B\u6A5F\uFF0C\u8ACB\u6309\u4F4F BOOT\u3001\u9EDE\u4E00\u4E0B RESET \u518D\u8A66\u3002",
      "zh-CN": "\u5199\u5165\u5931\u8D25\uFF1A{msg}\u3002\u95EA\u5B58\u91CC\u53EF\u80FD\u662F\u5199\u5230\u4E00\u534A\u7684\u6620\u50CF\u3002\u8BF7\u4E0D\u8981\u62D4\u7EBF\uFF1A\u70B9\u201C\u8FDE\u63A5\u5E76\u68C0\u67E5\u673A\u5668\u201D\u518D\u5199\u4E00\u6B21\u3002\u5982\u679C\u673A\u5668\u8FD8\u662F\u65E0\u6CD5\u542F\u52A8\uFF0C\u8BF7\u6309\u4F4F BOOT\u3001\u70B9\u4E00\u4E0B RESET \u518D\u8BD5\u3002"
    },
    "fl.fail.verify": { en: "The flash does not match the file ({name}). Write again.", "zh-TW": "\u5FEB\u9583\u5167\u5BB9\u548C\u6A94\u6848\u4E0D\u4E00\u81F4\uFF08{name}\uFF09\u3002\u8ACB\u518D\u5BEB\u4E00\u6B21\u3002", "zh-CN": "\u95EA\u5B58\u5185\u5BB9\u548C\u6587\u4EF6\u4E0D\u4E00\u81F4\uFF08{name}\uFF09\u3002\u8BF7\u518D\u5199\u4E00\u6B21\u3002" },
    "fl.fail.connect": { en: "Could not reach the bootloader: {msg}", "zh-TW": "\u7121\u6CD5\u9023\u5230\u555F\u52D5\u8F09\u5165\u5668\uFF1A{msg}", "zh-CN": "\u65E0\u6CD5\u8FDE\u5230\u5F15\u5BFC\u52A0\u8F7D\u5668\uFF1A{msg}" },
    "fl.manual.title": { en: "If the automatic reset does not work", "zh-TW": "\u5982\u679C\u81EA\u52D5\u91CD\u8A2D\u6C92\u6709\u7528", "zh-CN": "\u5982\u679C\u81EA\u52A8\u590D\u4F4D\u6CA1\u6709\u7528" },
    "fl.manual.body": {
      en: "Do it by hand: hold the BOOT button, tap RESET, keep holding BOOT and click \u201CConnect and check the board\u201D, release BOOT when it says Found. Also check that the cable is on the UART port (not the USB DAC port), that the cable carries data, and that no other program has the port open.",
      "zh-TW": "\u624B\u52D5\u9032\u5165\uFF1A\u6309\u4F4F BOOT \u9375\uFF0C\u9EDE\u4E00\u4E0B RESET\uFF0C\u7E7C\u7E8C\u6309\u4F4F BOOT \u4E26\u6309\u300C\u9023\u7DDA\u4E26\u6AA2\u67E5\u6A5F\u5668\u300D\uFF0C\u51FA\u73FE\u300C\u627E\u5230\u300D\u5F8C\u518D\u653E\u958B BOOT\u3002\u4E5F\u8ACB\u78BA\u8A8D\u7DDA\u63A5\u5728 UART \u57E0\uFF08\u4E0D\u662F USB DAC \u7684\u57E0\uFF09\u3001\u7DDA\u6750\u80FD\u50B3\u8CC7\u6599\uFF0C\u800C\u4E14\u6C92\u6709\u5176\u4ED6\u7A0B\u5F0F\u958B\u8457\u9019\u500B\u57E0\u3002",
      "zh-CN": "\u624B\u52A8\u8FDB\u5165\uFF1A\u6309\u4F4F BOOT \u952E\uFF0C\u70B9\u4E00\u4E0B RESET\uFF0C\u7EE7\u7EED\u6309\u4F4F BOOT \u5E76\u70B9\u201C\u8FDE\u63A5\u5E76\u68C0\u67E5\u673A\u5668\u201D\uFF0C\u51FA\u73B0\u201C\u627E\u5230\u201D\u540E\u518D\u677E\u5F00 BOOT\u3002\u4E5F\u8BF7\u786E\u8BA4\u7EBF\u63A5\u5728 UART \u53E3\uFF08\u4E0D\u662F USB DAC \u7684\u53E3\uFF09\u3001\u7EBF\u6750\u80FD\u4F20\u6570\u636E\uFF0C\u800C\u4E14\u6CA1\u6709\u5176\u4ED6\u7A0B\u5E8F\u5F00\u7740\u8FD9\u4E2A\u53E3\u3002"
    },
    "fl.err.none": { en: "No file chosen.", "zh-TW": "\u9084\u6C92\u6709\u9078\u6A94\u6848\u3002", "zh-CN": "\u8FD8\u6CA1\u6709\u9009\u6587\u4EF6\u3002" },
    "fl.err.count": { en: "Choose one merged image (\u2026-full.bin) or all three files (bootloader, partition table, app). You chose {n}.", "zh-TW": "\u8ACB\u9078\u4E00\u500B\u5408\u4F75\u6620\u50CF\uFF08\u2026-full.bin\uFF09\u6216\u5168\u90E8\u4E09\u500B\u6A94\u6848\uFF08\u555F\u52D5\u8F09\u5165\u5668\u3001\u5206\u5272\u8868\u3001\u61C9\u7528\u7A0B\u5F0F\uFF09\u3002\u4F60\u9078\u4E86 {n} \u500B\u3002", "zh-CN": "\u8BF7\u9009\u4E00\u4E2A\u5408\u5E76\u6620\u50CF\uFF08\u2026-full.bin\uFF09\u6216\u5168\u90E8\u4E09\u4E2A\u6587\u4EF6\uFF08\u5F15\u5BFC\u52A0\u8F7D\u5668\u3001\u5206\u533A\u8868\u3001\u5E94\u7528\u7A0B\u5E8F\uFF09\u3002\u4F60\u9009\u4E86 {n} \u4E2A\u3002" },
    "fl.err.need_all": { en: "That is only the {role}. Choose all three files, or the merged \u2026-full.bin.", "zh-TW": "\u9019\u53EA\u6709\u300C{role}\u300D\u3002\u8ACB\u9078\u5168\u90E8\u4E09\u500B\u6A94\u6848\uFF0C\u6216\u5408\u4F75\u7684 \u2026-full.bin\u3002", "zh-CN": "\u8FD9\u53EA\u6709\u201C{role}\u201D\u3002\u8BF7\u9009\u5168\u90E8\u4E09\u4E2A\u6587\u4EF6\uFF0C\u6216\u5408\u5E76\u7684 \u2026-full.bin\u3002" },
    "fl.err.not_merged": { en: "{file} is named like a merged image but is not one. Download it again.", "zh-TW": "{file} \u7684\u6A94\u540D\u50CF\u5408\u4F75\u6620\u50CF\uFF0C\u5167\u5BB9\u537B\u4E0D\u662F\u3002\u8ACB\u91CD\u65B0\u4E0B\u8F09\u3002", "zh-CN": "{file} \u7684\u6587\u4EF6\u540D\u50CF\u5408\u5E76\u6620\u50CF\uFF0C\u5185\u5BB9\u5374\u4E0D\u662F\u3002\u8BF7\u91CD\u65B0\u4E0B\u8F7D\u3002" },
    "fl.err.mixed": { en: "{file} is a merged image: choose it alone, not together with other files.", "zh-TW": "{file} \u662F\u5408\u4F75\u6620\u50CF\uFF1A\u8ACB\u55AE\u7368\u9078\u5B83\uFF0C\u4E0D\u8981\u548C\u5176\u4ED6\u6A94\u6848\u4E00\u8D77\u3002", "zh-CN": "{file} \u662F\u5408\u5E76\u6620\u50CF\uFF1A\u8BF7\u5355\u72EC\u9009\u5B83\uFF0C\u4E0D\u8981\u548C\u5176\u4ED6\u6587\u4EF6\u4E00\u8D77\u3002" },
    "fl.err.roles": { en: "The three files must be one bootloader, one partition table and one app: the {role} is missing or doubled.", "zh-TW": "\u4E09\u500B\u6A94\u6848\u5FC5\u9808\u662F\u4E00\u500B\u555F\u52D5\u8F09\u5165\u5668\u3001\u4E00\u500B\u5206\u5272\u8868\u3001\u4E00\u500B\u61C9\u7528\u7A0B\u5F0F\uFF1A\u300C{role}\u300D\u7F3A\u5C11\u6216\u91CD\u8907\u3002", "zh-CN": "\u4E09\u4E2A\u6587\u4EF6\u5FC5\u987B\u662F\u4E00\u4E2A\u5F15\u5BFC\u52A0\u8F7D\u5668\u3001\u4E00\u4E2A\u5206\u533A\u8868\u3001\u4E00\u4E2A\u5E94\u7528\u7A0B\u5E8F\uFF1A\u201C{role}\u201D\u7F3A\u5C11\u6216\u91CD\u590D\u3002" },
    "fl.err.no_magic": { en: "{file} is not an ESP firmware image (wrong magic byte).", "zh-TW": "{file} \u4E0D\u662F ESP \u97CC\u9AD4\u6620\u50CF\uFF08\u9B54\u8853\u4F4D\u5143\u7D44\u4E0D\u5C0D\uFF09\u3002", "zh-CN": "{file} \u4E0D\u662F ESP \u56FA\u4EF6\u6620\u50CF\uFF08\u9B54\u672F\u5B57\u8282\u4E0D\u5BF9\uFF09\u3002" },
    "fl.err.chip": { en: "{file}: this image is for another chip, not the ESP32-S31.", "zh-TW": "{file}\uFF1A\u9019\u500B\u6620\u50CF\u662F\u7D66\u5225\u7684\u6676\u7247\uFF0C\u4E0D\u662F ESP32-S31\u3002", "zh-CN": "{file}\uFF1A\u8FD9\u4E2A\u6620\u50CF\u662F\u7ED9\u522B\u7684\u82AF\u7247\uFF0C\u4E0D\u662F ESP32-S31\u3002" },
    "fl.err.hash": { en: "{file}: the image's checksum does not match (damaged download?).", "zh-TW": "{file}\uFF1A\u6620\u50CF\u7684\u6AA2\u67E5\u78BC\u4E0D\u7B26\uFF08\u4E0B\u8F09\u640D\u58DE\uFF1F\uFF09\u3002", "zh-CN": "{file}\uFF1A\u6620\u50CF\u7684\u6821\u9A8C\u7801\u4E0D\u7B26\uFF08\u4E0B\u8F7D\u635F\u574F\uFF1F\uFF09\u3002" },
    "fl.err.size": { en: "{file}: the size is wrong for this part.", "zh-TW": "{file}\uFF1A\u5927\u5C0F\u4E0D\u7B26\u5408\u9019\u500B\u5340\u6BB5\u3002", "zh-CN": "{file}\uFF1A\u5927\u5C0F\u4E0D\u7B26\u5408\u8FD9\u4E2A\u533A\u6BB5\u3002" },
    "fl.err.no_pt": { en: "{file} has no partition table (wrong magic).", "zh-TW": "{file} \u88E1\u6C92\u6709\u5206\u5272\u8868\uFF08\u9B54\u8853\u4F4D\u5143\u7D44\u4E0D\u5C0D\uFF09\u3002", "zh-CN": "{file} \u91CC\u6CA1\u6709\u5206\u533A\u8868\uFF08\u9B54\u672F\u5B57\u8282\u4E0D\u5BF9\uFF09\u3002" },
    "fl.err.pt_md5": { en: "{file}: the partition table's own checksum does not match.", "zh-TW": "{file}\uFF1A\u5206\u5272\u8868\u81EA\u5DF1\u7684\u6AA2\u67E5\u78BC\u4E0D\u7B26\u3002", "zh-CN": "{file}\uFF1A\u5206\u533A\u8868\u81EA\u5DF1\u7684\u6821\u9A8C\u7801\u4E0D\u7B26\u3002" },
    "fl.err.pt_layout": { en: "The partition table does not fit this firmware ({what}).", "zh-TW": "\u5206\u5272\u8868\u548C\u9019\u4EFD\u97CC\u9AD4\u4E0D\u5408\uFF08{what}\uFF09\u3002", "zh-CN": "\u5206\u533A\u8868\u548C\u8FD9\u4EFD\u56FA\u4EF6\u4E0D\u5408\uFF08{what}\uFF09\u3002" },
    "fl.err.gap": { en: "The merged image holds data in a place that must be blank (at {at}). It is not a release file.", "zh-TW": "\u5408\u4F75\u6620\u50CF\u5728\u61C9\u8A72\u7A7A\u767D\u7684\u5730\u65B9\uFF08{at}\uFF09\u6709\u8CC7\u6599\u3002\u9019\u4E0D\u662F\u767C\u4F48\u6A94\u3002", "zh-CN": "\u5408\u5E76\u6620\u50CF\u5728\u5E94\u8BE5\u7A7A\u767D\u7684\u5730\u65B9\uFF08{at}\uFF09\u6709\u6570\u636E\u3002\u8FD9\u4E0D\u662F\u53D1\u5E03\u6587\u4EF6\u3002" },
    "fl.err.no_desc": { en: "{file} is an ESP image but not the application (no application description).", "zh-TW": "{file} \u662F ESP \u6620\u50CF\uFF0C\u4F46\u4E0D\u662F\u61C9\u7528\u7A0B\u5F0F\uFF08\u6C92\u6709\u61C9\u7528\u7A0B\u5F0F\u63CF\u8FF0\uFF09\u3002", "zh-CN": "{file} \u662F ESP \u6620\u50CF\uFF0C\u4F46\u4E0D\u662F\u5E94\u7528\u7A0B\u5E8F\uFF08\u6CA1\u6709\u5E94\u7528\u7A0B\u5E8F\u63CF\u8FF0\uFF09\u3002" },
    "fl.err.app_big": { en: "The application is bigger than its partition ({size}).", "zh-TW": "\u61C9\u7528\u7A0B\u5F0F\u6BD4\u5B83\u7684\u5206\u5272\u5340\uFF08{size}\uFF09\u9084\u5927\u3002", "zh-CN": "\u5E94\u7528\u7A0B\u5E8F\u6BD4\u5B83\u7684\u5206\u533A\uFF08{size}\uFF09\u8FD8\u5927\u3002" },
    "fl.err.read": { en: "The file could not be read.", "zh-TW": "\u8B80\u4E0D\u4E86\u9019\u500B\u6A94\u6848\u3002", "zh-CN": "\u8BFB\u4E0D\u4E86\u8FD9\u4E2A\u6587\u4EF6\u3002" },
    "fl.err.chipcheck": { en: "This is {chip}, not an ESP32-S31. Stopped.", "zh-TW": "\u9019\u662F {chip}\uFF0C\u4E0D\u662F ESP32-S31\u3002\u5DF2\u505C\u6B62\u3002", "zh-CN": "\u8FD9\u662F {chip}\uFF0C\u4E0D\u662F ESP32-S31\u3002\u5DF2\u505C\u6B62\u3002" },
    "fl.err.small": { en: "The flash is {size} but the partition table needs {need} MB. Stopped.", "zh-TW": "\u5FEB\u9583\u53EA\u6709 {size}\uFF0C\u5206\u5272\u8868\u9700\u8981 {need} MB\u3002\u5DF2\u505C\u6B62\u3002", "zh-CN": "\u95EA\u5B58\u53EA\u6709 {size}\uFF0C\u5206\u533A\u8868\u9700\u8981 {need} MB\u3002\u5DF2\u505C\u6B62\u3002" },
    "fl.err.lib": { en: "Could not load the flashing library: {msg}", "zh-TW": "\u7121\u6CD5\u8F09\u5165\u71D2\u9304\u51FD\u5F0F\u5EAB\uFF1A{msg}", "zh-CN": "\u65E0\u6CD5\u52A0\u8F7D\u70E7\u5F55\u5E93\uFF1A{msg}" },
    "fl.err.other": { en: "{msg}", "zh-TW": "{msg}", "zh-CN": "{msg}" },
    "fl.lost": { en: "The port did not come back after the restart. Click Connect to continue.", "zh-TW": "\u91CD\u65B0\u555F\u52D5\u5F8C\u5E8F\u5217\u57E0\u6C92\u6709\u56DE\u4F86\u3002\u8ACB\u6309\u300C\u9023\u7DDA\u300D\u7E7C\u7E8C\u3002", "zh-CN": "\u91CD\u542F\u540E\u4E32\u53E3\u6CA1\u6709\u56DE\u6765\u3002\u8BF7\u70B9\u201C\u8FDE\u63A5\u201D\u7EE7\u7EED\u3002" },
    // ---- hardware tab ----
    "hw.safety.title": { en: "Read this before changing anything", "zh-TW": "\u66F4\u6539\u4EFB\u4F55\u8A2D\u5B9A\u524D\u8ACB\u5148\u8B80", "zh-CN": "\u66F4\u6539\u4EFB\u4F55\u8BBE\u7F6E\u524D\u8BF7\u5148\u8BFB" },
    "hw.safety.1": { en: "Change the pins only if your board differs from the defaults.", "zh-TW": "\u53EA\u6709\u4F60\u7684\u677F\u5B50\u548C\u9810\u8A2D\u4E0D\u540C\u6642\u624D\u9700\u8981\u6539\u8173\u4F4D\u3002", "zh-CN": "\u53EA\u6709\u4F60\u7684\u677F\u5B50\u548C\u9ED8\u8BA4\u4E0D\u540C\u65F6\u624D\u9700\u8981\u6539\u5F15\u811A\u3002" },
    "hw.safety.2": { en: "Wrong pins can stop the SD card or the LED from working. The box still starts, and you can come back here to fix them or restore the defaults.", "zh-TW": "\u8173\u4F4D\u932F\u4E86\uFF0CSD \u5361\u6216 LED \u53EF\u80FD\u7121\u6CD5\u904B\u4F5C\u3002\u6A5F\u5668\u4ECD\u7136\u80FD\u958B\u6A5F\uFF0C\u4F60\u53EF\u4EE5\u56DE\u5230\u9019\u88E1\u4FEE\u6B63\u6216\u9084\u539F\u9810\u8A2D\u503C\u3002", "zh-CN": "\u5F15\u811A\u9519\u4E86\uFF0CSD \u5361\u6216 LED \u53EF\u80FD\u65E0\u6CD5\u5DE5\u4F5C\u3002\u673A\u5668\u4ECD\u7136\u80FD\u542F\u52A8\uFF0C\u4F60\u53EF\u4EE5\u56DE\u5230\u8FD9\u91CC\u4FEE\u6B63\u6216\u6062\u590D\u9ED8\u8BA4\u503C\u3002" },
    "hw.safety.3": { en: "3.3 V only: never put 5 V on a GPIO. SD cards are 3.3 V parts.", "zh-TW": "\u53EA\u80FD\u662F 3.3 V\uFF1A\u7D55\u5C0D\u4E0D\u8981\u628A 5 V \u63A5\u5230 GPIO\u3002SD \u5361\u662F 3.3 V \u5143\u4EF6\u3002", "zh-CN": "\u53EA\u80FD\u662F 3.3 V\uFF1A\u7EDD\u5BF9\u4E0D\u8981\u628A 5 V \u63A5\u5230 GPIO\u3002SD \u5361\u662F 3.3 V \u5668\u4EF6\u3002" },
    "hw.safety.4": { en: "The SD lines CMD and D0 to D3 (in SPI mode: MOSI, MISO and CS) need pull-ups, about 10 k\u03A9 to 3.3 V. Many breakout boards have them; a bare socket does not.", "zh-TW": "SD \u7684 CMD\u3001D0 \u5230 D3 \u8A0A\u865F\u7DDA\uFF08SPI \u6A21\u5F0F\u662F MOSI\u3001MISO\u3001CS\uFF09\u9700\u8981\u4E0A\u62C9\u96FB\u963B\uFF0C\u7D04 10 k\u03A9 \u63A5\u5230 3.3 V\u3002\u5F88\u591A\u8F49\u63A5\u677F\u5DF2\u7D93\u6709\uFF1B\u88F8\u7684\u5361\u5EA7\u6C92\u6709\u3002", "zh-CN": "SD \u7684 CMD\u3001D0 \u5230 D3 \u4FE1\u53F7\u7EBF\uFF08SPI \u6A21\u5F0F\u662F MOSI\u3001MISO\u3001CS\uFF09\u9700\u8981\u4E0A\u62C9\u7535\u963B\uFF0C\u7EA6 10 k\u03A9 \u63A5\u5230 3.3 V\u3002\u5F88\u591A\u8F6C\u63A5\u677F\u5DF2\u7ECF\u6709\uFF1B\u88F8\u7684\u5361\u5EA7\u6CA1\u6709\u3002" },
    "hw.safety.5": { en: "WS2812 LED: a 3.3 V data line usually works on a short wire. If the LED is powered from 5 V, use a level shifter (for example 74AHCT125), a resistor of about 330 \u03A9 in series with the data line, a capacitor of about 1000 \xB5F across its supply, and a common ground. The colour order differs between LED types.", "zh-TW": "WS2812 LED\uFF1A\u77ED\u7DDA\u6642 3.3 V \u7684\u8CC7\u6599\u7DDA\u901A\u5E38\u53EF\u4EE5\u7528\u3002\u5982\u679C LED \u7528 5 V \u4F9B\u96FB\uFF0C\u8ACB\u52A0\u96FB\u5E73\u8F49\u63DB\uFF08\u4F8B\u5982 74AHCT125\uFF09\u3001\u8CC7\u6599\u7DDA\u4E32\u4E00\u500B\u7D04 330 \u03A9 \u7684\u96FB\u963B\u3001\u96FB\u6E90\u5169\u7AEF\u4E26\u4E00\u500B\u7D04 1000 \xB5F \u7684\u96FB\u5BB9\uFF0C\u4E26\u4E14\u5171\u5730\u3002\u4E0D\u540C LED \u7684\u984F\u8272\u9806\u5E8F\u4E0D\u4E00\u6A23\u3002", "zh-CN": "WS2812 LED\uFF1A\u77ED\u7EBF\u65F6 3.3 V \u7684\u6570\u636E\u7EBF\u901A\u5E38\u53EF\u4EE5\u7528\u3002\u5982\u679C LED \u7528 5 V \u4F9B\u7535\uFF0C\u8BF7\u52A0\u7535\u5E73\u8F6C\u6362\uFF08\u4F8B\u5982 74AHCT125\uFF09\u3001\u6570\u636E\u7EBF\u4E32\u4E00\u4E2A\u7EA6 330 \u03A9 \u7684\u7535\u963B\u3001\u7535\u6E90\u4E24\u7AEF\u5E76\u4E00\u4E2A\u7EA6 1000 \xB5F \u7684\u7535\u5BB9\uFF0C\u5E76\u4E14\u5171\u5730\u3002\u4E0D\u540C LED \u7684\u989C\u8272\u987A\u5E8F\u4E0D\u4E00\u6837\u3002" },
    "hw.notconn": { en: "Connect the box (Connection, above) to read and change its hardware settings.", "zh-TW": "\u8ACB\u5148\u9023\u7DDA\u6A5F\u5668\uFF08\u4E0A\u65B9\u7684\u300C\u9023\u7DDA\u300D\uFF09\uFF0C\u624D\u80FD\u8B80\u53D6\u548C\u66F4\u6539\u786C\u9AD4\u8A2D\u5B9A\u3002", "zh-CN": "\u8BF7\u5148\u8FDE\u63A5\u673A\u5668\uFF08\u4E0A\u65B9\u7684\u201C\u8FDE\u63A5\u201D\uFF09\uFF0C\u624D\u80FD\u8BFB\u53D6\u548C\u66F4\u6539\u786C\u4EF6\u8BBE\u7F6E\u3002" },
    "hw.reading": { en: "Reading the settings\u2026", "zh-TW": "\u6B63\u5728\u8B80\u53D6\u8A2D\u5B9A\u2026", "zh-CN": "\u6B63\u5728\u8BFB\u53D6\u8BBE\u7F6E\u2026" },
    "hw.err.noanswer": { en: "The box did not answer the hw command (or not completely). Is the NightStand Nano firmware running, and is it recent enough to have this feature? Flash the latest release first.", "zh-TW": "\u6A5F\u5668\u6C92\u6709\u56DE\u61C9 hw \u6307\u4EE4\uFF08\u6216\u56DE\u61C9\u4E0D\u5B8C\u6574\uFF09\u3002NightStand Nano \u97CC\u9AD4\u6709\u5728\u57F7\u884C\u55CE\uFF1F\u7248\u672C\u5920\u65B0\u3001\u6709\u9019\u500B\u529F\u80FD\u55CE\uFF1F\u8ACB\u5148\u71D2\u9304\u6700\u65B0\u7684\u767C\u4F48\u7248\u3002", "zh-CN": "\u673A\u5668\u6CA1\u6709\u54CD\u5E94 hw \u6307\u4EE4\uFF08\u6216\u54CD\u5E94\u4E0D\u5B8C\u6574\uFF09\u3002NightStand Nano \u56FA\u4EF6\u6709\u5728\u8FD0\u884C\u5417\uFF1F\u7248\u672C\u591F\u65B0\u3001\u6709\u8FD9\u4E2A\u529F\u80FD\u5417\uFF1F\u8BF7\u5148\u70E7\u5F55\u6700\u65B0\u7684\u53D1\u5E03\u7248\u3002" },
    "btn.hw.reread": { en: "Read again", "zh-TW": "\u91CD\u65B0\u8B80\u53D6", "zh-CN": "\u91CD\u65B0\u8BFB\u53D6" },
    "hw.sd.title": { en: "SD card", "zh-TW": "SD \u5361", "zh-CN": "SD \u5361" },
    "hw.sd.state.mounted": { en: "Card detected and working", "zh-TW": "\u5075\u6E2C\u5230\u5361\u7247\uFF0C\u904B\u4F5C\u6B63\u5E38", "zh-CN": "\u68C0\u6D4B\u5230\u5361\u7247\uFF0C\u8FD0\u884C\u6B63\u5E38" },
    "hw.sd.state.unreadable": { en: "A card is there but cannot be read (wrong pins, missing pull-ups or an unsupported format)", "zh-TW": "\u6709\u5361\u7247\u4F46\u8B80\u4E0D\u4E86\uFF08\u8173\u4F4D\u932F\u3001\u5C11\u4E86\u4E0A\u62C9\u96FB\u963B\u6216\u683C\u5F0F\u4E0D\u652F\u63F4\uFF09", "zh-CN": "\u6709\u5361\u7247\u4F46\u8BFB\u4E0D\u4E86\uFF08\u5F15\u811A\u9519\u3001\u5C11\u4E86\u4E0A\u62C9\u7535\u963B\u6216\u683C\u5F0F\u4E0D\u652F\u6301\uFF09" },
    "hw.sd.state.none": { en: "No card detected", "zh-TW": "\u6C92\u6709\u5075\u6E2C\u5230\u5361\u7247", "zh-CN": "\u6CA1\u6709\u68C0\u6D4B\u5230\u5361\u7247" },
    "hw.sd.mode": { en: "Bus mode: {mode}", "zh-TW": "\u532F\u6D41\u6392\u6A21\u5F0F\uFF1A{mode}", "zh-CN": "\u603B\u7EBF\u6A21\u5F0F\uFF1A{mode}" },
    "hw.src.default": { en: "default pins", "zh-TW": "\u9810\u8A2D\u8173\u4F4D", "zh-CN": "\u9ED8\u8BA4\u5F15\u811A" },
    "hw.src.custom": { en: "custom pins", "zh-TW": "\u81EA\u8A02\u8173\u4F4D", "zh-CN": "\u81EA\u5B9A\u4E49\u5F15\u811A" },
    "hw.width": { en: "Bus width", "zh-TW": "\u532F\u6D41\u6392\u5BEC\u5EA6", "zh-CN": "\u603B\u7EBF\u5BBD\u5EA6" },
    "hw.width.4": { en: "4-bit", "zh-TW": "4 \u4F4D\u5143", "zh-CN": "4 \u4F4D" },
    "hw.width.1": { en: "1-bit (only CLK, CMD, D0)", "zh-TW": "1 \u4F4D\u5143\uFF08\u53EA\u7528 CLK\u3001CMD\u3001D0\uFF09", "zh-CN": "1 \u4F4D\uFF08\u53EA\u7528 CLK\u3001CMD\u3001D0\uFF09" },
    "hw.gpio.range": { en: "GPIO {min} to {max}", "zh-TW": "GPIO {min} \u5230 {max}", "zh-CN": "GPIO {min} \u5230 {max}" },
    "hw.reserved": { en: "Reserved pins, cannot be used (tap one to see why):", "zh-TW": "\u4FDD\u7559\u3001\u4E0D\u80FD\u4F7F\u7528\u7684\u8173\u4F4D\uFF08\u9EDE\u4E00\u4E0B\u770B\u539F\u56E0\uFF09\uFF1A", "zh-CN": "\u4FDD\u7559\u3001\u4E0D\u80FD\u4F7F\u7528\u7684\u5F15\u811A\uFF08\u70B9\u4E00\u4E0B\u770B\u539F\u56E0\uFF09\uFF1A" },
    "btn.hw.sd.apply": { en: "Save SD card settings", "zh-TW": "\u5132\u5B58 SD \u5361\u8A2D\u5B9A", "zh-CN": "\u4FDD\u5B58 SD \u5361\u8BBE\u7F6E" },
    "hw.led.title": { en: "Status LED (WS2812)", "zh-TW": "\u72C0\u614B LED\uFF08WS2812\uFF09", "zh-CN": "\u72B6\u6001 LED\uFF08WS2812\uFF09" },
    "hw.led.on": { en: "An LED is connected", "zh-TW": "\u6709\u63A5 LED", "zh-CN": "\u6709\u63A5 LED" },
    "hw.led.gpio": { en: "Data GPIO", "zh-TW": "\u8CC7\u6599 GPIO", "zh-CN": "\u6570\u636E GPIO" },
    "hw.led.order": { en: "Colour order", "zh-TW": "\u984F\u8272\u9806\u5E8F", "zh-CN": "\u989C\u8272\u987A\u5E8F" },
    "btn.hw.led.apply": { en: "Save LED", "zh-TW": "\u5132\u5B58 LED", "zh-CN": "\u4FDD\u5B58 LED" },
    "hw.iss.int": { en: "{field}: enter a whole number.", "zh-TW": "{field}\uFF1A\u8ACB\u8F38\u5165\u6574\u6578\u3002", "zh-CN": "{field}\uFF1A\u8BF7\u8F93\u5165\u6574\u6570\u3002" },
    "hw.iss.range": { en: "{field}: the GPIO must be {min} to {max}.", "zh-TW": "{field}\uFF1AGPIO \u5FC5\u9808\u5728 {min} \u5230 {max} \u4E4B\u9593\u3002", "zh-CN": "{field}\uFF1AGPIO \u5FC5\u987B\u5728 {min} \u5230 {max} \u4E4B\u95F4\u3002" },
    "hw.iss.reserved": { en: "{field}: GPIO {n} is reserved ({why}).", "zh-TW": "{field}\uFF1AGPIO {n} \u662F\u4FDD\u7559\u7684\uFF08{why}\uFF09\u3002", "zh-CN": "{field}\uFF1AGPIO {n} \u662F\u4FDD\u7559\u7684\uFF08{why}\uFF09\u3002" },
    "hw.iss.nopin": { en: "{field}: GPIO {n} does not exist on this chip.", "zh-TW": "{field}\uFF1A\u9019\u9846\u6676\u7247\u6C92\u6709 GPIO {n}\u3002", "zh-CN": "{field}\uFF1A\u8FD9\u9897\u82AF\u7247\u6CA1\u6709 GPIO {n}\u3002" },
    "hw.iss.width": { en: "The bus width must be 4-bit or 1-bit.", "zh-TW": "\u532F\u6D41\u6392\u5BEC\u5EA6\u53EA\u80FD\u662F 4 \u4F4D\u5143\u6216 1 \u4F4D\u5143\u3002", "zh-CN": "\u603B\u7EBF\u5BBD\u5EA6\u53EA\u80FD\u662F 4 \u4F4D\u6216 1 \u4F4D\u3002" },
    "hw.iss.notsup": { en: "This chip cannot move the SDMMC pins.", "zh-TW": "\u9019\u9846\u6676\u7247\u7121\u6CD5\u66F4\u52D5 SDMMC \u8173\u4F4D\u3002", "zh-CN": "\u8FD9\u9897\u82AF\u7247\u65E0\u6CD5\u66F4\u6539 SDMMC \u5F15\u811A\u3002" },
    "hw.iss.blank": { en: "Enter all four GPIO numbers.", "zh-TW": "\u8ACB\u586B\u5165\u56DB\u500B GPIO \u7DE8\u865F\u3002", "zh-CN": "\u8BF7\u586B\u5165\u56DB\u4E2A GPIO \u7F16\u53F7\u3002" },
    "hw.iss.duplicate": { en: "{field}: GPIO {n} is used twice.", "zh-TW": "{field}\uFF1AGPIO {n} \u91CD\u8907\u4F7F\u7528\u4E86\u3002", "zh-CN": "{field}\uFF1AGPIO {n} \u91CD\u590D\u4F7F\u7528\u4E86\u3002" },
    "hw.iss.conflict": { en: "{field}: GPIO {n} is also used by the {other}. Save the other one first.", "zh-TW": "{field}\uFF1AGPIO {n} \u540C\u6642\u88AB{other}\u4F7F\u7528\u3002\u8ACB\u5148\u5132\u5B58\u53E6\u4E00\u908A\u3002", "zh-CN": "{field}\uFF1AGPIO {n} \u540C\u65F6\u88AB{other}\u4F7F\u7528\u3002\u8BF7\u5148\u4FDD\u5B58\u53E6\u4E00\u8FB9\u3002" },
    "hw.iss.order": { en: "Pick a colour order.", "zh-TW": "\u8ACB\u9078\u984F\u8272\u9806\u5E8F\u3002", "zh-CN": "\u8BF7\u9009\u989C\u8272\u987A\u5E8F\u3002" },
    "hw.other.sd": { en: "SD card", "zh-TW": "SD \u5361", "zh-CN": "SD \u5361" },
    "hw.other.led": { en: "LED", "zh-TW": "LED", "zh-CN": "LED" },
    "hw.pending": { en: "The saved settings differ from what is running. Restart the board to apply them.", "zh-TW": "\u5DF2\u5132\u5B58\u7684\u8A2D\u5B9A\u548C\u6B63\u5728\u57F7\u884C\u7684\u4E0D\u540C\u3002\u8ACB\u91CD\u65B0\u555F\u52D5\u6A5F\u5668\u4F86\u5957\u7528\u3002", "zh-CN": "\u5DF2\u4FDD\u5B58\u7684\u8BBE\u7F6E\u548C\u6B63\u5728\u8FD0\u884C\u7684\u4E0D\u540C\u3002\u8BF7\u91CD\u542F\u673A\u5668\u6765\u5E94\u7528\u3002" },
    "btn.hw.restart": { en: "Restart the board", "zh-TW": "\u91CD\u65B0\u555F\u52D5\u6A5F\u5668", "zh-CN": "\u91CD\u542F\u673A\u5668" },
    "hw.restarting": { en: "Restarting\u2026 the settings are read again when the board is back.", "zh-TW": "\u91CD\u65B0\u555F\u52D5\u4E2D\u2026\u6A5F\u5668\u56DE\u4F86\u5F8C\u6703\u518D\u8B80\u4E00\u6B21\u8A2D\u5B9A\u3002", "zh-CN": "\u91CD\u542F\u4E2D\u2026\u673A\u5668\u56DE\u6765\u540E\u4F1A\u518D\u8BFB\u4E00\u6B21\u8BBE\u7F6E\u3002" },
    "hw.restarted": { en: "The board is back. Card state: {state}.", "zh-TW": "\u6A5F\u5668\u56DE\u4F86\u4E86\u3002\u5361\u7247\u72C0\u614B\uFF1A{state}\u3002", "zh-CN": "\u673A\u5668\u56DE\u6765\u4E86\u3002\u5361\u7247\u72B6\u6001\uFF1A{state}\u3002" },
    "btn.hw.defaults": { en: "Restore defaults", "zh-TW": "\u9084\u539F\u9810\u8A2D\u503C", "zh-CN": "\u6062\u590D\u9ED8\u8BA4\u503C" },
    "hw.defaults.ask": { en: "Put the SD and LED pins back to the defaults? (Takes effect after a restart.)", "zh-TW": "\u628A SD \u8207 LED \u8173\u4F4D\u6539\u56DE\u9810\u8A2D\u503C\uFF1F\uFF08\u91CD\u65B0\u555F\u52D5\u5F8C\u751F\u6548\u3002\uFF09", "zh-CN": "\u628A SD \u4E0E LED \u5F15\u811A\u6539\u56DE\u9ED8\u8BA4\u503C\uFF1F\uFF08\u91CD\u542F\u540E\u751F\u6548\u3002\uFF09" },
    "btn.hw.defaults.yes": { en: "Yes, restore", "zh-TW": "\u662F\uFF0C\u9084\u539F", "zh-CN": "\u662F\uFF0C\u6062\u590D" },
    "hw.ack.ok": { en: "Saved. Restart the board to apply it.", "zh-TW": "\u5DF2\u5132\u5B58\u3002\u8ACB\u91CD\u65B0\u555F\u52D5\u6A5F\u5668\u4F86\u5957\u7528\u3002", "zh-CN": "\u5DF2\u4FDD\u5B58\u3002\u8BF7\u91CD\u542F\u673A\u5668\u6765\u5E94\u7528\u3002" },
    "hw.ack.reset": { en: "Defaults stored. Restart the board to apply them.", "zh-TW": "\u9810\u8A2D\u503C\u5DF2\u5B58\u3002\u8ACB\u91CD\u65B0\u555F\u52D5\u6A5F\u5668\u4F86\u5957\u7528\u3002", "zh-CN": "\u9ED8\u8BA4\u503C\u5DF2\u5B58\u3002\u8BF7\u91CD\u542F\u673A\u5668\u6765\u5E94\u7528\u3002" },
    "hw.refused": { en: "The box refused: {why}.", "zh-TW": "\u6A5F\u5668\u62D2\u7D55\u4E86\uFF1A{why}\u3002", "zh-CN": "\u673A\u5668\u62D2\u7EDD\u4E86\uFF1A{why}\u3002" },
    "hw.why.range": { en: "a GPIO number that this chip does not have", "zh-TW": "\u6709 GPIO \u7DE8\u865F\u662F\u9019\u9846\u6676\u7247\u6C92\u6709\u7684", "zh-CN": "\u6709 GPIO \u7F16\u53F7\u662F\u8FD9\u9897\u82AF\u7247\u6CA1\u6709\u7684" },
    "hw.why.reserved": { en: "GPIO {n} is reserved ({why})", "zh-TW": "GPIO {n} \u662F\u4FDD\u7559\u7684\uFF08{why}\uFF09", "zh-CN": "GPIO {n} \u662F\u4FDD\u7559\u7684\uFF08{why}\uFF09" },
    "hw.why.duplicate": { en: "GPIO {n} is used twice", "zh-TW": "GPIO {n} \u91CD\u8907\u4F7F\u7528", "zh-CN": "GPIO {n} \u91CD\u590D\u4F7F\u7528" },
    "hw.why.width": { en: "the bus width must be 1 or 4", "zh-TW": "\u532F\u6D41\u6392\u5BEC\u5EA6\u53EA\u80FD\u662F 1 \u6216 4", "zh-CN": "\u603B\u7EBF\u5BBD\u5EA6\u53EA\u80FD\u662F 1 \u6216 4" },
    "hw.why.order": { en: "unknown colour order", "zh-TW": "\u4E0D\u8A8D\u8B58\u7684\u984F\u8272\u9806\u5E8F", "zh-CN": "\u4E0D\u8BA4\u8BC6\u7684\u989C\u8272\u987A\u5E8F" },
    "hw.why.conflict": { en: "GPIO {n} would be used by both the SD card and the LED", "zh-TW": "GPIO {n} \u6703\u540C\u6642\u88AB SD \u5361\u548C LED \u4F7F\u7528", "zh-CN": "GPIO {n} \u4F1A\u540C\u65F6\u88AB SD \u5361\u548C LED \u4F7F\u7528" },
    "hw.why.other": { en: "\u201C{raw}\u201D", "zh-TW": "\u300C{raw}\u300D", "zh-CN": "\u201C{raw}\u201D" },
    "hw.why.syntax": { en: "the command was not understood (a number is missing or not a number)", "zh-TW": "\u6307\u4EE4\u770B\u4E0D\u61C2\uFF08\u5C11\u4E86\u6578\u5B57\uFF0C\u6216\u4E0D\u662F\u6578\u5B57\uFF09", "zh-CN": "\u6307\u4EE4\u770B\u4E0D\u61C2\uFF08\u5C11\u4E86\u6570\u5B57\uFF0C\u6216\u4E0D\u662F\u6570\u5B57\uFF09" },
    "hw.why.storage": { en: "the box could not store the setting (flash write failed)", "zh-TW": "\u6A5F\u5668\u5B58\u4E0D\u9032\u9019\u500B\u8A2D\u5B9A\uFF08\u5BEB\u5165\u5FEB\u9583\u5931\u6557\uFF09", "zh-CN": "\u673A\u5668\u5B58\u4E0D\u8FDB\u8FD9\u4E2A\u8BBE\u7F6E\uFF08\u5199\u5165\u95EA\u5B58\u5931\u8D25\uFF09" },
    "hw.why.notsup.sdmmc": { en: "this chip cannot move the SDMMC pins (its SD host reaches only the fixed pins). For other pins use the SPI connection", "zh-TW": "\u9019\u9846\u6676\u7247\u7121\u6CD5\u66F4\u52D5 SDMMC \u8173\u4F4D\uFF08SD \u4E3B\u6A5F\u53EA\u63A5\u5F97\u5230\u56FA\u5B9A\u7684\u90A3\u7D44\uFF09\u3002\u8981\u7528\u5225\u7684\u8173\u4F4D\u8ACB\u6539\u9078 SPI \u9023\u7DDA", "zh-CN": "\u8FD9\u9897\u82AF\u7247\u65E0\u6CD5\u66F4\u6539 SDMMC \u5F15\u811A\uFF08SD \u4E3B\u673A\u53EA\u63A5\u5F97\u5230\u56FA\u5B9A\u7684\u90A3\u7EC4\uFF09\u3002\u8981\u7528\u522B\u7684\u5F15\u811A\u8BF7\u6539\u9009 SPI \u8FDE\u63A5" },
    "hw.why.notsup.other": { en: "the chip cannot do that: {raw}", "zh-TW": "\u6676\u7247\u505A\u4E0D\u5230\uFF1A{raw}", "zh-CN": "\u82AF\u7247\u505A\u4E0D\u5230\uFF1A{raw}" },
    "hw.refused.raw": { en: " (the box said: {raw})", "zh-TW": "\uFF08\u6A5F\u5668\u539F\u6587\uFF1A{raw}\uFF09", "zh-CN": "\uFF08\u673A\u5668\u539F\u6587\uFF1A{raw}\uFF09" },
    "hw.field.clk": { en: "CLK", "zh-TW": "CLK", "zh-CN": "CLK" },
    "hw.field.cmd": { en: "CMD", "zh-TW": "CMD", "zh-CN": "CMD" },
    "hw.field.d0": { en: "D0", "zh-TW": "D0", "zh-CN": "D0" },
    "hw.field.d1": { en: "D1", "zh-TW": "D1", "zh-CN": "D1" },
    "hw.field.d2": { en: "D2", "zh-TW": "D2", "zh-CN": "D2" },
    "hw.field.d3": { en: "D3", "zh-TW": "D3", "zh-CN": "D3" },
    "hw.field.mosi": { en: "MOSI", "zh-TW": "MOSI", "zh-CN": "MOSI" },
    "hw.field.miso": { en: "MISO", "zh-TW": "MISO", "zh-CN": "MISO" },
    "hw.field.cs": { en: "CS", "zh-TW": "CS", "zh-CN": "CS" },
    "hw.field.led": { en: "LED data", "zh-TW": "LED \u8CC7\u6599", "zh-CN": "LED \u6570\u636E" },
    "hw.field.order": { en: "Colour order", "zh-TW": "\u984F\u8272\u9806\u5E8F", "zh-CN": "\u989C\u8272\u987A\u5E8F" },
    "hw.sd.statenote": { en: "The card state above is what the box is running now. Restart the board to make it use the saved pins.", "zh-TW": "\u4E0A\u9762\u7684\u5361\u7247\u72C0\u614B\u662F\u6A5F\u5668\u76EE\u524D\u5BE6\u969B\u57F7\u884C\u7684\u7D50\u679C\u3002\u8981\u8B93\u5B83\u7528\u5DF2\u5132\u5B58\u7684\u8173\u4F4D\uFF0C\u8ACB\u91CD\u65B0\u555F\u52D5\u6A5F\u5668\u3002", "zh-CN": "\u4E0A\u9762\u7684\u5361\u7247\u72B6\u6001\u662F\u673A\u5668\u5F53\u524D\u5B9E\u9645\u8FD0\u884C\u7684\u7ED3\u679C\u3002\u8981\u8BA9\u5B83\u7528\u5DF2\u4FDD\u5B58\u7684\u5F15\u811A\uFF0C\u8BF7\u91CD\u542F\u673A\u5668\u3002" },
    "hw.conn.title": { en: "How the SD card is connected", "zh-TW": "SD \u5361\u7684\u63A5\u6CD5", "zh-CN": "SD \u5361\u7684\u63A5\u6CD5" },
    "hw.conn.sdmmc": { en: "Built-in card slot \xB7 SDMMC (fixed pins)", "zh-TW": "\u5167\u5EFA\u5361\u5EA7\uFF0FSDMMC\uFF08\u56FA\u5B9A\u8173\u4F4D\uFF09", "zh-CN": "\u5185\u7F6E\u5361\u5EA7\uFF0FSDMMC\uFF08\u56FA\u5B9A\u5F15\u811A\uFF09" },
    "hw.conn.spi": { en: "Wired by hand \xB7 SPI mode (pins of your choice)", "zh-TW": "\u81EA\u5DF1\u63A5\u7DDA\uFF0FSPI \u6A21\u5F0F\uFF08\u8173\u4F4D\u81EA\u9078\uFF09", "zh-CN": "\u81EA\u5DF1\u63A5\u7EBF\uFF0FSPI \u6A21\u5F0F\uFF08\u5F15\u811A\u81EA\u9009\uFF09" },
    "hw.mmc.fixed": { en: "This chip cannot move the SDMMC pins: its SD host reaches only the pins below, so they are shown but cannot be edited. Only the bus width can change. If your socket is wired to other pins, choose SPI mode.", "zh-TW": "\u9019\u9846\u6676\u7247\u7121\u6CD5\u66F4\u52D5 SDMMC \u8173\u4F4D\uFF1ASD \u4E3B\u6A5F\u53EA\u63A5\u5F97\u5230\u4E0B\u9762\u9019\u7D44\u8173\u4F4D\uFF0C\u6240\u4EE5\u53EA\u986F\u793A\u3001\u4E0D\u80FD\u6539\uFF0C\u53EA\u80FD\u9078\u532F\u6D41\u6392\u5BEC\u5EA6\u3002\u5982\u679C\u4F60\u7684\u5361\u5EA7\u63A5\u5728\u5225\u7684\u8173\u4F4D\uFF0C\u8ACB\u6539\u9078 SPI \u6A21\u5F0F\u3002", "zh-CN": "\u8FD9\u9897\u82AF\u7247\u65E0\u6CD5\u66F4\u6539 SDMMC \u5F15\u811A\uFF1ASD \u4E3B\u673A\u53EA\u63A5\u5F97\u5230\u4E0B\u9762\u8FD9\u7EC4\u5F15\u811A\uFF0C\u6240\u4EE5\u53EA\u663E\u793A\u3001\u4E0D\u80FD\u6539\uFF0C\u53EA\u80FD\u9009\u603B\u7EBF\u5BBD\u5EA6\u3002\u5982\u679C\u4F60\u7684\u5361\u5EA7\u63A5\u5728\u522B\u7684\u5F15\u811A\uFF0C\u8BF7\u6539\u9009 SPI \u6A21\u5F0F\u3002" },
    "hw.fixed.off": { en: "not used", "zh-TW": "\u4E0D\u4F7F\u7528", "zh-CN": "\u4E0D\u4F7F\u7528" },
    "hw.spi.intro": { en: "Type the GPIO numbers of the four SPI wires (any pins that are not reserved), and wire the socket or microSD breakout as in the table.", "zh-TW": "\u586B\u5165\u56DB\u689D SPI \u7DDA\u63A5\u7684 GPIO \u7DE8\u865F\uFF08\u4EFB\u4F55\u6C92\u88AB\u4FDD\u7559\u7684\u8173\u4F4D\u90FD\u53EF\u4EE5\uFF09\uFF0C\u4E26\u7167\u4E0B\u8868\u628A\u5361\u5EA7\u6216 microSD \u8F49\u63A5\u677F\u63A5\u597D\u3002", "zh-CN": "\u586B\u5165\u56DB\u6761 SPI \u7EBF\u63A5\u7684 GPIO \u7F16\u53F7\uFF08\u4EFB\u4F55\u6CA1\u88AB\u4FDD\u7559\u7684\u5F15\u811A\u90FD\u53EF\u4EE5\uFF09\uFF0C\u5E76\u7167\u4E0B\u8868\u628A\u5361\u5EA7\u6216 microSD \u8F6C\u63A5\u677F\u63A5\u597D\u3002" },
    "hw.wire.col.fw": { en: "Box pin (SPI)", "zh-TW": "\u672C\u6A5F\u8173\u4F4D\uFF08SPI\uFF09", "zh-CN": "\u672C\u673A\u5F15\u811A\uFF08SPI\uFF09" },
    "hw.wire.col.card": { en: "SD card pin", "zh-TW": "SD \u5361\u8173\u4F4D", "zh-CN": "SD \u5361\u5F15\u811A" },
    "hw.wire.col.note": { en: "Note", "zh-TW": "\u5099\u8A3B", "zh-CN": "\u5907\u6CE8" },
    "hw.wire.cs": { en: "10 k\u03A9 pull-up to 3.3 V (it also keeps the card unselected while the box starts)", "zh-TW": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u96FB\u963B\u5230 3.3 V\uFF08\u4E5F\u907F\u514D\u958B\u6A5F\u6642\u5361\u88AB\u8AA4\u9078\uFF09", "zh-CN": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u7535\u963B\u5230 3.3 V\uFF08\u4E5F\u907F\u514D\u5F00\u673A\u65F6\u5361\u88AB\u8BEF\u9009\uFF09" },
    "hw.wire.mosi": { en: "10 k\u03A9 pull-up to 3.3 V", "zh-TW": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u96FB\u963B\u5230 3.3 V", "zh-CN": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u7535\u963B\u5230 3.3 V" },
    "hw.wire.miso": { en: "10 k\u03A9 pull-up to 3.3 V", "zh-TW": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u96FB\u963B\u5230 3.3 V", "zh-CN": "\u63A5 10 k\u03A9 \u4E0A\u62C9\u7535\u963B\u5230 3.3 V" },
    "hw.wire.clk": { en: "Keep this wire as short as you can", "zh-TW": "\u9019\u689D\u7DDA\u76E1\u91CF\u77ED", "zh-CN": "\u8FD9\u6761\u7EBF\u5C3D\u91CF\u77ED" },
    "hw.wire.vdd": { en: "3.3 V only, never 5 V. Put 10 \xB5F and 0.1 \xB5F capacitors next to the socket", "zh-TW": "\u53EA\u80FD 3.3 V\uFF0C\u7D55\u5C0D\u4E0D\u80FD 5 V\u3002\u5361\u5EA7\u65C1\u653E 10 \xB5F \u8207 0.1 \xB5F \u96FB\u5BB9", "zh-CN": "\u53EA\u80FD 3.3 V\uFF0C\u7EDD\u5BF9\u4E0D\u80FD 5 V\u3002\u5361\u5EA7\u65C1\u653E 10 \xB5F \u4E0E 0.1 \xB5F \u7535\u5BB9" },
    "hw.wire.gnd": { en: "Common ground with the box", "zh-TW": "\u548C\u6A5F\u5668\u5171\u5730", "zh-CN": "\u548C\u673A\u5668\u5171\u5730" },
    "hw.spi.note.unused": { en: "The firmware does not use DAT1 and DAT2 in SPI mode. The chip's own pull-ups are too weak for hand-wired lines, so add the external ones; some breakout boards already have them.", "zh-TW": "SPI \u6A21\u5F0F\u4E0B\u97CC\u9AD4\u4E0D\u4F7F\u7528 DAT1\u3001DAT2\u3002\u6676\u7247\u5167\u5EFA\u7684\u4E0A\u62C9\u592A\u5F31\uFF0C\u624B\u63A5\u7684\u7DDA\u8ACB\u53E6\u5916\u52A0\u5916\u90E8\u4E0A\u62C9\uFF1B\u6709\u4E9B\u8F49\u63A5\u677F\u5DF2\u7D93\u88DD\u597D\u4E86\u3002", "zh-CN": "SPI \u6A21\u5F0F\u4E0B\u56FA\u4EF6\u4E0D\u4F7F\u7528 DAT1\u3001DAT2\u3002\u82AF\u7247\u5185\u7F6E\u7684\u4E0A\u62C9\u592A\u5F31\uFF0C\u624B\u63A5\u7684\u7EBF\u8BF7\u53E6\u5916\u52A0\u5916\u90E8\u4E0A\u62C9\uFF1B\u6709\u4E9B\u8F6C\u63A5\u677F\u5DF2\u7ECF\u88C5\u597D\u4E86\u3002" },
    "hw.spi.note.slow": { en: "SPI mode is slower than SDMMC. Very high sample rates in uncompressed files may not keep up.", "zh-TW": "SPI \u6A21\u5F0F\u6BD4 SDMMC \u8F03\u6162\u3002\u53D6\u6A23\u7387\u5F88\u9AD8\u7684\u672A\u58D3\u7E2E\u6A94\u6848\u53EF\u80FD\u8DDF\u4E0D\u4E0A\u3002", "zh-CN": "SPI \u6A21\u5F0F\u6BD4 SDMMC \u8F83\u6162\u3002\u91C7\u6837\u7387\u5F88\u9AD8\u7684\u672A\u538B\u7F29\u6587\u4EF6\u53EF\u80FD\u8DDF\u4E0D\u4E0A\u3002" },
    "hw.spi.powercycle": { en: "A card that has been used in SPI mode may need a power cycle (take it out and put it back, or power the box off and on) before it works in SDMMC mode again.", "zh-TW": "\u7528\u904E SPI \u6A21\u5F0F\u7684\u5361\uFF0C\u8981\u56DE\u5230 SDMMC \u6A21\u5F0F\u524D\u53EF\u80FD\u5F97\u5148\u65B7\u96FB\u91CD\u4E0A\u96FB\uFF08\u628A\u5361\u62D4\u6389\u518D\u63D2\uFF0C\u6216\u6574\u6A5F\u65B7\u96FB\u518D\u958B\uFF09\u624D\u6703\u6B63\u5E38\u3002", "zh-CN": "\u7528\u8FC7 SPI \u6A21\u5F0F\u7684\u5361\uFF0C\u8981\u56DE\u5230 SDMMC \u6A21\u5F0F\u524D\u53EF\u80FD\u5F97\u5148\u65AD\u7535\u91CD\u4E0A\u7535\uFF08\u628A\u5361\u62D4\u6389\u518D\u63D2\uFF0C\u6216\u6574\u673A\u65AD\u7535\u518D\u5F00\uFF09\u624D\u4F1A\u6B63\u5E38\u3002" },
    "hw.res.nopin": { en: "this chip has no such pin", "zh-TW": "\u9019\u9846\u6676\u7247\u6C92\u6709\u9019\u652F\u8173", "zh-CN": "\u8FD9\u9897\u82AF\u7247\u6CA1\u6709\u8FD9\u4E2A\u5F15\u811A" },
    "hw.res.flash": { en: "it connects the board's SPI flash, where the firmware lives", "zh-TW": "\u63A5\u677F\u4E0A\u7684 SPI \u5FEB\u9583\u8A18\u61B6\u9AD4\uFF0C\u97CC\u9AD4\u5C31\u5B58\u5728\u90A3\u88E1", "zh-CN": "\u63A5\u677F\u4E0A\u7684 SPI \u95EA\u5B58\uFF0C\u56FA\u4EF6\u5C31\u5B58\u5728\u90A3\u91CC" },
    "hw.res.usb": { en: "internal USB pin, the USB host port of the DAC", "zh-TW": "USB \u5167\u5EFA\u8173\u4F4D\uFF0C\u63A5 DAC \u7684 USB \u4E3B\u6A5F\u57E0", "zh-CN": "USB \u5185\u7F6E\u5F15\u811A\uFF0C\u63A5 DAC \u7684 USB \u4E3B\u673A\u53E3" },
    "hw.res.strap": { en: "boot strapping pin: its level at power-up decides how the chip boots", "zh-TW": "\u958B\u6A5F\u8A2D\u5B9A\u8173\uFF08strapping\uFF09\uFF1A\u4E0A\u96FB\u6642\u7684\u96FB\u4F4D\u6C7A\u5B9A\u6676\u7247\u600E\u9EBC\u958B\u6A5F", "zh-CN": "\u5F00\u673A\u8BBE\u7F6E\u5F15\u811A\uFF08strapping\uFF09\uFF1A\u4E0A\u7535\u65F6\u7684\u7535\u5E73\u51B3\u5B9A\u82AF\u7247\u600E\u4E48\u542F\u52A8" },
    "hw.res.strap60": { en: "boot strapping pin; only the stock LED may use it, because that is known to boot. \u201CRestore defaults\u201D brings the LED back to it", "zh-TW": "\u958B\u6A5F\u8A2D\u5B9A\uFF08strapping\uFF09\u8173\uFF1B\u53EA\u6709\u677F\u5B50\u9810\u8A2D\u7684 LED \u80FD\u7528\u5B83\uFF0C\u56E0\u70BA\u5BE6\u6E2C\u80FD\u958B\u6A5F\u3002\u8981\u8B93 LED \u56DE\u5230\u5B83\uFF0C\u8ACB\u6309\u300C\u9084\u539F\u9810\u8A2D\u503C\u300D", "zh-CN": "\u5F00\u673A\u8BBE\u7F6E\uFF08strapping\uFF09\u5F15\u811A\uFF1B\u53EA\u6709\u677F\u5B50\u9ED8\u8BA4\u7684 LED \u80FD\u7528\u5B83\uFF0C\u56E0\u4E3A\u5B9E\u6D4B\u80FD\u542F\u52A8\u3002\u8981\u8BA9 LED \u56DE\u5230\u5B83\uFF0C\u8BF7\u6309\u201C\u6062\u590D\u9ED8\u8BA4\u503C\u201D" },
    "hw.res.console": { en: "the serial console UART0, which this page and the flashing tool use", "zh-TW": "\u5E8F\u5217\u4E3B\u63A7\u53F0 UART0\uFF0C\u672C\u9801\u548C\u71D2\u9304\u5DE5\u5177\u90FD\u9760\u5B83", "zh-CN": "\u4E32\u53E3\u63A7\u5236\u53F0 UART0\uFF0C\u672C\u9875\u548C\u70E7\u5F55\u5DE5\u5177\u90FD\u9760\u5B83" },
    "hw.res.other": { en: "reserved on this board", "zh-TW": "\u9019\u584A\u677F\u5B50\u4FDD\u7559\u7684\u8173", "zh-CN": "\u8FD9\u5757\u677F\u5B50\u4FDD\u7559\u7684\u5F15\u811A" },
    "hw.reserved.why": { en: "GPIO {n}: {why}.", "zh-TW": "GPIO {n}\uFF1A{why}\u3002", "zh-CN": "GPIO {n}\uFF1A{why}\u3002" },
    "fl.adv.title": { en: "Advanced", "zh-TW": "\u9032\u968E", "zh-CN": "\u9AD8\u7EA7" },
    "fl.adv.speed": { en: "Flashing speed (baud)", "zh-TW": "\u71D2\u9304\u901F\u7387\uFF08baud\uFF09", "zh-CN": "\u70E7\u5F55\u901F\u7387\uFF08baud\uFF09" },
    "fl.baud.460800": { en: "460800 (default, fastest)", "zh-TW": "460800\uFF08\u9810\u8A2D\uFF0C\u6700\u5FEB\uFF09", "zh-CN": "460800\uFF08\u9ED8\u8BA4\uFF0C\u6700\u5FEB\uFF09" },
    "fl.baud.230400": { en: "230400", "zh-TW": "230400", "zh-CN": "230400" },
    "fl.baud.115200": { en: "115200 (slowest, most tolerant)", "zh-TW": "115200\uFF08\u6700\u6162\uFF0C\u6700\u7A69\uFF09", "zh-CN": "115200\uFF08\u6700\u6162\uFF0C\u6700\u7A33\uFF09" },
    "fl.adv.hint": { en: "Leave it at 460800. If switching to 460800 or the first write fails, the page retries once by itself at 115200. Pick a lower speed here if flashing keeps failing with a long or poor cable or a cheap USB-serial adapter.", "zh-TW": "\u4FDD\u6301 460800 \u5C31\u597D\u3002\u5982\u679C\u5207\u63DB\u5230 460800 \u5931\u6557\uFF0C\u6216\u7B2C\u4E00\u6B21\u5BEB\u5165\u5931\u6557\uFF0C\u9801\u9762\u6703\u81EA\u52D5\u7528 115200 \u91CD\u8A66\u4E00\u6B21\u3002\u7DDA\u5F88\u9577\u3001\u7DDA\u6750\u5DEE\u6216 USB \u5E8F\u5217\u8F49\u63A5\u5668\u4E0D\u597D\u3001\u71D2\u9304\u4E00\u76F4\u5931\u6557\u6642\uFF0C\u53EF\u4EE5\u5728\u9019\u88E1\u9078\u8F03\u6162\u7684\u901F\u7387\u3002", "zh-CN": "\u4FDD\u6301 460800 \u5C31\u597D\u3002\u5982\u679C\u5207\u6362\u5230 460800 \u5931\u8D25\uFF0C\u6216\u7B2C\u4E00\u6B21\u5199\u5165\u5931\u8D25\uFF0C\u9875\u9762\u4F1A\u81EA\u52A8\u7528 115200 \u91CD\u8BD5\u4E00\u6B21\u3002\u7EBF\u5F88\u957F\u3001\u7EBF\u6750\u5DEE\u6216 USB \u4E32\u53E3\u8F6C\u63A5\u5668\u4E0D\u597D\u3001\u70E7\u5F55\u4E00\u76F4\u5931\u8D25\u65F6\uFF0C\u53EF\u4EE5\u5728\u8FD9\u91CC\u9009\u8F83\u6162\u7684\u901F\u7387\u3002" },
    "fl.speed.fb.connect": { en: "The board would not switch to {from} baud, so the page set the session up again at {to} baud. It is slower, but more forgiving of cables and USB-serial bridges.", "zh-TW": "\u6A5F\u5668\u6C92\u80FD\u5207\u63DB\u5230 {from} baud\uFF0C\u9801\u9762\u5DF2\u6539\u7528 {to} baud \u91CD\u65B0\u5EFA\u7ACB\u9023\u7DDA\u3002\u901F\u5EA6\u8F03\u6162\uFF0C\u4F46\u5C0D\u7DDA\u6750\u548C USB \u5E8F\u5217\u6A4B\u63A5\u6676\u7247\u6BD4\u8F03\u5BEC\u5BB9\u3002", "zh-CN": "\u673A\u5668\u6CA1\u80FD\u5207\u6362\u5230 {from} baud\uFF0C\u9875\u9762\u5DF2\u6539\u7528 {to} baud \u91CD\u65B0\u5EFA\u7ACB\u8FDE\u63A5\u3002\u901F\u5EA6\u8F83\u6162\uFF0C\u4F46\u5BF9\u7EBF\u6750\u548C USB \u4E32\u53E3\u6865\u63A5\u82AF\u7247\u6BD4\u8F83\u5BBD\u5BB9\u3002" },
    "fl.speed.fb.write": { en: "The first write at {from} baud failed, so the page set the session up again at {to} baud and tried once more. It is slower, but more forgiving of cables and USB-serial bridges.", "zh-TW": "\u7528 {from} baud \u7684\u7B2C\u4E00\u6B21\u5BEB\u5165\u5931\u6557\uFF0C\u9801\u9762\u5DF2\u6539\u7528 {to} baud \u91CD\u65B0\u9023\u7DDA\u518D\u8A66\u4E00\u6B21\u3002\u901F\u5EA6\u8F03\u6162\uFF0C\u4F46\u5C0D\u7DDA\u6750\u548C USB \u5E8F\u5217\u6A4B\u63A5\u6676\u7247\u6BD4\u8F03\u5BEC\u5BB9\u3002", "zh-CN": "\u7528 {from} baud \u7684\u7B2C\u4E00\u6B21\u5199\u5165\u5931\u8D25\uFF0C\u9875\u9762\u5DF2\u6539\u7528 {to} baud \u91CD\u65B0\u8FDE\u63A5\u518D\u8BD5\u4E00\u6B21\u3002\u901F\u5EA6\u8F83\u6162\uFF0C\u4F46\u5BF9\u7EBF\u6750\u548C USB \u4E32\u53E3\u6865\u63A5\u82AF\u7247\u6BD4\u8F83\u5BBD\u5BB9\u3002" },
    // ---- console ----
    "con.title": { en: "Console (for diagnostics)", "zh-TW": "\u4E3B\u63A7\u53F0\uFF08\u8A3A\u65B7\u7528\uFF09", "zh-CN": "\u63A7\u5236\u53F0\uFF08\u8BCA\u65AD\u7528\uFF09" },
    "con.note": { en: "Read-only log of the box's serial output, 115200 baud, with one line to type a command (try ver or hw). What follows \u201Cwifi set\u201D is hidden here.", "zh-TW": "\u6A5F\u5668\u5E8F\u5217\u8F38\u51FA\u7684\u552F\u8B80\u8A18\u9304\uFF08115200 baud\uFF09\uFF0C\u4E0B\u9762\u53EF\u4EE5\u8F38\u5165\u4E00\u884C\u6307\u4EE4\uFF08\u8A66\u8A66 ver \u6216 hw\uFF09\u3002\u300Cwifi set\u300D\u5F8C\u9762\u7684\u5167\u5BB9\u5728\u9019\u88E1\u6703\u88AB\u96B1\u85CF\u3002", "zh-CN": "\u673A\u5668\u4E32\u53E3\u8F93\u51FA\u7684\u53EA\u8BFB\u65E5\u5FD7\uFF08115200 baud\uFF09\uFF0C\u4E0B\u9762\u53EF\u4EE5\u8F93\u5165\u4E00\u884C\u6307\u4EE4\uFF08\u8BD5\u8BD5 ver \u6216 hw\uFF09\u3002\u201Cwifi set\u201D\u540E\u9762\u7684\u5185\u5BB9\u5728\u8FD9\u91CC\u4F1A\u88AB\u9690\u85CF\u3002" },
    "con.placeholder": { en: "command, e.g. ver", "zh-TW": "\u6307\u4EE4\uFF0C\u4F8B\u5982 ver", "zh-CN": "\u6307\u4EE4\uFF0C\u4F8B\u5982 ver" },
    "btn.con.send": { en: "Send", "zh-TW": "\u9001\u51FA", "zh-CN": "\u53D1\u9001" },
    "btn.con.clear": { en: "Clear", "zh-TW": "\u6E05\u9664", "zh-CN": "\u6E05\u9664" },
    "con.aria": { en: "Console log", "zh-TW": "\u4E3B\u63A7\u53F0\u8A18\u9304", "zh-CN": "\u63A7\u5236\u53F0\u65E5\u5FD7" },
    // ---- footer ----
    "foot.build": { en: "Build {stamp}", "zh-TW": "\u5EFA\u7F6E {stamp}", "zh-CN": "\u6784\u5EFA {stamp}" },
    "foot.legal": { en: "Page: LGPL-2.1-or-later. Flashing uses esptool-js (Apache-2.0).", "zh-TW": "\u9801\u9762\uFF1ALGPL-2.1-or-later\u3002\u71D2\u9304\u4F7F\u7528 esptool-js\uFF08Apache-2.0\uFF09\u3002", "zh-CN": "\u9875\u9762\uFF1ALGPL-2.1-or-later\u3002\u70E7\u5F55\u4F7F\u7528 esptool-js\uFF08Apache-2.0\uFF09\u3002" }
  };

  // src/js/core/i18n.js
  var g = globalThis;
  var KEY = "nsn.lang";
  var LANGS = ["en", "zh-TW", "zh-CN"];
  var CHAIN = { en: ["en"], "zh-TW": ["zh-TW", "en"], "zh-CN": ["zh-CN", "zh-TW", "en"] };
  var lang = "zh-TW";
  var urlLang = null;
  var listeners = [];
  var table = () => STRINGS;
  function normalize(tag) {
    const s = String(tag || "").trim().replace(/_/g, "-").toLowerCase();
    if (s === "en" || s.startsWith("en-")) return "en";
    if (s === "zh" || s.startsWith("zh-")) {
      const p = s.split("-").slice(1);
      if (p.includes("hans")) return "zh-CN";
      if (p.includes("hant")) return "zh-TW";
      return p.some((x) => x === "tw" || x === "hk" || x === "mo") ? "zh-TW" : "zh-CN";
    }
    return null;
  }
  function exact(tag) {
    const s = String(tag || "").toLowerCase();
    return LANGS.find((l) => l.toLowerCase() === s) || null;
  }
  function detect(list) {
    for (const tag of list || []) {
      const l = normalize(tag);
      if (l) return l;
    }
    return "en";
  }
  var browserLangs = () => g.navigator && (g.navigator.languages && g.navigator.languages.length ? g.navigator.languages : [g.navigator.language]) || [];
  function saved() {
    try {
      return exact(g.localStorage.getItem(KEY));
    } catch {
      return null;
    }
  }
  function has(key) {
    const row = table()[key];
    return !!row && CHAIN[lang].some((l) => typeof row[l] === "string");
  }
  function t(key, params) {
    const row = table()[key];
    let s = null;
    if (row) {
      for (const l of CHAIN[lang]) if (typeof row[l] === "string") {
        s = row[l];
        break;
      }
    }
    if (s === null) return String(key);
    if (params === void 0 || params === null) return s;
    return s.replace(/\{(\w+)\}/g, (m, name) => {
      if (!Object.prototype.hasOwnProperty.call(params, name)) return m;
      const v = params[name];
      return v && typeof v === "object" && "t" in v ? t(v.t, v.p) : String(v);
    });
  }
  function paramsOf(el2) {
    const p = el2.getAttribute("data-i18n-p");
    if (!p) return void 0;
    try {
      return JSON.parse(p);
    } catch {
      return void 0;
    }
  }
  var ATTRS = [["data-i18n-aria", "aria-label"], ["data-i18n-placeholder", "placeholder"], ["data-i18n-title", "title"]];
  function applyDom(root) {
    root = root || g.document;
    const doc = g.document;
    if (root === doc) {
      doc.documentElement.lang = lang;
      doc.title = t("app.title");
    }
    for (const el2 of root.querySelectorAll("[data-i18n]")) el2.textContent = t(el2.getAttribute("data-i18n"), paramsOf(el2));
    for (const [from, to] of ATTRS) for (const el2 of root.querySelectorAll("[" + from + "]")) el2.setAttribute(to, t(el2.getAttribute(from), paramsOf(el2)));
  }
  function setT(node, key, params) {
    node.setAttribute("data-i18n", key);
    if (params) node.setAttribute("data-i18n-p", JSON.stringify(params));
    else node.removeAttribute("data-i18n-p");
    node.textContent = t(key, params);
  }
  function setRaw(node, text) {
    node.removeAttribute("data-i18n");
    node.removeAttribute("data-i18n-p");
    node.textContent = text;
  }
  function init() {
    let u = null;
    try {
      u = normalize(new URLSearchParams(g.location.search).get("lang"));
    } catch {
    }
    urlLang = u;
    lang = u || saved() || detect(browserLangs());
    if (g.document) {
      applyDom(g.document);
    }
    return lang;
  }
  var pref = () => urlLang || saved() || "auto";
  function setLang(choice) {
    urlLang = null;
    if (choice === "auto") {
      try {
        g.localStorage.removeItem(KEY);
      } catch {
      }
      lang = detect(browserLangs());
    } else {
      const l = exact(choice);
      if (!l) return;
      lang = l;
      try {
        g.localStorage.setItem(KEY, l);
      } catch {
      }
    }
    if (g.document) applyDom(g.document);
    for (const fn of listeners.slice()) {
      try {
        fn(lang);
      } catch (e) {
        if (g.console) g.console.error(e);
      }
    }
  }
  function onChange(fn) {
    listeners.push(fn);
  }
  var I18N = g.NSN_I18N = { LANGS, t, has, init, setLang, pref, applyDom, setT, setRaw, onChange, lang: () => lang, detect, normalize };

  // src/js/core/proto.js
  var SD_PINS = ["clk", "cmd", "d0", "d1", "d2", "d3"];
  var SPI_PINS = ["clk", "mosi", "miso", "cs"];
  var LED_ORDERS = ["rgb", "grb", "brg", "rbg", "gbr", "bgr"];
  var ANSI = /\x1b\[[0-9;?]*[A-Za-z]/g;
  var stripAnsi = (s) => String(s).replace(ANSI, "");
  function createFramer(maxLine = 4096) {
    const dec = typeof TextDecoder !== "undefined" ? new TextDecoder("utf-8") : null;
    let buf = "";
    return {
      push(chunk) {
        buf += typeof chunk === "string" ? chunk : dec.decode(chunk, { stream: true });
        const out = [];
        const parts = buf.replace(/\r\n/g, "\n").replace(/\r/g, "\n").split("\n");
        buf = parts.pop();
        for (const p of parts) {
          const l = stripAnsi(p).trim();
          if (l) out.push(l);
        }
        if (buf.length > maxLine) {
          const l = stripAnsi(buf).trim();
          buf = "";
          if (l) out.push(l);
        }
        return out;
      },
      reset() {
        buf = "";
      }
    };
  }
  var kv = (s) => {
    const o = {};
    for (const m of s.matchAll(/(\w+)=(\S*)/g)) o[m[1]] = m[2];
    return o;
  };
  var int = (v, d = null) => {
    const n = Number.parseInt(v, 10);
    return Number.isFinite(n) ? n : d;
  };
  function describeVersion(v) {
    let s = String(v || "").trim();
    const dirty = /-dirty$/.test(s);
    if (dirty) s = s.slice(0, -6);
    s = s.replace(/^v(?=\d)/, "");
    const m = /^(\d[\w.+-]*?)(?:-\d+-g([0-9a-f]{4,})|-([0-9a-f]{6,}))?$/.exec(s);
    return { base: m ? m[1] : s, commit: m ? m[2] || m[3] || "" : "", dirty };
  }
  function parseVer(line) {
    const m = /^ver\s+(\S+)(.*)$/.exec(stripAnsi(line).trim());
    if (!m) return null;
    const built = /\bbuilt=(.*)$/.exec(m[2]);
    const o = kv(built ? m[2].slice(0, built.index) : m[2]);
    return { version: m[1], ...describeVersion(m[1]), idf: o.idf || "", chip: o.chip || "", built: built ? built[1].trim() : "" };
  }
  var isVerLine = (line) => parseVer(line) !== null;
  function parseHwLine(line, acc) {
    const s = stripAnsi(line).trim();
    let m;
    if (m = /^hw sdmmc\s+(\w+=.*)$/.exec(s)) {
      const o = kv(m[1]);
      acc.sdmmc = { routing: o.routing || "", clk: int(o.clk), cmd: int(o.cmd), d0: int(o.d0), d1: int(o.d1), d2: int(o.d2), d3: int(o.d3), spi: o.spi || "" };
      return true;
    }
    if (m = /^hw sd (\w+=.*)$/.exec(s)) {
      const o = kv(m[1]);
      const common = { src: o.src || "default", state: o.state || "none" };
      acc.sd = o.mode === "spi" || o.mode === void 0 && o.mosi !== void 0 ? { mode: "spi", clk: int(o.clk), mosi: int(o.mosi), miso: int(o.miso), cs: int(o.cs), ...common } : { mode: "sdmmc", clk: int(o.clk), cmd: int(o.cmd), d0: int(o.d0), d1: int(o.d1), d2: int(o.d2), d3: int(o.d3), width: int(o.width, 4), ...common };
      return true;
    }
    if (m = /^hw led (\w+=.*)$/.exec(s)) {
      const o = kv(m[1]);
      acc.led = { gpio: int(o.gpio, -1), order: o.order || "grb", on: int(o.on, 0) === 1, src: o.src || "default" };
      return true;
    }
    if (m = /^hw reserved(?:\s+(.*))?$/.exec(s)) {
      acc.reserved = (m[1] || "").split(/[\s,]+/).filter(Boolean).map((x) => int(x)).filter((n) => n !== null);
      return true;
    }
    if (m = /^hw gpios\s+(\d+)\s*-\s*(\d+)\s*$/.exec(s)) {
      acc.gpios = { min: int(m[1]), max: int(m[2]) };
      return true;
    }
    if (m = /^hw pending\s+([01])\b/.exec(s)) {
      acc.pending = m[1] === "1";
      return true;
    }
    return false;
  }
  function parseHw(lines) {
    const acc = {};
    for (const l of lines) parseHwLine(l, acc);
    return { sd: acc.sd || null, sdmmc: acc.sdmmc || null, led: acc.led || null, reserved: acc.reserved || [], gpios: acc.gpios || { min: 0, max: 61 }, pending: !!acc.pending, complete: acc.pending !== void 0 };
  }
  var isHwReadLine = (line) => parseHwLine(line, {});
  function parseAck(line) {
    const m = /^hw (sd|led|reset):\s*(ok|refused)\b[\s,:]*(.*)$/.exec(stripAnsi(line).trim());
    if (!m) return null;
    return { kind: m[1], ok: m[2] === "ok", text: m[3].trim() };
  }
  var isAckLine = (kind) => (line) => {
    const a = parseAck(line);
    return !!a && a.kind === kind;
  };
  function parseReason(text) {
    const s = String(text || "").trim();
    let m;
    if (/^range\b/.test(s)) return { code: "range" };
    if (/^syntax\b/.test(s)) return { code: "syntax" };
    if (/^storage\b/.test(s)) return { code: "storage" };
    if (m = /^reserved\s+(-?\d+)/.exec(s)) return { code: "reserved", n: int(m[1]) };
    if (m = /^duplicate\s+(-?\d+)/.exec(s)) return { code: "duplicate", n: int(m[1]) };
    if (/^width\b/.test(s)) return { code: "width" };
    if (/^order\b/.test(s)) return { code: "order" };
    if (m = /^sd\/led conflict\s+(-?\d+)/.exec(s)) return { code: "conflict", n: int(m[1]) };
    if (m = /^not supported\s*(.*)$/.exec(s)) return { code: "notsup", why: m[1].trim() };
    return { code: "other", raw: s };
  }
  function sdCommand(sd) {
    if (sd.mode === "spi") return `hw sd spi ${sd.clk} ${sd.mosi} ${sd.miso} ${sd.cs}`;
    const w = sd.width === 1 ? 1 : 4;
    const d = (k) => w === 1 ? -1 : sd[k];
    return `hw sd ${sd.clk} ${sd.cmd} ${sd.d0} ${d("d1")} ${d("d2")} ${d("d3")} ${w}`;
  }
  function ledCommand(led) {
    return led.on ? `hw led ${led.gpio} ${led.order}` : "hw led off";
  }
  function redact(line) {
    return String(line).replace(/^(\s*wifi\s+set)\b.*$/i, "$1 ***");
  }
  var isInt = (v) => typeof v === "number" && Number.isInteger(v);
  var sdFields = (sd) => sd.mode === "spi" ? SPI_PINS : sd.width === 1 ? ["clk", "cmd", "d0"] : SD_PINS;
  var sdPinList = (sd) => sdFields(sd).map((f) => sd[f]);
  var NOPIN = [29, 41];
  function pinReason(n) {
    if (NOPIN.includes(n)) return "nopin";
    if (n >= 26 && n <= 28 || n >= 30 && n <= 32) return "flash";
    if (n === 33 || n === 34) return "usb";
    if (n >= 36 && n <= 40) return "strap";
    if (n === 58 || n === 59) return "console";
    if (n === 60) return "strap60";
    if (n === 61) return "strap";
    return "other";
  }
  var pinExists = (n, hw) => n >= hw.gpios.min && n <= hw.gpios.max && !(NOPIN.includes(n) && hw.reserved.includes(n));
  var sdmmcPins = (hw) => hw.sdmmc ? SD_PINS.map((f) => hw.sdmmc[f]) : [];
  function validateSd(sd, hw, led = hw && hw.led) {
    const out = [], fields = sdFields(sd);
    if (sd.mode !== "spi") {
      if (sd.width !== 1 && sd.width !== 4) return [{ field: "width", code: "width" }];
      if (sd.width === 1 && ["d1", "d2", "d3"].some((f) => sd[f] !== -1)) return [{ field: "width", code: "width" }];
    }
    const bad = /* @__PURE__ */ new Set();
    for (const f of fields) if (!isInt(sd[f])) {
      out.push({ field: f, code: "int" });
      bad.add(f);
    }
    for (const f of fields) if (!bad.has(f) && !pinExists(sd[f], hw)) {
      out.push({ field: f, code: NOPIN.includes(sd[f]) ? "nopin" : "range", n: sd[f] });
      bad.add(f);
    }
    const exempt = new Set(sdmmcPins(hw)), res = new Set(hw.reserved);
    for (const f of fields) if (!bad.has(f) && res.has(sd[f]) && !exempt.has(sd[f])) {
      out.push({ field: f, code: "reserved", n: sd[f] });
      bad.add(f);
    }
    const seen = /* @__PURE__ */ new Map();
    for (const f of fields) {
      if (bad.has(f)) continue;
      const n = sd[f];
      if (seen.has(n)) {
        out.push({ field: f, code: "duplicate", n });
        out.push({ field: seen.get(n), code: "duplicate", n });
      } else seen.set(n, f);
    }
    if (led && led.on && led.gpio >= 0) {
      for (const f of fields) if (sd[f] === led.gpio && !bad.has(f)) {
        out.push({ field: f, code: "conflict", n: led.gpio });
        break;
      }
    }
    if (sd.mode !== "spi" && hw.sdmmc && !out.length) {
      const fixed = sd.clk === hw.sdmmc.clk && sd.cmd === hw.sdmmc.cmd && sd.d0 === hw.sdmmc.d0 && (sd.width === 1 || sd.d1 === hw.sdmmc.d1 && sd.d2 === hw.sdmmc.d2 && sd.d3 === hw.sdmmc.d3);
      if (!fixed) out.push({ field: "mode", code: "notsup" });
    }
    return dedupe(out);
  }
  function validateLed(led, hw, sd = hw && hw.sd) {
    const out = [], n = led.gpio;
    if (!led.on) return out;
    if (!LED_ORDERS.includes(led.order)) out.push({ field: "order", code: "order" });
    if (!isInt(n)) out.push({ field: "led", code: "int" });
    else if (!pinExists(n, hw)) out.push({ field: "led", code: NOPIN.includes(n) ? "nopin" : "range", n });
    else if (hw.reserved.includes(n) && n !== hw.ledDefault) out.push({ field: "led", code: "reserved", n });
    else if (sd && sdPinList(sd).includes(n)) out.push({ field: "led", code: "conflict", n });
    return out;
  }
  function dedupe(list) {
    const seen = /* @__PURE__ */ new Set();
    return list.filter((i) => {
      const k = i.field + ":" + i.code;
      if (seen.has(k)) return false;
      seen.add(k);
      return true;
    });
  }
  function stepGpio(n, dir, hw) {
    const res = new Set(hw.reserved);
    for (let v = n + dir; v >= hw.gpios.min && v <= hw.gpios.max; v += dir) if (!res.has(v)) return v;
    return null;
  }

  // src/js/core/conn.js
  var FILTERS = [{ usbVendorId: 6790 }, { usbVendorId: 4292 }, { usbVendorId: 1027 }, { usbVendorId: 12346 }];
  var VENDORS = { 6790: "CH340-family", 4292: "CP210x", 1027: "FTDI", 12346: "Espressif" };
  var hex4 = (n) => n.toString(16).padStart(4, "0");
  var defaultSleep = (ms) => new Promise((r) => setTimeout(r, ms));
  function portLabel(port) {
    let i = {};
    try {
      i = port && port.getInfo && port.getInfo() || {};
    } catch {
    }
    if (!i.usbVendorId) return "serial port";
    return (VENDORS[i.usbVendorId] || "USB serial") + " " + hex4(i.usbVendorId) + ":" + hex4(i.usbProductId || 0);
  }
  async function resetToApp(port, sleep = defaultSleep) {
    try {
      await port.setSignals({ dataTerminalReady: false, requestToSend: true });
      await sleep(100);
      await port.setSignals({ dataTerminalReady: false, requestToSend: false });
    } catch {
    }
  }
  function createConn({ serial = globalThis.navigator && globalThis.navigator.serial, sleep = defaultSleep, resetOnOpen = true, bootSettle = 150 } = {}) {
    let port = null, writer = null, reader = null, loop = null, state = "idle", why = "", stopping = false, chain = Promise.resolve();
    const subs = { state: [], line: [], tx: [], close: [] };
    const taps = /* @__PURE__ */ new Set();
    const enc = new TextEncoder();
    const err3 = (code, detail) => {
      const e = new Error(code + (detail ? ": " + detail : ""));
      e.code = code;
      e.detail = detail || "";
      return e;
    };
    const emit = (k, ...a) => {
      for (const f of subs[k]) try {
        f(...a);
      } catch (e) {
        console.error(e);
      }
    };
    const setState = (s, w) => {
      if (s === state && (w || "") === why) return;
      state = s;
      why = w || "";
      emit("state", s, why);
    };
    const failAll = (e) => {
      for (const t2 of [...taps]) t2.fail(e);
      taps.clear();
    };
    function onLine(l) {
      emit("line", l);
      for (const t2 of [...taps]) t2.line(l);
    }
    async function readLoop() {
      const fr = createFramer();
      reader = port.readable.getReader();
      try {
        for (; ; ) {
          const { value, done } = await reader.read();
          if (done) break;
          for (const l of fr.push(value)) onLine(l);
        }
      } catch {
      } finally {
        try {
          reader.releaseLock();
        } catch {
        }
        reader = null;
      }
      if (!stopping) teardown(false);
    }
    async function startIO() {
      await port.open({ baudRate: 115200, bufferSize: 65536 });
      stopping = false;
      writer = port.writable.getWriter();
      loop = readLoop();
      if (resetOnOpen) {
        await resetToApp(port, sleep);
        await sleep(bootSettle);
      }
    }
    async function stopIO() {
      stopping = true;
      const l = loop;
      if (reader) try {
        await reader.cancel();
      } catch {
      }
      if (l) try {
        await l;
      } catch {
      }
      loop = null;
      try {
        if (writer) writer.releaseLock();
      } catch {
      }
      writer = null;
      try {
        await port.close();
      } catch {
      }
    }
    function teardown(byUser, closed) {
      const was = state !== "idle";
      try {
        if (writer) writer.releaseLock();
      } catch {
      }
      writer = null;
      const p = port;
      port = null;
      loop = null;
      failAll(err3("lost"));
      if (p && !closed) Promise.resolve().then(() => p.close()).catch(() => {
      });
      setState("idle");
      if (was) emit("close", byUser);
    }
    if (serial && serial.addEventListener) serial.addEventListener("disconnect", (e) => {
      if (port && e.target === port) teardown(false);
    });
    async function open(o = {}) {
      if (state !== "idle") throw err3("busy");
      let p = o.port;
      if (!p) {
        if (!serial) throw err3("nosupport");
        try {
          p = await serial.requestPort(o.any ? {} : { filters: FILTERS });
        } catch (e) {
          if (e && e.name === "NotFoundError") return false;
          throw err3("open", e && e.message);
        }
      }
      port = p;
      try {
        await startIO();
      } catch (e) {
        try {
          if (writer) writer.releaseLock();
        } catch {
        }
        writer = null;
        port = null;
        throw err3("open", e && e.message);
      }
      setState("console");
      return true;
    }
    async function close() {
      if (!port) return;
      if (state === "flashing") {
        const p = port;
        port = null;
        setState("idle");
        emit("close", true);
        try {
          await p.close();
        } catch {
        }
        return;
      }
      failAll(err3("lost"));
      await stopIO();
      teardown(true, true);
    }
    const write = async (line) => {
      if (!writer) throw err3("notconn");
      await writer.write(enc.encode(line + "\n"));
    };
    function request(cmd, { accept = () => true, until = () => true, timeout = 2500, partial = false } = {}) {
      const run = async () => {
        if (state === "flashing") throw err3("busy");
        if (state !== "console" || !writer) throw err3("notconn");
        return new Promise((res, rej) => {
          const got = [];
          const tap = {
            line: (l) => {
              if (accept(l)) {
                got.push(l);
                if (until(got)) {
                  cleanup();
                  res(got);
                }
              }
            },
            fail: (e) => {
              cleanup();
              rej(e);
            }
          };
          const t2 = setTimeout(() => {
            cleanup();
            if (partial && got.length) res(got);
            else rej(err3("timeout"));
          }, timeout);
          const cleanup = () => {
            clearTimeout(t2);
            taps.delete(tap);
          };
          taps.add(tap);
          emit("tx", cmd);
          write(cmd).catch((e) => {
            cleanup();
            rej(err3("lost", e && e.message));
          });
        });
      };
      const p = chain.then(run, run);
      chain = p.catch(() => {
      });
      return p;
    }
    async function poll(cmd, opts = {}) {
      const { every = 700, total = 1e4, ...rest } = opts, t0 = Date.now();
      for (; ; ) {
        try {
          return await request(cmd, { timeout: every, ...rest });
        } catch (e) {
          if (e.code !== "timeout" || Date.now() - t0 + every > total) throw e;
        }
      }
    }
    async function send(cmd) {
      if (state === "flashing") throw err3("busy");
      if (state !== "console" || !writer) throw err3("notconn");
      emit("tx", cmd);
      await write(cmd);
    }
    async function pause(r = "") {
      if (state !== "console") throw err3(state === "flashing" ? "busy" : "notconn");
      setState("flashing", r);
      failAll(err3("busy"));
      await stopIO();
      return port;
    }
    async function resume() {
      if (state !== "flashing" || !port) throw err3("notconn");
      try {
        await startIO();
      } catch {
        teardown(false);
        throw err3("lost");
      }
      setState("console");
    }
    return {
      get state() {
        return state;
      },
      get why() {
        return why;
      },
      get port() {
        return port;
      },
      get up() {
        return state === "console" && !!writer;
      },
      get supported() {
        return !!serial;
      },
      get label() {
        return port ? portLabel(port) : "";
      },
      open,
      close,
      request,
      poll,
      send,
      pause,
      resume,
      on(k, f) {
        subs[k].push(f);
      }
    };
  }

  // src/js/core/md5.js
  function md5Hex(u8) {
    const K = new Int32Array(64), R = [7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21];
    for (let i = 0; i < 64; i++) K[i] = Math.floor(Math.abs(Math.sin(i + 1)) * 4294967296) | 0;
    const n = u8.length, total = (n + 8 >> 6) * 64 + 64, b = new Uint8Array(total);
    b.set(u8);
    b[n] = 128;
    const dv = new DataView(b.buffer);
    dv.setUint32(total - 8, n << 3 >>> 0, true);
    dv.setUint32(total - 4, Math.floor(n / 536870912), true);
    let a0 = 1732584193 | 0, b0 = 4023233417 | 0, c0 = 2562383102 | 0, d0 = 271733878 | 0;
    const M = new Int32Array(16);
    for (let o = 0; o < total; o += 64) {
      for (let i = 0; i < 16; i++) M[i] = dv.getInt32(o + 4 * i, true);
      let A = a0, B = b0, C = c0, D = d0;
      for (let i = 0; i < 64; i++) {
        let F, g2;
        if (i < 16) {
          F = B & C | ~B & D;
          g2 = i;
        } else if (i < 32) {
          F = D & B | ~D & C;
          g2 = 5 * i + 1 & 15;
        } else if (i < 48) {
          F = B ^ C ^ D;
          g2 = 3 * i + 5 & 15;
        } else {
          F = C ^ (B | ~D);
          g2 = 7 * i & 15;
        }
        F = F + A + K[i] + M[g2] | 0;
        A = D;
        D = C;
        C = B;
        const r = R[(i >> 4) * 4 + (i & 3)];
        B = B + (F << r | F >>> 32 - r) | 0;
      }
      a0 = a0 + A | 0;
      b0 = b0 + B | 0;
      c0 = c0 + C | 0;
      d0 = d0 + D | 0;
    }
    const out = new DataView(new ArrayBuffer(16));
    out.setInt32(0, a0, true);
    out.setInt32(4, b0, true);
    out.setInt32(8, c0, true);
    out.setInt32(12, d0, true);
    return [...new Uint8Array(out.buffer)].map((x) => x.toString(16).padStart(2, "0")).join("");
  }
  async function sha256Hex(bytes) {
    const d = await globalThis.crypto.subtle.digest("SHA-256", bytes);
    return [...new Uint8Array(d)].map((b) => b.toString(16).padStart(2, "0")).join("");
  }

  // src/js/demo/mock.js
  var DEFAULT_SD = { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4, mosi: -1, miso: -1, cs: -1 };
  var DEFAULT_LED = { gpio: 60, order: "grb" };
  var SLOT0 = { clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23 };
  var RESERVED = [26, 27, 28, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 58, 59, 60, 61];
  var GPIO_MAX = 61;
  var NO_GPIO = [29, 41];
  var ORDERS = ["rgb", "grb", "brg", "rbg", "gbr", "bgr"];
  var ESC = "\x1B";
  var NOISE = [
    "I (%t) bt_audio: a2dp idle",
    "I (%t) nsn: heap free 214380 min 198112",
    "W (%t) wifi: no stored network",
    "I (%t) card: no change",
    "D (%t) night: tick"
  ];
  var gpioExists = (g2) => g2 >= 0 && g2 <= GPIO_MAX && !NO_GPIO.includes(g2);
  var gpioReserved = (g2) => !gpioExists(g2) || RESERVED.includes(g2);
  var sdSpi = (clk, mosi, miso, cs) => ({ mode: "spi", clk, cmd: -1, d0: -1, d1: -1, d2: -1, d3: -1, width: 1, mosi, miso, cs });
  var sdUsed = (p) => p.mode === "spi" ? [p.clk, p.mosi, p.miso, p.cs] : p.width === 4 ? [p.clk, p.cmd, p.d0, p.d1, p.d2, p.d3] : [p.clk, p.cmd, p.d0];
  var sdSame = (a, b) => {
    const pa = sdUsed(a), pb = sdUsed(b);
    return a.mode === b.mode && pa.length === pb.length && pa.every((v, i) => v === pb[i]) && (a.mode === "spi" || a.width === b.width);
  };
  var isDefaultSdPin = (g2) => sdUsed(DEFAULT_SD).includes(g2);
  var isDefaultLedPin = (g2) => DEFAULT_LED.gpio === g2;
  function checkPins(pins, exempt) {
    for (const g2 of pins) if (!gpioExists(g2)) return "range";
    for (const g2 of pins) if (gpioReserved(g2) && !(exempt && exempt(g2))) return `reserved ${g2}`;
    for (let i = 0; i < pins.length; i++) for (let j = 0; j < i; j++) if (pins[i] === pins[j]) return `duplicate ${pins[i]}`;
    return null;
  }
  function checkSd(p, ledGpio) {
    if (p.mode === "sdmmc") {
      if (p.width !== 1 && p.width !== 4) return "width";
      if (p.width === 1 && (p.d1 !== -1 || p.d2 !== -1 || p.d3 !== -1)) return "width";
    }
    const pins = sdUsed(p), bad = checkPins(pins, isDefaultSdPin);
    if (bad) return bad;
    if (ledGpio >= 0 && pins.includes(ledGpio)) return `sd/led conflict ${ledGpio}`;
    if (p.mode === "sdmmc") {
      const fixed = p.clk === SLOT0.clk && p.cmd === SLOT0.cmd && p.d0 === SLOT0.d0 && (p.width === 1 || p.d1 === SLOT0.d1 && p.d2 === SLOT0.d2 && p.d3 === SLOT0.d3);
      if (!fixed) return "not supported sdmmc-pins-fixed";
    }
    return null;
  }
  function checkLed(l, sd) {
    if (!ORDERS.includes(l.order)) return "order";
    if (l.gpio < 0) return null;
    const bad = checkPins([l.gpio], isDefaultLedPin);
    if (bad) return bad;
    if (sdUsed(sd).includes(l.gpio)) return `sd/led conflict ${l.gpio}`;
    return null;
  }
  function parseSd(args) {
    const words = args.split(" ").filter(Boolean), k = words.length > 8 ? 9 : words.length;
    const spi = k > 0 && words[0] === "spi", n = spi ? 4 : 7, off = spi ? 1 : 0;
    if (k !== n + off) return { why: "syntax" };
    const v = [];
    for (let i = 0; i < n; i++) {
      const w = words[i + off];
      if (!/^[+-]?\d+$/.test(w)) return { why: "syntax" };
      const x = Number(w);
      if (x < -1e3 || x > 1e3) return { why: "syntax" };
      v.push(x);
    }
    if (v.some((x) => x < -1 || x > 127)) return { why: "range" };
    return { p: spi ? sdSpi(v[0], v[1], v[2], v[3]) : { mode: "sdmmc", clk: v[0], cmd: v[1], d0: v[2], d1: v[3], d2: v[4], d3: v[5], width: v[6], mosi: -1, miso: -1, cs: -1 } };
  }
  function createMockBoard(opts = {}) {
    const o = { noise: true, split: 0, bootMs: 900, sd: "mounted", spiCard: "none", refuse: "", version: "0.1.0-ee14d98-dirty", built: "874f52d83", old: false, silent: false, ...opts };
    const timers = /* @__PURE__ */ new Set();
    let out = null;
    let mode = "app";
    let t0 = Date.now();
    const board = {
      opts: o,
      store: { sd: null, led: null },
      // NVS: absent = the board default (hw_sd / hw_led)
      running: { sd: { ...DEFAULT_SD }, led: { ...DEFAULT_LED } },
      reserved: [...RESERVED, ...NO_GPIO].sort((a, b) => a - b),
      gpios: { min: 0, max: GPIO_MAX },
      log: [],
      // every line the host sent (for tests)
      get mode() {
        return mode;
      },
      attach(fn) {
        out = fn;
      }
    };
    const ms = () => Date.now() - t0 + 300;
    const later = (fn, ms2) => {
      const h = setTimeout(() => {
        timers.delete(h);
        fn();
      }, ms2);
      if (h && h.unref) h.unref();
      timers.add(h);
    };
    const emit = (text) => {
      if (!out) return;
      if (!o.split) {
        out(text);
        return;
      }
      for (let i = 0; i < text.length; i += o.split) out(text.slice(i, i + o.split));
    };
    const line = (s) => emit(s + "\r\n");
    function loadBoth() {
      let sd = board.store.sd ? { ...board.store.sd } : { ...DEFAULT_SD }, led = board.store.led ? { ...board.store.led } : { ...DEFAULT_LED };
      let sdC = !!board.store.sd, ledC = !!board.store.led;
      if (ledC && checkLed(led, sdC ? sd : DEFAULT_SD)) {
        led = { ...DEFAULT_LED };
        ledC = false;
      }
      if (sdC && checkSd(sd, led.gpio)) {
        sd = { ...DEFAULT_SD };
        sdC = false;
      }
      if (sdC && sdSame(sd, DEFAULT_SD)) sdC = false;
      if (ledC && led.gpio === DEFAULT_LED.gpio && led.order === DEFAULT_LED.order) ledC = false;
      return { sd, led, sdC, ledC };
    }
    const cardState = () => {
      const sd = board.running.sd;
      if (sd.mode === "spi") return o.spiCard;
      return o.sd;
    };
    const pending = () => {
      const c = loadBoth();
      return !sdSame(c.sd, board.running.sd) || c.led.order !== board.running.led.order || c.led.gpio !== board.running.led.gpio;
    };
    function noiseLine() {
      return NOISE[Math.floor(Math.random() * NOISE.length)].replace("%t", String(ms()));
    }
    function scheduleNoise() {
      if (!o.noise) return;
      later(() => {
        if (mode === "app") {
          line(ESC + "[0;32m" + noiseLine() + ESC + "[0m");
          scheduleNoise();
        }
      }, 600 + Math.random() * 900);
    }
    board.boot = () => {
      for (const h of timers) clearTimeout(h);
      timers.clear();
      mode = "booting";
      t0 = Date.now();
      const c = loadBoth();
      board.running = { sd: c.sd, led: c.led };
      if (o.silent) return;
      const sd = c.sd, pins = sdUsed(sd).join(" ");
      const steps = [
        "ESP-ROM:esp32s31-20260101",
        "Build:Jan  1 2026",
        "rst:0x1 (POWERON)",
        "I (31) boot: ESP-IDF v6.1 2nd stage bootloader",
        "I (120) boot: Loaded app from partition at offset 0x10000",
        "I (480) nsn: NightStand Nano " + o.version,
        `I (520) hw: sd ${sd.mode === "spi" ? "spi" : "sdmmc"} ${c.sdC ? "custom" : "default"}: ${pins}; led ${c.ledC ? "custom" : "default"} gpio ${c.led.gpio} ${c.led.order}`,
        "I (700) card: mount " + (cardState() === "mounted" ? "ok" : cardState() === "none" ? "no card" : "failed 0x107"),
        "I (780) nsn: ready"
      ];
      steps.forEach((s, i) => later(() => line(s), o.bootMs * i / steps.length));
      later(() => {
        mode = "app";
        scheduleNoise();
      }, o.bootMs);
    };
    board.enterDownload = () => {
      for (const h of timers) clearTimeout(h);
      timers.clear();
      mode = "download";
    };
    board.stop = () => {
      for (const h of timers) clearTimeout(h);
      timers.clear();
      mode = "off";
      out = null;
    };
    function show2() {
      const c = loadBoth(), sd = c.sd, src = c.sdC ? "custom" : "default";
      line(sd.mode === "spi" ? `hw sd clk=${sd.clk} mosi=${sd.mosi} miso=${sd.miso} cs=${sd.cs} src=${src} state=${cardState()} mode=spi` : `hw sd clk=${sd.clk} cmd=${sd.cmd} d0=${sd.d0} d1=${sd.d1} d2=${sd.d2} d3=${sd.d3} width=${sd.width} src=${src} state=${cardState()} mode=sdmmc`);
      line(`hw sdmmc routing=iomux clk=${SLOT0.clk} cmd=${SLOT0.cmd} d0=${SLOT0.d0} d1=${SLOT0.d1} d2=${SLOT0.d2} d3=${SLOT0.d3} spi=any`);
      if (o.noise) line(ESC + "[0;33mW (" + ms() + ") wifi: scan done" + ESC + "[0m");
      line(`hw led gpio=${c.led.gpio} order=${c.led.order} on=${c.led.gpio >= 0 ? 1 : 0} src=${c.ledC ? "custom" : "default"}`);
      line("hw reserved " + board.reserved.join(","));
      line(`hw gpios 0-${GPIO_MAX}`);
      line("hw pending " + (pending() ? 1 : 0));
    }
    const saveSd = (p) => {
      board.store.sd = sdSame(p, DEFAULT_SD) ? null : { ...p };
      return true;
    };
    const saveLed = (l) => {
      board.store.led = l.gpio === DEFAULT_LED.gpio && l.order === DEFAULT_LED.order ? null : { ...l };
      return true;
    };
    function setSd(args) {
      const r = parseSd(args);
      if (!r.p) return line(`hw sd: refused ${r.why}`);
      const why = checkSd(r.p, loadBoth().led.gpio);
      if (why) return line(`hw sd: refused ${why}`);
      line(saveSd(r.p) ? "hw sd: ok, reboot to apply" : "hw sd: refused storage");
    }
    function setLed(args) {
      const t2 = args.split(" ").filter(Boolean), k = t2.length > 2 ? 3 : t2.length;
      const l = { ...DEFAULT_LED };
      if (k === 1 && t2[0] === "off") l.gpio = -1;
      else {
        if (k < 1 || k > 2 || !/^[+-]?\d+$/.test(t2[0]) || Math.abs(Number(t2[0])) > 1e3) return line("hw led: refused syntax");
        const g2 = Number(t2[0]);
        if (g2 < 0 || g2 > 127) return line("hw led: refused range");
        l.gpio = g2;
        if (k === 2) {
          if (t2[1].length >= 4) return line("hw led: refused order");
          l.order = t2[1];
        }
      }
      const why = checkLed(l, loadBoth().sd);
      if (why) return line(`hw led: refused ${why}`);
      line(saveLed(l) ? "hw led: ok, reboot to apply" : "hw led: refused storage");
    }
    function hwExec(args) {
      if (!args) return show2();
      const a = args.slice(0, 95), i = a.indexOf(" "), head = i < 0 ? a : a.slice(0, i), rest = i < 0 ? null : a.slice(i + 1);
      if (o.refuse && ((head === "sd" || head === "led") && rest !== null)) return line(`hw ${head}: refused ${o.refuse}`);
      if (head === "sd" && rest !== null) return setSd(rest);
      if (head === "led" && rest !== null) return setLed(rest);
      if (head === "reset" && rest === null) {
        board.store.sd = null;
        board.store.led = null;
        return line("hw reset: ok, reboot to apply");
      }
      line("?");
    }
    function cmd(raw) {
      const s = raw.trim();
      board.log.push(s);
      if (!s) return;
      if (mode !== "app") return;
      if (o.silent) return;
      if (s === "ver") return line(`ver ${o.version} idf=v6.1 chip=esp32s31 built=${o.built}`);
      if (o.old && /^hw\b/.test(s)) return line("unknown command: " + s);
      if (/^hw(\s|$)/.test(s)) return hwExec(s.slice(2).trim());
      if (s === "reboot") return board.boot();
      if (/^wifi set\b/.test(s)) return line("wifi: ok");
      line("unknown command: " + s.split(/\s+/)[0]);
    }
    board.receive = (text) => {
      for (const l of String(text).split(/\r?\n/)) if (l.length) cmd(l);
    };
    return board;
  }
  var MockPort = class {
    constructor(opts = {}) {
      this.board = createMockBoard(opts);
      this.signals = [];
      this.baud = 0;
      this.isOpen = false;
      this.readable = null;
      this.writable = null;
      this.vid = opts.vid || 6790;
      this.pid = opts.pid || 29987;
      this._rts = false;
      this._dtr = false;
      this.booted = false;
    }
    getInfo() {
      return { usbVendorId: this.vid, usbProductId: this.pid };
    }
    async open({ baudRate }) {
      if (this.isOpen) throw new Error("InvalidStateError: port already open");
      this.isOpen = true;
      this.baud = baudRate;
      const enc = new TextEncoder(), dec = new TextDecoder();
      let ctl;
      this.readable = new ReadableStream({ start: (c) => {
        ctl = c;
      }, cancel: () => {
        this.board.attach(null);
      } });
      this.board.attach((text) => {
        try {
          ctl.enqueue(enc.encode(text));
        } catch {
        }
      });
      this.writable = new WritableStream({ write: (chunk) => {
        this.board.receive(dec.decode(chunk, { stream: true }));
      } });
      this._ctl = ctl;
      if (!this.booted) {
        this.booted = true;
        this.board.boot();
      }
    }
    async close() {
      this.isOpen = false;
      this.board.attach(null);
      const r = this.readable, w = this.writable;
      this.readable = null;
      this.writable = null;
      try {
        if (r && !r.locked) await r.cancel();
      } catch {
      }
      try {
        if (w && !w.locked) await w.close();
      } catch {
      }
    }
    async setSignals(s) {
      this.signals.push(s);
      if ("requestToSend" in s) {
        const was = this._rts;
        this._rts = !!s.requestToSend;
        if ("dataTerminalReady" in s) this._dtr = !!s.dataTerminalReady;
        if (was && !this._rts && !this._dtr) this.board.boot();
      }
    }
    unplug() {
      this.board.stop();
      const ctl = this._ctl;
      try {
        ctl && ctl.close();
      } catch {
      }
      this.isOpen = false;
      if (this.onunplug) this.onunplug(this);
    }
  };
  function createMockSerial(opts = {}) {
    const ports = [];
    const listeners2 = [];
    return {
      ports,
      requestCalls: [],
      async requestPort(o) {
        this.requestCalls.push(o);
        const p = new MockPort(opts);
        p.addEventListener = () => {
        };
        p.onunplug = (pp) => {
          for (const l of listeners2) l({ target: pp });
        };
        ports.push(p);
        return p;
      },
      addEventListener(t2, f) {
        if (t2 === "disconnect") listeners2.push(f);
      }
    };
  }
  function createMockEsp(opts = {}) {
    const st = { stubCalls: 0, mains: 0, written: /* @__PURE__ */ new Map(), writeCalls: [], opened: null, ...opts };
    const o = { speed: 8, steps: 40, ...opts };
    class Transport {
      constructor(port) {
        this.port = port;
        this.tracing = false;
      }
      async connect(baud) {
        await this.port.open({ baudRate: baud });
      }
      async disconnect() {
        await this.port.close();
      }
      getInfo() {
        return "mock";
      }
      getVid() {
        return this.port.vid;
      }
      getPid() {
        return this.port.pid;
      }
    }
    class ESPLoader {
      constructor(a) {
        this.transport = a.transport;
        this.baudrate = a.baudrate;
        this.romBaudrate = a.romBaudrate;
        this.IS_STUB = false;
        this.terminal = a.terminal;
        this.chip = null;
        st.loader = this;
      }
      async runStub() {
        st.stubCalls++;
        this.IS_STUB = true;
        return this.chip;
      }
      async main() {
        st.mains++;
        await this.transport.connect(this.romBaudrate);
        const b = this.transport.port.board;
        if (b) b.enterDownload();
        this.terminal.writeLine("Connecting...");
        this.chip = { CHIP_NAME: o.chip || "ESP32-S31" };
        this.terminal.writeLine("Chip is " + this.chip.CHIP_NAME);
        await this.runStub();
        if (this.romBaudrate !== this.baudrate) {
          if (o.maxBaud !== void 0 && this.baudrate > o.maxBaud) throw new Error("Failed to change the baud rate to " + this.baudrate + " (mock)");
          this.terminal.writeLine("Changed baud rate to " + this.baudrate);
        }
        return this.chip;
      }
      async detectFlashSize() {
        return o.flashSize === void 0 ? "16MB" : o.flashSize;
      }
      async writeFlash({ fileArray, reportProgress }) {
        st.writeCalls.push(fileArray.map((f) => ({ address: f.address, length: f.data.length, baud: this.baudrate })));
        if (o.writeMaxBaud !== void 0 && this.baudrate > o.writeMaxBaud) throw new Error("A fatal error occurred: write failed at " + this.baudrate + " baud (mock)");
        for (let i = 0; i < fileArray.length; i++) {
          const f = fileArray[i], n = o.steps;
          reportProgress(i, 0, f.data.length);
          for (let k = 1; k <= n; k++) {
            await new Promise((r) => setTimeout(r, o.speed));
            const frac = (i + k / n) / fileArray.length;
            if (o.stopAt !== void 0 && frac >= o.stopAt) await new Promise(() => {
            });
            if (o.failAt !== void 0 && frac >= o.failAt) throw new Error("A fatal error occurred: write failed (mock)");
            reportProgress(i, Math.round(f.data.length * k / n), f.data.length);
          }
          const copy = new Uint8Array(f.data);
          if (o.corrupt) copy[0] ^= 1;
          st.written.set(f.address, copy);
        }
      }
      async flashMd5sum(offset, len) {
        const d = st.written.get(offset);
        const hexs = md5Hex(d ? d.subarray(0, len) : new Uint8Array(len));
        if (o.noRomMd5Shape) return hexs;
        return [...hexs].map((c) => c.charCodeAt(0).toString(16)).join("");
      }
    }
    return { Transport, ESPLoader, state: st };
  }

  // src/js/core/flasher.js
  var ESPTOOL_VERSION = "0.7.1";
  var ESPTOOL_PATH = "vendor/esptool-js/" + ESPTOOL_VERSION + "/bundle.js";
  var FLASH_BAUD = 460800;
  var ROM_BAUD = 115200;
  var BASE = globalThis.document && globalThis.document.currentScript && globalThis.document.currentScript.src || globalThis.location && globalThis.location.href || "";
  var err = (code, extra) => {
    const e = new Error(code + (extra ? ": " + extra : ""));
    e.code = code;
    e.extra = extra;
    return e;
  };
  var lib = null;
  async function loadEsptool() {
    if (lib) return lib;
    const url = new URL(ESPTOOL_PATH, BASE).href;
    try {
      lib = await import(
        /* webpackIgnore: true */
        url
      );
    } catch (e) {
      throw err("lib", e && e.message);
    }
    return lib;
  }
  var setEsptool = (m) => {
    lib = m;
  };
  async function romMd5(loader, offset, len) {
    const r = String(await loader.flashMd5sum(offset, len)).toLowerCase();
    if (r.length === 64 && /^[0-9a-f]+$/.test(r)) {
      let s = "";
      for (let i = 0; i < 64; i += 2) s += String.fromCharCode(parseInt(r.slice(i, i + 2), 16));
      return s.toLowerCase();
    }
    return r;
  }
  var flashBytesOf = (fs) => {
    if (!fs) return 0;
    const n = parseInt(fs, 10);
    return /KB/i.test(fs) ? n * 1024 : n * 1048576;
  };
  async function beginSession({ conn: conn2, esp: esp2, log = () => {
  }, baud = FLASH_BAUD, romBaud = ROM_BAUD, onFallback = () => {
  } }) {
    const E = esp2 || await loadEsptool();
    if (conn2.state === "idle") throw err("notconn");
    const port = await conn2.pause("flash");
    let loader = null, transport = null, ended = false, cur = baud;
    const release = async () => {
      if (ended) return false;
      ended = true;
      try {
        if (transport) await transport.disconnect();
      } catch {
      }
      try {
        await conn2.resume();
        return false;
      } catch {
        return true;
      }
    };
    const connect = async (b) => {
      transport = new E.Transport(port, false);
      loader = new E.ESPLoader({ transport, baudrate: b, romBaudrate: romBaud, terminal: { clean() {
      }, writeLine: log, write: log }, debugLogging: false });
      loader.runStub = async function() {
        log("ROM loader only (no stub)");
        return this.chip;
      };
      cur = b;
      await loader.main("default_reset");
    };
    const info = { chip: "", flashSize: "", flashBytes: 0, baud };
    const fallBack = async (stage, e) => {
      const from = cur;
      log(`${stage === "write" ? "write" : "baud change"} at ${from} failed (${e && e.message || e}): again at ${romBaud}`);
      try {
        if (transport) await transport.disconnect();
      } catch {
      }
      info.baud = romBaud;
      onFallback({ from, to: romBaud, stage, message: e && e.message || String(e) });
      await connect(romBaud);
    };
    try {
      try {
        await connect(baud);
      } catch (e) {
        if (baud <= romBaud) throw e;
        await fallBack("connect", e);
      }
      info.chip = loader.chip.CHIP_NAME;
      const flashSize = await loader.detectFlashSize();
      info.flashSize = flashSize || "";
      info.flashBytes = flashBytesOf(flashSize);
      return {
        info,
        // parts [{name, offset, data}]; progress f in 0..1 over all parts, plus what is happening. A failed first write above romBaud means "fall back and go on"
        // (the retries that are left then run at romBaud); at romBaud a failure is simply retried.
        async write(parts, { onProgress = () => {
        }, retries = 2 } = {}) {
          const sizes = parts.map((p) => p.data.length), total = sizes.reduce((a, b) => a + b, 0);
          let lastErr;
          for (let attempt = 0; attempt <= retries; attempt++) {
            try {
              await loader.writeFlash({
                fileArray: parts.map((p) => ({ data: p.data, address: p.offset })),
                flashMode: "keep",
                flashFreq: "keep",
                flashSize: "keep",
                // the build's bootloader header (dio / 80 MHz / 16 MB) is written as it is
                eraseAll: false,
                compress: true,
                reportProgress: (i, written, t2) => onProgress((sizes.slice(0, i).reduce((a, b) => a + b, 0) + written * sizes[i] / t2) / total, { phase: "write", index: i, name: parts[i].name })
              });
              lastErr = null;
              break;
            } catch (e) {
              lastErr = e;
              log("retry " + (attempt + 1) + ": " + (e && e.message));
              if (cur > romBaud && attempt < retries) {
                try {
                  await fallBack("write", e);
                } catch (e2) {
                  lastErr = e2;
                  break;
                }
              }
            }
          }
          if (lastErr) throw lastErr;
          for (let i = 0; i < parts.length; i++) {
            onProgress(1, { phase: "verify", index: i, name: parts[i].name });
            const want = md5Hex(parts[i].data), got = await romMd5(loader, parts[i].offset, parts[i].data.length);
            if (want !== got) throw err("verify", parts[i].name);
          }
        },
        async end() {
          return release();
        }
      };
    } catch (e) {
      await release();
      throw e;
    }
  }

  // src/js/ui/dom.js
  var $ = (sel, root = document) => root.querySelector(sel);
  function el(tag, props = {}, ...kids) {
    const n = document.createElement(tag);
    for (const [k, v] of Object.entries(props || {})) {
      if (v === false || v === null || v === void 0) continue;
      if (k === "class") n.className = v;
      else if (k === "text") n.textContent = v;
      else if (k.startsWith("on")) n.addEventListener(k.slice(2), v);
      else n.setAttribute(k, v === true ? "" : String(v));
    }
    for (const c of kids.flat()) if (c !== null && c !== void 0 && c !== false) n.append(c.nodeType ? c : document.createTextNode(String(c)));
    return n;
  }
  var show = (n, on) => {
    n.hidden = !on;
  };

  // src/js/core/release.js
  var LAYOUT = {
    bootloader: { off: 8192, max: 24576 },
    // up to the partition table
    "partition-table": { off: 32768, size: 3072 },
    app: { off: 65536 },
    flashBytes: 16 << 20
  };
  var DEFAULT_NVS = { off: 36864, size: 24576 };
  var CHIP_ID = 32;
  var APP_DESC_MAGIC = 2882360370;
  var EXPECT_FLASH = { mode: "dio", freq: "80m", size: "16MB" };
  var ROLES = ["bootloader", "partition-table", "app"];
  var err2 = (code, extra) => {
    const e = new Error(code + (extra !== void 0 ? ": " + extra : ""));
    e.code = code;
    e.extra = extra;
    return e;
  };
  var hex = (n) => "0x" + n.toString(16);
  var u32 = (d, o) => (d[o] | d[o + 1] << 8 | d[o + 2] << 16 | d[o + 3] << 24) >>> 0;
  var cstr = (d, o, n) => {
    let e = o;
    while (e < o + n && d[e]) e++;
    return String.fromCharCode(...d.subarray(o, e));
  };
  var allFF = (d, a, b) => {
    for (let i = a; i < b; i++) if (d[i] !== 255) return false;
    return true;
  };
  var hexOf = (d) => [...d].map((b) => b.toString(16).padStart(2, "0")).join("");
  var VER = "([0-9A-Za-z][0-9A-Za-z._+]*(?:-[0-9A-Za-z._+]+)*?)";
  function parseName(name) {
    const n = String(name || "").split(/[\\/]/).pop();
    let m = new RegExp("^NightStandNano-" + VER + "-full\\.bin$").exec(n);
    if (m) return { kind: "full", version: m[1] };
    m = new RegExp("^NightStandNano-" + VER + "-(bootloader|partition-table|app)\\.bin$").exec(n);
    if (m) return { kind: "part", role: m[2], version: m[1] };
    if (n === "bootloader.bin") return { kind: "raw", role: "bootloader" };
    if (n === "partition-table.bin") return { kind: "raw", role: "partition-table" };
    if (n === "nightstand_nano.bin") return { kind: "raw", role: "app" };
    return null;
  }
  var MODES = ["qio", "qout", "dio", "dout"];
  var FREQS = { 15: "80m", 0: "40m", 1: "26m", 2: "20m" };
  var SIZES = ["1MB", "2MB", "4MB", "8MB", "16MB", "32MB", "64MB", "128MB"];
  function flashParams(d, off) {
    const fp = { mode: MODES[d[off + 2]] || "?", freq: FREQS[d[off + 3] & 15] || "?", size: SIZES[d[off + 3] >> 4] || "?" };
    fp.matches = fp.mode === EXPECT_FLASH.mode && fp.freq === EXPECT_FLASH.freq && fp.size === EXPECT_FLASH.size;
    return fp;
  }
  async function espImage(d, off, who) {
    if (off + 32 > d.length || d[off] !== 233) throw err2("no_magic", who);
    const chip = d[off + 12] | d[off + 13] << 8;
    if (chip !== CHIP_ID) throw err2("chip", who + " (" + chip + ")");
    let p = off + 24;
    for (let i = 0; i < d[off + 1]; i++) {
      if (p + 8 > d.length) throw err2("no_magic", who);
      p += 8 + u32(d, p + 4);
      if (p > d.length) throw err2("size", who);
    }
    p = (p - off | 15) + off + 1;
    let end = p;
    if (d[off + 23] === 1) {
      if (end + 32 > d.length) throw err2("hash", who);
      if (await sha256Hex(d.subarray(off, end)) !== hexOf(d.subarray(end, end + 32))) throw err2("hash", who);
      end += 32;
    }
    const hasDesc = u32(d, off + 32) === APP_DESC_MAGIC;
    return { start: off, end, hasDesc, flash: flashParams(d, off) };
  }
  function appDesc(d, off) {
    return { version: cstr(d, off + 48, 32), projectName: cstr(d, off + 80, 32), buildTime: cstr(d, off + 112, 16), buildDate: cstr(d, off + 128, 16), idf: cstr(d, off + 144, 32) };
  }
  function partitionTable(d, who = "partition-table") {
    const entries = [];
    let p = 0, md5ok = false;
    for (; p + 32 <= d.length && p < LAYOUT["partition-table"].size; p += 32) {
      const m = d[p] | d[p + 1] << 8;
      if (m === 20650) entries.push({ type: d[p + 2], subtype: d[p + 3], offset: u32(d, p + 4), size: u32(d, p + 8), name: cstr(d, p + 12, 16) });
      else if (m === 60395) {
        md5ok = md5Hex(d.subarray(0, p)) === hexOf(d.subarray(p + 16, p + 32));
        break;
      } else break;
    }
    if (!entries.length) throw err2("no_pt", who);
    if (!md5ok) throw err2("pt_md5", who);
    return { entries, usedBytes: p + 32 };
  }
  function checkTable(entries) {
    const app = entries.find((e) => e.type === 0 && e.offset === LAYOUT.app.off);
    if (!app) throw err2("pt_layout", hex(LAYOUT.app.off));
    for (const e of entries) if (e.offset + e.size > LAYOUT.flashBytes || e.offset < 36864) throw err2("pt_layout", e.name);
    const nvsE = entries.find((e) => e.type === 1 && e.subtype === 2);
    return { app, nvs: nvsE ? { off: nvsE.offset, size: nvsE.size, fromTable: true } : { ...DEFAULT_NVS, fromTable: false } };
  }
  async function classify(f) {
    const d = f.data;
    if (d.length >= 8192 + 32 && d[8192] === 233 && allFF(d, 0, 8192)) return "merged";
    if (d[0] === 170 && d[1] === 80) return "partition-table";
    if (d[0] === 233 && d.length >= 36) return u32(d, 32) === APP_DESC_MAGIC ? "app" : "bootloader";
    throw err2("no_magic", f.name);
  }
  async function analyze(files) {
    if (!files || !files.length) throw err2("none");
    if (files.length !== 1 && files.length !== 3) throw err2("count", String(files.length));
    const kinds = [];
    for (const f of files) kinds.push(await classify(f));
    let rel;
    if (files.length === 1) {
      const ni = parseName(files[0].name);
      if (kinds[0] !== "merged" && ni && ni.kind === "full") throw err2("not_merged", files[0].name);
      if (kinds[0] !== "merged") {
        if (ROLES.includes(kinds[0])) throw err2("need_all", kinds[0]);
        throw err2("no_magic", files[0].name);
      }
      rel = await analyzeMerged(files[0]);
    } else {
      if (kinds.includes("merged")) throw err2("mixed", files.find((f, i) => kinds[i] === "merged").name);
      for (const r of ROLES) if (kinds.filter((k) => k === r).length !== 1) throw err2("roles", r);
      rel = await analyzeThree(Object.fromEntries(files.map((f, i) => [kinds[i], f])));
    }
    rel.sha256 = Object.fromEntries(await Promise.all(files.map(async (f) => [f.name, await sha256Hex(f.data)])));
    return rel;
  }
  async function finish(kind, parts, tableEntries, appImgDesc, bootFlash, names) {
    const { app, nvs } = checkTable(tableEntries);
    const appPart = parts.find((p) => p.role === "app");
    if (appPart.data.length > app.size) throw err2("app_big", hex(app.size));
    const desc = appDesc(appPart.data, 0);
    const name0 = names.find(Boolean) || null;
    return {
      kind,
      parts,
      entries: tableEntries,
      nvs,
      version: desc.version || name0 && name0.version || "",
      projectName: desc.projectName,
      buildTime: desc.buildTime,
      buildDate: desc.buildDate,
      idf: desc.idf,
      flash: bootFlash,
      flashNeeded: Math.max(...tableEntries.map((e) => e.offset + e.size)),
      nameInfo: name0
    };
  }
  var part = (role, file, off, data) => ({ role, file, offset: off, data, size: data.length });
  async function analyzeMerged(f) {
    const d = f.data;
    if (d.length < LAYOUT.app.off + 64 || d.length > LAYOUT.flashBytes) throw err2("size", f.name);
    const boot = await espImage(d, LAYOUT.bootloader.off, f.name + " @0x2000");
    if (boot.hasDesc) throw err2("no_magic", f.name + " @0x2000");
    if (boot.end > LAYOUT["partition-table"].off) throw err2("size", "bootloader");
    const ptEnd = LAYOUT["partition-table"].off + LAYOUT["partition-table"].size;
    const pt = partitionTable(d.subarray(LAYOUT["partition-table"].off, ptEnd), f.name + " @0x8000");
    const app = await espImage(d, LAYOUT.app.off, f.name + " @0x10000");
    if (!app.hasDesc) throw err2("no_desc", f.name);
    if (!allFF(d, boot.end, LAYOUT["partition-table"].off)) throw err2("gap", hex(boot.end));
    if (!allFF(d, ptEnd, LAYOUT.app.off)) throw err2("gap", hex(ptEnd));
    if (!allFF(d, app.end, d.length)) throw err2("gap", hex(app.end));
    const parts = [
      part("bootloader", f.name, LAYOUT.bootloader.off, d.subarray(LAYOUT.bootloader.off, boot.end)),
      part("partition-table", f.name, LAYOUT["partition-table"].off, d.subarray(LAYOUT["partition-table"].off, LAYOUT["partition-table"].off + pt.usedBytes)),
      part("app", f.name, LAYOUT.app.off, d.subarray(LAYOUT.app.off, app.end))
    ];
    return finish("merged", parts, pt.entries, null, boot.flash, [parseName(f.name)]);
  }
  async function analyzeThree(by) {
    const b = by.bootloader, t2 = by["partition-table"], a = by.app;
    const boot = await espImage(b.data, 0, b.name);
    if (b.data.length > LAYOUT.bootloader.max) throw err2("size", b.name);
    if (!allFF(b.data, boot.end, b.data.length)) throw err2("gap", b.name);
    if (t2.data.length > LAYOUT["partition-table"].size) throw err2("size", t2.name);
    const pt = partitionTable(t2.data, t2.name);
    const app = await espImage(a.data, 0, a.name);
    if (!app.hasDesc) throw err2("no_desc", a.name);
    if (!allFF(a.data, app.end, a.data.length)) throw err2("gap", a.name);
    const parts = [
      part("bootloader", b.name, LAYOUT.bootloader.off, b.data.subarray(0, boot.end)),
      part("partition-table", t2.name, LAYOUT["partition-table"].off, t2.data.subarray(0, pt.usedBytes)),
      part("app", a.name, LAYOUT.app.off, a.data.subarray(0, app.end))
    ];
    return finish("three", parts, pt.entries, null, boot.flash, [parseName(a.name), parseName(b.name), parseName(t2.name)]);
  }
  var pad4 = (d) => {
    if (d.length % 4 === 0) return d;
    const o = new Uint8Array(d.length + 4 - d.length % 4).fill(255);
    o.set(d);
    return o;
  };
  function planWrite(rel, { eraseSettings = false } = {}) {
    const parts = rel.parts.map((p) => ({ name: p.role, offset: p.offset, data: pad4(p.data) }));
    const dataParts = rel.entries.filter((e) => e.type === 1).map((e) => ({ name: e.name, offset: e.offset, size: e.size }));
    const nvs = rel.nvs;
    const erased = eraseSettings ? [{ name: "nvs", offset: nvs.off, size: nvs.size }] : [];
    for (const e of erased) parts.push({ name: e.name, offset: e.offset, data: new Uint8Array(e.size).fill(255) });
    const kept = dataParts.filter((e) => !(eraseSettings && e.offset === nvs.off));
    return { parts, erased, kept, totalBytes: parts.reduce((a, p) => a + p.data.length, 0) };
  }
  function chipCheck(info, rel) {
    const errors = [];
    if (info.chip !== "ESP32-S31") errors.push("chip");
    if (info.flashBytes && info.flashBytes < rel.flashNeeded) errors.push("flash_small");
    return errors;
  }

  // src/js/ui/flashtab.js
  function initFlashTab({ conn: conn2, esp: esp2, t: t2, lang: lang2 }) {
    let files = [], rel = null, relErr = null, sess = null, phase = "idle", stat = null, parsing = 0, flashErr = null, found = null, blocked = [], done = false, lostPort = false, speedNote = null;
    const kb = (n) => n >= 1048576 ? (n / 1048576).toFixed(2) + " MB" : Math.max(1, Math.round(n / 1024)) + " KB";
    const partName = (r) => t2("fl.part." + r);
    function errText(e) {
      const x = e && e.extra;
      switch (e && e.code) {
        case "none":
        case "read":
          return t2("fl.err." + e.code);
        case "count":
          return t2("fl.err.count", { n: x });
        case "need_all":
        case "roles":
          return t2("fl.err." + e.code, { role: partName(x) });
        case "not_merged":
        case "mixed":
          return t2("fl.err." + e.code, { file: x });
        case "no_magic":
        case "chip":
        case "hash":
        case "size":
        case "no_pt":
        case "pt_md5":
        case "no_desc":
          return t2("fl.err." + e.code, { file: x || "" });
        case "pt_layout":
          return t2("fl.err.pt_layout", { what: x || "" });
        case "gap":
          return t2("fl.err.gap", { at: x || "" });
        case "app_big":
          return t2("fl.err.app_big", { size: x || "" });
        case "lib":
          return t2("fl.err.lib", { msg: x || "" });
        case "verify":
          return t2("fl.fail.verify", { name: partName(x) });
        default:
          return t2("fl.err.other", { msg: e && e.message || String(e) });
      }
    }
    async function take(list) {
      if (!list.length || phase === "writing" || phase === "checking") return;
      if (sess) await cancel();
      const my = ++parsing;
      files = [];
      rel = null;
      relErr = null;
      flashErr = null;
      done = false;
      found = null;
      try {
        for (const f of list) files.push({ name: f.name, data: new Uint8Array(await f.arrayBuffer()) });
      } catch {
        relErr = { code: "read" };
      }
      if (!relErr) {
        try {
          rel = await analyze(files);
        } catch (e) {
          relErr = e;
        }
      }
      if (my === parsing) render();
    }
    function render() {
      const busy = phase === "checking" || phase === "writing";
      const info = $("#fl-info");
      info.replaceChildren();
      if (relErr) info.append(el("p", { class: "chip bad" }, errText(relErr)));
      else if (rel) {
        info.append(el("p", { class: "small" }, rel.kind === "merged" ? t2("fl.kind.merged") : t2("fl.kind.three")));
        info.append(el(
          "table",
          { class: "parts" },
          el("thead", {}, el("tr", {}, ...["fl.col.part", "fl.col.file", "fl.col.offset", "fl.col.size"].map((k) => el("th", {}, t2(k))))),
          el("tbody", {}, ...rel.parts.map((p) => el("tr", {}, el("td", {}, partName(p.role)), el("td", { class: "mono" }, p.file), el("td", { class: "num mono" }, hex(p.offset)), el("td", { class: "num" }, kb(p.size)))))
        ));
        info.append(el("p", { class: "small mut" }, t2("fl.ver", { version: rel.version || "?", built: ((rel.buildDate || "") + " " + (rel.buildTime || "")).trim() || "?", idf: rel.idf || "?" })));
        info.append(el("p", { class: "small mut" }, t2("fl.flashparams", rel.flash)));
        if (!rel.flash.matches) info.append(el("p", { class: "chip warn small" }, t2("fl.flashparams.diff")));
        if (rel.projectName && rel.projectName !== "nightstand_nano") info.append(el("p", { class: "chip warn small" }, t2("fl.project.diff", { name: rel.projectName })));
        info.append(el("p", { class: "chip ok small" }, t2("fl.ok")));
      }
      show($("#fl-opts"), !!rel);
      if (rel) {
        const erase = $("#fl-erase").checked, plan = planWrite(rel, { eraseSettings: erase });
        show($("#fl-erase-warn"), erase);
        $("#fl-erase-warn").textContent = t2("fl.erase.warn", { off: hex(rel.nvs.off), size: kb(rel.nvs.size) });
        const list = (a) => a.map((p) => partName(p.name) + " " + hex(p.offset)).join(", ");
        const kept = plan.kept.map((e) => e.name + " " + hex(e.offset)).join(", ");
        const pl = $("#fl-plan");
        pl.replaceChildren(...[
          el("div", {}, el("b", {}, t2("fl.plan.title"))),
          ...plan.parts.filter((p) => !plan.erased.some((e) => e.offset === p.offset)).map((p) => el("div", { class: "mono" }, `${partName(p.name)}  ${hex(p.offset)}  ${kb(p.data.length)}`)),
          plan.erased.length ? el("div", { class: "warn" }, t2("fl.plan.erased", { list: list(plan.erased) + " (" + kb(rel.nvs.size) + ")" })) : null,
          kept ? el("div", { class: "mut" }, t2("fl.plan.kept", { list: kept })) : null
        ].filter(Boolean));
      }
      const ok = !!rel && conn2.supported;
      show($("#fl-check"), phase === "idle" || phase === "checking" || phase === "done");
      $("#fl-check").disabled = !ok || busy;
      show($("#fl-write"), phase === "ready" || phase === "confirm");
      $("#fl-write").disabled = phase !== "ready";
      show($("#fl-cancel"), phase === "ready" || phase === "confirm" || phase === "blocked");
      $("#fl-erase").disabled = busy || phase === "confirm";
      $("#fl-baud").disabled = busy || phase === "ready" || phase === "confirm" || phase === "blocked";
      show($("#fl-speed-note"), !!speedNote);
      $("#fl-speed-note").textContent = speedNote ? t2("fl.speed.fb." + speedNote.stage, { from: speedNote.from, to: speedNote.to }) : "";
      show($("#fl-confirm"), phase === "confirm");
      show($("#fl-found"), !!found && (phase === "ready" || phase === "confirm" || phase === "blocked"));
      if (found) $("#fl-found").textContent = t2("fl.found", found) + (blocked.length ? "" : " " + t2("fl.ready"));
      const prog = phase === "checking" || phase === "writing";
      show($("#fl-prog"), prog);
      $("#fl-stat").textContent = stat ? t2(stat.key, stat.p) : "";
      const errs = blocked.map((c) => c === "chip" ? t2("fl.err.chipcheck", { chip: sess ? sess.info.chip : "?" }) : t2("fl.err.small", { size: sess && sess.info.flashSize || "?", need: Math.ceil(rel.flashNeeded / 1048576) }));
      const fe = flashErr ? [flashErr()] : errs;
      show($("#fl-err"), fe.length > 0);
      $("#fl-err").textContent = fe.join(" ");
      show($("#fl-done"), done);
      if (lostPort) {
        show($("#fl-err"), true);
        $("#fl-err").textContent = t2("fl.lost");
      }
      $("#fl-bar").hidden = phase !== "writing";
    }
    const logEl = $("#fl-log");
    const log = (s) => {
      logEl.append(document.createTextNode(String(s).replace(/\s+$/, "") + "\n"));
      while (logEl.childNodes.length > 300) logEl.firstChild.remove();
      logEl.scrollTop = logEl.scrollHeight;
    };
    async function check() {
      if (!rel || phase === "checking" || phase === "writing") return;
      flashErr = null;
      done = false;
      lostPort = false;
      blocked = [];
      found = null;
      speedNote = null;
      phase = "checking";
      stat = { key: "fl.checking" };
      render();
      try {
        if (conn2.state === "idle") {
          const ok = await conn2.open();
          if (!ok) {
            phase = "idle";
            stat = null;
            render();
            return;
          }
        }
        sess = await beginSession({ conn: conn2, esp: esp2, log, baud: Number($("#fl-baud").value) || void 0, onFallback: (f) => {
          speedNote = f;
          if (found) found.baud = f.to;
          render();
        } });
        blocked = chipCheck(sess.info, rel);
        found = { chip: sess.info.chip, size: sess.info.flashSize || "?", baud: sess.info.baud };
        phase = blocked.length ? "blocked" : "ready";
        stat = null;
      } catch (e) {
        console.error(e);
        sess = null;
        phase = "idle";
        stat = null;
        flashErr = () => e.code === "lib" ? errText(e) : e.code === "open" ? t2("conn.err.open", { msg: e.detail }) : t2("fl.fail.connect", { msg: e.message });
        $("#fl-manual").open = true;
      }
      render();
    }
    async function cancel() {
      const s = sess;
      sess = null;
      phase = "idle";
      stat = null;
      found = null;
      blocked = [];
      render();
      if (s) lostPort = await s.end().catch(() => true);
      render();
    }
    function askWrite() {
      if (phase !== "ready") return;
      if ($("#fl-erase").checked) {
        $("#fl-confirm-text").textContent = t2("fl.erase.warn", { off: hex(rel.nvs.off), size: kb(rel.nvs.size) });
        phase = "confirm";
        render();
      } else write();
    }
    async function write() {
      if (!sess || !rel || phase !== "ready" && phase !== "confirm") return;
      const plan = planWrite(rel, { eraseSettings: $("#fl-erase").checked });
      phase = "writing";
      flashErr = null;
      stat = { key: "fl.writing", p: { name: "", pct: 0 } };
      $("#fl-bar").value = 0;
      render();
      const s = sess;
      try {
        await s.write(plan.parts, {
          onProgress: (f, m) => {
            $("#fl-bar").value = Math.round(f * 1e3);
            const name = partName(m.name);
            stat = m.phase === "verify" ? { key: "fl.verifying", p: { name } } : { key: "fl.writing", p: { name, pct: Math.floor(f * 100) } };
            $("#fl-stat").textContent = t2(stat.key, stat.p);
          }
        });
        stat = { key: "fl.finishing" };
        render();
        sess = null;
        lostPort = await s.end();
        phase = "done";
        done = true;
        stat = null;
        found = null;
        window.__nsnFlashDone = true;
      } catch (e) {
        console.error(e);
        sess = null;
        stat = null;
        flashErr = () => e.code === "verify" ? errText(e) : t2("fl.fail.write", { msg: e.message || String(e) });
        phase = "idle";
        render();
        lostPort = await s.end().catch(() => true);
        $("#fl-manual").open = true;
        window.__nsnFlashFailed = true;
      }
      render();
    }
    const input = $("#fl-file");
    $("#fl-pick").addEventListener("click", () => input.click());
    input.addEventListener("change", () => {
      const l = [...input.files];
      input.value = "";
      take(l);
    });
    const drop = $("#fl-drop");
    drop.addEventListener("dragover", (e) => {
      e.preventDefault();
      drop.classList.add("over");
    });
    drop.addEventListener("dragleave", () => drop.classList.remove("over"));
    drop.addEventListener("drop", (e) => {
      e.preventDefault();
      drop.classList.remove("over");
      take([...e.dataTransfer.files]);
    });
    $("#fl-check").addEventListener("click", check);
    $("#fl-write").addEventListener("click", askWrite);
    $("#fl-cancel").addEventListener("click", cancel);
    $("#fl-confirm-yes").addEventListener("click", write);
    $("#fl-confirm-no").addEventListener("click", () => {
      phase = "ready";
      render();
    });
    $("#fl-erase").addEventListener("change", render);
    conn2.on("state", (s) => {
      if (s === "console" && phase === "done") lostPort = false;
      render();
    });
    lang2.onChange(render);
    render();
    return { render, take, get phase() {
      return phase;
    } };
  }

  // src/js/ui/hwtab.js
  function initHwTab({ conn: conn2, t: t2, lang: lang2, timing = {} }) {
    const T = { restartWait: 1500, pollEvery: 800, pollTotal: 25e3, recheck: 3e3, ...timing };
    let hw = null, form = null, view = "off", result = null, askDefaults = false, restarting = false, seq = 0, restartedMsg = null, whyPin = null, ledDefault = null;
    const touched = /* @__PURE__ */ new Set();
    const fieldName = (f) => t2("hw.field." + f);
    const formFrom = (h) => ({
      sd: {
        mode: h.sd.mode,
        width: h.sd.mode === "sdmmc" && h.sd.width === 1 ? 1 : 4,
        spi: h.sd.mode === "spi" ? { clk: h.sd.clk, mosi: h.sd.mosi, miso: h.sd.miso, cs: h.sd.cs } : { clk: NaN, mosi: NaN, miso: NaN, cs: NaN }
      },
      led: { on: h.led.on, gpio: h.led.on && h.led.gpio >= 0 ? h.led.gpio : ledDefault ?? NaN, order: LED_ORDERS.includes(h.led.order) ? h.led.order : "grb" }
    });
    const sdOf = (f, h) => f.sd.mode === "spi" ? { mode: "spi", ...f.sd.spi } : { mode: "sdmmc", ...Object.fromEntries(SD_PINS.map((k) => [k, f.sd.width === 1 && ["d1", "d2", "d3"].includes(k) ? -1 : h.sdmmc[k]])), width: f.sd.width };
    async function read({ keepForm = false } = {}) {
      if (!conn2.up) {
        view = "off";
        render();
        return null;
      }
      const my = ++seq;
      if (view !== "body") view = "reading";
      render();
      try {
        const lines = await conn2.request("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), timeout: 3e3, partial: true });
        return accept(parseHw(lines), my, keepForm);
      } catch (e) {
        if (my === seq) {
          view = conn2.up ? "noanswer" : "off";
          hw = view === "noanswer" ? null : hw;
          render();
        }
        return null;
      }
    }
    function accept(h, my, keepForm) {
      if (my !== seq) return null;
      if (!h.sd || !h.led || !h.sdmmc) {
        hw = null;
        view = "noanswer";
        render();
        return null;
      }
      if (h.led.src === "default" && h.led.on) ledDefault = h.led.gpio;
      h.ledDefault = ledDefault;
      hw = h;
      if (!keepForm || !form) {
        form = formFrom(h);
        touched.clear();
      }
      view = "body";
      render();
      return h;
    }
    async function send(cmd, kind) {
      try {
        const [line] = await conn2.request(cmd, { accept: isAckLine(kind), until: () => true, timeout: 3500 });
        return parseAck(line);
      } catch {
        view = conn2.up ? "noanswer" : "off";
        render();
        return null;
      }
    }
    const resWhy = (n) => t2("hw.res." + (hw ? pinReason(n) : "other"));
    const reasonText = (reason) => {
      const r = parseReason(reason);
      if (r.code === "notsup") return /sdmmc-pins-fixed/.test(r.why) ? t2("hw.why.notsup.sdmmc") : t2("hw.why.notsup.other", { raw: r.why || reason });
      if (r.code === "other") return t2("hw.why.other", { raw: reason });
      return t2("hw.why." + r.code, { n: r.n, why: r.code === "reserved" ? resWhy(r.n) : "" });
    };
    async function apply(kind) {
      const issues = kind === "sd" ? validateSd(sdOf(form, hw), hw) : validateLed(form.led, hw);
      if (issues.length) return;
      result = null;
      restartedMsg = null;
      render();
      const ack = await send(kind === "sd" ? sdCommand(sdOf(form, hw)) : ledCommand(form.led), kind);
      if (!ack) return;
      if (ack.ok) {
        result = { ok: true, key: "hw.ack.ok" };
        await read({ keepForm: false });
      } else result = { ok: false, key: "hw.refused", p: { why: reasonText(ack.text) }, raw: `hw ${ack.kind}: refused ${ack.text}` };
      render();
    }
    async function defaults() {
      askDefaults = false;
      result = null;
      restartedMsg = null;
      render();
      const ack = await send("hw reset", "reset");
      if (!ack) return;
      if (ack.ok) {
        result = { ok: true, key: "hw.ack.reset" };
        await read();
      } else result = { ok: false, key: "hw.refused", p: { why: reasonText(ack.text) }, raw: `hw ${ack.kind}: refused ${ack.text}` };
      render();
    }
    async function restart() {
      if (!conn2.up || restarting) return;
      restarting = true;
      result = null;
      restartedMsg = null;
      render();
      try {
        await conn2.send("reboot");
      } catch {
        restarting = false;
        render();
        return;
      }
      await new Promise((r) => setTimeout(r, T.restartWait));
      const my = ++seq;
      try {
        const lines = await conn2.poll("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), every: T.pollEvery, total: T.pollTotal, partial: true });
        restarting = false;
        const h = accept(parseHw(lines), my, false);
        if (h) {
          restartedMsg = h.sd.state;
          render();
          setTimeout(recheck, T.recheck);
        }
      } catch {
        restarting = false;
        if (my === seq) {
          view = conn2.up ? "noanswer" : "off";
          render();
        }
      }
      render();
    }
    async function recheck() {
      if (!conn2.up || restarting) return;
      const my = ++seq;
      try {
        const lines = await conn2.request("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), timeout: 3e3, partial: true });
        const h = parseHw(lines);
        if (my === seq && h.sd && h.led && hw && h.sd.state !== hw.sd.state) {
          hw = h;
          restartedMsg = h.sd.state;
          render();
        }
      } catch {
      }
    }
    const spiBox = $("#hw-spi-pins"), mmcBox = $("#hw-sdmmc-pins");
    const inputs = {}, fixedOut = {};
    for (const f of SPI_PINS) {
      const input = el("input", { type: "number", step: "1", inputmode: "numeric", id: "hw-pin-" + f });
      const label = el("label", { class: "field", id: "hw-field-" + f }, el("span"), input);
      inputs[f] = input;
      spiBox.append(label);
      wirePin(input, f, (v) => {
        form.sd.spi[f] = v;
        touched.add(f);
      });
    }
    for (const f of SD_PINS) {
      const out = el("output", { class: "mono", id: "hw-fixed-" + f });
      fixedOut[f] = out;
      mmcBox.append(el("div", { class: "field fixed", id: "hw-fixedf-" + f }, el("span"), out));
    }
    const ledGpio = $("#hw-led-gpio");
    wirePin(ledGpio, "led", (v) => {
      form.led.gpio = v;
    });
    function wirePin(input, f, set) {
      let prev = null;
      input.addEventListener("focus", () => {
        prev = input.value === "" ? null : Number(input.value);
      });
      input.addEventListener("keydown", (e) => {
        if (e.key !== "ArrowUp" && e.key !== "ArrowDown" || !hw) return;
        e.preventDefault();
        const cur = input.value === "" ? hw ? hw.gpios.min - 1 : 0 : Number(input.value);
        const n = stepGpio(cur, e.key === "ArrowUp" ? 1 : -1, hw);
        if (n !== null) {
          input.value = n;
          set(n);
          prev = n;
          render();
        }
      });
      input.addEventListener("input", (e) => {
        let v = input.value === "" ? NaN : Number(input.value);
        if (Number.isInteger(v) && prev !== null && Math.abs(v - prev) === 1 && hw && hw.reserved.includes(v) && !e.inputType) {
          const n = stepGpio(v, v - prev, hw);
          if (n !== null) {
            v = n;
            input.value = n;
          }
        }
        set(Number.isInteger(v) ? v : NaN);
        prev = Number.isInteger(v) ? v : null;
        render();
      });
    }
    for (const r of document.querySelectorAll('input[name="hw-sd-mode"]')) r.addEventListener("change", (e) => {
      if (e.target.checked && form) {
        form.sd.mode = e.target.value;
        render();
      }
    });
    $("#hw-width").addEventListener("change", (e) => {
      form.sd.width = Number(e.target.value);
      render();
    });
    $("#hw-led-on").addEventListener("change", (e) => {
      form.led.on = e.target.checked;
      render();
    });
    const orderSel = $("#hw-led-order");
    for (const o of LED_ORDERS) orderSel.append(el("option", { value: o }, o.toUpperCase()));
    orderSel.addEventListener("change", (e) => {
      form.led.order = e.target.value;
      render();
    });
    $("#hw-sd-apply").addEventListener("click", () => apply("sd"));
    $("#hw-led-apply").addEventListener("click", () => apply("led"));
    $("#hw-restart").addEventListener("click", restart);
    $("#hw-reread").addEventListener("click", () => {
      result = null;
      restartedMsg = null;
      read();
    });
    $("#hw-defaults").addEventListener("click", () => {
      askDefaults = true;
      render();
    });
    $("#hw-defaults-no").addEventListener("click", () => {
      askDefaults = false;
      render();
    });
    $("#hw-defaults-yes").addEventListener("click", defaults);
    function issueText(i, hwv) {
      const field = i.field === "mode" || i.field === "width" ? "" : fieldName(i.field), n = i.n;
      switch (i.code) {
        case "int":
          return t2("hw.iss.int", { field });
        case "range":
          return t2("hw.iss.range", { field, min: hwv.gpios.min, max: hwv.gpios.max });
        case "nopin":
          return t2("hw.iss.nopin", { field, n });
        case "reserved":
          return t2("hw.iss.reserved", { field, n, why: resWhy(n) });
        case "duplicate":
          return t2("hw.iss.duplicate", { field, n });
        case "conflict":
          return t2("hw.iss.conflict", { field, n, other: t2(i.field === "led" ? "hw.other.sd" : "hw.other.led") });
        case "order":
          return t2("hw.iss.order");
        case "width":
          return t2("hw.iss.width");
        case "notsup":
          return t2("hw.iss.notsup");
        default:
          return i.code;
      }
    }
    function renderReserved() {
      const box = $("#hw-reserved");
      if (!hw.reserved.length) {
        box.replaceChildren();
        show($("#hw-reserved-why"), false);
        return;
      }
      box.replaceChildren(
        el("span", { class: "pinchips-label" }, t2("hw.reserved")),
        " ",
        el("span", { class: "pinchips" }, ...hw.reserved.map((n) => el("button", {
          type: "button",
          class: "pinchip",
          title: resWhy(n),
          "aria-pressed": String(whyPin === n),
          "aria-label": "GPIO " + n + ": " + resWhy(n),
          onclick: () => {
            whyPin = whyPin === n ? null : n;
            renderReserved();
          }
        }, String(n))))
      );
      const w = $("#hw-reserved-why");
      show(w, whyPin !== null);
      w.textContent = whyPin !== null ? t2("hw.reserved.why", { n: whyPin, why: resWhy(whyPin) }) : "";
    }
    function render() {
      show($("#hw-off"), view === "off");
      show($("#hw-reading"), view === "reading");
      show($("#hw-noanswer"), view === "noanswer");
      show($("#hw-body"), view === "body" && !!hw);
      for (const f of SPI_PINS) $("#hw-field-" + f).querySelector("span").textContent = fieldName(f);
      for (const f of SD_PINS) $("#hw-fixedf-" + f).querySelector("span").textContent = fieldName(f);
      if (view !== "body" || !hw || !form) return;
      const busy = restarting, spi = form.sd.mode === "spi";
      const sdv = sdOf(form, hw);
      const sdIssues = validateSd(sdv, hw).filter((i) => !(i.code === "int" && !touched.has(i.field))), ledIssues = validateLed(form.led, hw);
      const sdBlank = spi && SPI_PINS.some((f) => !Number.isInteger(form.sd.spi[f]));
      const st = hw.sd.state;
      const chip = $("#hw-sd-state");
      chip.textContent = t2("hw.sd.state." + (["mounted", "unreadable", "none"].includes(st) ? st : "none"));
      chip.className = "chip " + (st === "mounted" ? "ok" : st === "unreadable" ? "warn" : "");
      $("#hw-sd-meta").textContent = t2("hw.src." + (hw.sd.src === "custom" ? "custom" : "default")) + " \xB7 " + t2("hw.sd.mode", { mode: hw.sd.mode === "spi" ? "SPI" : "SDMMC" });
      show($("#hw-sd-statenote"), hw.pending);
      $("#hw-range").textContent = t2("hw.gpio.range", hw.gpios);
      renderReserved();
      $("#hw-mode-sdmmc").checked = !spi;
      $("#hw-mode-spi").checked = spi;
      for (const r2 of document.querySelectorAll('input[name="hw-sd-mode"]')) r2.disabled = busy;
      show($("#hw-sd-sdmmc"), !spi);
      show($("#hw-sd-spi"), spi);
      show($("#hw-powercycle"), spi || hw.sd.mode === "spi");
      $("#hw-width").value = String(form.sd.width);
      $("#hw-width").disabled = busy;
      for (const f of SD_PINS) {
        const off = form.sd.width === 1 && (f === "d1" || f === "d2" || f === "d3");
        fixedOut[f].textContent = off ? t2("hw.fixed.off") : String(hw.sdmmc[f]);
        $("#hw-fixedf-" + f).classList.toggle("off", off);
      }
      const bad = new Set(sdIssues.map((i) => i.field));
      for (const f of SPI_PINS) {
        const inp = inputs[f];
        inp.min = hw.gpios.min;
        inp.max = hw.gpios.max;
        inp.disabled = busy;
        const v = Number.isNaN(form.sd.spi[f]) ? "" : String(form.sd.spi[f]);
        if (inp.value !== v) inp.value = v;
        $("#hw-field-" + f).classList.toggle("bad", spi && bad.has(f));
      }
      const lines = sdIssues.map((i) => issueText(i, hw));
      if (sdBlank && !lines.length) lines.push(t2("hw.iss.blank"));
      $("#hw-sd-issues").replaceChildren(...lines.map((x) => el("li", {}, x)));
      $("#hw-sd-apply").disabled = sdBlank || validateSd(sdv, hw).length > 0 || busy;
      $("#hw-led-on").checked = form.led.on;
      $("#hw-led-on").disabled = busy;
      ledGpio.min = hw.gpios.min;
      ledGpio.max = hw.gpios.max;
      ledGpio.disabled = !form.led.on || busy;
      const lv = Number.isNaN(form.led.gpio) ? "" : String(form.led.gpio);
      if (ledGpio.value !== lv) ledGpio.value = lv;
      orderSel.value = form.led.order;
      orderSel.disabled = !form.led.on || busy;
      ledGpio.parentElement.classList.toggle("bad", ledIssues.some((i) => i.field === "led"));
      ledGpio.parentElement.classList.toggle("off", !form.led.on);
      $("#hw-led-src").textContent = t2("hw.src." + (hw.led.src === "custom" ? "custom" : "default"));
      $("#hw-led-issues").replaceChildren(...ledIssues.map((i) => el("li", {}, issueText(i, hw))));
      $("#hw-led-apply").disabled = ledIssues.length > 0 || busy;
      show($("#hw-pending"), hw.pending && !restarting);
      $("#hw-restart").className = hw.pending ? "primary" : "";
      $("#hw-restart").disabled = busy;
      $("#hw-defaults").disabled = busy;
      $("#hw-reread").disabled = busy;
      show($("#hw-defaults-ask"), askDefaults);
      show($("#hw-restarting"), restarting);
      const r = $("#hw-result");
      if (restartedMsg && !restarting) {
        show(r, true);
        r.className = "chip " + (restartedMsg === "mounted" ? "ok" : "warn");
        r.textContent = t2("hw.restarted", { state: t2("hw.sd.state." + (["mounted", "unreadable"].includes(restartedMsg) ? restartedMsg : "none")) });
      } else if (result) {
        show(r, true);
        r.className = "chip " + (result.ok ? "ok" : "bad");
        r.textContent = t2(result.key, result.p) + (result.raw ? t2("hw.refused.raw", { raw: result.raw }) : "");
      } else show(r, false);
    }
    conn2.on("state", (s) => {
      if (s === "idle") {
        hw = null;
        form = null;
        whyPin = null;
        ledDefault = null;
        view = "off";
        result = null;
        restarting = false;
        restartedMsg = null;
        render();
      } else if (s === "flashing") {
        view = "off";
        render();
      }
    });
    lang2.onChange(render);
    render();
    return { read, render, get hw() {
      return hw;
    }, get form() {
      return form;
    } };
  }

  // src/js/ui/app.js
  function startApp({ conn: conn2, esp: esp2, I18N: I18N2, stamp, demo: demo2, tab }) {
    const t2 = (k, p) => I18N2.t(k, p);
    const lang2 = { onChange: (f) => I18N2.onChange(f) };
    let ver = null, verState = "none", connErr = null;
    const sel = $("#lang");
    const fillLang = () => {
      const cur = I18N2.pref();
      sel.replaceChildren(el("option", { value: "auto" }, t2("lang.auto")), el("option", { value: "en" }, "English"), el("option", { value: "zh-TW" }, "\u7E41\u9AD4\u4E2D\u6587"), el("option", { value: "zh-CN" }, "\u7B80\u4F53\u4E2D\u6587"));
      sel.value = cur;
    };
    sel.addEventListener("change", () => I18N2.setLang(sel.value));
    const tabs = { flash: [$("#tab-btn-flash"), $("#tab-flash")], hw: [$("#tab-btn-hw"), $("#tab-hw")] };
    function pickTab(name) {
      for (const [k, [b, p]] of Object.entries(tabs)) {
        b.setAttribute("aria-selected", String(k === name));
        p.hidden = k !== name;
      }
    }
    for (const [k, [b]] of Object.entries(tabs)) b.addEventListener("click", () => pickTab(k));
    pickTab(tab === "hw" ? "hw" : "flash");
    const flash = initFlashTab({ conn: conn2, esp: esp2, t: t2, lang: lang2 });
    const hw = initHwTab({ conn: conn2, t: t2, lang: lang2 });
    const verDetail = (v) => v.commit ? t2(v.dirty ? "conn.fw.dirty" : "conn.fw.commit", { commit: v.commit }) : v.dirty ? t2("conn.fw.dirty0") : "";
    function renderConn() {
      const s = conn2.state;
      $("#conn-dot").className = "dot" + (s === "console" ? " on" : s === "flashing" ? " busy" : "");
      $("#conn-text").textContent = s === "console" ? t2("conn.on", { port: conn2.label }) : s === "flashing" ? t2("conn.flashing") : t2("conn.off");
      show($("#btn-connect"), s === "idle");
      $("#btn-connect").disabled = !conn2.supported;
      $("#btn-anyport").disabled = !conn2.supported;
      show($("#btn-disconnect"), s === "console");
      show($("#btn-anyport").parentElement, s === "idle");
      const fw = $("#conn-fw");
      show(fw, s === "console" && verState !== "none");
      fw.textContent = verState === "ok" ? t2("conn.fw", { version: ver.base || ver.version, detail: verDetail(ver), idf: ver.idf || "?", built: ver.built || "?" }) : verState === "waiting" ? t2("conn.waitver") : verState === "silent" ? t2("conn.nover") : "";
      const ce = $("#conn-err");
      show(ce, !!connErr);
      ce.textContent = connErr ? connErr() : "";
      $("#con-input").disabled = s !== "console";
      $("#con-send").disabled = s !== "console";
      document.body.dataset.conn = s;
    }
    async function connect(any) {
      connErr = null;
      renderConn();
      try {
        await conn2.open({ any });
      } catch (e) {
        connErr = () => e.code === "open" ? t2("conn.err.open", { msg: e.detail }) : t2(e.code === "nosupport" ? "warn.noserial" : "conn.lost");
        renderConn();
      }
    }
    $("#btn-connect").addEventListener("click", () => connect(false));
    $("#btn-anyport").addEventListener("click", () => connect(true));
    $("#btn-disconnect").addEventListener("click", () => conn2.close());
    async function consoleUp() {
      ver = null;
      verState = "waiting";
      renderConn();
      try {
        const lines = await conn2.poll("ver", { accept: isVerLine, until: () => true, every: 700, total: 14e3 });
        ver = parseVer(lines[0]);
        verState = "ok";
      } catch (e) {
        if (conn2.state !== "console") return;
        verState = "silent";
      }
      renderConn();
      if (conn2.state === "console") await hw.read();
    }
    conn2.on("state", (s) => {
      if (s === "console") consoleUp();
      else {
        if (s === "idle") {
          ver = null;
          verState = "none";
        }
        renderConn();
      }
    });
    conn2.on("close", (byUser) => {
      if (!byUser) {
        connErr = () => t2("conn.lost");
        renderConn();
      }
    });
    lang2.onChange(() => {
      fillLang();
      renderConn();
      $("#build-stamp").textContent = t2("foot.build", { stamp });
    });
    const log = $("#con-log"), MAX = 500;
    function addLog(text) {
      const near = log.scrollHeight - log.scrollTop - log.clientHeight < 40;
      log.append(document.createTextNode(text + "\n"));
      while (log.childNodes.length > MAX) log.firstChild.remove();
      if (near) log.scrollTop = log.scrollHeight;
    }
    conn2.on("line", (l) => addLog(redact(l)));
    conn2.on("tx", (c) => addLog("> " + redact(c)));
    conn2.on("close", () => addLog("--"));
    $("#con-form").addEventListener("submit", (e) => {
      e.preventDefault();
      const v = $("#con-input").value.trim();
      if (!v) return;
      $("#con-input").value = "";
      conn2.send(v).catch(() => {
      });
    });
    $("#con-clear").addEventListener("click", () => log.replaceChildren());
    show($("#demo-banner"), !!demo2);
    show($("#noserial"), !conn2.supported);
    fillLang();
    renderConn();
    $("#build-stamp").textContent = t2("foot.build", { stamp });
    return { flash, hw, pickTab };
  }

  // src/js/main.js
  var q = new URLSearchParams(location.search);
  var demo = q.has("demo");
  I18N.init();
  var conn;
  var esp = null;
  if (demo) {
    const num = (k) => q.has(k) ? Number(q.get(k)) : void 0;
    const serial = createMockSerial({
      sd: q.get("sd") || "mounted",
      spiCard: q.get("spi") || "none",
      refuse: q.get("refuse") || "",
      old: q.has("old"),
      silent: q.has("silent"),
      noise: q.get("noise") !== "0",
      split: num("split") || 0,
      bootMs: num("bootms") ?? 900
    });
    conn = createConn({ serial, sleep: (ms) => new Promise((r) => setTimeout(r, ms / (q.has("fast") ? 10 : 1))) });
    esp = createMockEsp({ speed: num("speed") ?? 8, stopAt: num("stop"), failAt: num("fail"), chip: q.get("chip") || void 0, flashSize: q.has("flashsize") ? q.get("flashsize") : void 0, corrupt: q.has("corrupt"), maxBaud: num("maxbaud"), writeMaxBaud: num("writemaxbaud") });
    setEsptool(esp);
    globalThis.__nsnDemo = { serial, esp, conn };
  } else conn = createConn();
  startApp({ conn, esp, I18N, stamp: true ? "78cf3f5d" : "dev", demo, tab: q.get("tab") });
})();
