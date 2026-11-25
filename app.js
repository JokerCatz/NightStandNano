(() => {
  var __defProp = Object.defineProperty;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __esm = (fn, res, err) => function __init() {
    if (err) throw err[0];
    try {
      return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
    } catch (e) {
      throw err = [e], e;
    }
  };
  var __export = (target, all2) => {
    for (var name in all2)
      __defProp(target, name, { get: all2[name], enumerable: true });
  };

  // src/js/core/strings.js
  var STRINGS;
  var init_strings = __esm({
    "src/js/core/strings.js"() {
      STRINGS = {
        "app.title": { en: "NightStand Nano \u2013 control", "zh-TW": "NightStand Nano \u2013 \u63A7\u5236\u9801", "zh-CN": "NightStand Nano \u2013 \u63A7\u5236\u9875" },
        "conn.off": { en: "Not connected", "zh-TW": "\u672A\u9023\u7DDA", "zh-CN": "\u672A\u8FDE\u63A5" },
        "conn.on": { en: "Connected", "zh-TW": "\u5DF2\u9023\u7DDA", "zh-CN": "\u5DF2\u8FDE\u63A5" },
        "btn.connect": { en: "Connect", "zh-TW": "\u9023\u7DDA", "zh-CN": "\u8FDE\u63A5" },
        "btn.disconnect": { en: "Disconnect", "zh-TW": "\u4E2D\u65B7", "zh-CN": "\u65AD\u5F00" },
        "warn.nobt": { en: "This browser doesn't support Web Bluetooth. Use Chrome or Edge on a computer or Android; on iPhone, use Bluefy or the native app.", "zh-TW": "\u9019\u500B\u700F\u89BD\u5668\u4E0D\u652F\u63F4 Web Bluetooth\u3002\u8ACB\u7528\u96FB\u8166\u6216 Android \u7684 Chrome / Edge\uFF1BiPhone \u8ACB\u7528 Bluefy \u6216\u539F\u751F App\u3002", "zh-CN": "\u8FD9\u4E2A\u6D4F\u89C8\u5668\u4E0D\u652F\u6301 Web Bluetooth\u3002\u8BF7\u7528\u7535\u8111\u6216 Android \u7684 Chrome / Edge\uFF1BiPhone \u8BF7\u7528 Bluefy \u6216\u539F\u751F App\u3002" },
        "now.aria": { en: "Now playing", "zh-TW": "\u6B63\u5728\u64AD\u653E", "zh-CN": "\u6B63\u5728\u64AD\u653E" },
        "now.viz.aria": { en: "Visualizer", "zh-TW": "\u8996\u89BA\u6548\u679C", "zh-CN": "\u89C6\u89C9\u6548\u679C" },
        "meta.off": { en: "Not connected", "zh-TW": "\u5C1A\u672A\u9023\u7DDA", "zh-CN": "\u5C1A\u672A\u8FDE\u63A5" },
        "tr.prev": { en: "Previous track", "zh-TW": "\u4E0A\u4E00\u9996", "zh-CN": "\u4E0A\u4E00\u9996" },
        "tr.play": { en: "Play or pause", "zh-TW": "\u64AD\u653E\u6216\u66AB\u505C", "zh-CN": "\u64AD\u653E\u6216\u6682\u505C" },
        "tr.next": { en: "Next track", "zh-TW": "\u4E0B\u4E00\u9996", "zh-CN": "\u4E0B\u4E00\u9996" },
        "tr.stop": { en: "Stop", "zh-TW": "\u505C\u6B62", "zh-CN": "\u505C\u6B62" },
        "vol.label": { en: "Volume", "zh-TW": "\u97F3\u91CF", "zh-CN": "\u97F3\u91CF" },
        "out.title": { en: "Output", "zh-TW": "\u8F38\u51FA", "zh-CN": "\u8F93\u51FA" },
        "out.auto": { en: "Auto", "zh-TW": "\u81EA\u52D5", "zh-CN": "\u81EA\u52A8" },
        "out.dac": { en: "DAC", "zh-TW": "\u5C0F\u5C3E\u5DF4", "zh-CN": "\u5C0F\u5C3E\u5DF4" },
        "out.bt": { en: "Bluetooth headphones", "zh-TW": "\u85CD\u7259\u8033\u6A5F", "zh-CN": "\u84DD\u7259\u8033\u673A" },
        "out.btShort": { en: "Bluetooth", "zh-TW": "\u85CD\u7259", "zh-CN": "\u84DD\u7259" },
        "out.cast": { en: "Broadcast (Auracast)", "zh-TW": "\u5EE3\u64AD\uFF08Auracast\uFF09", "zh-CN": "\u5E7F\u64AD\uFF08Auracast\uFF09" },
        "out.castShort": { en: "Broadcast", "zh-TW": "\u5EE3\u64AD", "zh-CN": "\u5E7F\u64AD" },
        "cast.info": { en: "Others can tune in to \u201CNightStand Nano\u201D from Auracast-capable earbuds or a phone. It can't be used together with Bluetooth headphones.", "zh-TW": "\u5176\u4ED6\u4EBA\u53EF\u4EE5\u7528\u652F\u63F4 Auracast \u7684\u8033\u6A5F\u6216\u624B\u6A5F\uFF0C\u641C\u5C0B\u4E26\u6536\u807D\u300CNightStand Nano\u300D\u5EE3\u64AD\u3002\u7121\u6CD5\u8207\u85CD\u7259\u8033\u6A5F\u540C\u6642\u4F7F\u7528\u3002", "zh-CN": "\u5176\u4ED6\u4EBA\u53EF\u4EE5\u7528\u652F\u6301 Auracast \u7684\u8033\u673A\u6216\u624B\u673A\uFF0C\u641C\u7D22\u5E76\u6536\u542C\u201CNightStand Nano\u201D\u5E7F\u64AD\u3002\u65E0\u6CD5\u4E0E\u84DD\u7259\u8033\u673A\u540C\u65F6\u4F7F\u7528\u3002" },
        "cast.wait": { en: "Checking the broadcast\u2026", "zh-TW": "\u6B63\u5728\u67E5\u8A62\u5EE3\u64AD\u72C0\u614B\u2026", "zh-CN": "\u6B63\u5728\u67E5\u8BE2\u5E7F\u64AD\u72B6\u6001\u2026" },
        "cast.off": { en: "The broadcast is not running.", "zh-TW": "\u5EE3\u64AD\u5C1A\u672A\u958B\u59CB\u3002", "zh-CN": "\u5E7F\u64AD\u5C1A\u672A\u5F00\u59CB\u3002" },
        "cast.on": { en: "On air: both streams are running.", "zh-TW": "\u5EE3\u64AD\u4E2D\uFF1A\u5169\u500B\u8072\u9053\u7684\u4E32\u6D41\u90FD\u5728\u904B\u4F5C\u3002", "zh-CN": "\u5E7F\u64AD\u4E2D\uFF1A\u4E24\u8DEF\u97F3\u9891\u6D41\u90FD\u5728\u8FD0\u884C\u3002" },
        "cast.part": { en: "On air, but only {n} of 2 streams are running.", "zh-TW": "\u5EE3\u64AD\u4E2D\uFF0C\u4F46\u53EA\u6709 {n} / 2 \u500B\u4E32\u6D41\u5728\u904B\u4F5C\u3002", "zh-CN": "\u5E7F\u64AD\u4E2D\uFF0C\u4F46\u53EA\u6709 {n} / 2 \u8DEF\u97F3\u9891\u6D41\u5728\u8FD0\u884C\u3002" },
        "cast.counts": { en: "Underflows: {u}, send errors: {e}", "zh-TW": "\u7DE9\u885D\u4E0D\u8DB3 {u} \u6B21\u3001\u50B3\u9001\u932F\u8AA4 {e} \u6B21", "zh-CN": "\u7F13\u51B2\u4E0D\u8DB3 {u} \u6B21\u3001\u53D1\u9001\u9519\u8BEF {e} \u6B21" },
        "cast.err": { en: "Could not start the broadcast: {e}", "zh-TW": "\u7121\u6CD5\u958B\u59CB\u5EE3\u64AD\uFF1A{e}", "zh-CN": "\u65E0\u6CD5\u5F00\u59CB\u5E7F\u64AD\uFF1A{e}" },
        "fx.castNote": { en: "Heavy sound effects are paused while broadcasting.", "zh-TW": "\u5EE3\u64AD\u671F\u9593\uFF0C\u8F03\u5403\u8CC7\u6E90\u7684\u97F3\u6548\u6703\u66AB\u505C\u3002", "zh-CN": "\u5E7F\u64AD\u671F\u95F4\uFF0C\u8F83\u5360\u8D44\u6E90\u7684\u97F3\u6548\u4F1A\u6682\u505C\u3002" },
        "fx.shedNote": { en: "The device is busy: heavy sound effects are paused and come back when this playback ends.", "zh-TW": "\u88DD\u7F6E\u592A\u5FD9\uFF0C\u8F03\u5403\u8CC7\u6E90\u7684\u97F3\u6548\u5DF2\u66AB\u505C\uFF1B\u9019\u6B21\u64AD\u653E\u7D50\u675F\u5F8C\u6703\u81EA\u52D5\u6062\u5FA9\u3002", "zh-CN": "\u8BBE\u5907\u592A\u5FD9\uFF0C\u8F83\u5360\u8D44\u6E90\u7684\u97F3\u6548\u5DF2\u6682\u505C\uFF1B\u8FD9\u6B21\u64AD\u653E\u7ED3\u675F\u540E\u4F1A\u81EA\u52A8\u6062\u590D\u3002" },
        "conn.reconnecting": { en: "Reconnecting\u2026", "zh-TW": "\u91CD\u65B0\u9023\u7DDA\u4E2D\u2026", "zh-CN": "\u91CD\u65B0\u8FDE\u63A5\u4E2D\u2026" },
        "conn.castStarting": { en: "Starting the broadcast\u2026", "zh-TW": "\u6B63\u5728\u555F\u52D5\u5EE3\u64AD\u2026", "zh-CN": "\u6B63\u5728\u542F\u52A8\u5E7F\u64AD\u2026" },
        "btn.cancel": { en: "Cancel", "zh-TW": "\u53D6\u6D88", "zh-CN": "\u53D6\u6D88" },
        "st.idle": { en: "Idle", "zh-TW": "\u5F85\u6A5F", "zh-CN": "\u5F85\u673A" },
        "st.playing": { en: "Playing", "zh-TW": "\u64AD\u653E\u4E2D", "zh-CN": "\u64AD\u653E\u4E2D" },
        "st.paused": { en: "Paused", "zh-TW": "\u66AB\u505C", "zh-CN": "\u6682\u505C" },
        "files.up": { en: "Up", "zh-TW": "\u4E0A\u4E00\u5C64", "zh-CN": "\u4E0A\u4E00\u7EA7" },
        "files.playDir": { en: "Play folder", "zh-TW": "\u64AD\u653E\u6B64\u8CC7\u6599\u593E", "zh-CN": "\u64AD\u653E\u6B64\u6587\u4EF6\u5939" },
        "files.empty": { en: "(empty)", "zh-TW": "\uFF08\u7A7A\u7684\uFF09", "zh-CN": "\uFF08\u7A7A\uFF09" },
        "mode.title": { en: "Play mode", "zh-TW": "\u64AD\u653E\u65B9\u5F0F", "zh-CN": "\u64AD\u653E\u65B9\u5F0F" },
        "mode.folder": { en: "Loop folder", "zh-TW": "\u8CC7\u6599\u593E\u5FAA\u74B0", "zh-CN": "\u6587\u4EF6\u5939\u5FAA\u73AF" },
        "mode.shuffle": { en: "Shuffle", "zh-TW": "\u96A8\u6A5F", "zh-CN": "\u968F\u673A" },
        "mode.one": { en: "Repeat 1", "zh-TW": "\u55AE\u66F2\u91CD\u8907", "zh-CN": "\u5355\u66F2\u5FAA\u73AF" },
        "mode.once": { en: "Once", "zh-TW": "\u64AD\u5B8C\u505C\u6B62", "zh-CN": "\u64AD\u5B8C\u505C\u6B62" },
        "lyr.idle": { en: "Lyrics show up here when you connect and play a track that has a .lrc file.", "zh-TW": "\u9023\u7DDA\u4E26\u64AD\u653E\u6709 .lrc \u7684\u66F2\u76EE\u5F8C\uFF0C\u6B4C\u8A5E\u6703\u986F\u793A\u5728\u9019\u88E1\u3002", "zh-CN": "\u8FDE\u63A5\u5E76\u64AD\u653E\u5E26 .lrc \u7684\u66F2\u76EE\u540E\uFF0C\u6B4C\u8BCD\u4F1A\u663E\u793A\u5728\u8FD9\u91CC\u3002" },
        "lyr.truncated": { en: "The lyrics are too long; only the first part is shown.", "zh-TW": "\u6B4C\u8A5E\u592A\u9577\uFF0C\u53EA\u986F\u793A\u524D\u9762\u7684\u90E8\u5206\u3002", "zh-CN": "\u6B4C\u8BCD\u592A\u957F\uFF0C\u53EA\u663E\u793A\u524D\u9762\u7684\u90E8\u5206\u3002" },
        "lyr.none": { en: "This track has no lyrics file (a .lrc with the same name), or playback hasn't started yet.", "zh-TW": "\u9019\u9996\u6C92\u6709\u6B4C\u8A5E\u6A94\uFF08\u540C\u540D .lrc\uFF09\uFF0C\u6216\u9084\u6C92\u958B\u59CB\u64AD\u653E\u3002", "zh-CN": "\u8FD9\u9996\u6CA1\u6709\u6B4C\u8BCD\u6587\u4EF6\uFF08\u540C\u540D .lrc\uFF09\uFF0C\u6216\u8FD8\u6CA1\u5F00\u59CB\u64AD\u653E\u3002" },
        "radio.stateNone": { en: "Radio status: \u2014", "zh-TW": "\u96FB\u53F0\u72C0\u614B\uFF1A\u2014", "zh-CN": "\u7535\u53F0\u72B6\u6001\uFF1A\u2014" },
        "rs.connecting": { en: "Connecting\u2026", "zh-TW": "\u9023\u7DDA\u4E2D\u2026", "zh-CN": "\u8FDE\u63A5\u4E2D\u2026" },
        "rs.buffering": { en: "Buffering\u2026", "zh-TW": "\u7DE9\u885D\u4E2D\u2026", "zh-CN": "\u7F13\u51B2\u4E2D\u2026" },
        "rs.reconnecting": { en: "Reconnecting\u2026", "zh-TW": "\u91CD\u65B0\u9023\u7DDA\u4E2D\u2026", "zh-CN": "\u91CD\u65B0\u8FDE\u63A5\u4E2D\u2026" },
        "rs.no_connection": { en: "Can't reach the station", "zh-TW": "\u9023\u4E0D\u4E0A\u96FB\u53F0", "zh-CN": "\u8FDE\u4E0D\u4E0A\u7535\u53F0" },
        "rs.server_refused": { en: "The server refused the connection", "zh-TW": "\u4F3A\u670D\u5668\u62D2\u7D55\u9023\u7DDA", "zh-CN": "\u670D\u52A1\u5668\u62D2\u7EDD\u8FDE\u63A5" },
        "rs.empty_playlist": { en: "The playlist is empty", "zh-TW": "\u64AD\u653E\u6E05\u55AE\u662F\u7A7A\u7684", "zh-CN": "\u64AD\u653E\u5217\u8868\u662F\u7A7A\u7684" },
        "rs.too_many_redirects": { en: "Too many redirects", "zh-TW": "\u8F49\u5740\u6B21\u6578\u592A\u591A", "zh-CN": "\u91CD\u5B9A\u5411\u6B21\u6570\u8FC7\u591A" },
        "rs.hls_not_supported": { en: "HLS is not supported", "zh-TW": "\u4E0D\u652F\u63F4 HLS", "zh-CN": "\u4E0D\u652F\u6301 HLS" },
        "fav.title": { en: "Favourites (stored on the card)", "zh-TW": "\u6536\u85CF\uFF08\u5B58\u5728\u5361\u4E0A\uFF09", "zh-CN": "\u6536\u85CF\uFF08\u4FDD\u5B58\u5728\u5361\u4E0A\uFF09" },
        "fav.hint": { en: "The station list on the card loads after you connect.", "zh-TW": "\u9023\u7DDA\u5F8C\u81EA\u52D5\u8F09\u5165\u5361\u4E0A\u7684\u96FB\u53F0\u6E05\u55AE\u3002", "zh-CN": "\u8FDE\u63A5\u540E\u81EA\u52A8\u52A0\u8F7D\u5361\u4E0A\u7684\u7535\u53F0\u5217\u8868\u3002" },
        "find.title": { en: "Find stations (radio-browser.info, looked up online from this device)", "zh-TW": "\u641C\u5C0B\u96FB\u53F0\uFF08radio-browser.info\uFF0C\u7531\u624B\u6A5F\u4E0A\u7DB2\u67E5\u8A62\uFF09", "zh-CN": "\u641C\u7D22\u7535\u53F0\uFF08radio-browser.info\uFF0C\u7531\u624B\u673A\u8054\u7F51\u67E5\u8BE2\uFF09" },
        "common.loading": { en: "Loading\u2026", "zh-TW": "\u8F09\u5165\u4E2D\u2026", "zh-CN": "\u52A0\u8F7D\u4E2D\u2026" },
        "url.title": { en: "Enter a URL", "zh-TW": "\u76F4\u63A5\u8F38\u5165\u7DB2\u5740", "zh-CN": "\u76F4\u63A5\u8F93\u5165\u7F51\u5740" },
        "url.aria": { en: "Station URL", "zh-TW": "\u96FB\u53F0\u7DB2\u5740", "zh-CN": "\u7535\u53F0\u7F51\u5740" },
        "url.play": { en: "Play", "zh-TW": "\u64AD\u653E", "zh-CN": "\u64AD\u653E" },
        "url.save": { en: "Save to card", "zh-TW": "\u5B58\u5230\u5361\u4E0A", "zh-CN": "\u4FDD\u5B58\u5230\u5361\u4E0A" },
        "url.bad": { en: "Enter a URL that starts with http:// or https://.", "zh-TW": "\u8ACB\u8F38\u5165 http:// \u6216 https:// \u958B\u982D\u7684\u7DB2\u5740\u3002", "zh-CN": "\u8BF7\u8F93\u5165\u4EE5 http:// \u6216 https:// \u5F00\u5934\u7684\u7F51\u5740\u3002" },
        "url.noFav": { en: "Favourites aren't loaded yet.", "zh-TW": "\u6536\u85CF\u529F\u80FD\u5C1A\u672A\u8F09\u5165\u3002", "zh-CN": "\u6536\u85CF\u529F\u80FD\u5C1A\u672A\u52A0\u8F7D\u3002" },
        "noise.title": { en: "Noise", "zh-TW": "\u767D\u566A\u97F3", "zh-CN": "\u767D\u566A\u97F3" },
        "noise.white": { en: "White", "zh-TW": "\u767D", "zh-CN": "\u767D" },
        "noise.pink": { en: "Pink", "zh-TW": "\u7C89\u7D05", "zh-CN": "\u7C89\u7EA2" },
        "noise.brown": { en: "Brown", "zh-TW": "\u68D5", "zh-CN": "\u68D5" },
        "noise.blue": { en: "Blue", "zh-TW": "\u85CD", "zh-CN": "\u84DD" },
        "noise.violet": { en: "Violet", "zh-TW": "\u7D2B", "zh-CN": "\u7D2B" },
        "noise.grey": { en: "Grey", "zh-TW": "\u7070", "zh-CN": "\u7070" },
        "noise.green": { en: "Green", "zh-TW": "\u7DA0", "zh-CN": "\u7EFF" },
        "noise.velvet": { en: "Velvet", "zh-TW": "\u7D72\u7D68", "zh-CN": "\u4E1D\u7ED2" },
        "scenes.title": { en: "Scenes (layered ambient sounds)", "zh-TW": "\u5834\u666F\uFF08\u591A\u5C64\u74B0\u5883\u97F3\uFF09", "zh-CN": "\u573A\u666F\uFF08\u591A\u5C42\u73AF\u5883\u97F3\uFF09" },
        "scenes.wait": { en: "Loads after you connect", "zh-TW": "\u9023\u7DDA\u5F8C\u81EA\u52D5\u8F09\u5165", "zh-CN": "\u8FDE\u63A5\u540E\u81EA\u52A8\u52A0\u8F7D" },
        "scenes.none": { en: "No scenes on the card. Generate them with tools/make_scenes.py and put them in /sdcard/_player/scenes/.", "zh-TW": "\u5361\u4E0A\u6C92\u6709\u5834\u666F\u3002\u7528 tools/make_scenes.py \u7522\u751F\u5F8C\u653E\u5230 /sdcard/_player/scenes/\u3002", "zh-CN": "\u5361\u4E0A\u6CA1\u6709\u573A\u666F\u3002\u7528 tools/make_scenes.py \u751F\u6210\u540E\u653E\u5230 /sdcard/_player/scenes/\u3002" },
        "bed.sleep": { en: "Sleep mode", "zh-TW": "\u7761\u7720\u6A21\u5F0F", "zh-CN": "\u7761\u7720\u6A21\u5F0F" },
        "bed.wake": { en: "End night mode", "zh-TW": "\u7D50\u675F\u591C\u9593\u6A21\u5F0F", "zh-CN": "\u7ED3\u675F\u591C\u95F4\u6A21\u5F0F" },
        "bed.hint": { en: "Sleep mode: plays noise or a scene, fades out after 30 minutes, turns on night sound and dims the lights.", "zh-TW": "\u7761\u7720\u6A21\u5F0F\uFF1A\u64AD\u653E\u566A\u97F3\u6216\u5834\u666F\u300130 \u5206\u9418\u5F8C\u6DE1\u51FA\u3001\u958B\u591C\u9593\u97F3\u6548\u3001\u71C8\u5149\u8ABF\u6697\u3002", "zh-CN": "\u7761\u7720\u6A21\u5F0F\uFF1A\u64AD\u653E\u566A\u97F3\u6216\u573A\u666F\uFF0C30 \u5206\u949F\u540E\u6DE1\u51FA\uFF0C\u5F00\u542F\u591C\u95F4\u97F3\u6548\uFF0C\u706F\u5149\u8C03\u6697\u3002" },
        "timer.title": { en: "Sleep timer", "zh-TW": "\u7761\u7720\u8A08\u6642", "zh-CN": "\u7761\u7720\u5B9A\u65F6" },
        "timer.off": { en: "Off", "zh-TW": "\u95DC", "zh-CN": "\u5173" },
        "timer.90": { en: "90 min", "zh-TW": "90 \u5206", "zh-CN": "90 \u5206\u949F" },
        "timer.left": { en: "{m} min left", "zh-TW": "\u5269 {m} \u5206", "zh-CN": "\u5269 {m} \u5206\u949F" },
        "night.on": { en: "On", "zh-TW": "\u958B", "zh-CN": "\u5F00" },
        "night.off": { en: "Off", "zh-TW": "\u95DC", "zh-CN": "\u5173" },
        "night.auto": { en: "Auto", "zh-TW": "\u81EA\u52D5", "zh-CN": "\u81EA\u52A8" },
        "night.to": { en: "to", "zh-TW": "\u5230", "zh-CN": "\u5230" },
        "night.fromAria": { en: "Night mode start", "zh-TW": "\u591C\u9593\u958B\u59CB", "zh-CN": "\u591C\u95F4\u5F00\u59CB" },
        "night.toAria": { en: "Night mode end", "zh-TW": "\u591C\u9593\u7D50\u675F", "zh-CN": "\u591C\u95F4\u7ED3\u675F" },
        "night.set": { en: "Set", "zh-TW": "\u8A2D\u5B9A", "zh-CN": "\u8BBE\u7F6E" },
        "night.clear": { en: "Clear", "zh-TW": "\u53D6\u6D88", "zh-CN": "\u53D6\u6D88" },
        "night.sched": { en: "Turns on automatically every day, {from} to {to}", "zh-TW": "\u6BCF\u5929 {from} \u5230 {to} \u81EA\u52D5\u958B\u555F", "zh-CN": "\u6BCF\u5929 {from} \u5230 {to} \u81EA\u52A8\u5F00\u542F" },
        "night.nosched": { en: "No automatic schedule", "zh-TW": "\u6C92\u6709\u8A2D\u5B9A\u81EA\u52D5\u6642\u6BB5", "zh-CN": "\u6CA1\u6709\u8BBE\u7F6E\u81EA\u52A8\u65F6\u6BB5" },
        "led.title": { en: "Indicator light brightness", "zh-TW": "\u6307\u793A\u71C8\u4EAE\u5EA6", "zh-CN": "\u6307\u793A\u706F\u4EAE\u5EA6" },
        "fx.hint": { en: "Sound is processed on the device; this page makes no sound itself.", "zh-TW": "\u5728\u88DD\u7F6E\u4E0A\u8655\u7406\u8072\u97F3\uFF1B\u9019\u500B\u7DB2\u9801\u672C\u8EAB\u4E0D\u51FA\u8072\u3002", "zh-CN": "\u5728\u8BBE\u5907\u4E0A\u5904\u7406\u58F0\u97F3\uFF1B\u8FD9\u4E2A\u7F51\u9875\u672C\u8EAB\u4E0D\u53D1\u58F0\u3002" },
        "fx.off": { en: "All off", "zh-TW": "\u5168\u90E8\u95DC\u9589", "zh-CN": "\u5168\u90E8\u5173\u95ED" },
        "fx.wait": { en: "Loads after you connect", "zh-TW": "\u9023\u7DDA\u5F8C\u81EA\u52D5\u8F09\u5165", "zh-CN": "\u8FDE\u63A5\u540E\u81EA\u52A8\u52A0\u8F7D" },
        "fx.none": { en: "The device didn't report an effects list.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u5831\u97F3\u6548\u6E05\u55AE\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u4E0A\u62A5\u97F3\u6548\u5217\u8868\u3002" },
        "fx.app.gain": { en: "Gain", "zh-TW": "\u589E\u76CA", "zh-CN": "\u589E\u76CA" },
        "fx.app.balance": { en: "Balance", "zh-TW": "\u5DE6\u53F3\u5E73\u8861", "zh-CN": "\u5DE6\u53F3\u5E73\u8861" },
        "fx.app.widener": { en: "Stereo width", "zh-TW": "\u7ACB\u9AD4\u8072\u5BEC\u5EA6", "zh-CN": "\u7ACB\u4F53\u58F0\u5BBD\u5EA6" },
        "fx.app.crossfeed": { en: "Headphone crossfeed", "zh-TW": "\u8033\u6A5F\u4EA4\u53C9\u994B\u9001", "zh-CN": "\u8033\u673A\u4EA4\u53C9\u9988\u9001" },
        "fx.app.velvet_widener": { en: "Soft widener", "zh-TW": "\u67D4\u548C\u52A0\u5BEC", "zh-CN": "\u67D4\u548C\u52A0\u5BBD" },
        "fx.app.equalizer": { en: "Equalizer", "zh-TW": "\u7B49\u5316\u5668", "zh-CN": "\u5747\u8861\u5668" },
        "fx.app.tone": { en: "Tone", "zh-TW": "\u97F3\u8272", "zh-CN": "\u97F3\u8272" },
        "fx.app.bass": { en: "Bass boost", "zh-TW": "\u4F4E\u97F3\u589E\u5F37", "zh-CN": "\u4F4E\u97F3\u589E\u5F3A" },
        "fx.app.loudness": { en: "Loudness (low volume)", "zh-TW": "\u97FF\u5EA6\u88DC\u511F\uFF08\u5C0F\u97F3\u91CF\uFF09", "zh-CN": "\u54CD\u5EA6\u8865\u507F\uFF08\u5C0F\u97F3\u91CF\uFF09" },
        "fx.app.tube": { en: "Tube warmth", "zh-TW": "\u771F\u7A7A\u7BA1\u6696\u5316", "zh-CN": "\u7535\u5B50\u7BA1\u6696\u5316" },
        "fx.app.compressor": { en: "Compressor (tames dynamics)", "zh-TW": "\u58D3\u7E2E\u5668\uFF08\u58D3\u5C0F\u52D5\u614B\uFF09", "zh-CN": "\u538B\u7F29\u5668\uFF08\u538B\u5C0F\u52A8\u6001\uFF09" },
        "fx.app.freeverb": { en: "Reverb", "zh-TW": "\u6B98\u97FF", "zh-CN": "\u6DF7\u54CD" },
        "fx.app.limiter": { en: "Limiter (anti-clipping)", "zh-TW": "\u9650\u5E45\u5668\uFF08\u9632\u7206\u97F3\uFF09", "zh-CN": "\u9650\u5E45\u5668\uFF08\u9632\u7206\u97F3\uFF09" },
        "fx.p.gain_db": { en: "Gain (dB)", "zh-TW": "\u589E\u76CA dB", "zh-CN": "\u589E\u76CA dB" },
        "fx.p.balance": { en: "Left / right", "zh-TW": "\u504F\u5DE6\uFF0F\u504F\u53F3", "zh-CN": "\u504F\u5DE6\uFF0F\u504F\u53F3" },
        "fx.p.width": { en: "Width", "zh-TW": "\u5BEC\u5EA6", "zh-CN": "\u5BBD\u5EA6" },
        "fx.p.feed_db": { en: "Feed (dB)", "zh-TW": "\u994B\u9001\u91CF dB", "zh-CN": "\u9988\u9001\u91CF dB" },
        "fx.p.cutoff_hz": { en: "Cutoff (Hz)", "zh-TW": "\u622A\u6B62\u983B\u7387 Hz", "zh-CN": "\u622A\u6B62\u9891\u7387 Hz" },
        "fx.p.amount": { en: "Amount", "zh-TW": "\u5F37\u5EA6", "zh-CN": "\u5F3A\u5EA6" },
        "fx.p.length_ms": { en: "Length (ms)", "zh-TW": "\u9577\u5EA6 ms", "zh-CN": "\u957F\u5EA6 ms" },
        "fx.p.mode": { en: "Mode", "zh-TW": "\u6A21\u5F0F", "zh-CN": "\u6A21\u5F0F" },
        "fx.p.seed": { en: "Seed", "zh-TW": "\u7A2E\u5B50", "zh-CN": "\u79CD\u5B50" },
        "fx.p.preamp_db": { en: "Preamp (dB)", "zh-TW": "\u524D\u7D1A dB", "zh-CN": "\u524D\u7EA7 dB" },
        "fx.p.count": { en: "Bands", "zh-TW": "\u6BB5\u6578", "zh-CN": "\u6BB5\u6570" },
        "fx.p.warmth": { en: "Warmth", "zh-TW": "\u6EAB\u6696", "zh-CN": "\u6E29\u6696" },
        "fx.p.depth": { en: "Depth", "zh-TW": "\u6DF1\u5EA6", "zh-CN": "\u6DF1\u5EA6" },
        "fx.p.punch": { en: "Punch", "zh-TW": "\u885D\u64CA", "zh-CN": "\u51B2\u51FB\u611F" },
        "fx.p.body": { en: "Body", "zh-TW": "\u539A\u5EA6", "zh-CN": "\u539A\u5EA6" },
        "fx.p.voice": { en: "Voice", "zh-TW": "\u4EBA\u8072", "zh-CN": "\u4EBA\u58F0" },
        "fx.p.presence": { en: "Presence", "zh-TW": "\u81E8\u5834", "zh-CN": "\u4E34\u573A\u611F" },
        "fx.p.bite": { en: "Bite", "zh-TW": "\u92B3\u5229", "zh-CN": "\u9510\u5229" },
        "fx.p.sibilance": { en: "Sibilance", "zh-TW": "\u9F52\u97F3", "zh-CN": "\u9F7F\u97F3" },
        "fx.p.air": { en: "Air", "zh-TW": "\u7A7A\u6C23\u611F", "zh-CN": "\u7A7A\u6C14\u611F" },
        "fx.p.reach_hz": { en: "Reach (Hz)", "zh-TW": "\u5EF6\u4F38 Hz", "zh-CN": "\u5EF6\u4F38 Hz" },
        "fx.p.cut": { en: "Cut", "zh-TW": "\u524A\u6E1B", "zh-CN": "\u524A\u51CF" },
        "fx.p.quieter_db": { en: "Boost (dB)", "zh-TW": "\u88DC\u511F\u91CF dB", "zh-CN": "\u8865\u507F\u91CF dB" },
        "fx.p.drive_db": { en: "Drive (dB)", "zh-TW": "\u9A45\u52D5 dB", "zh-CN": "\u9A71\u52A8 dB" },
        "fx.p.even": { en: "Even harmonics", "zh-TW": "\u5076\u6B21\u8AE7\u6CE2", "zh-CN": "\u5076\u6B21\u8C10\u6CE2" },
        "fx.p.level_db": { en: "Output (dB)", "zh-TW": "\u8F38\u51FA dB", "zh-CN": "\u8F93\u51FA dB" },
        "fx.p.threshold_db": { en: "Threshold (dB)", "zh-TW": "\u9580\u6ABB dB", "zh-CN": "\u9608\u503C dB" },
        "fx.p.ratio": { en: "Ratio", "zh-TW": "\u6BD4\u4F8B", "zh-CN": "\u6BD4\u7387" },
        "fx.p.knee_db": { en: "Knee (dB)", "zh-TW": "\u819D\u90E8 dB", "zh-CN": "\u62D0\u70B9 dB" },
        "fx.p.attack_ms": { en: "Attack (ms)", "zh-TW": "\u8D77\u97F3 ms", "zh-CN": "\u542F\u52A8 ms" },
        "fx.p.release_ms": { en: "Release (ms)", "zh-TW": "\u91CB\u653E ms", "zh-CN": "\u91CA\u653E ms" },
        "fx.p.makeup_db": { en: "Makeup (dB)", "zh-TW": "\u88DC\u511F dB", "zh-CN": "\u8865\u507F dB" },
        "fx.p.auto_makeup": { en: "Auto makeup", "zh-TW": "\u81EA\u52D5\u88DC\u511F", "zh-CN": "\u81EA\u52A8\u8865\u507F" },
        "fx.p.room": { en: "Room", "zh-TW": "\u7A7A\u9593", "zh-CN": "\u7A7A\u95F4" },
        "fx.p.damp": { en: "Damping", "zh-TW": "\u963B\u5C3C", "zh-CN": "\u963B\u5C3C" },
        "fx.p.level": { en: "Wet level", "zh-TW": "\u6DF7\u5165\u91CF", "zh-CN": "\u6DF7\u5165\u91CF" },
        "fx.p.ceiling_db": { en: "Ceiling (dB)", "zh-TW": "\u4E0A\u9650 dB", "zh-CN": "\u4E0A\u9650 dB" },
        "fx.p.lookahead_ms": { en: "Lookahead (ms)", "zh-TW": "\u9810\u770B ms", "zh-CN": "\u9884\u8BFB ms" },
        "fx.p.mix": { en: "Mix", "zh-TW": "\u6FC3\u5EA6", "zh-CN": "\u6DF7\u5408" },
        "viz.unavailable": { en: "This visual couldn't be loaded.", "zh-TW": "\u9019\u500B\u8996\u89BA\u6548\u679C\u7121\u6CD5\u8F09\u5165\u3002", "zh-CN": "\u8FD9\u4E2A\u89C6\u89C9\u6548\u679C\u65E0\u6CD5\u52A0\u8F7D\u3002" },
        "viz.needHttp": { en: "The visualizer needs this page to be opened over http (not as a file).", "zh-TW": "\u8996\u89BA\u6548\u679C\u9700\u8981\u7528 http \u958B\u555F\u9019\u500B\u9801\u9762\uFF08\u4E0D\u80FD\u76F4\u63A5\u958B\u6A94\u6848\uFF09\u3002", "zh-CN": "\u89C6\u89C9\u6548\u679C\u9700\u8981\u901A\u8FC7 http \u6253\u5F00\u8FD9\u4E2A\u9875\u9762\uFF08\u4E0D\u80FD\u76F4\u63A5\u6253\u5F00\u6587\u4EF6\uFF09\u3002" },
        "sys.title": { en: "Status", "zh-TW": "\u72C0\u614B", "zh-CN": "\u72B6\u6001" },
        "sys.line": { en: "Wi-Fi {wifi} \xB7 Free memory {kb} KB \xB7 Dropouts {n}", "zh-TW": "Wi-Fi {wifi} \xB7 \u53EF\u7528\u8A18\u61B6\u9AD4 {kb} KB \xB7 \u65B7\u97F3 {n} \u6B21", "zh-CN": "Wi-Fi {wifi} \xB7 \u53EF\u7528\u5185\u5B58 {kb} KB \xB7 \u97F3\u9891\u4E2D\u65AD {n} \u6B21" },
        "sys.wifiOn": { en: "connected", "zh-TW": "\u5DF2\u9023\u7DDA", "zh-CN": "\u5DF2\u8FDE\u63A5" },
        "sys.wifiOff": { en: "not connected", "zh-TW": "\u672A\u9023\u7DDA", "zh-CN": "\u672A\u8FDE\u63A5" },
        "sys.log": { en: "Log", "zh-TW": "\u7D00\u9304", "zh-CN": "\u65E5\u5FD7" },
        "lang.title": { en: "Language", "zh-TW": "\u8A9E\u8A00", "zh-CN": "\u8BED\u8A00" },
        "lang.auto": { en: "Browser default", "zh-TW": "\u8DDF\u96A8\u700F\u89BD\u5668", "zh-CN": "\u8DDF\u968F\u6D4F\u89C8\u5668" },
        "rn.needConn": { en: "Connect to the device first.", "zh-TW": "\u8ACB\u5148\u9023\u7DDA\u5230\u88DD\u7F6E\u3002", "zh-CN": "\u8BF7\u5148\u8FDE\u63A5\u5230\u8BBE\u5907\u3002" },
        "rn.sent": { en: "Sent. Waiting for the station to connect\u2026", "zh-TW": "\u5DF2\u9001\u51FA\uFF0C\u7B49\u5F85\u96FB\u53F0\u9023\u7DDA\u2026", "zh-CN": "\u5DF2\u53D1\u9001\uFF0C\u7B49\u5F85\u7535\u53F0\u8FDE\u63A5\u2026" },
        "rn.export": { en: "Export", "zh-TW": "\u532F\u51FA", "zh-CN": "\u5BFC\u51FA" },
        "rn.import": { en: "Import", "zh-TW": "\u532F\u5165", "zh-CN": "\u5BFC\u5165" },
        "rn.removeAria": { en: "Remove {name}", "zh-TW": "\u79FB\u9664 {name}", "zh-CN": "\u79FB\u9664 {name}" },
        "rn.removeTitle": { en: "Remove from the card", "zh-TW": "\u5F9E\u5361\u4E0A\u79FB\u9664", "zh-CN": "\u4ECE\u5361\u4E0A\u79FB\u9664" },
        "rn.empty": { en: "No stations on the card. Tap \u2606 in the search results to add one, or import a tunetap favourites file.", "zh-TW": "\u5361\u4E0A\u9084\u6C92\u6709\u96FB\u53F0\u3002\u5728\u641C\u5C0B\u7D50\u679C\u6309 \u2606 \u52A0\u5165\uFF0C\u6216\u532F\u5165 tunetap \u7684\u6536\u85CF\u6A94\u3002", "zh-CN": "\u5361\u4E0A\u8FD8\u6CA1\u6709\u7535\u53F0\u3002\u5728\u641C\u7D22\u7ED3\u679C\u4E2D\u70B9 \u2606 \u6DFB\u52A0\uFF0C\u6216\u5BFC\u5165 tunetap \u7684\u6536\u85CF\u6587\u4EF6\u3002" },
        "rn.removed": { en: "Removed from the card.", "zh-TW": "\u5DF2\u5F9E\u5361\u4E0A\u79FB\u9664\u3002", "zh-CN": "\u5DF2\u4ECE\u5361\u4E0A\u79FB\u9664\u3002" },
        "rn.removeFailed": { en: "Couldn't remove it ({why}).", "zh-TW": "\u79FB\u9664\u5931\u6557\uFF08{why}\uFF09\u3002", "zh-CN": "\u79FB\u9664\u5931\u8D25\uFF08{why}\uFF09\u3002" },
        "rn.noReply": { en: "no response", "zh-TW": "\u6C92\u6709\u56DE\u61C9", "zh-CN": "\u6CA1\u6709\u54CD\u5E94" },
        "rn.urlLong": { en: "The URL is too long to save on the card (255 bytes max).", "zh-TW": "\u7DB2\u5740\u592A\u9577\uFF08\u4E0A\u9650 255 \u4F4D\u5143\u7D44\uFF09\uFF0C\u7121\u6CD5\u5B58\u5230\u5361\u4E0A\u3002", "zh-CN": "\u7F51\u5740\u592A\u957F\uFF08\u4E0A\u9650 255 \u5B57\u8282\uFF09\uFF0C\u65E0\u6CD5\u4FDD\u5B58\u5230\u5361\u4E0A\u3002" },
        "rn.urlBad": { en: "This URL can't be saved on the card (it must start with http:// or https:// and can't contain |).", "zh-TW": "\u7DB2\u5740\u4E0D\u80FD\u5B58\u5230\u5361\u4E0A\uFF08\u8981 http:// \u6216 https:// \u958B\u982D\uFF0C\u4E0D\u80FD\u542B |\uFF09\u3002", "zh-CN": "\u7F51\u5740\u4E0D\u80FD\u4FDD\u5B58\u5230\u5361\u4E0A\uFF08\u8981\u4EE5 http:// \u6216 https:// \u5F00\u5934\uFF0C\u4E0D\u80FD\u542B |\uFF09\u3002" },
        "rn.saved": { en: "Saved to the card.", "zh-TW": "\u5DF2\u5B58\u5230\u5361\u4E0A\u3002", "zh-CN": "\u5DF2\u4FDD\u5B58\u5230\u5361\u4E0A\u3002" },
        "rn.add.refused": { en: "The device refused this station.", "zh-TW": "\u88DD\u7F6E\u62D2\u7D55\u4E86\u9019\u500B\u96FB\u53F0\u3002", "zh-CN": "\u8BBE\u5907\u62D2\u7EDD\u4E86\u8FD9\u4E2A\u7535\u53F0\u3002" },
        "rn.add.list_full": { en: "The station list on the card is full.", "zh-TW": "\u5361\u4E0A\u7684\u96FB\u53F0\u6E05\u55AE\u5DF2\u6EFF\u3002", "zh-CN": "\u5361\u4E0A\u7684\u7535\u53F0\u5217\u8868\u5DF2\u6EE1\u3002" },
        "rn.add.cannot_write_the_card": { en: "The device can't write to the card.", "zh-TW": "\u88DD\u7F6E\u7121\u6CD5\u5BEB\u5165\u5361\u7247\u3002", "zh-CN": "\u8BBE\u5907\u65E0\u6CD5\u5199\u5165\u5B58\u50A8\u5361\u3002" },
        "rn.add.other": { en: "The device refused: {why}", "zh-TW": "\u88DD\u7F6E\u62D2\u7D55\uFF1A{why}", "zh-CN": "\u8BBE\u5907\u62D2\u7EDD\uFF1A{why}" },
        "rn.devNoReply": { en: "The device didn't respond.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u54CD\u5E94\u3002" },
        "rn.exported": { en: "Exported {n} stations (tunetap-favorites.json; it can be imported into tunetap)", "zh-TW": "\u5DF2\u532F\u51FA {n} \u500B\u96FB\u53F0\uFF08tunetap-favorites.json\uFF0C\u53EF\u532F\u5165 tunetap\uFF09", "zh-CN": "\u5DF2\u5BFC\u51FA {n} \u4E2A\u7535\u53F0\uFF08tunetap-favorites.json\uFF0C\u53EF\u5BFC\u5165 tunetap\uFF09" },
        "rn.fileBig": { en: "File too large (1 MB max)", "zh-TW": "\u6A94\u6848\u592A\u5927\uFF08\u4E0A\u9650 1 MB\uFF09", "zh-CN": "\u6587\u4EF6\u592A\u5927\uFF08\u4E0A\u9650 1 MB\uFF09" },
        "rn.badFile": { en: "This isn't a valid favourites file (JSON exported from tunetap)", "zh-TW": "\u9019\u4E0D\u662F\u6709\u6548\u7684\u6536\u85CF\u6A94\uFF08tunetap \u532F\u51FA\u7684 JSON\uFF09", "zh-CN": "\u8FD9\u4E0D\u662F\u6709\u6548\u7684\u6536\u85CF\u6587\u4EF6\uFF08tunetap \u5BFC\u51FA\u7684 JSON\uFF09" },
        "rn.importing": { en: "Importing\u2026 {i} / {n}", "zh-TW": "\u532F\u5165\u4E2D\u2026 {i} / {n}", "zh-CN": "\u5BFC\u5165\u4E2D\u2026 {i} / {n}" },
        "rn.importDone": { en: "Import finished: {added} added (or already in the list), {refused} not added.", "zh-TW": "\u532F\u5165\u5B8C\u6210\uFF1A\u52A0\u5165\uFF08\u6216\u5DF2\u5728\u6E05\u55AE\uFF09{added} \u500B\uFF0C\u672A\u52A0\u5165 {refused} \u500B\u3002", "zh-CN": "\u5BFC\u5165\u5B8C\u6210\uFF1A\u5DF2\u6DFB\u52A0\uFF08\u6216\u5DF2\u5728\u5217\u8868\u4E2D\uFF09{added} \u4E2A\uFF0C\u672A\u6DFB\u52A0 {refused} \u4E2A\u3002" },
        "rn.importDoneWhy": { en: "Import finished: {added} added (or already in the list), {refused} not added ({why}).", "zh-TW": "\u532F\u5165\u5B8C\u6210\uFF1A\u52A0\u5165\uFF08\u6216\u5DF2\u5728\u6E05\u55AE\uFF09{added} \u500B\uFF0C\u672A\u52A0\u5165 {refused} \u500B\uFF08{why}\uFF09\u3002", "zh-CN": "\u5BFC\u5165\u5B8C\u6210\uFF1A\u5DF2\u6DFB\u52A0\uFF08\u6216\u5DF2\u5728\u5217\u8868\u4E2D\uFF09{added} \u4E2A\uFF0C\u672A\u6DFB\u52A0 {refused} \u4E2A\uFF08{why}\uFF09\u3002" },
        "rn.q": { en: "Station name", "zh-TW": "\u96FB\u53F0\u540D\u7A31", "zh-CN": "\u7535\u53F0\u540D\u79F0" },
        "rn.region": { en: "Region", "zh-TW": "\u5730\u5340", "zh-CN": "\u5730\u533A" },
        "rn.regionAll": { en: "All regions", "zh-TW": "\u5168\u90E8\u5730\u5340", "zh-CN": "\u5168\u90E8\u5730\u533A" },
        "rn.genre": { en: "Genre", "zh-TW": "\u985E\u578B", "zh-CN": "\u7C7B\u578B" },
        "rn.genreAll": { en: "All genres", "zh-TW": "\u5168\u90E8\u985E\u578B", "zh-CN": "\u5168\u90E8\u7C7B\u578B" },
        "rn.codec": { en: "Codec", "zh-TW": "\u7DE8\u78BC", "zh-CN": "\u7F16\u7801" },
        "rn.codecAll": { en: "All codecs", "zh-TW": "\u5168\u90E8\u7DE8\u78BC", "zh-CN": "\u5168\u90E8\u7F16\u7801" },
        "rn.kbps": { en: "Minimum bitrate", "zh-TW": "\u6700\u4F4E\u4F4D\u5143\u7387", "zh-CN": "\u6700\u4F4E\u6BD4\u7279\u7387" },
        "rn.kbpsAll": { en: "Any bitrate", "zh-TW": "\u4E0D\u9650\u4F4D\u5143\u7387", "zh-CN": "\u4E0D\u9650\u6BD4\u7279\u7387" },
        "rn.search": { en: "Search", "zh-TW": "\u641C\u5C0B", "zh-CN": "\u641C\u7D22" },
        "rn.hlsToo": { en: "Also list HLS (not supported by the device)", "zh-TW": "\u4E5F\u5217\u51FA HLS\uFF08\u88DD\u7F6E\u4E0D\u652F\u63F4\uFF09", "zh-CN": "\u4E5F\u5217\u51FA HLS\uFF08\u8BBE\u5907\u4E0D\u652F\u6301\uFF09" },
        "rn.more": { en: "Load more", "zh-TW": "\u8F09\u5165\u66F4\u591A", "zh-CN": "\u52A0\u8F7D\u66F4\u591A" },
        "rn.hlsTitle": { en: "The device doesn't support HLS", "zh-TW": "\u88DD\u7F6E\u4E0D\u652F\u63F4 HLS", "zh-CN": "\u8BBE\u5907\u4E0D\u652F\u6301 HLS" },
        "rn.hlsTag": { en: "Not supported (HLS)", "zh-TW": "\u4E0D\u652F\u63F4\uFF08HLS\uFF09", "zh-CN": "\u4E0D\u652F\u6301\uFF08HLS\uFF09" },
        "rn.maybeTag": { en: "May not play", "zh-TW": "\u53EF\u80FD\u4E0D\u652F\u63F4", "zh-CN": "\u53EF\u80FD\u4E0D\u652F\u6301" },
        "rn.savedTag": { en: "On the card", "zh-TW": "\u5DF2\u5B58\u5728\u5361\u4E0A", "zh-CN": "\u5DF2\u4FDD\u5B58\u5728\u5361\u4E0A" },
        "rn.saveAria": { en: "Save {name} to the card", "zh-TW": "\u5B58\u5230\u5361\u4E0A {name}", "zh-CN": "\u4FDD\u5B58\u5230\u5361\u4E0A {name}" },
        "rn.unsaveAria": { en: "Remove {name} from the card", "zh-TW": "\u5F9E\u5361\u4E0A\u79FB\u9664 {name}", "zh-CN": "\u4ECE\u5361\u4E0A\u79FB\u9664 {name}" },
        "rn.searching": { en: "Searching\u2026", "zh-TW": "\u641C\u5C0B\u4E2D\u2026", "zh-CN": "\u641C\u7D22\u4E2D\u2026" },
        "rn.rbFail": { en: "Can't reach radio-browser.info. Check your internet connection and try again later.", "zh-TW": "\u9023\u4E0D\u5230 radio-browser.info\u3002\u8ACB\u78BA\u8A8D\u624B\u6A5F\u7684\u7DB2\u8DEF\uFF0C\u7A0D\u5F8C\u518D\u8A66\u3002", "zh-CN": "\u8FDE\u4E0D\u4E0A radio-browser.info\u3002\u8BF7\u68C0\u67E5\u624B\u673A\u7684\u7F51\u7EDC\u8FDE\u63A5\uFF0C\u7A0D\u540E\u518D\u8BD5\u3002" },
        "rn.noMatch": { en: "No matching stations.", "zh-TW": "\u6C92\u6709\u7B26\u5408\u7684\u96FB\u53F0\u3002", "zh-CN": "\u6CA1\u6709\u7B26\u5408\u6761\u4EF6\u7684\u7535\u53F0\u3002" },
        "rn.noMatchHid": { en: "No matching stations ({n} hidden by the filters).", "zh-TW": "\u6C92\u6709\u7B26\u5408\u7684\u96FB\u53F0\uFF08\u6709 {n} \u7B46\u88AB\u7BE9\u9078\u96B1\u85CF\uFF09\u3002", "zh-CN": "\u6CA1\u6709\u7B26\u5408\u6761\u4EF6\u7684\u7535\u53F0\uFF08\u6709 {n} \u6761\u88AB\u7B5B\u9009\u9690\u85CF\uFF09\u3002" },
        "rn.results": { en: "{n} stations. Tap one to play it; tap \u2606 to save it to the card.", "zh-TW": "{n} \u500B\u96FB\u53F0\u3002\u9EDE\u4E00\u4E0B\u64AD\u653E\uFF1B\u2606 \u5B58\u5230\u5361\u4E0A\u3002", "zh-CN": "{n} \u4E2A\u7535\u53F0\u3002\u70B9\u4E00\u4E0B\u64AD\u653E\uFF1B\u2606 \u4FDD\u5B58\u5230\u5361\u4E0A\u3002" },
        "rn.resultsHls": { en: "{n} stations; {h} HLS hidden (not supported by the device). Tap one to play it; tap \u2606 to save it to the card.", "zh-TW": "{n} \u500B\u96FB\u53F0\uFF0C\u5DF2\u96B1\u85CF {h} \u500B HLS\uFF08\u88DD\u7F6E\u4E0D\u652F\u63F4\uFF09\u3002\u9EDE\u4E00\u4E0B\u64AD\u653E\uFF1B\u2606 \u5B58\u5230\u5361\u4E0A\u3002", "zh-CN": "{n} \u4E2A\u7535\u53F0\uFF0C\u5DF2\u9690\u85CF {h} \u4E2A HLS\uFF08\u8BBE\u5907\u4E0D\u652F\u6301\uFF09\u3002\u70B9\u4E00\u4E0B\u64AD\u653E\uFF1B\u2606 \u4FDD\u5B58\u5230\u5361\u4E0A\u3002" },
        "rn.savedOne": { en: "\u201C{name}\u201D saved to the card.", "zh-TW": "\u300C{name}\u300D\u5DF2\u5B58\u5230\u5361\u4E0A\u3002", "zh-CN": "\u201C{name}\u201D\u5DF2\u4FDD\u5B58\u5230\u5361\u4E0A\u3002" },
        // ---- visualizer (viz-nano*.js, viz/) ----
        "viz.mode": { en: "Visual", "zh-TW": "\u8996\u89BA\u6548\u679C", "zh-CN": "\u89C6\u89C9\u6548\u679C" },
        "viz.group.step": { en: "Next {0}", "zh-TW": "\u63DB{0}", "zh-CN": "\u6362{0}" },
        "viz.mode.spectrum": { en: "Spectrum", "zh-TW": "\u983B\u8B5C", "zh-CN": "\u9891\u8C31" },
        "viz.preset": { en: "Preset", "zh-TW": "\u9810\u8A2D", "zh-CN": "\u9884\u8BBE" },
        "viz.spectrum.preset.0": { en: "Classic neon", "zh-TW": "\u7D93\u5178\u9713\u8679", "zh-CN": "\u7ECF\u5178\u9713\u8679" },
        "viz.spectrum.preset.1": { en: "Flame", "zh-TW": "\u706B\u7130", "zh-CN": "\u706B\u7130" },
        "viz.spectrum.preset.2": { en: "Ice flame", "zh-TW": "\u51B0\u7130", "zh-CN": "\u51B0\u7130" },
        "viz.spectrum.preset.3": { en: "Winamp", "zh-TW": "Winamp", "zh-CN": "Winamp" },
        "viz.spectrum.preset.4": { en: "Rainbow bands", "zh-TW": "\u5F69\u8679\u983B\u6BB5", "zh-CN": "\u5F69\u8679\u9891\u6BB5" },
        "viz.spectrum.preset.5": { en: "Pulse", "zh-TW": "\u8108\u52D5", "zh-CN": "\u8109\u52A8" },
        "viz.spectrum.preset.6": { en: "Neon tube", "zh-TW": "\u9713\u8679\u71C8\u7BA1", "zh-CN": "\u9713\u8679\u706F\u7BA1" },
        "viz.spectrum.preset.7": { en: "Aurora", "zh-TW": "\u6975\u5149", "zh-CN": "\u6781\u5149" },
        "viz.spectrum.preset.8": { en: "Solid + reflection", "zh-TW": "\u7D14\u8272\uFF0B\u5012\u5F71", "zh-CN": "\u7EAF\u8272\uFF0B\u5012\u5F71" },
        "viz.preset.follow": { en: "Follow track", "zh-TW": "\u96A8\u66F2\u76EE", "zh-CN": "\u968F\u66F2\u76EE" },
        "viz.preset.custom": { en: "Custom", "zh-TW": "\u81EA\u8A02", "zh-CN": "\u81EA\u5B9A" },
        "viz.follow": { en: "Follow track", "zh-TW": "\u96A8\u66F2\u76EE", "zh-CN": "\u968F\u66F2\u76EE" },
        "viz.group.form": { en: "Form", "zh-TW": "\u5F62\u9AD4", "zh-CN": "\u5F62\u4F53" },
        "viz.group.colour": { en: "Colour", "zh-TW": "\u984F\u8272", "zh-CN": "\u989C\u8272" },
        "viz.group.pos": { en: "{0} {1}/{2}", "zh-TW": "{0} {1}/{2}", "zh-CN": "{0} {1}/{2}" },
        "viz.group.follow": { en: "{0}: follow track", "zh-TW": "{0}\uFF1A\u96A8\u66F2\u76EE", "zh-CN": "{0}\uFF1A\u968F\u66F2\u76EE" },
        "viz.shape": { en: "Shape", "zh-TW": "\u5F62\u72C0", "zh-CN": "\u5F62\u72B6" },
        "viz.shape.0": { en: "LED segments", "zh-TW": "LED \u5206\u6BB5", "zh-CN": "LED \u5206\u6BB5" },
        "viz.shape.1": { en: "Solid bars", "zh-TW": "\u5BE6\u5FC3\u67F1", "zh-CN": "\u5B9E\u5FC3\u67F1" },
        "viz.shape.2": { en: "Neon line", "zh-TW": "\u9713\u8679\u71C8\u7BA1", "zh-CN": "\u9713\u8679\u706F\u7BA1" },
        "viz.shape.3": { en: "Ring", "zh-TW": "\u74B0\u5F62", "zh-CN": "\u73AF\u5F62" },
        "viz.map": { en: "Colouring", "zh-TW": "\u4E0A\u8272", "zh-CN": "\u4E0A\u8272" },
        "viz.map.0": { en: "By height", "zh-TW": "\u4F9D\u9AD8\u5EA6", "zh-CN": "\u4F9D\u9AD8\u5EA6" },
        "viz.map.1": { en: "Flame tip", "zh-TW": "\u67F1\u9802\u706B\u82D7", "zh-CN": "\u67F1\u9876\u706B\u82D7" },
        "viz.map.2": { en: "By band", "zh-TW": "\u4F9D\u983B\u6BB5", "zh-CN": "\u4F9D\u9891\u6BB5" },
        "viz.map.3": { en: "By loudness", "zh-TW": "\u4F9D\u97F3\u91CF", "zh-CN": "\u4F9D\u97F3\u91CF" },
        "viz.ramp": { en: "Colour ramp", "zh-TW": "\u8272\u5E36", "zh-CN": "\u8272\u5E26" },
        "viz.ramp.0": { en: "Theme", "zh-TW": "\u914D\u8272", "zh-CN": "\u914D\u8272" },
        "viz.ramp.1": { en: "Fire", "zh-TW": "\u706B\u7130", "zh-CN": "\u706B\u7130" },
        "viz.ramp.2": { en: "Ice", "zh-TW": "\u51B0", "zh-CN": "\u51B0" },
        "viz.ramp.3": { en: "Winamp", "zh-TW": "Winamp", "zh-CN": "Winamp" },
        "viz.ramp.4": { en: "Rainbow", "zh-TW": "\u5F69\u8679", "zh-CN": "\u5F69\u8679" },
        "viz.ramp.5": { en: "Two colours", "zh-TW": "\u81EA\u9078\u5169\u8272", "zh-CN": "\u81EA\u9009\u4E24\u8272" },
        "viz.ramp.6": { en: "Single colour", "zh-TW": "\u55AE\u8272", "zh-CN": "\u5355\u8272" },
        "viz.hue1": { en: "Hue 1", "zh-TW": "\u8272\u5E36\u8272\u76F8 1", "zh-CN": "\u8272\u5E26\u8272\u76F8 1" },
        "viz.hue2": { en: "Hue 2", "zh-TW": "\u8272\u5E36\u8272\u76F8 2", "zh-CN": "\u8272\u5E26\u8272\u76F8 2" },
        "viz.peaks": { en: "Falling peaks", "zh-TW": "\u9802\u9EDE\u4E0B\u843D", "zh-CN": "\u9876\u70B9\u4E0B\u843D" },
        "viz.reflect": { en: "Reflection", "zh-TW": "\u5012\u5F71", "zh-CN": "\u5012\u5F71" },
        "viz.on": { en: "On", "zh-TW": "\u958B", "zh-CN": "\u5F00" },
        "viz.off": { en: "Off", "zh-TW": "\u95DC", "zh-CN": "\u5173" },
        "viz.theme": { en: "Palette", "zh-TW": "\u914D\u8272", "zh-CN": "\u914D\u8272" },
        "viz.theme.app": { en: "Match the page", "zh-TW": "\u8DDF\u8457\u9801\u9762", "zh-CN": "\u8DDF\u968F\u9875\u9762" },
        "viz.theme.neon": { en: "Neon", "zh-TW": "\u9713\u8679", "zh-CN": "\u9713\u8679" },
        "viz.theme.miku": { en: "Miku", "zh-TW": "\u521D\u97F3", "zh-CN": "\u521D\u97F3" },
        "viz.theme.vapor": { en: "Vaporwave", "zh-TW": "\u84B8\u6C7D\u6CE2", "zh-CN": "\u84B8\u6C7D\u6CE2" },
        "viz.theme.amber": { en: "Amber", "zh-TW": "\u7425\u73C0", "zh-CN": "\u7425\u73C0" },
        "viz.theme.phosphor": { en: "Phosphor", "zh-TW": "\u78F7\u5149", "zh-CN": "\u78F7\u5149" },
        "viz.theme.gameboy": { en: "Game Boy", "zh-TW": "Game Boy", "zh-CN": "Game Boy" },
        "viz.theme.paper": { en: "Paper", "zh-TW": "\u7D19", "zh-CN": "\u7EB8" },
        "viz.theme.solar": { en: "Solarized", "zh-TW": "Solarized", "zh-CN": "Solarized" },
        "viz.theme.gruv": { en: "Gruvbox", "zh-TW": "Gruvbox", "zh-CN": "Gruvbox" },
        "viz.theme.unit": { en: "Unit", "zh-TW": "\u7D2B\u7532", "zh-CN": "\u7D2B\u7532" },
        "viz.theme.ink": { en: "Ink", "zh-TW": "\u58A8", "zh-CN": "\u58A8" },
        "viz.theme.snes": { en: "SNES", "zh-TW": "SNES", "zh-CN": "SNES" },
        "viz.theme.sfc": { en: "SFC", "zh-TW": "SFC", "zh-CN": "SFC" },
        "viz.theme.caution": { en: "Caution", "zh-TW": "\u8B66\u6212", "zh-CN": "\u8B66\u6212" },
        "viz.theme.aqua": { en: "Aqua", "zh-TW": "\u6C34\u8272", "zh-CN": "\u6C34\u8272" },
        "viz.theme.cozy": { en: "Cozy", "zh-TW": "\u6696", "zh-CN": "\u6696" },
        "viz.theme.mono": { en: "Mono", "zh-TW": "\u9ED1\u767D", "zh-CN": "\u9ED1\u767D" },
        "viz.theme.carbon": { en: "Carbon", "zh-TW": "\u78B3\u7E96", "zh-CN": "\u78B3\u7EA4" },
        "viz.mode.drive": { en: "Night drive", "zh-TW": "\u591C\u884C", "zh-CN": "\u591C\u884C" },
        "viz.route": { en: "Route", "zh-TW": "\u8DEF\u7DDA", "zh-CN": "\u8DEF\u7EBF" },
        "viz.route.1": { en: "Sunset Hills", "zh-TW": "\u5915\u967D\u5C71\u8DEF", "zh-CN": "\u5915\u9633\u5C71\u8DEF" },
        "viz.route.2": { en: "Neon City", "zh-TW": "\u9713\u8679\u57CE\u5E02", "zh-CN": "\u9713\u8679\u57CE\u5E02" },
        "viz.route.3": { en: "Coast Road", "zh-TW": "\u6D77\u5CB8\u516C\u8DEF", "zh-CN": "\u6D77\u5CB8\u516C\u8DEF" },
        "viz.route.4": { en: "Aurora", "zh-TW": "\u6975\u5149", "zh-CN": "\u6781\u5149" },
        "viz.route.5": { en: "Live Stage", "zh-TW": "\u6F14\u5531\u6703", "zh-CN": "\u6F14\u5531\u4F1A" },
        "viz.hour": { en: "Time of day", "zh-TW": "\u6642\u523B", "zh-CN": "\u65F6\u523B" },
        "viz.hour.0": { en: "Night", "zh-TW": "\u591C", "zh-CN": "\u591C" },
        "viz.hour.1": { en: "Dusk", "zh-TW": "\u9EC3\u660F", "zh-CN": "\u9EC4\u660F" },
        "viz.hour.2": { en: "Dawn", "zh-TW": "\u9ECE\u660E", "zh-CN": "\u9ECE\u660E" },
        "viz.hour.3": { en: "Day", "zh-TW": "\u767D\u5929", "zh-CN": "\u767D\u5929" },
        "viz.hour.4": { en: "Local clock", "zh-TW": "\u8DDF\u8457\u6642\u9418", "zh-CN": "\u8DDF\u7740\u65F6\u949F" },
        "viz.weather": { en: "Weather (your choice, no forecast)", "zh-TW": "\u5929\u6C23\uFF08\u81EA\u5DF1\u9078\uFF0C\u4E0D\u8B80\u9810\u5831\uFF09", "zh-CN": "\u5929\u6C14\uFF08\u81EA\u5DF1\u9009\uFF0C\u4E0D\u8BFB\u9884\u62A5\uFF09" },
        "viz.weather.0": { en: "Clear", "zh-TW": "\u6674", "zh-CN": "\u6674" },
        "viz.weather.1": { en: "Rain", "zh-TW": "\u96E8", "zh-CN": "\u96E8" },
        "viz.weather.2": { en: "Snow", "zh-TW": "\u96EA", "zh-CN": "\u96EA" },
        "viz.weather.3": { en: "Fog", "zh-TW": "\u9727", "zh-CN": "\u96FE" },
        "viz.weather_react": { en: "Weather hears the music", "zh-TW": "\u5929\u6C23\u96A8\u97F3\u6A02", "zh-CN": "\u5929\u6C14\u968F\u97F3\u4E50" },
        "viz.weather_react.0": { en: "Off", "zh-TW": "\u95DC", "zh-CN": "\u5173" },
        "viz.weather_react.1": { en: "Soft", "zh-TW": "\u5F31", "zh-CN": "\u5F31" },
        "viz.weather_react.2": { en: "Strong", "zh-TW": "\u5F37", "zh-CN": "\u5F3A" },
        "viz.car": { en: "Car in the palette colours", "zh-TW": "\u8ECA\u8EAB\u8DDF\u914D\u8272", "zh-CN": "\u8F66\u8EAB\u8DDF\u914D\u8272" },
        "viz.drive.preset.0": { en: "Original night", "zh-TW": "\u539F\u7248\u591C\u8272", "zh-CN": "\u539F\u7248\u591C\u8272" },
        "viz.drive.preset.1": { en: "Local clock", "zh-TW": "\u8DDF\u8457\u6642\u9418", "zh-CN": "\u8DDF\u7740\u65F6\u949F" },
        "viz.drive.preset.2": { en: "Dusk", "zh-TW": "\u9EC3\u660F", "zh-CN": "\u9EC4\u660F" },
        "viz.drive.preset.3": { en: "Rainy dawn", "zh-TW": "\u9ECE\u660E\u96E8", "zh-CN": "\u9ECE\u660E\u96E8" },
        "viz.drive.preset.4": { en: "Snowy night", "zh-TW": "\u96EA\u591C", "zh-CN": "\u96EA\u591C" },
        "viz.drive.preset.5": { en: "Morning fog", "zh-TW": "\u6668\u9727", "zh-CN": "\u6668\u96FE" },
        "viz.group.sky": { en: "Sky", "zh-TW": "\u5929\u7A7A", "zh-CN": "\u5929\u7A7A" },
        "viz.group.road": { en: "Route", "zh-TW": "\u8DEF\u7DDA", "zh-CN": "\u8DEF\u7EBF" },
        "viz.mode.waterfall": { en: "Waterfall", "zh-TW": "\u7011\u5E03", "zh-CN": "\u7011\u5E03" },
        "viz.wf_layout": { en: "Layout", "zh-TW": "\u7248\u9762", "zh-CN": "\u7248\u9762" },
        "viz.wf_layout.0": { en: "L | R mirrored", "zh-TW": "L\uFF5CR \u93E1\u50CF", "zh-CN": "L\uFF5CR \u955C\u50CF" },
        "viz.wf_layout.1": { en: "L+R full width", "zh-TW": "L+R \u5168\u5BEC", "zh-CN": "L+R \u5168\u5BBD" },
        "viz.wf_layout.2": { en: "L over R", "zh-TW": "L \u4E0A R \u4E0B", "zh-CN": "L \u4E0A R \u4E0B" },
        "viz.wf_dir": { en: "Direction", "zh-TW": "\u65B9\u5411", "zh-CN": "\u65B9\u5411" },
        "viz.wf_dir.0": { en: "Flows down", "zh-TW": "\u5F80\u4E0B\u6D41", "zh-CN": "\u5F80\u4E0B\u6D41" },
        "viz.wf_dir.1": { en: "Flows up", "zh-TW": "\u5F80\u4E0A\u6D41", "zh-CN": "\u5F80\u4E0A\u6D41" },
        "viz.wf_dir.2": { en: "Spectrogram across", "zh-TW": "\u6A6B\u5411\u983B\u8B5C\u5716", "zh-CN": "\u6A2A\u5411\u9891\u8C31\u56FE" },
        "viz.waterfall.preset.0": { en: "Mirrored waterfall", "zh-TW": "\u93E1\u50CF\u7011\u5E03", "zh-CN": "\u955C\u50CF\u7011\u5E03" },
        "viz.waterfall.preset.1": { en: "Full-width waterfall", "zh-TW": "\u5168\u5BEC\u7011\u5E03", "zh-CN": "\u5168\u5BBD\u7011\u5E03" },
        "viz.waterfall.preset.2": { en: "Two-lane waterfall", "zh-TW": "\u96D9\u5C64\u7011\u5E03", "zh-CN": "\u53CC\u5C42\u7011\u5E03" },
        "viz.waterfall.preset.3": { en: "Rising", "zh-TW": "\u4E0A\u5347", "zh-CN": "\u4E0A\u5347" },
        "viz.waterfall.preset.4": { en: "Spectrogram", "zh-TW": "\u983B\u8B5C\u5716", "zh-CN": "\u9891\u8C31\u56FE" },
        "viz.waterfall.preset.5": { en: "Spectrogram L / R", "zh-TW": "\u983B\u8B5C\u5716 L / R", "zh-CN": "\u9891\u8C31\u56FE L / R" },
        "viz.group.layout": { en: "Layout", "zh-TW": "\u7248\u9762", "zh-CN": "\u7248\u9762" },
        "viz.group.ramp": { en: "Ramp", "zh-TW": "\u8272\u5E36", "zh-CN": "\u8272\u5E26" },
        "viz.mode.cassette": { en: "Cassette", "zh-TW": "\u9304\u97F3\u5E36", "zh-CN": "\u5F55\u97F3\u5E26" },
        "viz.tape_shell": { en: "Shell", "zh-TW": "\u5916\u6BBC", "zh-CN": "\u5916\u58F3" },
        "viz.tape_shell.0": { en: "Black", "zh-TW": "\u9ED1", "zh-CN": "\u9ED1" },
        "viz.tape_shell.1": { en: "Clear", "zh-TW": "\u900F\u660E", "zh-CN": "\u900F\u660E" },
        "viz.tape_shell.2": { en: "White", "zh-TW": "\u767D", "zh-CN": "\u767D" },
        "viz.tape_shell.3": { en: "Palette", "zh-TW": "\u914D\u8272", "zh-CN": "\u914D\u8272" },
        "viz.tape_label": { en: "Label", "zh-TW": "\u6A19\u7C64", "zh-CN": "\u6807\u7B7E" },
        "viz.tape_label.0": { en: "Cream", "zh-TW": "\u7C73\u8272", "zh-CN": "\u7C73\u8272" },
        "viz.tape_label.1": { en: "Palette", "zh-TW": "\u914D\u8272", "zh-CN": "\u914D\u8272" },
        "viz.tape_alpha": { en: "Label opacity", "zh-TW": "\u6A19\u7C64\u900F\u660E\u5EA6", "zh-CN": "\u6807\u7B7E\u900F\u660E\u5EA6" },
        "viz.tape_alpha.0": { en: "Opaque", "zh-TW": "\u4E0D\u900F\u660E", "zh-CN": "\u4E0D\u900F\u660E" },
        "viz.tape_alpha.1": { en: "75%", "zh-TW": "75%", "zh-CN": "75%" },
        "viz.tape_alpha.2": { en: "50%", "zh-TW": "50%", "zh-CN": "50%" },
        "viz.tape_alpha.3": { en: "25%", "zh-TW": "25%", "zh-CN": "25%" },
        "viz.cassette.preset.0": { en: "Classic", "zh-TW": "\u7D93\u5178", "zh-CN": "\u7ECF\u5178" },
        "viz.cassette.preset.1": { en: "Clear", "zh-TW": "\u900F\u660E", "zh-CN": "\u900F\u660E" },
        "viz.cassette.preset.2": { en: "White", "zh-TW": "\u767D\u8272", "zh-CN": "\u767D\u8272" },
        "viz.cassette.preset.3": { en: "Palette", "zh-TW": "\u914D\u8272", "zh-CN": "\u914D\u8272" },
        "viz.cassette.preset.4": { en: "See-through", "zh-TW": "\u900F\u8996", "zh-CN": "\u900F\u89C6" },
        "viz.group.shell": { en: "Shell", "zh-TW": "\u5916\u6BBC", "zh-CN": "\u5916\u58F3" },
        "viz.group.label": { en: "Label", "zh-TW": "\u6A19\u7C64", "zh-CN": "\u6807\u7B7E" },
        "viz.prev": { en: "Previous visual", "zh-TW": "\u4E0A\u4E00\u500B\u8996\u89BA\u6548\u679C", "zh-CN": "\u4E0A\u4E00\u4E2A\u89C6\u89C9\u6548\u679C" },
        "viz.next": { en: "Next visual", "zh-TW": "\u4E0B\u4E00\u500B\u8996\u89BA\u6548\u679C", "zh-CN": "\u4E0B\u4E00\u4E2A\u89C6\u89C9\u6548\u679C" },
        "viz.controls": { en: "Visual controls", "zh-TW": "\u8996\u89BA\u6548\u679C\u63A7\u5236", "zh-CN": "\u89C6\u89C9\u6548\u679C\u63A7\u5236" },
        "eq.graph.aria": { en: "Equalizer frequency response", "zh-TW": "\u7B49\u5316\u5668\u983B\u7387\u97FF\u61C9\u5716", "zh-CN": "\u5747\u8861\u5668\u9891\u7387\u54CD\u5E94\u56FE" },
        "eq.bands.title": { en: "Bands {n} / {max}", "zh-TW": "\u983B\u6BB5 {n} / {max}", "zh-CN": "\u9891\u6BB5 {n} / {max}" },
        "eq.type.pk": { en: "Peaking", "zh-TW": "\u5CF0\u503C", "zh-CN": "\u5CF0\u503C" },
        "eq.type.ls": { en: "Low shelf", "zh-TW": "\u4F4E\u983B\u64F1\u67B6", "zh-CN": "\u4F4E\u9891\u6401\u67B6" },
        "eq.type.hs": { en: "High shelf", "zh-TW": "\u9AD8\u983B\u64F1\u67B6", "zh-CN": "\u9AD8\u9891\u6401\u67B6" },
        "eq.type.aria": { en: "Band {n} type", "zh-TW": "\u7B2C {n} \u6BB5\u985E\u578B", "zh-CN": "\u7B2C {n} \u6BB5\u7C7B\u578B" },
        "eq.fc": { en: "Freq Hz", "zh-TW": "\u983B\u7387 Hz", "zh-CN": "\u9891\u7387 Hz" },
        "eq.gain": { en: "Gain dB", "zh-TW": "\u589E\u76CA dB", "zh-CN": "\u589E\u76CA dB" },
        "eq.q": { en: "Q", "zh-TW": "Q \u503C", "zh-CN": "Q \u503C" },
        "eq.fc.aria": { en: "Band {n} frequency", "zh-TW": "\u7B2C {n} \u6BB5\u983B\u7387", "zh-CN": "\u7B2C {n} \u6BB5\u9891\u7387" },
        "eq.gain.aria": { en: "Band {n} gain", "zh-TW": "\u7B2C {n} \u6BB5\u589E\u76CA", "zh-CN": "\u7B2C {n} \u6BB5\u589E\u76CA" },
        "eq.q.aria": { en: "Band {n} Q", "zh-TW": "\u7B2C {n} \u6BB5 Q \u503C", "zh-CN": "\u7B2C {n} \u6BB5 Q \u503C" },
        "eq.fcnum.aria": { en: "Band {n} frequency in Hz", "zh-TW": "\u7B2C {n} \u6BB5\u983B\u7387\uFF08Hz\uFF09", "zh-CN": "\u7B2C {n} \u6BB5\u9891\u7387\uFF08Hz\uFF09" },
        "eq.gainnum.aria": { en: "Band {n} gain in dB", "zh-TW": "\u7B2C {n} \u6BB5\u589E\u76CA\uFF08dB\uFF09", "zh-CN": "\u7B2C {n} \u6BB5\u589E\u76CA\uFF08dB\uFF09" },
        "eq.qnum.aria": { en: "Band {n} Q value", "zh-TW": "\u7B2C {n} \u6BB5 Q \u6578\u503C", "zh-CN": "\u7B2C {n} \u6BB5 Q \u6570\u503C" },
        "eq.add": { en: "Add band", "zh-TW": "\u65B0\u589E\u983B\u6BB5", "zh-CN": "\u65B0\u589E\u9891\u6BB5" },
        "eq.remove": { en: "Remove band {n}", "zh-TW": "\u79FB\u9664\u7B2C {n} \u6BB5", "zh-CN": "\u5220\u9664\u7B2C {n} \u6BB5" },
        "eq.reset": { en: "Reset", "zh-TW": "\u91CD\u8A2D", "zh-CN": "\u91CD\u7F6E" },
        "eq.empty": { en: "No bands yet. Add one, or import a preset.", "zh-TW": "\u9084\u6C92\u6709\u983B\u6BB5\u3002\u8ACB\u65B0\u589E\u4E00\u6BB5\uFF0C\u6216\u532F\u5165\u8A2D\u5B9A\u3002", "zh-CN": "\u8FD8\u6CA1\u6709\u9891\u6BB5\u3002\u8BF7\u65B0\u589E\u4E00\u6BB5\uFF0C\u6216\u5BFC\u5165\u9884\u8BBE\u3002" },
        "eq.imp.title": { en: "Import AutoEq / Equalizer APO preset", "zh-TW": "\u532F\u5165 AutoEq / Equalizer APO \u8A2D\u5B9A", "zh-CN": "\u5BFC\u5165 AutoEq / Equalizer APO \u9884\u8BBE" },
        "eq.imp.aria": { en: "ParametricEQ text", "zh-TW": "ParametricEQ \u6587\u5B57", "zh-CN": "ParametricEQ \u6587\u672C" },
        "eq.imp.btn": { en: "Import", "zh-TW": "\u532F\u5165", "zh-CN": "\u5BFC\u5165" },
        "eq.imp.busy": { en: "Importing\u2026", "zh-TW": "\u532F\u5165\u4E2D\u2026", "zh-CN": "\u5BFC\u5165\u4E2D\u2026" },
        "eq.imp.none": { en: "No usable filters found in that text.", "zh-TW": "\u9019\u6BB5\u6587\u5B57\u88E1\u6C92\u6709\u53EF\u532F\u5165\u7684\u6FFE\u6CE2\u5668\u3002", "zh-CN": "\u8FD9\u6BB5\u6587\u672C\u91CC\u6CA1\u6709\u53EF\u5BFC\u5165\u7684\u6EE4\u6CE2\u5668\u3002" },
        "eq.imp.done": { en: "{n} bands imported, {m} skipped (OFF or unsupported type).", "zh-TW": "\u5DF2\u532F\u5165 {n} \u500B\u983B\u6BB5\uFF0C\u7565\u904E {m} \u500B\uFF08\u95DC\u9589\u6216\u4E0D\u652F\u63F4\u7684\u985E\u578B\uFF09\u3002", "zh-CN": "\u5DF2\u5BFC\u5165 {n} \u4E2A\u9891\u6BB5\uFF0C\u8DF3\u8FC7 {m} \u4E2A\uFF08\u5DF2\u5173\u95ED\u6216\u4E0D\u652F\u6301\u7684\u7C7B\u578B\uFF09\u3002" },
        "eq.imp.over": { en: "{n} bands imported, {m} skipped (OFF or unsupported type). The limit is {max} bands; {k} more were not imported.", "zh-TW": "\u5DF2\u532F\u5165 {n} \u500B\u983B\u6BB5\uFF0C\u7565\u904E {m} \u500B\uFF08\u95DC\u9589\u6216\u4E0D\u652F\u63F4\u7684\u985E\u578B\uFF09\u3002\u4E0A\u9650\u70BA {max} \u6BB5\uFF0C\u53E6\u6709 {k} \u500B\u672A\u532F\u5165\u3002", "zh-CN": "\u5DF2\u5BFC\u5165 {n} \u4E2A\u9891\u6BB5\uFF0C\u8DF3\u8FC7 {m} \u4E2A\uFF08\u5DF2\u5173\u95ED\u6216\u4E0D\u652F\u6301\u7684\u7C7B\u578B\uFF09\u3002\u4E0A\u9650\u4E3A {max} \u6BB5\uFF0C\u53E6\u6709 {k} \u4E2A\u672A\u5BFC\u5165\u3002" },
        "now.resumeHint": { en: "Press play to continue where you stopped", "zh-TW": "\u53EF\u5F9E\u4E0A\u6B21\u505C\u6B62\u7684\u5730\u65B9\u7E7C\u7E8C", "zh-CN": "\u53EF\u4ECE\u4E0A\u6B21\u505C\u6B62\u7684\u5730\u65B9\u7EE7\u7EED" },
        "now.pickHint": { en: "Pick a file, station or noise to start playing", "zh-TW": "\u9078\u4E00\u500B\u6A94\u6848\u3001\u96FB\u53F0\u6216\u566A\u97F3\u958B\u59CB\u64AD\u653E", "zh-CN": "\u9009\u4E00\u4E2A\u6587\u4EF6\u3001\u7535\u53F0\u6216\u566A\u97F3\u5F00\u59CB\u64AD\u653E" },
        "sys.cpu": { en: "CPU {a}% / {b}%", "zh-TW": "CPU {a}% / {b}%", "zh-CN": "CPU {a}% / {b}%" },
        "sys.spark.cpu0": { en: "CPU core 0", "zh-TW": "CPU \u6838\u5FC3 0", "zh-CN": "CPU \u6838\u5FC3 0" },
        "sys.spark.cpu1": { en: "CPU core 1", "zh-TW": "CPU \u6838\u5FC3 1", "zh-CN": "CPU \u6838\u5FC3 1" },
        "sys.spark.mem": { en: "Free memory", "zh-TW": "\u53EF\u7528\u8A18\u61B6\u9AD4", "zh-CN": "\u53EF\u7528\u5185\u5B58" },
        "rg.title": { en: "Volume levelling (ReplayGain)", "zh-TW": "\u97F3\u91CF\u5E73\u8861\uFF08ReplayGain\uFF09", "zh-CN": "\u97F3\u91CF\u5747\u8861\uFF08ReplayGain\uFF09" },
        "rg.off": { en: "Off", "zh-TW": "\u95DC", "zh-CN": "\u5173" },
        "rg.track": { en: "Track", "zh-TW": "\u55AE\u66F2", "zh-CN": "\u5355\u66F2" },
        "rg.album": { en: "Album", "zh-TW": "\u5C08\u8F2F", "zh-CN": "\u4E13\u8F91" },
        "rg.hint": { en: "Uses the tags in the files; applies from the next track.", "zh-TW": "\u4F7F\u7528\u6A94\u6848\u5167\u7684\u6A19\u7C64\uFF0C\u5F9E\u4E0B\u4E00\u9996\u958B\u59CB\u751F\u6548\u3002", "zh-CN": "\u4F7F\u7528\u6587\u4EF6\u5185\u7684\u6807\u7B7E\uFF0C\u4ECE\u4E0B\u4E00\u9996\u5F00\u59CB\u751F\u6548\u3002" },
        "keys.title": { en: "Headset buttons", "zh-TW": "\u8033\u6A5F\u7DDA\u63A7\u6309\u9375", "zh-CN": "\u8033\u673A\u7EBF\u63A7\u6309\u952E" },
        "keys.hint.wired": { en: "Needs a wired headset plugged into the USB-C DAC dongle.", "zh-TW": "\u9700\u8981\u628A\u6709\u7DDA\u8033\u6A5F\u63D2\u5728 USB-C \u97F3\u6548\u8F49\u63A5\u982D\uFF08USB DAC\uFF09\u4E0A\u3002", "zh-CN": "\u9700\u8981\u628A\u6709\u7EBF\u8033\u673A\u63D2\u5728 USB-C \u97F3\u9891\u8F6C\u63A5\u5934\uFF08USB DAC\uFF09\u4E0A\u3002" },
        "keys.hint.bt": { en: "The buttons on Bluetooth headphones work on their own and are not set here.", "zh-TW": "\u85CD\u7259\u8033\u6A5F\u81EA\u5DF1\u7684\u6309\u9375\u53E6\u5916\u904B\u4F5C\uFF0C\u4E0D\u5728\u9019\u88E1\u8A2D\u5B9A\u3002", "zh-CN": "\u84DD\u7259\u8033\u673A\u81EA\u5E26\u7684\u6309\u952E\u72EC\u7ACB\u5DE5\u4F5C\uFF0C\u4E0D\u5728\u8FD9\u91CC\u8BBE\u7F6E\u3002" },
        "keys.hint.delay": { en: "A button with no two-click and no three-click action acts the instant you press it; otherwise a single click waits 0.4 s to see whether another click follows.", "zh-TW": "\u6C92\u6709\u8A2D\u5B9A\u9023\u6309\u5169\u4E0B\u3001\u9023\u6309\u4E09\u4E0B\u7684\u6309\u9375\uFF0C\u6309\u4E0B\u53BB\u7ACB\u523B\u52D5\u4F5C\uFF1B\u5426\u5247\u6309\u4E00\u4E0B\u8981\u7B49 0.4 \u79D2\uFF0C\u78BA\u8A8D\u6C92\u6709\u63A5\u8457\u9023\u6309\u3002", "zh-CN": "\u6CA1\u6709\u8BBE\u7F6E\u8FDE\u6309\u4E24\u4E0B\u3001\u8FDE\u6309\u4E09\u4E0B\u7684\u6309\u952E\uFF0C\u6309\u4E0B\u53BB\u7ACB\u5373\u751F\u6548\uFF1B\u5426\u5219\u6309\u4E00\u4E0B\u8981\u7B49 0.4 \u79D2\uFF0C\u786E\u8BA4\u6CA1\u6709\u63A5\u7740\u8FDE\u6309\u3002" },
        "keys.preset.1": { en: "1-button headset", "zh-TW": "\u55AE\u9375\u8033\u6A5F", "zh-CN": "\u5355\u952E\u8033\u673A" },
        "keys.preset.2": { en: "2-button headset (+ \u2212)", "zh-TW": "\u96D9\u9375\u8033\u6A5F\uFF08+ \u2212\uFF09", "zh-CN": "\u53CC\u952E\u8033\u673A\uFF08+ \u2212\uFF09" },
        "keys.preset.3": { en: "3-button headset", "zh-TW": "\u4E09\u9375\u8033\u6A5F", "zh-CN": "\u4E09\u952E\u8033\u673A" },
        "keys.custom": { en: "Custom", "zh-TW": "\u81EA\u8A02", "zh-CN": "\u81EA\u5B9A\u4E49" },
        "keys.btn.c": { en: "Centre", "zh-TW": "\u4E2D\u9375", "zh-CN": "\u4E2D\u952E" },
        "keys.btn.p": { en: "Plus", "zh-TW": "\u52A0\u9375", "zh-CN": "\u52A0\u952E" },
        "keys.btn.m": { en: "Minus", "zh-TW": "\u6E1B\u9375", "zh-CN": "\u51CF\u952E" },
        "keys.g.1": { en: "1 click", "zh-TW": "\u6309\u4E00\u4E0B", "zh-CN": "\u6309\u4E00\u4E0B" },
        "keys.g.2": { en: "2 clicks", "zh-TW": "\u9023\u6309\u5169\u4E0B", "zh-CN": "\u8FDE\u6309\u4E24\u4E0B" },
        "keys.g.3": { en: "3 clicks", "zh-TW": "\u9023\u6309\u4E09\u4E0B", "zh-CN": "\u8FDE\u6309\u4E09\u4E0B" },
        "keys.g.h": { en: "Hold", "zh-TW": "\u9577\u6309", "zh-CN": "\u957F\u6309" },
        "keys.a.none": { en: "None", "zh-TW": "\u7121", "zh-CN": "\u65E0" },
        "keys.a.play": { en: "Play / pause", "zh-TW": "\u64AD\u653E\u30FB\u66AB\u505C", "zh-CN": "\u64AD\u653E\u30FB\u6682\u505C" },
        "keys.a.next": { en: "Next", "zh-TW": "\u4E0B\u4E00\u9996", "zh-CN": "\u4E0B\u4E00\u9996" },
        "keys.a.prev": { en: "Previous", "zh-TW": "\u4E0A\u4E00\u9996", "zh-CN": "\u4E0A\u4E00\u9996" },
        "keys.a.volup": { en: "Volume +", "zh-TW": "\u97F3\u91CF +", "zh-CN": "\u97F3\u91CF +" },
        "keys.a.voldn": { en: "Volume \u2212", "zh-TW": "\u97F3\u91CF \u2212", "zh-CN": "\u97F3\u91CF \u2212" },
        "keys.a.stop": { en: "Stop", "zh-TW": "\u505C\u6B62", "zh-CN": "\u505C\u6B62" },
        "keys.sel": { en: "{b}, {g}", "zh-TW": "{b} {g}", "zh-CN": "{b} {g}" },
        "keys.last.some": { en: "Just detected: {b}, {g} \u2192 {a} ({s} s ago)", "zh-TW": "\u525B\u624D\u5075\u6E2C\u5230\uFF1A{b} {g} \u2192 {a}\uFF08{s} \u79D2\u524D\uFF09", "zh-CN": "\u521A\u624D\u68C0\u6D4B\u5230\uFF1A{b} {g} \u2192 {a}\uFF08{s} \u79D2\u524D\uFF09" },
        "keys.last.none": { en: "No button press detected yet. Press a button on the headset to try it.", "zh-TW": "\u9084\u6C92\u6709\u5075\u6E2C\u5230\u6309\u9375\u3002\u6309\u4E00\u4E0B\u8033\u6A5F\u4E0A\u7684\u6309\u9375\u8A66\u8A66\u3002", "zh-CN": "\u8FD8\u6CA1\u6709\u68C0\u6D4B\u5230\u6309\u952E\u3002\u6309\u4E00\u4E0B\u8033\u673A\u4E0A\u7684\u6309\u952E\u8BD5\u8BD5\u3002" },
        "keys.err": { en: "The box did not accept that setting.", "zh-TW": "\u8033\u6A5F\u6309\u9375\u8A2D\u5B9A\u6C92\u6709\u88AB\u63A5\u53D7\u3002", "zh-CN": "\u8033\u673A\u6309\u952E\u8BBE\u7F6E\u6CA1\u6709\u88AB\u63A5\u53D7\u3002" },
        "card.title": { en: "Memory card", "zh-TW": "\u8A18\u61B6\u5361", "zh-CN": "\u5B58\u50A8\u5361" },
        "card.wait": { en: "Loads after you connect", "zh-TW": "\u9023\u7DDA\u5F8C\u81EA\u52D5\u8F09\u5165", "zh-CN": "\u8FDE\u63A5\u540E\u81EA\u52A8\u52A0\u8F7D" },
        "card.checking": { en: "checking\u2026", "zh-TW": "\u78BA\u8A8D\u4E2D\u2026", "zh-CN": "\u786E\u8BA4\u4E2D\u2026" },
        "card.unknown": { en: "unknown", "zh-TW": "\u4E0D\u660E", "zh-CN": "\u672A\u77E5" },
        "card.noname": { en: "no name", "zh-TW": "\u6C92\u6709\u540D\u7A31", "zh-CN": "\u6CA1\u6709\u540D\u79F0" },
        "card.na": { en: "No card information from the box.", "zh-TW": "\u6C92\u6709\u53D6\u5F97\u8A18\u61B6\u5361\u8CC7\u8A0A\u3002", "zh-CN": "\u6CA1\u6709\u83B7\u53D6\u5230\u5B58\u50A8\u5361\u4FE1\u606F\u3002" },
        "card.facts.ok": { en: "{size} ({free} free) \xB7 {fs} \xB7 \u201C{name}\u201D", "zh-TW": "{size}\uFF08\u53EF\u7528 {free}\uFF09\xB7 {fs} \xB7 \u300C{name}\u300D", "zh-CN": "{size}\uFF08\u53EF\u7528 {free}\uFF09\xB7 {fs} \xB7 \u201C{name}\u201D" },
        "card.facts.bad": { en: "{size} \xB7 \u201C{name}\u201D", "zh-TW": "{size} \xB7 \u300C{name}\u300D", "zh-CN": "{size} \xB7 \u201C{name}\u201D" },
        "card.state.none": { en: "No memory card is inserted.", "zh-TW": "\u6C92\u6709\u63D2\u5165\u8A18\u61B6\u5361\u3002", "zh-CN": "\u6CA1\u6709\u63D2\u5165\u5B58\u50A8\u5361\u3002" },
        "card.state.bad": { en: "The card is in, but the box cannot read it.", "zh-TW": "\u8A18\u61B6\u5361\u5DF2\u63D2\u5165\uFF0C\u4F46\u88DD\u7F6E\u8B80\u4E0D\u5230\u3002", "zh-CN": "\u5B58\u50A8\u5361\u5DF2\u63D2\u5165\uFF0C\u4F46\u8BBE\u5907\u8BFB\u4E0D\u51FA\u6765\u3002" },
        "card.warn.none": { en: "No memory card: music, the station list, Wi-Fi settings and most other functions need one.", "zh-TW": "\u6C92\u6709\u8A18\u61B6\u5361\uFF1A\u97F3\u6A02\u3001\u96FB\u53F0\u6E05\u55AE\u3001Wi-Fi \u8A2D\u5B9A\u7B49\u5927\u90E8\u5206\u529F\u80FD\u90FD\u9700\u8981\u8A18\u61B6\u5361\u3002", "zh-CN": "\u6CA1\u6709\u5B58\u50A8\u5361\uFF1A\u97F3\u4E50\u3001\u7535\u53F0\u5217\u8868\u3001Wi-Fi \u8BBE\u7F6E\u7B49\u5927\u90E8\u5206\u529F\u80FD\u90FD\u9700\u8981\u5B58\u50A8\u5361\u3002" },
        "card.warn.bad": { en: "The memory card cannot be read: music, the station list, Wi-Fi settings and most other functions are unavailable for now.", "zh-TW": "\u8A18\u61B6\u5361\u8B80\u4E0D\u5230\uFF1A\u97F3\u6A02\u3001\u96FB\u53F0\u6E05\u55AE\u3001Wi-Fi \u8A2D\u5B9A\u7B49\u5927\u90E8\u5206\u529F\u80FD\u66AB\u6642\u7121\u6CD5\u4F7F\u7528\u3002", "zh-CN": "\u5B58\u50A8\u5361\u8BFB\u4E0D\u51FA\u6765\uFF1A\u97F3\u4E50\u3001\u7535\u53F0\u5217\u8868\u3001Wi-Fi \u8BBE\u7F6E\u7B49\u5927\u90E8\u5206\u529F\u80FD\u6682\u65F6\u65E0\u6CD5\u4F7F\u7528\u3002" },
        "card.warn.open": { en: "Details", "zh-TW": "\u67E5\u770B", "zh-CN": "\u67E5\u770B" },
        "card.test.title": { en: "Speed test", "zh-TW": "\u901F\u5EA6\u6E2C\u8A66", "zh-CN": "\u901F\u5EA6\u6D4B\u8BD5" },
        "card.test.raw": { en: "Read test", "zh-TW": "\u8B80\u53D6\u6E2C\u8A66", "zh-CN": "\u8BFB\u53D6\u6D4B\u8BD5" },
        "card.test.file": { en: "Read/write test", "zh-TW": "\u8B80\u5BEB\u6E2C\u8A66", "zh-CN": "\u8BFB\u5199\u6D4B\u8BD5" },
        "card.test.note": { en: "The read test reads the card directly; the read/write test writes a temporary file and reads it back to check it. A test stops playback and takes a moment.", "zh-TW": "\u8B80\u53D6\u6E2C\u8A66\u76F4\u63A5\u8B80\u5361\uFF1B\u8B80\u5BEB\u6E2C\u8A66\u6703\u5BEB\u4E00\u500B\u66AB\u5B58\u6A94\u518D\u8B80\u56DE\u4F86\u6838\u5C0D\u3002\u6E2C\u8A66\u6703\u505C\u6B62\u64AD\u653E\uFF0C\u9700\u8981\u4E00\u9EDE\u6642\u9593\u3002", "zh-CN": "\u8BFB\u53D6\u6D4B\u8BD5\u76F4\u63A5\u8BFB\u5361\uFF1B\u8BFB\u5199\u6D4B\u8BD5\u4F1A\u5199\u4E00\u4E2A\u4E34\u65F6\u6587\u4EF6\u518D\u8BFB\u56DE\u6765\u6838\u5BF9\u3002\u6D4B\u8BD5\u4F1A\u505C\u6B62\u64AD\u653E\uFF0C\u9700\u8981\u4E00\u70B9\u65F6\u95F4\u3002" },
        "card.test.running.raw": { en: "Read test running\u2026 {pct}%", "zh-TW": "\u8B80\u53D6\u6E2C\u8A66\u9032\u884C\u4E2D\u2026 {pct}%", "zh-CN": "\u8BFB\u53D6\u6D4B\u8BD5\u8FDB\u884C\u4E2D\u2026 {pct}%" },
        "card.test.running.file": { en: "Read/write test running\u2026 {pct}%", "zh-TW": "\u8B80\u5BEB\u6E2C\u8A66\u9032\u884C\u4E2D\u2026 {pct}%", "zh-CN": "\u8BFB\u5199\u6D4B\u8BD5\u8FDB\u884C\u4E2D\u2026 {pct}%" },
        "card.test.cancelled": { en: "The test was cancelled.", "zh-TW": "\u6E2C\u8A66\u5DF2\u53D6\u6D88\u3002", "zh-CN": "\u6D4B\u8BD5\u5DF2\u53D6\u6D88\u3002" },
        "card.test.refused": { en: "The test could not start: {why}.", "zh-TW": "\u7121\u6CD5\u958B\u59CB\u6E2C\u8A66\uFF1A{why}\u3002", "zh-CN": "\u65E0\u6CD5\u5F00\u59CB\u6D4B\u8BD5\uFF1A{why}\u3002" },
        "card.test.noreply": { en: "The box did not answer. Please try again.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\uFF0C\u8ACB\u518D\u8A66\u4E00\u6B21\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u56DE\u5E94\uFF0C\u8BF7\u518D\u8BD5\u4E00\u6B21\u3002" },
        "card.test.fail": { en: "The {kind} failed: {why}.", "zh-TW": "{kind}\u5931\u6557\uFF1A{why}\u3002", "zh-CN": "{kind}\u5931\u8D25\uFF1A{why}\u3002" },
        "card.reason.no_card": { en: "there is no card", "zh-TW": "\u6C92\u6709\u8A18\u61B6\u5361", "zh-CN": "\u6CA1\u6709\u5B58\u50A8\u5361" },
        "card.reason.card_unreadable": { en: "the card cannot be read", "zh-TW": "\u8A18\u61B6\u5361\u8B80\u4E0D\u5230", "zh-CN": "\u5B58\u50A8\u5361\u8BFB\u4E0D\u51FA\u6765" },
        "card.reason.already_running": { en: "a test is already running", "zh-TW": "\u5DF2\u7D93\u6709\u6E2C\u8A66\u5728\u9032\u884C", "zh-CN": "\u5DF2\u7ECF\u6709\u6D4B\u8BD5\u5728\u8FDB\u884C" },
        "card.reason.formatting": { en: "the card is being formatted", "zh-TW": "\u6B63\u5728\u683C\u5F0F\u5316", "zh-CN": "\u6B63\u5728\u683C\u5F0F\u5316" },
        "card.reason.small": { en: "the card is too small for the test file", "zh-TW": "\u5361\u592A\u5C0F\uFF0C\u653E\u4E0D\u4E0B\u6E2C\u8A66\u6A94", "zh-CN": "\u5361\u592A\u5C0F\uFF0C\u653E\u4E0D\u4E0B\u6D4B\u8BD5\u6587\u4EF6" },
        "card.reason.nospace": { en: "not enough free space for the test file", "zh-TW": "\u5269\u9918\u7A7A\u9593\u4E0D\u5920\u653E\u6E2C\u8A66\u6A94", "zh-CN": "\u5269\u4F59\u7A7A\u95F4\u4E0D\u591F\u653E\u6D4B\u8BD5\u6587\u4EF6" },
        "card.reason.io": { en: "the card gave a read or write error", "zh-TW": "\u8B80\u5BEB\u5361\u6642\u767C\u751F\u932F\u8AA4", "zh-CN": "\u8BFB\u5199\u5361\u65F6\u51FA\u9519" },
        "card.reason.nomem": { en: "the box is short of memory", "zh-TW": "\u88DD\u7F6E\u8A18\u61B6\u9AD4\u4E0D\u8DB3", "zh-CN": "\u8BBE\u5907\u5185\u5B58\u4E0D\u8DB3" },
        "card.res.read3": { en: "Read (average / slowest / best)", "zh-TW": "\u8B80\u53D6\uFF08\u5E73\u5747 / \u6700\u6162 / \u6700\u4F73\uFF09", "zh-CN": "\u8BFB\u53D6\uFF08\u5E73\u5747 / \u6700\u6162 / \u6700\u4F73\uFF09" },
        "card.res.worst": { en: "Slowest single read", "zh-TW": "\u6700\u6162\u7684\u4E00\u6B21\u8B80\u53D6", "zh-CN": "\u6700\u6162\u7684\u4E00\u6B21\u8BFB\u53D6" },
        "card.res.write": { en: "Write", "zh-TW": "\u5BEB\u5165", "zh-CN": "\u5199\u5165" },
        "card.res.read": { en: "Read back", "zh-TW": "\u8B80\u56DE", "zh-CN": "\u8BFB\u56DE" },
        "card.res.verify": { en: "Check of the data", "zh-TW": "\u8CC7\u6599\u6838\u5C0D", "zh-CN": "\u6570\u636E\u6838\u5BF9" },
        "card.res.pass": { en: "passed", "zh-TW": "\u901A\u904E", "zh-CN": "\u901A\u8FC7" },
        "card.res.nopass": { en: "did NOT match", "zh-TW": "\u4E0D\u4E00\u81F4", "zh-CN": "\u4E0D\u4E00\u81F4" },
        "card.ladder.title": { en: "Which audio formats the card can keep up with", "zh-TW": "\u5404\u7A2E\u97F3\u6A94\u683C\u5F0F\u5920\u4E0D\u5920\u7528", "zh-CN": "\u5404\u79CD\u97F3\u9891\u683C\u5F0F\u591F\u4E0D\u591F\u7528" },
        "card.ladder.basis": { en: "Graded by the {what}.", "zh-TW": "\u5206\u7D1A\u4F9D\u64DA\uFF1A{what}\u3002", "zh-CN": "\u5206\u7EA7\u4F9D\u636E\uFF1A{what}\u3002" },
        "card.ladder.need": { en: "needs {v} MB/s", "zh-TW": "\u9700\u8981 {v} MB/s", "zh-CN": "\u9700\u8981 {v} MB/s" },
        "card.ladder.markRaw": { en: "This card reads at {v} MB/s", "zh-TW": "\u9019\u5F35\u5361\u7684\u8B80\u53D6\u901F\u5EA6 {v} MB/s", "zh-CN": "\u8FD9\u5F20\u5361\u7684\u8BFB\u53D6\u901F\u5EA6 {v} MB/s" },
        "card.ladder.markFile": { en: "This card reads/writes at {v} MB/s", "zh-TW": "\u9019\u5F35\u5361\u7684\u8B80\u5BEB\u901F\u5EA6 {v} MB/s", "zh-CN": "\u8FD9\u5F20\u5361\u7684\u8BFB\u5199\u901F\u5EA6 {v} MB/s" },
        "card.ladder.hint": { en: "\u201CMarginal\u201D usually plays but may stutter now and then; \u201Ctoo slow\u201D will stutter often.", "zh-TW": "\u300C\u52C9\u5F37\u300D\u901A\u5E38\u80FD\u64AD\uFF0C\u4F46\u5076\u723E\u53EF\u80FD\u65B7\u97F3\uFF1B\u300C\u592A\u6162\u300D\u6703\u5E38\u5E38\u65B7\u97F3\u3002", "zh-CN": "\u201C\u52C9\u5F3A\u201D\u901A\u5E38\u80FD\u64AD\uFF0C\u4F46\u5076\u5C14\u53EF\u80FD\u65AD\u97F3\uFF1B\u201C\u592A\u6162\u201D\u4F1A\u7ECF\u5E38\u65AD\u97F3\u3002" },
        "card.grade.ok": { en: "fine", "zh-TW": "\u8DB3\u5920", "zh-CN": "\u8DB3\u591F" },
        "card.grade.marginal": { en: "marginal", "zh-TW": "\u52C9\u5F37", "zh-CN": "\u52C9\u5F3A" },
        "card.grade.slow": { en: "too slow", "zh-TW": "\u592A\u6162", "zh-CN": "\u592A\u6162" },
        "card.grade.unknown": { en: "not tested", "zh-TW": "\u672A\u6E2C", "zh-CN": "\u672A\u6D4B" },
        "card.fmt.title": { en: "The card cannot be read", "zh-TW": "\u9019\u5F35\u5361\u8B80\u4E0D\u5230", "zh-CN": "\u8FD9\u5F20\u5361\u8BFB\u4E0D\u51FA\u6765" },
        "card.fmt.explain": { en: "The box cannot read this card. If you format it, everything on it is erased, including the Wi-Fi settings file. Before that, consider trying the card in a computer: it may be readable there, and you can save its files.", "zh-TW": "\u88DD\u7F6E\u8B80\u4E0D\u5230\u9019\u5F35\u5361\u3002\u683C\u5F0F\u5316\u6703\u6E05\u9664\u5361\u4E0A\u7684\u6240\u6709\u5167\u5BB9\uFF0C\u5305\u62EC Wi-Fi \u8A2D\u5B9A\u6A94\u3002\u5EFA\u8B70\u5148\u628A\u5361\u62FF\u5230\u96FB\u8166\u4E0A\u8A66\u8A66\uFF0C\u4E5F\u8A31\u5728\u96FB\u8166\u4E0A\u8B80\u5F97\u5230\uFF0C\u53EF\u4EE5\u5148\u628A\u6A94\u6848\u6551\u51FA\u4F86\u3002", "zh-CN": "\u8BBE\u5907\u8BFB\u4E0D\u51FA\u8FD9\u5F20\u5361\u3002\u683C\u5F0F\u5316\u4F1A\u6E05\u9664\u5361\u4E0A\u7684\u6240\u6709\u5185\u5BB9\uFF0C\u5305\u62EC Wi-Fi \u8BBE\u7F6E\u6587\u4EF6\u3002\u5EFA\u8BAE\u5148\u628A\u5361\u62FF\u5230\u7535\u8111\u4E0A\u8BD5\u8BD5\uFF0C\u4E5F\u8BB8\u5728\u7535\u8111\u4E0A\u8BFB\u5F97\u51FA\u6765\uFF0C\u53EF\u4EE5\u5148\u628A\u6587\u4EF6\u6551\u51FA\u6765\u3002" },
        "card.fmt.start": { en: "Format this card", "zh-TW": "\u683C\u5F0F\u5316\u9019\u5F35\u5361", "zh-CN": "\u683C\u5F0F\u5316\u8FD9\u5F20\u5361" },
        "card.fmt.plan": { en: "It will be formatted as {fs}, named \u201C{label}\u201D, size {size}.", "zh-TW": "\u5C07\u683C\u5F0F\u5316\u70BA {fs}\uFF0C\u540D\u7A31\u300C{label}\u300D\uFF0C\u5BB9\u91CF {size}\u3002", "zh-CN": "\u5C06\u683C\u5F0F\u5316\u4E3A {fs}\uFF0C\u540D\u79F0\u201C{label}\u201D\uFF0C\u5BB9\u91CF {size}\u3002" },
        "card.fmt.warn": { en: "Everything on the card will be erased, including the Wi-Fi settings file. This cannot be undone.", "zh-TW": "\u5361\u4E0A\u7684\u6240\u6709\u5167\u5BB9\u90FD\u6703\u88AB\u6E05\u9664\uFF0C\u5305\u62EC Wi-Fi \u8A2D\u5B9A\u6A94\uFF0C\u800C\u4E14\u7121\u6CD5\u5FA9\u539F\u3002", "zh-CN": "\u5361\u4E0A\u7684\u6240\u6709\u5185\u5BB9\u90FD\u4F1A\u88AB\u6E05\u9664\uFF0C\u5305\u62EC Wi-Fi \u8BBE\u7F6E\u6587\u4EF6\uFF0C\u800C\u4E14\u65E0\u6CD5\u6062\u590D\u3002" },
        "card.fmt.s1.title": { en: "Step 1 of 2: confirm", "zh-TW": "\u7B2C 1 \u6B65\uFF08\u5171 2 \u6B65\uFF09\uFF1A\u78BA\u8A8D", "zh-CN": "\u7B2C 1 \u6B65\uFF08\u5171 2 \u6B65\uFF09\uFF1A\u786E\u8BA4" },
        "card.fmt.s1.how": { en: "Type these four digits into the box below, then press Confirm.", "zh-TW": "\u8ACB\u628A\u4E0B\u9762\u9019\u56DB\u500B\u6578\u5B57\u8F38\u5165\u5230\u8F38\u5165\u6846\uFF0C\u518D\u6309\u300C\u78BA\u8A8D\u300D\u3002", "zh-CN": "\u8BF7\u628A\u4E0B\u9762\u8FD9\u56DB\u4E2A\u6570\u5B57\u8F93\u5165\u5230\u8F93\u5165\u6846\uFF0C\u518D\u6309\u201C\u786E\u8BA4\u201D\u3002" },
        "card.fmt.s2.title": { en: "Final confirmation", "zh-TW": "\u6700\u5F8C\u78BA\u8A8D", "zh-CN": "\u6700\u540E\u786E\u8BA4" },
        "card.fmt.s2.how": { en: "Type the new four digits below. When you press the button, formatting starts at once and cannot be stopped.", "zh-TW": "\u8ACB\u8F38\u5165\u4E0B\u9762\u7684\u65B0\u56DB\u4F4D\u6578\u5B57\u3002\u6309\u4E0B\u6309\u9215\u5F8C\u6703\u7ACB\u523B\u958B\u59CB\u683C\u5F0F\u5316\uFF0C\u7121\u6CD5\u4E2D\u9014\u505C\u6B62\u3002", "zh-CN": "\u8BF7\u8F93\u5165\u4E0B\u9762\u7684\u65B0\u56DB\u4F4D\u6570\u5B57\u3002\u6309\u4E0B\u6309\u94AE\u540E\u4F1A\u7ACB\u523B\u5F00\u59CB\u683C\u5F0F\u5316\uFF0C\u65E0\u6CD5\u4E2D\u9014\u505C\u6B62\u3002" },
        "card.fmt.s2.go": { en: "Confirm and format", "zh-TW": "\u78BA\u8A8D\u4E26\u683C\u5F0F\u5316", "zh-CN": "\u786E\u8BA4\u5E76\u683C\u5F0F\u5316" },
        "card.fmt.code.aria": { en: "Confirmation code", "zh-TW": "\u78BA\u8A8D\u78BC", "zh-CN": "\u786E\u8BA4\u7801" },
        "card.fmt.input.aria": { en: "Type the confirmation code", "zh-TW": "\u8F38\u5165\u78BA\u8A8D\u78BC", "zh-CN": "\u8F93\u5165\u786E\u8BA4\u7801" },
        "card.fmt.input.ph": { en: "4 digits", "zh-TW": "\u56DB\u4F4D\u6578\u5B57", "zh-CN": "\u56DB\u4F4D\u6570\u5B57" },
        "card.fmt.confirm": { en: "Confirm", "zh-TW": "\u78BA\u8A8D", "zh-CN": "\u786E\u8BA4" },
        "card.fmt.countdown": { en: "{s} s left", "zh-TW": "\u9084\u5269 {s} \u79D2", "zh-CN": "\u8FD8\u5269 {s} \u79D2" },
        "card.fmt.expired": { en: "More than 30 seconds passed, so the request was cancelled. Please start again.", "zh-TW": "\u8D85\u904E 30 \u79D2\uFF0C\u5DF2\u53D6\u6D88\u3002\u8ACB\u5F9E\u982D\u958B\u59CB\u3002", "zh-CN": "\u8D85\u8FC7 30 \u79D2\uFF0C\u5DF2\u53D6\u6D88\u3002\u8BF7\u4ECE\u5934\u5F00\u59CB\u3002" },
        "card.fmt.cancelled": { en: "Formatting was cancelled.", "zh-TW": "\u5DF2\u53D6\u6D88\u683C\u5F0F\u5316\u3002", "zh-CN": "\u5DF2\u53D6\u6D88\u683C\u5F0F\u5316\u3002" },
        "card.fmt.refused": { en: "Formatting was cancelled: {why}. Please start again.", "zh-TW": "\u683C\u5F0F\u5316\u5DF2\u53D6\u6D88\uFF1A{why}\u3002\u8ACB\u5F9E\u982D\u958B\u59CB\u3002", "zh-CN": "\u683C\u5F0F\u5316\u5DF2\u53D6\u6D88\uFF1A{why}\u3002\u8BF7\u4ECE\u5934\u5F00\u59CB\u3002" },
        "card.fmt.noreply": { en: "The box did not answer. Please try again.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\uFF0C\u8ACB\u518D\u8A66\u4E00\u6B21\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u56DE\u5E94\uFF0C\u8BF7\u518D\u8BD5\u4E00\u6B21\u3002" },
        "card.fmt.why.wrong": { en: "the code was not the one shown", "zh-TW": "\u8F38\u5165\u7684\u78BA\u8A8D\u78BC\u4E0D\u5C0D", "zh-CN": "\u8F93\u5165\u7684\u786E\u8BA4\u7801\u4E0D\u5BF9" },
        "card.fmt.why.late": { en: "it took longer than 30 seconds", "zh-TW": "\u8D85\u904E\u4E86\u6642\u9593", "zh-CN": "\u8D85\u8FC7\u4E86\u65F6\u95F4" },
        "card.fmt.why.readable": { en: "the card can be read, and only a card that cannot be read may be formatted", "zh-TW": "\u9019\u5F35\u5361\u8B80\u5F97\u5230\uFF0C\u53EA\u6709\u8B80\u4E0D\u5230\u7684\u5361\u624D\u80FD\u683C\u5F0F\u5316", "zh-CN": "\u8FD9\u5F20\u5361\u8BFB\u5F97\u51FA\u6765\uFF0C\u53EA\u6709\u8BFB\u4E0D\u51FA\u6765\u7684\u5361\u624D\u80FD\u683C\u5F0F\u5316" },
        "card.fmt.running": { en: "Formatting\u2026 do not remove the card or switch the box off.", "zh-TW": "\u6B63\u5728\u683C\u5F0F\u5316\u2026 \u8ACB\u4E0D\u8981\u62D4\u5361\uFF0C\u4E5F\u4E0D\u8981\u65B7\u96FB\u3002", "zh-CN": "\u6B63\u5728\u683C\u5F0F\u5316\u2026 \u8BF7\u4E0D\u8981\u62D4\u5361\uFF0C\u4E5F\u4E0D\u8981\u65AD\u7535\u3002" },
        "card.fmt.done": { en: "Formatting is done and the card works again. The Wi-Fi settings file is gone, so set up Wi-Fi again.", "zh-TW": "\u683C\u5F0F\u5316\u5B8C\u6210\uFF0C\u8A18\u61B6\u5361\u53C8\u53EF\u4EE5\u7528\u4E86\u3002Wi-Fi \u8A2D\u5B9A\u6A94\u5DF2\u7D93\u6E05\u9664\uFF0C\u9700\u8981\u91CD\u65B0\u8A2D\u5B9A Wi-Fi\u3002", "zh-CN": "\u683C\u5F0F\u5316\u5B8C\u6210\uFF0C\u5B58\u50A8\u5361\u53C8\u53EF\u4EE5\u7528\u4E86\u3002Wi-Fi \u8BBE\u7F6E\u6587\u4EF6\u5DF2\u7ECF\u6E05\u9664\uFF0C\u9700\u8981\u91CD\u65B0\u8BBE\u7F6E Wi-Fi\u3002" },
        "card.fmt.failed": { en: "Formatting failed: {why}. The card still cannot be read.", "zh-TW": "\u683C\u5F0F\u5316\u5931\u6557\uFF1A{why}\u3002\u9019\u5F35\u5361\u4ECD\u7136\u8B80\u4E0D\u5230\u3002", "zh-CN": "\u683C\u5F0F\u5316\u5931\u8D25\uFF1A{why}\u3002\u8FD9\u5F20\u5361\u4ECD\u7136\u8BFB\u4E0D\u51FA\u6765\u3002" },
        "card.fmt.failed0": { en: "Formatting failed. The card still cannot be read.", "zh-TW": "\u683C\u5F0F\u5316\u5931\u6557\u3002\u9019\u5F35\u5361\u4ECD\u7136\u8B80\u4E0D\u5230\u3002", "zh-CN": "\u683C\u5F0F\u5316\u5931\u8D25\u3002\u8FD9\u5F20\u5361\u4ECD\u7136\u8BFB\u4E0D\u51FA\u6765\u3002" },
        "card.fmt.dismiss": { en: "OK", "zh-TW": "\u77E5\u9053\u4E86", "zh-CN": "\u77E5\u9053\u4E86" },
        "wifi.title": { en: "Wi-Fi", "zh-TW": "Wi-Fi", "zh-CN": "Wi-Fi" },
        "wifi.reading": { en: "Reading Wi-Fi status\u2026", "zh-TW": "\u8B80\u53D6 Wi-Fi \u72C0\u614B\u4E2D\u2026", "zh-CN": "\u6B63\u5728\u8BFB\u53D6 Wi-Fi \u72B6\u6001\u2026" },
        "wifi.state.connected": { en: "Connected", "zh-TW": "\u5DF2\u9023\u7DDA", "zh-CN": "\u5DF2\u8FDE\u63A5" },
        "wifi.state.connecting": { en: "Connecting\u2026", "zh-TW": "\u9023\u7DDA\u4E2D\u2026", "zh-CN": "\u6B63\u5728\u8FDE\u63A5\u2026" },
        "wifi.state.wrong_password": { en: "Wrong password", "zh-TW": "\u5BC6\u78BC\u4E0D\u5C0D", "zh-CN": "\u5BC6\u7801\u4E0D\u5BF9" },
        "wifi.state.not_found": { en: "Network not found", "zh-TW": "\u627E\u4E0D\u5230\u9019\u500B\u7DB2\u8DEF", "zh-CN": "\u627E\u4E0D\u5230\u8FD9\u4E2A\u7F51\u7EDC" },
        "wifi.state.off": { en: "Wi-Fi is off", "zh-TW": "Wi-Fi \u5DF2\u95DC\u9589", "zh-CN": "Wi-Fi \u5DF2\u5173\u95ED" },
        "wifi.state.no_credentials": { en: "No Wi-Fi set up yet", "zh-TW": "\u9084\u6C92\u6709\u8A2D\u5B9A Wi-Fi", "zh-CN": "\u8FD8\u6CA1\u6709\u8BBE\u7F6E Wi-Fi" },
        "wifi.k.net": { en: "Network", "zh-TW": "\u7DB2\u8DEF", "zh-CN": "\u7F51\u7EDC" },
        "wifi.k.signal": { en: "Signal", "zh-TW": "\u8A0A\u865F", "zh-CN": "\u4FE1\u53F7" },
        "wifi.k.ip": { en: "IP address", "zh-TW": "IP \u4F4D\u5740", "zh-CN": "IP \u5730\u5740" },
        "wifi.k.src": { en: "Settings from", "zh-TW": "\u8A2D\u5B9A\u4F86\u6E90", "zh-CN": "\u8BBE\u7F6E\u6765\u6E90" },
        "wifi.sig.4": { en: "very strong", "zh-TW": "\u5F88\u5F37", "zh-CN": "\u5F88\u5F3A" },
        "wifi.sig.3": { en: "strong", "zh-TW": "\u5F37", "zh-CN": "\u5F3A" },
        "wifi.sig.2": { en: "fair", "zh-TW": "\u666E\u901A", "zh-CN": "\u4E00\u822C" },
        "wifi.sig.1": { en: "weak", "zh-TW": "\u5F31", "zh-CN": "\u5F31" },
        "wifi.sig.fmt": { en: "{q} ({rssi} dBm)", "zh-TW": "{q}\uFF08{rssi} dBm\uFF09", "zh-CN": "{q}\uFF08{rssi} dBm\uFF09" },
        "wifi.src.nvs": { en: "Set from this page", "zh-TW": "\u5F9E\u9019\u500B\u9801\u9762\u8A2D\u5B9A", "zh-CN": "\u901A\u8FC7\u8FD9\u4E2A\u9875\u9762\u8BBE\u7F6E" },
        "wifi.src.card": { en: "From the file on the memory card", "zh-TW": "\u4F86\u81EA\u8A18\u61B6\u5361\u4E0A\u7684\u6A94\u6848", "zh-CN": "\u6765\u81EA\u5B58\u50A8\u5361\u4E0A\u7684\u6587\u4EF6" },
        "wifi.scan.btn": { en: "Scan", "zh-TW": "\u6383\u63CF", "zh-CN": "\u626B\u63CF" },
        "wifi.scan.busyBtn": { en: "Scanning\u2026", "zh-TW": "\u6383\u63CF\u4E2D\u2026", "zh-CN": "\u626B\u63CF\u4E2D\u2026" },
        "wifi.scan.running": { en: "Scanning, this takes a few seconds\u2026", "zh-TW": "\u6383\u63CF\u4E2D\uFF0C\u9700\u8981\u5E7E\u79D2\u9418\u2026", "zh-CN": "\u6B63\u5728\u626B\u63CF\uFF0C\u9700\u8981\u51E0\u79D2\u949F\u2026" },
        "wifi.scan.found": { en: "{n} networks found. Tap one to fill in the name.", "zh-TW": "\u627E\u5230 {n} \u500B\u7DB2\u8DEF\uFF0C\u9EDE\u4E00\u4E0B\u53EF\u586B\u5165\u540D\u7A31\u3002", "zh-CN": "\u627E\u5230 {n} \u4E2A\u7F51\u7EDC\uFF0C\u70B9\u4E00\u4E0B\u5373\u53EF\u586B\u5165\u540D\u79F0\u3002" },
        "wifi.scan.none": { en: "No networks found nearby.", "zh-TW": "\u9644\u8FD1\u6C92\u6709\u627E\u5230\u7DB2\u8DEF\u3002", "zh-CN": "\u9644\u8FD1\u6CA1\u6709\u627E\u5230\u7F51\u7EDC\u3002" },
        "wifi.scan.isBusy": { en: "The box is busy scanning. Try again in a moment.", "zh-TW": "\u88DD\u7F6E\u6B63\u5728\u5FD9\u8457\u6383\u63CF\uFF0C\u8ACB\u7A0D\u5F8C\u518D\u8A66\u3002", "zh-CN": "\u8BBE\u5907\u6B63\u5FD9\u4E8E\u626B\u63CF\uFF0C\u8BF7\u7A0D\u540E\u518D\u8BD5\u3002" },
        "wifi.scan.failed": { en: "The scan failed. Please try again.", "zh-TW": "\u6383\u63CF\u5931\u6557\uFF0C\u8ACB\u518D\u8A66\u4E00\u6B21\u3002", "zh-CN": "\u626B\u63CF\u5931\u8D25\uFF0C\u8BF7\u518D\u8BD5\u4E00\u6B21\u3002" },
        "wifi.scan.noreply": { en: "The box did not answer the scan.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\u6383\u63CF\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u54CD\u5E94\u626B\u63CF\u3002" },
        "wifi.scan.open": { en: "open", "zh-TW": "\u958B\u653E", "zh-CN": "\u5F00\u653E" },
        "wifi.scan.secured": { en: "Password protected", "zh-TW": "\u6709\u5BC6\u78BC\u4FDD\u8B77", "zh-CN": "\u6709\u5BC6\u7801\u4FDD\u62A4" },
        "wifi.name.label": { en: "Network name (SSID)", "zh-TW": "\u7DB2\u8DEF\u540D\u7A31\uFF08SSID\uFF09", "zh-CN": "\u7F51\u7EDC\u540D\u79F0\uFF08SSID\uFF09" },
        "wifi.pw.label": { en: "Password", "zh-TW": "\u5BC6\u78BC", "zh-CN": "\u5BC6\u7801" },
        "wifi.pw.ph": { en: "Leave empty for an open network", "zh-TW": "\u958B\u653E\u7DB2\u8DEF\u8ACB\u7559\u7A7A", "zh-CN": "\u5F00\u653E\u7F51\u7EDC\u8BF7\u7559\u7A7A" },
        "wifi.pw.show": { en: "Show", "zh-TW": "\u986F\u793A", "zh-CN": "\u663E\u793A" },
        "wifi.pw.hide": { en: "Hide", "zh-TW": "\u96B1\u85CF", "zh-CN": "\u9690\u85CF" },
        "wifi.insecure": { en: "The password is sent to the box unencrypted over Bluetooth; this link is not encrypted.", "zh-TW": "\u5BC6\u78BC\u6703\u4EE5\u660E\u6587\u900F\u904E\u85CD\u7259\u50B3\u7D66\u88DD\u7F6E\uFF0C\u9019\u6BB5\u9023\u7DDA\u6C92\u6709\u52A0\u5BC6\u3002", "zh-CN": "\u5BC6\u7801\u4F1A\u4EE5\u660E\u6587\u901A\u8FC7\u84DD\u7259\u53D1\u7ED9\u8BBE\u5907\uFF0C\u8FD9\u6BB5\u8FDE\u63A5\u6CA1\u6709\u52A0\u5BC6\u3002" },
        "wifi.limits": { en: 'The name can be up to 32 bytes and the password 8 to 63 bytes (a Chinese character takes 3 bytes). A name containing "|" cannot be set from this page.', "zh-TW": "\u540D\u7A31\u6700\u591A 32 \u4F4D\u5143\u7D44\u3001\u5BC6\u78BC 8 \u5230 63 \u4F4D\u5143\u7D44\uFF08\u4E00\u500B\u4E2D\u6587\u5B57\u4F54 3 \u4F4D\u5143\u7D44\uFF09\u3002\u540D\u7A31\u88E1\u6709\u300C|\u300D\u7684\u7DB2\u8DEF\u7121\u6CD5\u5F9E\u9019\u500B\u9801\u9762\u8A2D\u5B9A\u3002", "zh-CN": "\u540D\u79F0\u6700\u591A 32 \u5B57\u8282\uFF0C\u5BC6\u7801 8 \u5230 63 \u5B57\u8282\uFF08\u4E00\u4E2A\u6C49\u5B57\u5360 3 \u5B57\u8282\uFF09\u3002\u540D\u79F0\u91CC\u5E26\u6709\u201C|\u201D\u7684\u7F51\u7EDC\u65E0\u6CD5\u901A\u8FC7\u8FD9\u4E2A\u9875\u9762\u8BBE\u7F6E\u3002" },
        "wifi.join": { en: "Join", "zh-TW": "\u9023\u7DDA", "zh-CN": "\u8FDE\u63A5" },
        "wifi.forget": { en: "Clear settings", "zh-TW": "\u6E05\u9664\u8A2D\u5B9A", "zh-CN": "\u6E05\u9664\u8BBE\u7F6E" },
        "wifi.forget.confirm": { en: "Clear the Wi-Fi settings stored on the box? If there is a Wi-Fi file on the memory card, the box will use that instead; otherwise it will not join any network.", "zh-TW": "\u8981\u6E05\u9664\u88DD\u7F6E\u4E0A\u5132\u5B58\u7684 Wi-Fi \u8A2D\u5B9A\u55CE\uFF1F\u5982\u679C\u8A18\u61B6\u5361\u4E0A\u6709 Wi-Fi \u8A2D\u5B9A\u6A94\uFF0C\u88DD\u7F6E\u6703\u6539\u7528\u5B83\uFF1B\u6C92\u6709\u7684\u8A71\u5C31\u4E0D\u6703\u9023\u4E0A\u4EFB\u4F55\u7DB2\u8DEF\u3002", "zh-CN": "\u8981\u6E05\u9664\u8BBE\u5907\u4E0A\u4FDD\u5B58\u7684 Wi-Fi \u8BBE\u7F6E\u5417\uFF1F\u5982\u679C\u5B58\u50A8\u5361\u4E0A\u6709 Wi-Fi \u914D\u7F6E\u6587\u4EF6\uFF0C\u8BBE\u5907\u4F1A\u6539\u7528\u5B83\uFF1B\u6CA1\u6709\u7684\u8BDD\u5C31\u4E0D\u4F1A\u8FDE\u63A5\u4EFB\u4F55\u7F51\u7EDC\u3002" },
        "wifi.forget.done": { en: "The stored settings were cleared.", "zh-TW": "\u5DF2\u6E05\u9664\u5132\u5B58\u7684\u8A2D\u5B9A\u3002", "zh-CN": "\u5DF2\u6E05\u9664\u4FDD\u5B58\u7684\u8BBE\u7F6E\u3002" },
        "wifi.forget.fail": { en: "The box did not confirm clearing the settings.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u78BA\u8A8D\u6E05\u9664\u8A2D\u5B9A\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u786E\u8BA4\u6E05\u9664\u8BBE\u7F6E\u3002" },
        "wifi.err.ssidEmpty": { en: "Enter the network name.", "zh-TW": "\u8ACB\u8F38\u5165\u7DB2\u8DEF\u540D\u7A31\u3002", "zh-CN": "\u8BF7\u8F93\u5165\u7F51\u7EDC\u540D\u79F0\u3002" },
        "wifi.err.ssidLong": { en: "The network name is {n} bytes; the limit is 32 (a Chinese character takes 3 bytes).", "zh-TW": "\u7DB2\u8DEF\u540D\u7A31\u6709 {n} \u4F4D\u5143\u7D44\uFF0C\u4E0A\u9650\u662F 32 \u4F4D\u5143\u7D44\uFF08\u4E00\u500B\u4E2D\u6587\u5B57\u4F54 3 \u4F4D\u5143\u7D44\uFF09\u3002", "zh-CN": "\u7F51\u7EDC\u540D\u79F0\u6709 {n} \u5B57\u8282\uFF0C\u4E0A\u9650\u662F 32 \u5B57\u8282\uFF08\u4E00\u4E2A\u6C49\u5B57\u5360 3 \u5B57\u8282\uFF09\u3002" },
        "wifi.err.bar": { en: 'The name contains "|", which this page cannot send. Set this network another way, for example with the Wi-Fi file on the memory card.', "zh-TW": "\u540D\u7A31\u88E1\u6709\u300C|\u300D\uFF0C\u9019\u500B\u9801\u9762\u7121\u6CD5\u50B3\u9001\u3002\u8ACB\u6539\u7528\u5176\u4ED6\u65B9\u5F0F\u8A2D\u5B9A\uFF0C\u4F8B\u5982\u8A18\u61B6\u5361\u4E0A\u7684 Wi-Fi \u8A2D\u5B9A\u6A94\u3002", "zh-CN": "\u540D\u79F0\u91CC\u5E26\u6709\u201C|\u201D\uFF0C\u8FD9\u4E2A\u9875\u9762\u65E0\u6CD5\u53D1\u9001\u3002\u8BF7\u6539\u7528\u5176\u4ED6\u65B9\u5F0F\u8BBE\u7F6E\uFF0C\u4F8B\u5982\u5B58\u50A8\u5361\u4E0A\u7684 Wi-Fi \u914D\u7F6E\u6587\u4EF6\u3002" },
        "wifi.err.pwLen": { en: "The password is {n} bytes; it must be 8 to 63 bytes, or empty for an open network.", "zh-TW": "\u5BC6\u78BC\u6709 {n} \u4F4D\u5143\u7D44\uFF0C\u5FC5\u9808\u662F 8 \u5230 63 \u4F4D\u5143\u7D44\uFF1B\u958B\u653E\u7DB2\u8DEF\u8ACB\u7559\u7A7A\u3002", "zh-CN": "\u5BC6\u7801\u6709 {n} \u5B57\u8282\uFF0C\u5FC5\u987B\u662F 8 \u5230 63 \u5B57\u8282\uFF1B\u5F00\u653E\u7F51\u7EDC\u8BF7\u7559\u7A7A\u3002" },
        "wifi.err.ctrl": { en: "The name or password contains a control character (for example a line break), which cannot be used.", "zh-TW": "\u540D\u7A31\u6216\u5BC6\u78BC\u88E1\u6709\u63A7\u5236\u5B57\u5143\uFF08\u4F8B\u5982\u63DB\u884C\uFF09\uFF0C\u7121\u6CD5\u4F7F\u7528\u3002", "zh-CN": "\u540D\u79F0\u6216\u5BC6\u7801\u91CC\u6709\u63A7\u5236\u5B57\u7B26\uFF08\u4F8B\u5982\u6362\u884C\uFF09\uFF0C\u65E0\u6CD5\u4F7F\u7528\u3002" },
        "wifi.refused": { en: "The box refused these settings: {why}", "zh-TW": "\u88DD\u7F6E\u62D2\u7D55\u4E86\u9019\u7D44\u8A2D\u5B9A\uFF1A{why}", "zh-CN": "\u8BBE\u5907\u62D2\u7EDD\u4E86\u8FD9\u7EC4\u8BBE\u7F6E\uFF1A{why}" },
        "wifi.why.usage": { en: "wrong command format", "zh-TW": "\u6307\u4EE4\u683C\u5F0F\u4E0D\u5C0D", "zh-CN": "\u547D\u4EE4\u683C\u5F0F\u4E0D\u5BF9" },
        "wifi.why.ssid": { en: "the name must be 1 to 32 bytes", "zh-TW": "\u540D\u7A31\u5FC5\u9808\u662F 1 \u5230 32 \u4F4D\u5143\u7D44", "zh-CN": "\u540D\u79F0\u5FC5\u987B\u662F 1 \u5230 32 \u5B57\u8282" },
        "wifi.why.pw": { en: "the password must be empty or 8 to 63 bytes", "zh-TW": "\u5BC6\u78BC\u5FC5\u9808\u7559\u7A7A\uFF0C\u6216\u662F 8 \u5230 63 \u4F4D\u5143\u7D44", "zh-CN": "\u5BC6\u7801\u5FC5\u987B\u7559\u7A7A\uFF0C\u6216\u662F 8 \u5230 63 \u5B57\u8282" },
        "wifi.why.ctrl": { en: "control characters are not allowed", "zh-TW": "\u4E0D\u80FD\u542B\u6709\u63A7\u5236\u5B57\u5143", "zh-CN": "\u4E0D\u80FD\u5305\u542B\u63A7\u5236\u5B57\u7B26" },
        "wifi.why.store": { en: "the box could not store them", "zh-TW": "\u88DD\u7F6E\u7121\u6CD5\u5132\u5B58", "zh-CN": "\u8BBE\u5907\u65E0\u6CD5\u4FDD\u5B58" },
        "wifi.set.sending": { en: "Sending\u2026", "zh-TW": "\u50B3\u9001\u4E2D\u2026", "zh-CN": "\u6B63\u5728\u53D1\u9001\u2026" },
        "wifi.set.noreply": { en: "The box did not answer. Check the connection and try again.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\uFF0C\u8ACB\u78BA\u8A8D\u9023\u7DDA\u5F8C\u518D\u8A66\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u54CD\u5E94\uFF0C\u8BF7\u68C0\u67E5\u8FDE\u63A5\u540E\u518D\u8BD5\u3002" },
        "wifi.set.trying": { en: "Connecting to the network\u2026 ({s} s left)", "zh-TW": "\u6B63\u5728\u9023\u4E0A\u7DB2\u8DEF\u2026\uFF08\u9084\u5269 {s} \u79D2\uFF09", "zh-CN": "\u6B63\u5728\u8FDE\u63A5\u7F51\u7EDC\u2026\uFF08\u8FD8\u5269 {s} \u79D2\uFF09" },
        "wifi.set.ok": { en: 'Connected to "{ssid}".', "zh-TW": "\u5DF2\u9023\u4E0A\u300C{ssid}\u300D\u3002", "zh-CN": "\u5DF2\u8FDE\u63A5\u5230\u201C{ssid}\u201D\u3002" },
        "wifi.set.wrong": { en: "Wrong password. Check it and try again.", "zh-TW": "\u5BC6\u78BC\u4E0D\u5C0D\uFF0C\u8ACB\u78BA\u8A8D\u5F8C\u518D\u8A66\u3002", "zh-CN": "\u5BC6\u7801\u4E0D\u5BF9\uFF0C\u8BF7\u786E\u8BA4\u540E\u518D\u8BD5\u3002" },
        "wifi.set.notfound": { en: 'Could not find "{ssid}". Check the name, or move the box closer to the router.', "zh-TW": "\u627E\u4E0D\u5230\u300C{ssid}\u300D\u3002\u8ACB\u78BA\u8A8D\u540D\u7A31\uFF0C\u6216\u628A\u88DD\u7F6E\u79FB\u8FD1\u8DEF\u7531\u5668\u3002", "zh-CN": "\u627E\u4E0D\u5230\u201C{ssid}\u201D\u3002\u8BF7\u786E\u8BA4\u540D\u79F0\uFF0C\u6216\u628A\u8BBE\u5907\u79FB\u8FD1\u8DEF\u7531\u5668\u3002" },
        "wifi.set.timeout": { en: "No result after 20 seconds. The box keeps trying; check the status above in a moment.", "zh-TW": "\u7B49\u4E86 20 \u79D2\u9084\u6C92\u6709\u7D50\u679C\u3002\u88DD\u7F6E\u6703\u7E7C\u7E8C\u5617\u8A66\uFF0C\u8ACB\u7A0D\u5F8C\u518D\u770B\u4E0A\u9762\u7684\u72C0\u614B\u3002", "zh-CN": "\u7B49\u4E86 20 \u79D2\u8FD8\u6CA1\u6709\u7ED3\u679C\u3002\u8BBE\u5907\u4F1A\u7EE7\u7EED\u5C1D\u8BD5\uFF0C\u8BF7\u7A0D\u540E\u518D\u770B\u4E0A\u9762\u7684\u72B6\u6001\u3002" },
        "time.title": { en: "Time", "zh-TW": "\u6642\u9593", "zh-CN": "\u65F6\u95F4" },
        "time.k.now": { en: "Box time", "zh-TW": "\u88DD\u7F6E\u6642\u9593", "zh-CN": "\u8BBE\u5907\u65F6\u95F4" },
        "time.k.src": { en: "Source", "zh-TW": "\u4F86\u6E90", "zh-CN": "\u6765\u6E90" },
        "time.k.zone": { en: "Time zone", "zh-TW": "\u6642\u5340", "zh-CN": "\u65F6\u533A" },
        "time.src.ntp": { en: "Network time", "zh-TW": "\u7DB2\u8DEF\u5C0D\u6642", "zh-CN": "\u7F51\u7EDC\u5BF9\u65F6" },
        "time.src.client": { en: "Set from this page", "zh-TW": "\u7531\u9019\u500B\u9801\u9762\u8A2D\u5B9A", "zh-CN": "\u7531\u8FD9\u4E2A\u9875\u9762\u8BBE\u7F6E" },
        "time.src.none": { en: "No time yet", "zh-TW": "\u9084\u6C92\u6709\u6642\u9593", "zh-CN": "\u8FD8\u6CA1\u6709\u65F6\u95F4" },
        "time.none.note": { en: "The box has no battery clock. After each power-up it needs Wi-Fi time or this page to learn the time, and the night-mode schedule needs it.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u96FB\u6C60\u6642\u9418\uFF0C\u6BCF\u6B21\u958B\u6A5F\u5F8C\u8981\u7B49 Wi-Fi \u5C0D\u6642\uFF0C\u6216\u7531\u9019\u500B\u9801\u9762\u8A2D\u5B9A\u3002\u591C\u9593\u6A21\u5F0F\u7684\u6392\u7A0B\u9700\u8981\u6B63\u78BA\u7684\u6642\u9593\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u7535\u6C60\u65F6\u949F\uFF0C\u6BCF\u6B21\u5F00\u673A\u540E\u8981\u7B49 Wi-Fi \u5BF9\u65F6\uFF0C\u6216\u7531\u8FD9\u4E2A\u9875\u9762\u8BBE\u7F6E\u3002\u591C\u95F4\u6A21\u5F0F\u7684\u5B9A\u65F6\u9700\u8981\u6B63\u786E\u7684\u65F6\u95F4\u3002" },
        "time.sync": { en: "Sync now", "zh-TW": "\u7ACB\u5373\u5C0D\u6642", "zh-CN": "\u7ACB\u5373\u5BF9\u65F6" },
        "time.auto": { en: "Set the box's time zone to match this phone", "zh-TW": "\u81EA\u52D5\u628A\u9019\u53F0\u88DD\u7F6E\u7684\u6642\u5340\u8A2D\u6210\u8DDF\u9019\u652F\u624B\u6A5F\u4E00\u6A23", "zh-CN": "\u81EA\u52A8\u628A\u8FD9\u53F0\u8BBE\u5907\u7684\u65F6\u533A\u8BBE\u6210\u548C\u8FD9\u90E8\u624B\u673A\u4E00\u6837" },
        "time.res.ok": { en: "The box clock was set from this device.", "zh-TW": "\u5DF2\u7528\u9019\u652F\u624B\u6A5F\u7684\u6642\u9593\u5C0D\u6642\u3002", "zh-CN": "\u5DF2\u7528\u8FD9\u90E8\u624B\u673A\u7684\u65F6\u95F4\u5BF9\u65F6\u3002" },
        "time.res.zone": { en: "The clock comes from network time, so only the time zone was updated.", "zh-TW": "\u6642\u9418\u4F86\u81EA\u7DB2\u8DEF\u5C0D\u6642\uFF0C\u53EA\u66F4\u65B0\u4E86\u6642\u5340\u3002", "zh-CN": "\u65F6\u949F\u6765\u81EA\u7F51\u7EDC\u5BF9\u65F6\uFF0C\u53EA\u66F4\u65B0\u4E86\u65F6\u533A\u3002" },
        "time.res.refused": { en: "The box refused: {why}", "zh-TW": "\u88DD\u7F6E\u62D2\u7D55\u4E86\uFF1A{why}", "zh-CN": "\u8BBE\u5907\u62D2\u7EDD\u4E86\uFF1A{why}" },
        "time.res.noreply": { en: "The box did not answer.", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u56DE\u61C9\u3002", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u54CD\u5E94\u3002" },
        "vol.for.dac": { en: "DAC volume", "zh-TW": "\u5C0F\u5C3E\u5DF4\u7684\u97F3\u91CF", "zh-CN": "\u5C0F\u5C3E\u5DF4\u7684\u97F3\u91CF" },
        "vol.for.bt": { en: "Bluetooth headphones volume", "zh-TW": "\u85CD\u7259\u8033\u6A5F\u7684\u97F3\u91CF", "zh-CN": "\u84DD\u7259\u8033\u673A\u7684\u97F3\u91CF" },
        "vol.for.cast": { en: "Broadcast volume", "zh-TW": "\u5EE3\u64AD\u7684\u97F3\u91CF", "zh-CN": "\u5E7F\u64AD\u7684\u97F3\u91CF" },
        "vmax.title": { en: "Volume limit", "zh-TW": "\u97F3\u91CF\u4E0A\u9650", "zh-CN": "\u97F3\u91CF\u4E0A\u9650" },
        "vmax.hint": { en: "The volume never goes above this number, however it is changed; the volume slider at the bottom stops here too.", "zh-TW": "\u4E0D\u8AD6\u7528\u54EA\u7A2E\u65B9\u5F0F\u8ABF\u6574\uFF0C\u97F3\u91CF\u90FD\u4E0D\u6703\u8D85\u904E\u9019\u500B\u6578\u5B57\uFF1B\u4E0B\u9762\u7684\u97F3\u91CF\u6ED1\u687F\u4E5F\u53EA\u80FD\u8ABF\u5230\u9019\u88E1\u3002", "zh-CN": "\u4E0D\u8BBA\u7528\u54EA\u79CD\u65B9\u5F0F\u8C03\u6574\uFF0C\u97F3\u91CF\u90FD\u4E0D\u4F1A\u8D85\u8FC7\u8FD9\u4E2A\u6570\u5B57\uFF1B\u4E0B\u9762\u7684\u97F3\u91CF\u6ED1\u6746\u4E5F\u53EA\u80FD\u8C03\u5230\u8FD9\u91CC\u3002" },
        "gapless.label": { en: "Gapless playback", "zh-TW": "\u7121\u7E2B\u63A5\u7E8C", "zh-CN": "\u65E0\u7F1D\u8854\u63A5" },
        "gapless.note": { en: "Applies to consecutive tracks of the same sample rate: no gap between them.", "zh-TW": "\u53EA\u5C0D\u53D6\u6A23\u7387\u76F8\u540C\u7684\u9023\u7E8C\u66F2\u76EE\u6709\u6548\uFF0C\u66F2\u76EE\u4E4B\u9593\u4E0D\u6703\u6709\u7A7A\u6A94\u3002", "zh-CN": "\u53EA\u5BF9\u91C7\u6837\u7387\u76F8\u540C\u7684\u8FDE\u7EED\u66F2\u76EE\u6709\u6548\uFF0C\u66F2\u76EE\u4E4B\u95F4\u4E0D\u4F1A\u6709\u7A7A\u9699\u3002" },
        // ---- the app layout: navigation, help, the settings list, the control bar ----
        "nav.now": { en: "Playing", "zh-TW": "\u64AD\u653E\u4E2D", "zh-CN": "\u64AD\u653E\u4E2D" },
        "nav.music": { en: "Music", "zh-TW": "\u97F3\u6A02", "zh-CN": "\u97F3\u4E50" },
        "nav.radio": { en: "Radio", "zh-TW": "\u96FB\u53F0", "zh-CN": "\u7535\u53F0" },
        "nav.ambient": { en: "Ambient", "zh-TW": "\u74B0\u5883\u97F3", "zh-CN": "\u73AF\u5883\u97F3" },
        "nav.settings": { en: "Settings", "zh-TW": "\u8A2D\u5B9A", "zh-CN": "\u8BBE\u7F6E" },
        "nav.aria": { en: "Main menu", "zh-TW": "\u4E3B\u9078\u55AE", "zh-CN": "\u4E3B\u83DC\u5355" },
        "nav.back": { en: "Back", "zh-TW": "\u8FD4\u56DE", "zh-CN": "\u8FD4\u56DE" },
        "help.btn": { en: "Show or hide the explanations on this page", "zh-TW": "\u986F\u793A\u6216\u96B1\u85CF\u9019\u4E00\u9801\u7684\u8AAA\u660E", "zh-CN": "\u663E\u793A\u6216\u9690\u85CF\u8FD9\u4E00\u9875\u7684\u8BF4\u660E" },
        "help.switch": { en: "Show explanations", "zh-TW": "\u986F\u793A\u8AAA\u660E", "zh-CN": "\u663E\u793A\u8BF4\u660E" },
        "help.switch.note": { en: "Explanations appear under the settings. Warnings about consequences are always shown.", "zh-TW": "\u8AAA\u660E\u6703\u986F\u793A\u5728\u5404\u9805\u8A2D\u5B9A\u7684\u4E0B\u65B9\uFF1B\u6703\u9020\u6210\u5F8C\u679C\u7684\u8B66\u544A\u4E00\u5F8B\u6703\u986F\u793A\u3002", "zh-CN": "\u8BF4\u660E\u4F1A\u663E\u793A\u5728\u5404\u9879\u8BBE\u7F6E\u7684\u4E0B\u65B9\uFF1B\u4F1A\u9020\u6210\u540E\u679C\u7684\u8B66\u544A\u4E00\u5F8B\u4F1A\u663E\u793A\u3002" },
        "help.notice": { en: "Explanations are shown by default. Use the ? button at the top of a page to hide them there, or turn them all off under Settings \u203A Language & help.", "zh-TW": "\u8AAA\u660E\u9810\u8A2D\u6703\u986F\u793A\u3002\u6309\u9801\u9762\u4E0A\u65B9\u7684\u300C?\u300D\u53EF\u6536\u8D77\u9019\u4E00\u9801\u7684\u8AAA\u660E\uFF0C\u4E5F\u53EF\u4EE5\u5230\u300C\u8A2D\u5B9A \u203A \u8A9E\u8A00\u8207\u8AAA\u660E\u300D\u6574\u500B\u95DC\u6389\u3002", "zh-CN": "\u8BF4\u660E\u9ED8\u8BA4\u4F1A\u663E\u793A\u3002\u6309\u9875\u9762\u4E0A\u65B9\u7684\u201C?\u201D\u53EF\u6536\u8D77\u8FD9\u4E00\u9875\u7684\u8BF4\u660E\uFF0C\u4E5F\u53EF\u4EE5\u5230\u201C\u8BBE\u7F6E \u203A \u8BED\u8A00\u4E0E\u8BF4\u660E\u201D\u6574\u4E2A\u5173\u6389\u3002" },
        "help.notice.ok": { en: "Got it", "zh-TW": "\u77E5\u9053\u4E86", "zh-CN": "\u77E5\u9053\u4E86" },
        "help.notice.off": { en: "Hide explanations", "zh-TW": "\u96B1\u85CF\u8AAA\u660E", "zh-CN": "\u9690\u85CF\u8BF4\u660E" },
        "set.group.sound": { en: "Sound", "zh-TW": "\u8072\u97F3", "zh-CN": "\u58F0\u97F3" },
        "set.group.device": { en: "Device", "zh-TW": "\u88DD\u7F6E", "zh-CN": "\u8BBE\u5907" },
        "set.group.page": { en: "Display & diagnostics", "zh-TW": "\u986F\u793A\u8207\u8A3A\u65B7", "zh-CN": "\u663E\u793A\u4E0E\u8BCA\u65AD" },
        "set.output": { en: "Output & volume", "zh-TW": "\u8F38\u51FA\u8207\u97F3\u91CF", "zh-CN": "\u8F93\u51FA\u4E0E\u97F3\u91CF" },
        "set.fx": { en: "Effects & equalizer", "zh-TW": "\u97F3\u6548\u8207\u7B49\u5316\u5668", "zh-CN": "\u97F3\u6548\u4E0E\u5747\u8861\u5668" },
        "set.night": { en: "Night mode", "zh-TW": "\u591C\u9593\u6A21\u5F0F", "zh-CN": "\u591C\u95F4\u6A21\u5F0F" },
        "set.viz": { en: "Visualizer style", "zh-TW": "\u8996\u89BA\u6A23\u5F0F", "zh-CN": "\u89C6\u89C9\u6837\u5F0F" },
        "set.lang": { en: "Language & help", "zh-TW": "\u8A9E\u8A00\u8207\u8AAA\u660E", "zh-CN": "\u8BED\u8A00\u4E0E\u8BF4\u660E" },
        "set.diag": { en: "Diagnostics", "zh-TW": "\u8A3A\u65B7", "zh-CN": "\u8BCA\u65AD" },
        "set.problem": { en: "Needs attention", "zh-TW": "\u9700\u8981\u6CE8\u610F", "zh-CN": "\u9700\u8981\u6CE8\u610F" },
        "ent.output": { en: "Volume limit {n}", "zh-TW": "\u97F3\u91CF\u4E0A\u9650 {n}", "zh-CN": "\u97F3\u91CF\u4E0A\u9650 {n}" },
        "ent.fx.n": { en: "{n} on", "zh-TW": "\u5DF2\u958B\u555F {n} \u9805", "zh-CN": "\u5DF2\u5F00\u542F {n} \u9879" },
        "ent.fx.none": { en: "All off", "zh-TW": "\u5168\u90E8\u95DC\u9589", "zh-CN": "\u5168\u90E8\u5173\u95ED" },
        "ent.night": { en: "{s} \xB7 {r}", "zh-TW": "{s}\u30FB{r}", "zh-CN": "{s}\u30FB{r}" },
        "ent.wifi.on": { en: "Connected \xB7 {ssid}", "zh-TW": "\u5DF2\u9023\u7DDA\u30FB{ssid}", "zh-CN": "\u5DF2\u8FDE\u63A5\u30FB{ssid}" },
        "ent.time": { en: "{t} \xB7 {src}", "zh-TW": "{t}\u30FB{src}", "zh-CN": "{t}\u30FB{src}" },
        "ent.card.free": { en: "{free} free", "zh-TW": "{free} \u53EF\u7528", "zh-CN": "{free} \u53EF\u7528" },
        "ent.card.none": { en: "No card inserted", "zh-TW": "\u6C92\u6709\u63D2\u5165\u8A18\u61B6\u5361", "zh-CN": "\u6CA1\u6709\u63D2\u5165\u8BB0\u5FC6\u5361" },
        "ent.card.bad": { en: "Card unreadable", "zh-TW": "\u8A18\u61B6\u5361\u8B80\u4E0D\u5230", "zh-CN": "\u8BB0\u5FC6\u5361\u8BFB\u4E0D\u5230" },
        "ent.led": { en: "Adjust brightness", "zh-TW": "\u8ABF\u6574\u4EAE\u5EA6", "zh-CN": "\u8C03\u6574\u4EAE\u5EA6" },
        "ent.lang.on": { en: "{l} \xB7 explanations on", "zh-TW": "{l}\u30FB\u8AAA\u660E\u986F\u793A\u4E2D", "zh-CN": "{l}\u30FB\u8BF4\u660E\u663E\u793A\u4E2D" },
        "ent.lang.off": { en: "{l} \xB7 explanations off", "zh-TW": "{l}\u30FB\u8AAA\u660E\u5DF2\u96B1\u85CF", "zh-CN": "{l}\u30FB\u8BF4\u660E\u5DF2\u9690\u85CF" },
        "ent.diag": { en: "Free memory {kb} KB \xB7 {n} dropouts", "zh-TW": "\u53EF\u7528\u8A18\u61B6\u9AD4 {kb} KB\u30FB\u65B7\u97F3 {n} \u6B21", "zh-CN": "\u53EF\u7528\u5185\u5B58 {kb} KB\u30FB\u97F3\u9891\u4E2D\u65AD {n} \u6B21" },
        "cb.aria": { en: "Playback controls", "zh-TW": "\u64AD\u653E\u63A7\u5236", "zh-CN": "\u64AD\u653E\u63A7\u5236" },
        "tr.playOnly": { en: "Play", "zh-TW": "\u64AD\u653E", "zh-CN": "\u64AD\u653E" },
        "tr.pauseOnly": { en: "Pause", "zh-TW": "\u66AB\u505C", "zh-CN": "\u6682\u505C" },
        "lyr.title": { en: "Lyrics", "zh-TW": "\u6B4C\u8A5E", "zh-CN": "\u6B4C\u8BCD" },
        "lyr.open": { en: "Show all lyrics", "zh-TW": "\u986F\u793A\u5168\u90E8\u6B4C\u8A5E", "zh-CN": "\u663E\u793A\u5168\u90E8\u6B4C\u8BCD" },
        "files.title": { en: "Files", "zh-TW": "\u6A94\u6848", "zh-CN": "\u6587\u4EF6" },
        "amb.quick": { en: "One-tap sleep", "zh-TW": "\u4E00\u9375\u7761\u7720", "zh-CN": "\u4E00\u952E\u7761\u7720" },
        "timer.help": { en: "When the time is up the sound fades out and stops.", "zh-TW": "\u6642\u9593\u5230\u6703\u6DE1\u51FA\u4E26\u505C\u6B62\u64AD\u653E\u3002", "zh-CN": "\u65F6\u95F4\u5230\u4F1A\u6DE1\u51FA\u5E76\u505C\u6B62\u64AD\u653E\u3002" },
        "night.help": { en: "Quiet-volume boost, protection against sudden loud sounds, dimmed lights.", "zh-TW": "\u5C0F\u97F3\u91CF\u88DC\u511F\u3001\u9632\u7A81\u7136\u5927\u8072\u3001\u71C8\u5149\u8ABF\u6697\u3002", "zh-CN": "\u5C0F\u97F3\u91CF\u8865\u507F\u3001\u9632\u7A81\u7136\u5927\u58F0\u3001\u706F\u5149\u8C03\u6697\u3002" },
        "radio.help": { en: "Tap a station to play it; the star saves it to the memory card. The favourites list lives on the card, not in the browser.", "zh-TW": "\u9EDE\u4E00\u4E0B\u96FB\u53F0\u5C31\u6703\u958B\u59CB\u64AD\u653E\uFF1B\u2605 \u6703\u628A\u96FB\u53F0\u5B58\u5230\u8A18\u61B6\u5361\uFF0C\u6536\u85CF\u6E05\u55AE\u653E\u5728\u5361\u4E0A\uFF0C\u4E0D\u5728\u700F\u89BD\u5668\u88E1\u3002", "zh-CN": "\u70B9\u4E00\u4E0B\u7535\u53F0\u5C31\u4F1A\u5F00\u59CB\u64AD\u653E\uFF1B\u2605 \u4F1A\u628A\u7535\u53F0\u5B58\u5230\u8BB0\u5FC6\u5361\uFF0C\u6536\u85CF\u5217\u8868\u653E\u5728\u5361\u4E0A\uFF0C\u4E0D\u5728\u6D4F\u89C8\u5668\u91CC\u3002" },
        "output.perOut": { en: "Each output (DAC, Bluetooth headphones, broadcast) remembers its own volume; the volume bar shows which one you are adjusting.", "zh-TW": "\u6BCF\u500B\u8F38\u51FA\uFF08\u5C0F\u5C3E\u5DF4\u3001\u85CD\u7259\u8033\u6A5F\u3001\u5EE3\u64AD\uFF09\u5404\u81EA\u8A18\u4F4F\u81EA\u5DF1\u7684\u97F3\u91CF\uFF1B\u97F3\u91CF\u5217\u6703\u6A19\u793A\u73FE\u5728\u8ABF\u7684\u662F\u54EA\u4E00\u500B\u3002", "zh-CN": "\u6BCF\u4E2A\u8F93\u51FA\uFF08\u5C0F\u5C3E\u5DF4\u3001\u84DD\u7259\u8033\u673A\u3001\u5E7F\u64AD\uFF09\u5404\u81EA\u8BB0\u4F4F\u81EA\u5DF1\u7684\u97F3\u91CF\uFF1B\u97F3\u91CF\u680F\u4F1A\u6807\u793A\u73B0\u5728\u8C03\u7684\u662F\u54EA\u4E00\u4E2A\u3002" },
        "diag.build": { en: "Build", "zh-TW": "\u7248\u672C", "zh-CN": "\u7248\u672C" },
        "diag.web": { en: "Page version", "zh-TW": "\u7DB2\u9801\u7248\u672C", "zh-CN": "\u7F51\u9875\u7248\u672C" },
        "diag.fw": { en: "Firmware version", "zh-TW": "\u97CC\u9AD4\u7248\u672C", "zh-CN": "\u56FA\u4EF6\u7248\u672C" },
        "diag.fw.detail": { en: "ESP-IDF {idf} \xB7 {chip} \xB7 build {built}", "zh-TW": "ESP-IDF {idf}\u30FB{chip}\u30FB\u7DE8\u8B6F {built}", "zh-CN": "ESP-IDF {idf}\u30FB{chip}\u30FB\u7F16\u8BD1 {built}" },
        "diag.fw.none": { en: "Not reported (older firmware)", "zh-TW": "\u6C92\u6709\u7248\u672C\u8CC7\u8A0A\uFF08\u8F03\u820A\u7684\u97CC\u9AD4\uFF09", "zh-CN": "\u6CA1\u6709\u7248\u672C\u4FE1\u606F\uFF08\u8F83\u65E7\u7684\u56FA\u4EF6\uFF09" },
        "diag.links": { en: "Links", "zh-TW": "\u9023\u7D50", "zh-CN": "\u94FE\u63A5" },
        "hw.title": { en: "Hardware pins (read-only)", "zh-TW": "\u786C\u9AD4\u63A5\u8173\uFF08\u552F\u8B80\uFF09", "zh-CN": "\u786C\u4EF6\u5F15\u811A\uFF08\u53EA\u8BFB\uFF09" },
        "hw.led": { en: "Indicator light", "zh-TW": "\u6307\u793A\u71C8", "zh-CN": "\u6307\u793A\u706F" },
        "hw.mode.sdmmc": { en: "SDMMC, {width}-bit", "zh-TW": "SDMMC\uFF0C{width} \u4F4D\u5143", "zh-CN": "SDMMC\uFF0C{width} \u4F4D" },
        "hw.mode.spi": { en: "SPI", "zh-TW": "SPI", "zh-CN": "SPI" },
        "hw.src.default": { en: "default pins", "zh-TW": "\u9810\u8A2D\u63A5\u8173", "zh-CN": "\u9ED8\u8BA4\u5F15\u811A" },
        "hw.src.custom": { en: "custom pins", "zh-TW": "\u81EA\u8A02\u63A5\u8173", "zh-CN": "\u81EA\u5B9A\u4E49\u5F15\u811A" },
        "hw.sdstate.mounted": { en: "mounted", "zh-TW": "\u5DF2\u639B\u8F09", "zh-CN": "\u5DF2\u6302\u8F7D" },
        "hw.sdstate.none": { en: "no card", "zh-TW": "\u6C92\u6709\u5361", "zh-CN": "\u6CA1\u6709\u5361" },
        "hw.led.line": { en: "GPIO {gpio} \xB7 colour order {order}", "zh-TW": "GPIO {gpio}\u30FB\u8272\u5E8F {order}", "zh-CN": "GPIO {gpio}\u30FB\u8272\u5E8F {order}" },
        "hw.led.disabled": { en: "Disabled in the hardware settings", "zh-TW": "\u5DF2\u5728\u786C\u9AD4\u8A2D\u5B9A\u4E2D\u505C\u7528", "zh-CN": "\u5DF2\u5728\u786C\u4EF6\u8BBE\u7F6E\u4E2D\u505C\u7528" },
        "hw.pending": { en: "Hardware settings are waiting to be applied; they take effect after a restart.", "zh-TW": "\u6709\u5C1A\u672A\u5957\u7528\u7684\u786C\u9AD4\u8A2D\u5B9A\uFF0C\u91CD\u65B0\u555F\u52D5\u5F8C\u751F\u6548\u3002", "zh-CN": "\u6709\u5C1A\u672A\u5E94\u7528\u7684\u786C\u4EF6\u8BBE\u7F6E\uFF0C\u91CD\u542F\u540E\u751F\u6548\u3002" },
        "hw.flasher": { en: "These pins can only be changed in the flasher page over USB; over Bluetooth they are read-only.", "zh-TW": "\u9019\u4E9B\u63A5\u8173\u8981\u5728\u71D2\u9304\u9801\u900F\u904E USB \u624D\u80FD\u4FEE\u6539\uFF1B\u85CD\u7259\u9023\u7DDA\u4E0B\u53EA\u80FD\u67E5\u770B\u3002", "zh-CN": "\u8FD9\u4E9B\u5F15\u811A\u9700\u8981\u5728\u70E7\u5F55\u9875\u901A\u8FC7 USB \u4FEE\u6539\uFF1B\u84DD\u7259\u8FDE\u63A5\u4E0B\u53EA\u80FD\u67E5\u770B\u3002" },
        "hw.none": { en: "This firmware reports no hardware information.", "zh-TW": "\u9019\u500B\u97CC\u9AD4\u6C92\u6709\u63D0\u4F9B\u786C\u9AD4\u8CC7\u8A0A\u3002", "zh-CN": "\u8FD9\u4E2A\u56FA\u4EF6\u6CA1\u6709\u63D0\u4F9B\u786C\u4EF6\u4FE1\u606F\u3002" },
        "set.group.about": { en: "About", "zh-TW": "\u95DC\u65BC", "zh-CN": "\u5173\u4E8E" },
        "about.flash": { en: "Flash & hardware settings", "zh-TW": "\u71D2\u9304\u8207\u786C\u9AD4\u8A2D\u5B9A", "zh-CN": "\u70E7\u5F55\u4E0E\u786C\u4EF6\u8BBE\u7F6E" },
        "about.flash.d": { en: "Install or reflash the firmware over USB, set pins", "zh-TW": "\u7528 USB \u5B89\u88DD\u6216\u91CD\u5237\u97CC\u9AD4\u3001\u8A2D\u5B9A\u63A5\u8173", "zh-CN": "\u7528 USB \u5B89\u88C5\u6216\u91CD\u5237\u56FA\u4EF6\u3001\u8BBE\u7F6E\u5F15\u811A" },
        "about.releases": { en: "Download firmware", "zh-TW": "\u4E0B\u8F09\u97CC\u9AD4", "zh-CN": "\u4E0B\u8F7D\u56FA\u4EF6" },
        "about.releases.d": { en: "Latest release on GitHub", "zh-TW": "GitHub \u4E0A\u7684\u6700\u65B0\u7248\u672C", "zh-CN": "GitHub \u4E0A\u7684\u6700\u65B0\u7248\u672C" },
        "about.repo": { en: "Help & source code", "zh-TW": "\u8AAA\u660E\u8207\u539F\u59CB\u78BC", "zh-CN": "\u8BF4\u660E\u4E0E\u6E90\u4EE3\u7801" },
        "about.repo.d": { en: "README and repository on GitHub", "zh-TW": "GitHub \u4E0A\u7684\u8AAA\u660E\u6587\u4EF6\u8207\u7A0B\u5F0F\u78BC", "zh-CN": "GitHub \u4E0A\u7684\u8BF4\u660E\u6587\u6863\u4E0E\u4EE3\u7801" },
        "flash.hint": { en: "No firmware yet, or need to reflash?", "zh-TW": "\u9084\u6C92\u6709\u97CC\u9AD4\uFF0C\u6216\u9700\u8981\u91CD\u5237\uFF1F", "zh-CN": "\u8FD8\u6CA1\u6709\u56FA\u4EF6\uFF0C\u6216\u9700\u8981\u91CD\u5237\uFF1F" },
        "link.newtab": { en: "(opens in a new tab)", "zh-TW": "\uFF08\u5728\u65B0\u5206\u9801\u958B\u555F\uFF09", "zh-CN": "\uFF08\u5728\u65B0\u6807\u7B7E\u9875\u6253\u5F00\uFF09" },
        "rate.title": { en: "Highest sample rate", "zh-TW": "\u6700\u9AD8\u53D6\u6A23\u7387", "zh-CN": "\u6700\u9AD8\u91C7\u6837\u7387" },
        "rate.help": { en: "Files with a higher sample rate are converted down to this before playing. Auto lets the box decide from what the DAC can do.", "zh-TW": "\u53D6\u6A23\u7387\u6BD4\u9019\u500B\u9AD8\u7684\u6A94\u6848\u6703\u5148\u964D\u5230\u9019\u500B\u503C\u518D\u64AD\u653E\u3002\u300C\u81EA\u52D5\u300D\u7531\u88DD\u7F6E\u4F9D DAC \u7684\u80FD\u529B\u6C7A\u5B9A\u3002", "zh-CN": "\u91C7\u6837\u7387\u6BD4\u8FD9\u4E2A\u9AD8\u7684\u6587\u4EF6\u4F1A\u5148\u964D\u5230\u8FD9\u4E2A\u503C\u518D\u64AD\u653E\u3002\u201C\u81EA\u52A8\u201D\u7531\u8BBE\u5907\u6839\u636E DAC \u7684\u80FD\u529B\u51B3\u5B9A\u3002" },
        "ent.output.rate": { en: "Volume limit {n} \xB7 up to {r} kHz", "zh-TW": "\u97F3\u91CF\u4E0A\u9650 {n}\u30FB\u6700\u9AD8 {r} kHz", "zh-CN": "\u97F3\u91CF\u4E0A\u9650 {n}\u30FB\u6700\u9AD8 {r} kHz" },
        "rt.title": { en: "Test the sample rates this USB DAC supports", "zh-TW": "\u6E2C\u8A66\u9019\u500B USB DAC \u652F\u63F4\u7684\u53D6\u6A23\u7387", "zh-CN": "\u6D4B\u8BD5\u8FD9\u4E2A USB DAC \u652F\u6301\u7684\u91C7\u6837\u7387" },
        "rt.warn": { en: "This stops playback and plays a quiet test tone through the DAC, about 2 seconds per rate. The result only shows that the DAC accepted the data, not how it sounds.", "zh-TW": "\u6E2C\u8A66\u6703\u505C\u6B62\u76EE\u524D\u7684\u64AD\u653E\uFF0C\u4E26\u900F\u904E DAC \u64AD\u653E\u5B89\u975C\u7684\u6E2C\u8A66\u97F3\uFF0C\u6BCF\u500B\u53D6\u6A23\u7387\u7D04 2 \u79D2\u3002\u7D50\u679C\u53EA\u4EE3\u8868\u8CC7\u6599\u6709\u88AB DAC \u63A5\u53D7\uFF0C\u4E0D\u4EE3\u8868\u807D\u8D77\u4F86\u5982\u4F55\u3002", "zh-CN": "\u6D4B\u8BD5\u4F1A\u505C\u6B62\u5F53\u524D\u7684\u64AD\u653E\uFF0C\u5E76\u901A\u8FC7 DAC \u64AD\u653E\u5B89\u9759\u7684\u6D4B\u8BD5\u97F3\uFF0C\u6BCF\u4E2A\u91C7\u6837\u7387\u7EA6 2 \u79D2\u3002\u7ED3\u679C\u53EA\u8868\u793A\u6570\u636E\u88AB DAC \u63A5\u53D7\u4E86\uFF0C\u4E0D\u4EE3\u8868\u542C\u8D77\u6765\u5982\u4F55\u3002" },
        "rt.start": { en: "Start test", "zh-TW": "\u958B\u59CB\u6E2C\u8A66", "zh-CN": "\u5F00\u59CB\u6D4B\u8BD5" },
        "rt.cancel": { en: "Cancel", "zh-TW": "\u53D6\u6D88", "zh-CN": "\u53D6\u6D88" },
        "rt.state.none": { en: "No result yet", "zh-TW": "\u9084\u6C92\u6709\u6E2C\u8A66\u7D50\u679C", "zh-CN": "\u8FD8\u6CA1\u6709\u6D4B\u8BD5\u7ED3\u679C" },
        "rt.state.idle": { en: "Last result", "zh-TW": "\u4E0A\u6B21\u7684\u7D50\u679C", "zh-CN": "\u4E0A\u6B21\u7684\u7ED3\u679C" },
        "rt.state.running": { en: "Testing {rate} kHz ({done} of {n} done)", "zh-TW": "\u6E2C\u8A66\u4E2D\uFF1A{rate} kHz\uFF08\u5DF2\u5B8C\u6210 {done}\uFF0F{n}\uFF09", "zh-CN": "\u6D4B\u8BD5\u4E2D\uFF1A{rate} kHz\uFF08\u5DF2\u5B8C\u6210 {done}\uFF0F{n}\uFF09" },
        "rt.state.running0": { en: "Testing\u2026", "zh-TW": "\u6E2C\u8A66\u4E2D\u2026", "zh-CN": "\u6D4B\u8BD5\u4E2D\u2026" },
        "rt.state.done": { en: "Test finished", "zh-TW": "\u6E2C\u8A66\u5B8C\u6210", "zh-CN": "\u6D4B\u8BD5\u5B8C\u6210" },
        "rt.state.cancelled": { en: "Test cancelled", "zh-TW": "\u5DF2\u53D6\u6D88\u6E2C\u8A66", "zh-CN": "\u5DF2\u53D6\u6D88\u6D4B\u8BD5" },
        "rt.state.failed": { en: "Test failed", "zh-TW": "\u6E2C\u8A66\u5931\u6557", "zh-CN": "\u6D4B\u8BD5\u5931\u8D25" },
        "rt.state.failedWhy": { en: "Test failed: {why}", "zh-TW": "\u6E2C\u8A66\u5931\u6557\uFF1A{why}", "zh-CN": "\u6D4B\u8BD5\u5931\u8D25\uFF1A{why}" },
        "rt.col.rate": { en: "Rate", "zh-TW": "\u53D6\u6A23\u7387", "zh-CN": "\u91C7\u6837\u7387" },
        "rt.col.result": { en: "Result", "zh-TW": "\u7D50\u679C", "zh-CN": "\u7ED3\u679C" },
        "rt.col.measured": { en: "Measured", "zh-TW": "\u5BE6\u6E2C", "zh-CN": "\u5B9E\u6D4B" },
        "rt.v.ok": { en: "Usable", "zh-TW": "\u53EF\u7528", "zh-CN": "\u53EF\u7528" },
        "rt.v.bad": { en: "Not usable", "zh-TW": "\u4E0D\u53EF\u7528", "zh-CN": "\u4E0D\u53EF\u7528" },
        "rt.v.untested": { en: "Not tested", "zh-TW": "\u672A\u6E2C", "zh-CN": "\u672A\u6D4B" },
        "rt.why.setfail": { en: "the DAC did not accept this clock", "zh-TW": "DAC \u4E0D\u63A5\u53D7\u9019\u500B\u6642\u8108", "zh-CN": "DAC \u4E0D\u63A5\u53D7\u8FD9\u4E2A\u65F6\u949F" },
        "rt.why.xfer": { en: "{n} transfer errors", "zh-TW": "\u50B3\u8F38\u932F\u8AA4 {n} \u6B21", "zh-CN": "\u4F20\u8F93\u9519\u8BEF {n} \u6B21" },
        "rt.max": { en: "Highest usable rate: {r} kHz", "zh-TW": "\u6700\u9AD8\u53EF\u7528\u53D6\u6A23\u7387\uFF1A{r} kHz", "zh-CN": "\u6700\u9AD8\u53EF\u7528\u91C7\u6837\u7387\uFF1A{r} kHz" },
        "rt.max.none": { en: "No usable rate found", "zh-TW": "\u6C92\u6709\u627E\u5230\u53EF\u7528\u7684\u53D6\u6A23\u7387", "zh-CN": "\u6CA1\u6709\u627E\u5230\u53EF\u7528\u7684\u91C7\u6837\u7387" },
        "rt.refused.no_dac": { en: "No USB DAC is connected", "zh-TW": "\u6C92\u6709\u5075\u6E2C\u5230 USB DAC", "zh-CN": "\u6CA1\u6709\u68C0\u6D4B\u5230 USB DAC" },
        "rt.refused.card_test_running": { en: "A memory card test is running; try again when it is done", "zh-TW": "\u8A18\u61B6\u5361\u6E2C\u8A66\u9032\u884C\u4E2D\uFF0C\u7D50\u675F\u5F8C\u518D\u8A66", "zh-CN": "\u5B58\u50A8\u5361\u6D4B\u8BD5\u6B63\u5728\u8FDB\u884C\uFF0C\u7ED3\u675F\u540E\u518D\u8BD5" },
        "rt.refused.formatting": { en: "The memory card is being formatted", "zh-TW": "\u6B63\u5728\u683C\u5F0F\u5316\u8A18\u61B6\u5361", "zh-CN": "\u6B63\u5728\u683C\u5F0F\u5316\u5B58\u50A8\u5361" },
        "rt.refused.already_running": { en: "A test is already running", "zh-TW": "\u5DF2\u7D93\u5728\u6E2C\u8A66\u4E2D", "zh-CN": "\u5DF2\u7ECF\u5728\u6D4B\u8BD5\u4E2D" },
        "rt.refused.not_running": { en: "No test is running", "zh-TW": "\u76EE\u524D\u6C92\u6709\u9032\u884C\u4E2D\u7684\u6E2C\u8A66", "zh-CN": "\u5F53\u524D\u6CA1\u6709\u8FDB\u884C\u4E2D\u7684\u6D4B\u8BD5" },
        "rt.refused.other": { en: "The box refused: {why}", "zh-TW": "\u88DD\u7F6E\u62D2\u7D55\uFF1A{why}", "zh-CN": "\u8BBE\u5907\u62D2\u7EDD\uFF1A{why}" },
        "rt.unsupported": { en: "This firmware has no sample-rate test", "zh-TW": "\u9019\u500B\u97CC\u9AD4\u6C92\u6709\u53D6\u6A23\u7387\u6E2C\u8A66", "zh-CN": "\u8FD9\u4E2A\u56FA\u4EF6\u6CA1\u6709\u91C7\u6837\u7387\u6D4B\u8BD5" },
        "ent.fx.shed": { en: "Paused: the box is busy", "zh-TW": "\u66AB\u505C\u4E2D\uFF1A\u88DD\u7F6E\u5FD9\u788C", "zh-CN": "\u6682\u505C\u4E2D\uFF1A\u8BBE\u5907\u7E41\u5FD9" },
        "led.offNote": { en: "The indicator light is disabled in the hardware settings, so there is no brightness to adjust. Change it in the flasher page over USB.", "zh-TW": "\u6307\u793A\u71C8\u5DF2\u5728\u786C\u9AD4\u8A2D\u5B9A\u4E2D\u505C\u7528\uFF0C\u6240\u4EE5\u6C92\u6709\u4EAE\u5EA6\u53EF\u4EE5\u8ABF\u6574\u3002\u8981\u4FEE\u6539\u8ACB\u7528 USB \u5230\u71D2\u9304\u9801\u3002", "zh-CN": "\u6307\u793A\u706F\u5DF2\u5728\u786C\u4EF6\u8BBE\u7F6E\u4E2D\u505C\u7528\uFF0C\u6240\u4EE5\u6CA1\u6709\u4EAE\u5EA6\u53EF\u8C03\u3002\u8981\u4FEE\u6539\u8BF7\u7528 USB \u5230\u70E7\u5F55\u9875\u3002" },
        "files.playZip": { en: "Play this archive", "zh-TW": "\u64AD\u653E\u9019\u500B\u58D3\u7E2E\u6A94", "zh-CN": "\u64AD\u653E\u8FD9\u4E2A\u538B\u7F29\u5305" },
        "play.err.size": { en: "This file is too large (a single file inside an archive can be 8 MB at most)", "zh-TW": "\u9019\u500B\u6A94\u6848\u592A\u5927\uFF08\u58D3\u7E2E\u6A94\u5167\u55AE\u4E00\u6A94\u6848\u4E0A\u9650 8 MB\uFF09", "zh-CN": "\u8FD9\u4E2A\u6587\u4EF6\u592A\u5927\uFF08\u538B\u7F29\u5305\u5185\u5355\u4E2A\u6587\u4EF6\u4E0A\u9650 8 MB\uFF09" },
        "play.err.notfound": { en: "File not found", "zh-TW": "\u627E\u4E0D\u5230\u9019\u500B\u6A94\u6848", "zh-CN": "\u627E\u4E0D\u5230\u8FD9\u4E2A\u6587\u4EF6" },
        "play.err.timeout": { en: "The box did not answer in time (the card or the archive may be slow); try again", "zh-TW": "\u88DD\u7F6E\u6C92\u6709\u53CA\u6642\u56DE\u61C9\uFF08\u8A18\u61B6\u5361\u6216\u58D3\u7E2E\u6A94\u53EF\u80FD\u592A\u6162\uFF09\uFF0C\u8ACB\u518D\u8A66\u4E00\u6B21", "zh-CN": "\u8BBE\u5907\u6CA1\u6709\u53CA\u65F6\u54CD\u5E94\uFF08\u5B58\u50A8\u5361\u6216\u538B\u7F29\u5305\u53EF\u80FD\u592A\u6162\uFF09\uFF0C\u8BF7\u518D\u8BD5\u4E00\u6B21" },
        "play.err.busy": { en: "Can't play right now (another job, such as the sample-rate test, is running)", "zh-TW": "\u73FE\u5728\u7121\u6CD5\u64AD\u653E\uFF08\u53EF\u80FD\u6B63\u5728\u9032\u884C\u53D6\u6A23\u7387\u6E2C\u8A66\u7B49\u5176\u4ED6\u5DE5\u4F5C\uFF09", "zh-CN": "\u73B0\u5728\u65E0\u6CD5\u64AD\u653E\uFF08\u53EF\u80FD\u6B63\u5728\u8FDB\u884C\u91C7\u6837\u7387\u6D4B\u8BD5\u7B49\u5176\u4ED6\u5DE5\u4F5C\uFF09" },
        "play.err.nomem": { en: "Not enough memory to play this file", "zh-TW": "\u8A18\u61B6\u9AD4\u4E0D\u8DB3\uFF0C\u7121\u6CD5\u64AD\u653E\u9019\u500B\u6A94\u6848", "zh-CN": "\u5185\u5B58\u4E0D\u8DB3\uFF0C\u65E0\u6CD5\u64AD\u653E\u8FD9\u4E2A\u6587\u4EF6" },
        "play.err.notsup": { en: "This format can't be played", "zh-TW": "\u4E0D\u652F\u63F4\u9019\u7A2E\u683C\u5F0F", "zh-CN": "\u4E0D\u652F\u6301\u8FD9\u79CD\u683C\u5F0F" },
        "play.err.other": { en: "Could not play ({code})", "zh-TW": "\u7121\u6CD5\u64AD\u653E\uFF08{code}\uFF09", "zh-CN": "\u65E0\u6CD5\u64AD\u653E\uFF08{code}\uFF09" }
      };
    }
  });

  // src/js/core/i18n.js
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
  function saved() {
    try {
      return exact(g.localStorage.getItem(KEY));
    } catch {
      return null;
    }
  }
  function has(key) {
    const row2 = table()[key];
    return !!row2 && CHAIN[lang].some((l) => typeof row2[l] === "string");
  }
  function t(key, params) {
    const row2 = table()[key];
    let s = null;
    if (row2) {
      for (const l of CHAIN[lang]) if (typeof row2[l] === "string") {
        s = row2[l];
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
  function paramsOf(el4) {
    const p = el4.getAttribute("data-i18n-p");
    if (!p) return void 0;
    try {
      return JSON.parse(p);
    } catch {
      return void 0;
    }
  }
  function applyDom(root) {
    root = root || g.document;
    const doc = g.document;
    if (root === doc) {
      doc.documentElement.lang = lang;
      doc.title = t("app.title");
    }
    for (const el4 of root.querySelectorAll("[data-i18n]")) el4.textContent = t(el4.getAttribute("data-i18n"), paramsOf(el4));
    for (const [from, to] of ATTRS) for (const el4 of root.querySelectorAll("[" + from + "]")) el4.setAttribute(to, t(el4.getAttribute(from), paramsOf(el4)));
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
  var g, KEY, LANGS, CHAIN, lang, urlLang, listeners, table, browserLangs, ATTRS, pref, I18N;
  var init_i18n = __esm({
    "src/js/core/i18n.js"() {
      init_strings();
      g = globalThis;
      KEY = "nsn.lang";
      LANGS = ["en", "zh-TW", "zh-CN"];
      CHAIN = { en: ["en"], "zh-TW": ["zh-TW", "en"], "zh-CN": ["zh-CN", "zh-TW", "en"] };
      lang = "zh-TW";
      urlLang = null;
      listeners = [];
      table = () => STRINGS;
      browserLangs = () => g.navigator && (g.navigator.languages && g.navigator.languages.length ? g.navigator.languages : [g.navigator.language]) || [];
      ATTRS = [["data-i18n-aria", "aria-label"], ["data-i18n-placeholder", "placeholder"], ["data-i18n-title", "title"]];
      pref = () => urlLang || saved() || "auto";
      I18N = g.NSN_I18N = { LANGS, t, has, init, setLang, pref, applyDom, setT, setRaw, onChange, lang: () => lang, detect, normalize };
    }
  });

  // src/js/core/app.js
  function on(ev, fn) {
    if (!handlers.has(ev)) handlers.set(ev, []);
    handlers.get(ev).push(fn);
  }
  function emit(ev, ...args) {
    for (const fn of (handlers.get(ev) || []).slice()) fn(...args);
  }
  function segSet(id, v) {
    for (const b of $(id).children) b.classList.toggle("on", b.dataset.v === String(v));
  }
  function segBind(id, fn) {
    for (const b of $(id).children) b.onclick = () => {
      segSet(id, b.dataset.v);
      fn(b.dataset.v);
    };
  }
  function emptyLi(key) {
    const li = document.createElement("li");
    li.className = "mut";
    li.style.cursor = "default";
    setT2(li, key);
    return li;
  }
  var $, sleep, DEMO, t2, setT2, setRaw2, app, handlers, MODES, RGS, known;
  var init_app = __esm({
    "src/js/core/app.js"() {
      init_i18n();
      $ = (id) => document.getElementById(id);
      sleep = (ms) => new Promise((r) => setTimeout(r, ms));
      DEMO = new URLSearchParams(location.search).has("demo");
      ({ t: t2, setT: setT2, setRaw: setRaw2 } = I18N);
      app = {
        state: null,
        // the last parsed `st` reply, null while not connected
        link: "off",
        // "off" (not connected) | "on" | "reconnecting" (the box dropped the link; the page keeps trying)
        connected: false,
        // link === "on"
        dev: null,
        chCmd: null,
        chEvt: null,
        chSpec: null,
        userDisc: false,
        castStarting: false,
        cwd: "/sdcard",
        // the folder shown on the Music page
        selOut: "",
        // the output selected on the box ("" until `out` has been read)
        scanBusy: false,
        // a Wi-Fi scan is running: the box handles no commands meanwhile, so the page's own polling pauses
        target: new Uint8Array(32),
        // latest spectrum bands from the box, read by the visualizer
        viz: null,
        // set when the visualizer has been mounted
        radio: null,
        // set when the station finder has been mounted
        vmaxUntil: 0,
        gapUntil: 0,
        // after the user's own change a poll that was already in flight must not move the control back
        route: "",
        // the current route key, e.g. "now", "settings/wifi" (router.js)
        ui: {}
        // the mounted section modules: card, wifi, time (lib/*.js)
      };
      handlers = /* @__PURE__ */ new Map();
      MODES = ["folder", "shuffle", "one", "once"];
      RGS = ["off", "track", "album"];
      known = (prefix, v) => {
        const k2 = prefix + String(v).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
        return I18N.has(k2) ? t2(k2) : String(v);
      };
    }
  });

  // src/js/core/icons.js
  function setIcon(svg, name) {
    svg.setAttribute("data-ic", name);
    svg.setAttribute("viewBox", "0 0 24 24");
    svg.setAttribute("aria-hidden", "true");
    svg.innerHTML = ICONS[name] || "";
  }
  function hydrateIcons(root = document) {
    for (const s of root.querySelectorAll("svg[data-ic]")) setIcon(s, s.getAttribute("data-ic"));
  }
  var SOLID, ICONS;
  var init_icons = __esm({
    "src/js/core/icons.js"() {
      SOLID = ' fill="currentColor"';
      ICONS = {
        now: '<circle cx="12" cy="12" r="9"/><path d="M10 8.5l5.5 3.5-5.5 3.5z"/>',
        music: '<path d="M9 18V5l11-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="17" cy="16" r="3"/>',
        radio: '<rect x="3" y="8" width="18" height="12" rx="2"/><path d="M7 8l10-5"/><circle cx="8.5" cy="14" r="2.5"/><path d="M15 13h3M15 16h3"/>',
        ambient: '<path d="M3 7c3-3 6 3 9 0s6 3 9 0"/><path d="M3 12c3-3 6 3 9 0s6 3 9 0"/><path d="M3 17c3-3 6 3 9 0s6 3 9 0"/>',
        settings: '<path d="M4 6h8M18 6h2M4 12h2M12 12h8M4 18h10M20 18h0"/><circle cx="15" cy="6" r="2.5"/><circle cx="9" cy="12" r="2.5"/><circle cx="17" cy="18" r="2.5"/>',
        help: '<circle cx="12" cy="12" r="9"/><path d="M9.5 9.2a2.6 2.6 0 0 1 5 .8c0 1.8-2.5 2.1-2.5 3.7"/><path d="M12 17h.01"/>',
        back: '<path d="M15 5l-7 7 7 7"/>',
        chev: '<path d="M9 5l7 7-7 7"/>',
        up: '<path d="M12 19V6M6 11l6-6 6 6"/>',
        play: '<path d="M7 4.5v15l12-7.5z"' + SOLID + "/>",
        pause: '<rect x="6" y="5" width="4" height="14" rx="1"' + SOLID + '/><rect x="14" y="5" width="4" height="14" rx="1"' + SOLID + "/>",
        prev: '<path d="M6 5v14"/><path d="M19 5.5v13L9 12z"' + SOLID + "/>",
        next: '<path d="M18 5v14"/><path d="M5 5.5v13L15 12z"' + SOLID + "/>",
        stop: '<rect x="6" y="6" width="12" height="12" rx="2"' + SOLID + "/>",
        vol: '<path d="M4 9h4l5-4v14l-5-4H4z"/><path d="M16.5 8.5a5 5 0 0 1 0 7M19 6a8.5 8.5 0 0 1 0 12"/>',
        auto: '<path d="M12 3l2.2 5.8L20 11l-5.8 2.2L12 19l-2.2-5.8L4 11l5.8-2.2z"/>',
        dac: '<path d="M9 3v5M15 3v5M7 8h10v4a5 5 0 0 1-10 0zM12 17v4"/>',
        bt: '<path d="M7 7l10 10-5 4V3l5 4L7 17"/>',
        cast: '<circle cx="12" cy="12" r="1.8"/><path d="M8.3 8.3a5.2 5.2 0 0 0 0 7.4M15.7 8.3a5.2 5.2 0 0 1 0 7.4M5.4 5.4a9.3 9.3 0 0 0 0 13.2M18.6 5.4a9.3 9.3 0 0 1 0 13.2"/>',
        lyrics: '<path d="M4 6h16M4 11h12M4 16h16M4 21h8"/>',
        night: '<path d="M20 14.5A8 8 0 1 1 9.5 4a6.5 6.5 0 0 0 10.5 10.5z"/>',
        output: '<path d="M4 15v-3a8 8 0 0 1 16 0v3"/><rect x="3" y="14" width="4" height="7" rx="1.5"/><rect x="17" y="14" width="4" height="7" rx="1.5"/>',
        fx: '<path d="M4 21v-7M4 10V3M12 21v-9M12 8V3M20 21v-5M20 12V3M1.5 14h5M9.5 8h5M17.5 16h5"/>',
        wifi: '<path d="M2 9a15 15 0 0 1 20 0M5.5 12.5a10 10 0 0 1 13 0M9 16a5 5 0 0 1 6 0M12 19.5h.01"/>',
        clock: '<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3.5 2"/>',
        card: '<path d="M7 3h7l4 4v14H7z"/><path d="M10 7v3M13 7v3"/>',
        keys: '<rect x="7" y="2.5" width="10" height="19" rx="3"/><path d="M12 7v.01M9.5 12h5M9.5 16h5"/>',
        led: '<path d="M9 18h6M10 21h4"/><path d="M12 3a6 6 0 0 0-3.5 10.9c.6.5 1 1.2 1 2.1h5c0-.9.4-1.6 1-2.1A6 6 0 0 0 12 3z"/>',
        viz: '<path d="M3 12h2.5l2-6 3.5 12 3-9 2 5 1.5-2H21"/>',
        lang: '<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18"/>',
        flash: '<path d="M13 2L5 13.5h6L10 22l9-12h-6z"/>',
        download: '<path d="M12 3v12M7 10.5l5 5 5-5M4 20h16"/>',
        code: '<path d="M8.5 7L3.5 12l5 5M15.5 7l5 5-5 5M13.5 4.5l-3 15"/>',
        ext: '<path d="M14 4h6v6M20 4l-9 9M18 14v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h5"/>',
        diag: '<path d="M4 18a8 8 0 1 1 16 0"/><path d="M12 18l4-6"/><path d="M12 18h.01"/>'
      };
    }
  });

  // src/js/lib/wifi.js
  function utf8Len(s) {
    let n = 0;
    for (const ch of String(s)) {
      const c = ch.codePointAt(0);
      n += c < 128 ? 1 : c < 2048 ? 2 : c < 65536 ? 3 : 4;
    }
    return n;
  }
  function parseWifi(lines) {
    for (const raw of lines || []) {
      const m = RE_WIFI.exec(String(raw).trimEnd());
      if (!m) continue;
      return {
        state: m[1],
        ip: m[2],
        rssi: /^-?\d+$/.test(m[3]) ? +m[3] : null,
        synced: !/^\(unsynced\)/.test(m[4]),
        time: m[4].replace(/^\(unsynced\)\s*/, ""),
        src: m[5],
        ssid: unesc(m[6])
      };
    }
    return null;
  }
  function parseScan(lines) {
    const r = { aps: [], status: null, n: 0 };
    for (const raw of lines || []) {
      const l = String(raw).trimEnd();
      let m;
      if (m = /^wifi ap (-?\d+) (open|secured) "(.*)"$/.exec(l)) r.aps.push({ rssi: +m[1], secured: m[2] === "secured", ssid: unesc(m[3]) });
      else if (m = /^wifi scan: (?:(\d+) found|(busy)|(failed))/.exec(l)) {
        r.status = m[2] || m[3] || "found";
        r.n = m[1] ? +m[1] : r.aps.length;
      }
    }
    return r;
  }
  function sigLevel(rssi) {
    return rssi >= -55 ? 4 : rssi >= -65 ? 3 : rssi >= -75 ? 2 : 1;
  }
  function validate(ssid, pw) {
    if (/[\x00-\x1f\x7f]/.test(ssid) || /[\x00-\x1f\x7f]/.test(pw)) return { key: "wifi.err.ctrl" };
    const sb = utf8Len(ssid), pb = utf8Len(pw);
    if (!sb) return { key: "wifi.err.ssidEmpty" };
    if (ssid.includes("|")) return { key: "wifi.err.bar" };
    if (sb > 32) return { key: "wifi.err.ssidLong", params: { n: sb } };
    if (pb && (pb < 8 || pb > 63)) return { key: "wifi.err.pwLen", params: { n: pb } };
    return null;
  }
  function whyKey(r) {
    r = String(r || "");
    return /usage/i.test(r) ? "wifi.why.usage" : /^ssid/i.test(r) ? "wifi.why.ssid" : /^password/i.test(r) ? "wifi.why.pw" : /control/i.test(r) ? "wifi.why.ctrl" : /cannot store/i.test(r) ? "wifi.why.store" : null;
  }
  function redact(cmd) {
    if (typeof cmd !== "string" || !cmd.startsWith("wifi set ")) return cmd;
    const i = cmd.indexOf("|");
    return i >= 0 && i < cmd.length - 1 ? cmd.slice(0, i + 1) + "\u2022\u2022\u2022\u2022" : cmd;
  }
  function mount(o) {
    const I2 = I18N, setT4 = I2.setT, setRaw4 = I2.setRaw, t6 = I2.t;
    const { box, send: send2, sendUntil: sendUntil2, getState, isOn, tabOpen } = o;
    const el4 = (tag, cls, text) => {
      const e = document.createElement(tag);
      if (cls) e.className = cls;
      if (text !== void 0) e.textContent = text;
      return e;
    };
    const elT2 = (tag, cls, key, params) => {
      const e = el4(tag, cls);
      setT4(e, key, params);
      return e;
    };
    const S = { info: null, scan: { busy: false, aps: null, msg: null }, msg: null, joining: false, tok: 0, lastWifi: void 0 };
    let timer2 = 0, refreshing = null;
    const head = el4("div", "cstate"), facts = el4("div", "ckv");
    const scanRow = el4("div", "row"), scanBtn = elT2("button", "ctl", "wifi.scan.btn"), scanMsg = el4("span", "mut grow");
    scanMsg.setAttribute("role", "status");
    scanRow.append(scanBtn, scanMsg);
    const scanList = el4("ul", "wscan");
    scanList.hidden = true;
    const nameIn = el4("input", "ctl"), pwIn = el4("input", "ctl"), pwBtn = elT2("button", "ctl", "wifi.pw.show");
    nameIn.type = "text";
    nameIn.autocomplete = "off";
    nameIn.spellcheck = false;
    nameIn.setAttribute("autocapitalize", "off");
    nameIn.setAttribute("autocorrect", "off");
    pwIn.type = "password";
    pwIn.autocomplete = "off";
    pwIn.spellcheck = false;
    pwIn.setAttribute("autocapitalize", "off");
    pwIn.setAttribute("autocorrect", "off");
    pwIn.setAttribute("data-i18n-placeholder", "wifi.pw.ph");
    pwIn.placeholder = t6("wifi.pw.ph");
    pwBtn.setAttribute("aria-pressed", "false");
    const lab = (key, ...kids) => {
      const l = el4("label", "wlab");
      l.append(elT2("span", "mut", key), ...kids);
      return l;
    };
    const pwRow = el4("div", "wpw");
    pwRow.append(pwIn, pwBtn);
    const note = elT2("div", "mut cnote", "wifi.insecure");
    const limits = elT2("div", "mut cnote help", "wifi.limits");
    const joinBtn = elT2("button", "ctl primary", "wifi.join"), forgetBtn = elT2("button", "ctl", "wifi.forget");
    const act = el4("div", "row");
    act.style.marginTop = ".7rem";
    act.append(joinBtn, forgetBtn);
    const msgEl = el4("div", "cmsg");
    msgEl.setAttribute("role", "status");
    box.textContent = "";
    box.append(head, facts, scanRow, scanList, lab("wifi.name.label", nameIn), lab("wifi.pw.label", pwRow), note, limits, act, msgEl);
    scanRow.style.marginTop = ".8rem";
    function setMsg(key, params, kind) {
      S.msg = key ? { key, params, kind } : null;
      msgEl.className = "cmsg" + (kind ? " " + kind : "");
      if (key) setT4(msgEl, key, params);
      else setRaw4(msgEl, "");
    }
    const bars = (level) => {
      const s = el4("span", "sig");
      s.setAttribute("aria-hidden", "true");
      for (let i = 1; i <= 4; i++) s.append(el4("i", i <= level ? "on" : ""));
      return s;
    };
    function render3() {
      const on2 = isOn(), p = S.info;
      facts.textContent = "";
      head.className = "cstate";
      if (!p) setT4(head, on2 ? "wifi.reading" : "card.wait");
      else {
        const k2 = stateKey(p.state);
        if (I2.has(k2)) setT4(head, k2);
        else setRaw4(head, p.state);
        if (/wrong|not found/i.test(p.state)) head.classList.add("bad");
        const row2 = (key, fill) => {
          facts.append(elT2("span", "", key));
          const v = el4("span");
          fill(v);
          facts.append(v);
        };
        if (p.ssid) row2("wifi.k.net", (v) => setRaw4(v, p.ssid));
        if (p.state === "connected" && p.rssi !== null) row2("wifi.k.signal", (v) => {
          const lv = sigLevel(p.rssi);
          v.append(bars(lv));
          const w = el4("span");
          setT4(w, "wifi.sig.fmt", { q: { t: "wifi.sig." + lv }, rssi: p.rssi });
          v.append(w);
        });
        if (p.state === "connected" && p.ip && p.ip !== "0.0.0.0") row2("wifi.k.ip", (v) => setRaw4(v, p.ip));
        if (p.src !== "none") row2("wifi.k.src", (v) => setT4(v, "wifi.src." + p.src));
      }
      scanBtn.disabled = !on2 || S.scan.busy;
      setT4(scanBtn, S.scan.busy ? "wifi.scan.busyBtn" : "wifi.scan.btn");
      joinBtn.disabled = !on2 || S.joining;
      forgetBtn.disabled = !on2 || S.joining || !!p && p.src === "none";
      pwBtn.disabled = false;
      for (const e of [nameIn, pwIn]) e.disabled = !on2;
    }
    function renderScan() {
      const sc = S.scan;
      if (sc.busy) setT4(scanMsg, "wifi.scan.running");
      else if (sc.msg) setT4(scanMsg, sc.msg.key, sc.msg.params);
      else setRaw4(scanMsg, "");
      scanList.hidden = !(sc.aps && sc.aps.length);
      scanList.textContent = "";
      for (const ap of sc.aps || []) {
        const li = el4("li");
        li.tabIndex = 0;
        li.className = "wap";
        const nm = el4("span", "nm");
        nm.textContent = ap.ssid || "\xA0";
        li.append(bars(sigLevel(ap.rssi)), nm);
        if (ap.secured) {
          const k2 = el4("span", "lock", "\u{1F512}");
          k2.setAttribute("role", "img");
          k2.setAttribute("data-i18n-aria", "wifi.scan.secured");
          k2.setAttribute("aria-label", t6("wifi.scan.secured"));
          li.append(k2);
        } else li.append(elT2("span", "rtag", "wifi.scan.open"));
        li.onclick = () => {
          nameIn.value = ap.ssid;
          pwIn.value = "";
          setMsg(null);
          (ap.secured ? pwIn : joinBtn).focus({ preventScroll: true });
        };
        scanList.append(li);
      }
      render3();
    }
    function refresh() {
      if (refreshing) return refreshing;
      refreshing = (async () => {
        try {
          if (!isOn()) return;
          const p = parseWifi(await send2("wifi"));
          if (p) S.info = p;
        } finally {
          refreshing = null;
          render3();
        }
      })();
      return refreshing;
    }
    const load = () => refresh();
    async function scan() {
      if (S.scan.busy || !isOn()) return;
      S.scan.busy = true;
      S.scan.msg = null;
      o.onBusy(true);
      renderScan();
      let r = [];
      try {
        r = await sendUntil2("wifi scan", /^wifi scan:/, 1e4);
      } finally {
        S.scan.busy = false;
        o.onBusy(false);
      }
      const p = parseScan(r);
      if (p.status === "found") {
        S.scan.aps = p.aps;
        S.scan.msg = p.aps.length ? { key: "wifi.scan.found", params: { n: p.aps.length } } : { key: "wifi.scan.none" };
      } else if (p.status === "busy") S.scan.msg = { key: "wifi.scan.isBusy" };
      else if (p.status === "failed") S.scan.msg = { key: "wifi.scan.failed" };
      else S.scan.msg = { key: "wifi.scan.noreply" };
      renderScan();
    }
    scanBtn.onclick = scan;
    async function join() {
      if (S.joining || !isOn()) return;
      const ssid = nameIn.value, pw = pwIn.value, bad = validate(ssid, pw);
      if (bad) {
        setMsg(bad.key, bad.params, "bad");
        return;
      }
      S.joining = true;
      render3();
      const tok = ++S.tok;
      const reply = send2("wifi set " + ssid + "|" + pw);
      pwIn.value = "";
      hidePw();
      setMsg("wifi.set.sending", null, "");
      let r;
      try {
        r = await reply;
      } finally {
        S.joining = false;
      }
      const line2 = r.find((l) => /^wifi set:/.test(l)) || "";
      let m;
      if (/^wifi set: ok/.test(line2)) {
        render3();
        await watch(ssid, tok);
        return;
      }
      if (m = /^wifi set: refused\s*(.*)$/.exec(line2)) {
        const k2 = whyKey(m[1]);
        setMsg("wifi.refused", { why: k2 ? { t: k2 } : m[1] || "?" }, "bad");
      } else setMsg("wifi.set.noreply", null, "bad");
      refresh();
    }
    async function watch(ssid, tok) {
      const t0 = Date.now();
      let sawConnecting = false;
      setMsg("wifi.set.trying", { s: 20 }, "");
      while (Date.now() - t0 < 2e4) {
        await sleep2(1e3);
        if (tok !== S.tok || !isOn()) return;
        await refresh();
        const p = S.info, el0 = Date.now() - t0;
        if (tok !== S.tok) return;
        if (p && p.ssid === ssid) {
          if (p.state === "connecting") sawConnecting = true;
          if (p.state === "connected") {
            setMsg("wifi.set.ok", { ssid }, "good");
            return;
          }
          if ((sawConnecting || el0 >= 3e3) && p.state === "wrong password") {
            setMsg("wifi.set.wrong", null, "bad");
            return;
          }
          if ((sawConnecting || el0 >= 3e3) && p.state === "not found") {
            setMsg("wifi.set.notfound", { ssid }, "bad");
            return;
          }
        }
        setMsg("wifi.set.trying", { s: Math.max(0, 20 - Math.round(el0 / 1e3)) }, "");
      }
      if (tok === S.tok) setMsg("wifi.set.timeout", null, "");
    }
    async function forget() {
      if (S.joining || !isOn()) return;
      if (!globalThis.confirm(t6("wifi.forget.confirm"))) return;
      S.tok++;
      const r = await send2("wifi forget");
      if (r.some((l) => /^wifi forget: ok/.test(l))) setMsg("wifi.forget.done", null, "good");
      else setMsg("wifi.forget.fail", null, "bad");
      refresh();
    }
    function hidePw() {
      pwIn.type = "password";
      pwBtn.setAttribute("aria-pressed", "false");
      setT4(pwBtn, "wifi.pw.show");
    }
    pwBtn.onclick = () => {
      const show = pwIn.type === "password";
      pwIn.type = show ? "text" : "password";
      pwBtn.setAttribute("aria-pressed", String(show));
      setT4(pwBtn, show ? "wifi.pw.hide" : "wifi.pw.show");
    };
    joinBtn.onclick = join;
    forgetBtn.onclick = forget;
    for (const e of [nameIn, pwIn]) e.onkeydown = (ev) => {
      if (ev.key === "Enter") {
        ev.preventDefault();
        join();
      }
    };
    function onState(st2) {
      const w = st2 ? st2.wifi : void 0;
      if (st2 && w !== S.lastWifi) {
        const first2 = S.lastWifi === void 0;
        S.lastWifi = w;
        if (!first2 && isOn() && !S.scan.busy) refresh();
      }
      render3();
    }
    function reset() {
      S.info = null;
      S.tok++;
      S.joining = false;
      S.lastWifi = void 0;
      S.scan = { busy: false, aps: null, msg: null };
      o.onBusy(false);
      setMsg(null);
      renderScan();
    }
    timer2 = setInterval(() => {
      if (isOn() && tabOpen() && !document.hidden && !S.scan.busy && !S.joining) refresh();
    }, 1e4);
    render3();
    renderScan();
    return { load, onState, reset, rerender: () => {
      render3();
    }, info: () => S.info };
  }
  var sleep2, unesc, RE_WIFI, stateKey;
  var init_wifi = __esm({
    "src/js/lib/wifi.js"() {
      init_i18n();
      sleep2 = (ms) => new Promise((r) => setTimeout(r, ms));
      unesc = (s) => s.replace(/\\(["\\])/g, "$1");
      RE_WIFI = /^wifi (.+?) ip=(\S*) rssi=(-?\d+|\S*) time=(.*?) src=(nvs|card|none) ssid="(.*)"$/;
      stateKey = (s) => "wifi.state." + String(s).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
    }
  });

  // src/js/core/status.js
  async function refreshStatus() {
    if (app.scanBusy) return;
    const r = await send("st");
    try {
      app.state = JSON.parse(r.find((l) => l.startsWith("{")));
    } catch {
      return;
    }
    emit("status", app.state);
  }
  var init_status = __esm({
    "src/js/core/status.js"() {
      init_app();
      init_link();
    }
  });

  // src/js/lib/fw.js
  function kv(s) {
    const o = {}, re = /(\w+)=(?:"([^"]*)"|(\S+))/g;
    let m;
    while (m = re.exec(s)) o[m[1]] = m[2] !== void 0 ? m[2] : m[3];
    return o;
  }
  function parseVer(lines) {
    for (const l of lines) {
      const m = /^ver\s+(\S+)(.*)$/.exec(l);
      if (!m) continue;
      const o = kv(m[2]);
      return { version: m[1], idf: o.idf || "", chip: o.chip || "", built: o.built || "" };
    }
    return null;
  }
  function parseHw(lines) {
    const r = { sd: null, sdmmc: null, led: null, reserved: "", gpios: "", pending: 0 };
    let seen = false;
    for (const l of lines) {
      let m;
      if (m = /^hw (sd|sdmmc|led)\s+(.*)$/.exec(l)) {
        r[m[1]] = numify(kv(m[2]));
        seen = true;
      } else if (m = /^hw reserved\s*(.*)$/.exec(l)) {
        r.reserved = m[1].trim();
        seen = true;
      } else if (m = /^hw gpios\s*(.*)$/.exec(l)) {
        r.gpios = m[1].trim();
        seen = true;
      } else if (m = /^hw pending\s+(\d+)/.exec(l)) {
        r.pending = +m[1];
        seen = true;
      }
    }
    if (r.led) r.led.on = r.led.on === void 0 ? 1 : +r.led.on;
    if (r.sd && !r.sd.mode) r.sd.mode = r.sd.mosi !== void 0 ? "spi" : "sdmmc";
    return seen ? r : null;
  }
  function parseRatetest(lines) {
    let r = null;
    for (const l of lines) {
      let m;
      if (m = /^ratetest state=(\w+)(.*)$/.exec(l)) {
        const o = kv(m[2]);
        r = { state: m[1], dac: o.dac || "", now: o.now ? +o.now : 0, reason: o.reason || "", rates: [], max: null };
      } else if (r && (m = /^ratetest rate=(\d+)(.*)$/.exec(l))) {
        const o = kv(m[2]);
        r.rates.push({ rate: +m[1], set: o.set || "na", clock: +o.clock || 0, xferOk: +o.xfer_ok || 0, xferBad: +o.xfer_bad || 0, fps: +o.fps || 0, verdict: VERDICTS.includes(o.verdict) ? o.verdict : "untested" });
      } else if (r && (m = /^ratetest max=(\d+)/.exec(l))) r.max = +m[1];
    }
    return r;
  }
  function parseRatetestReply(lines) {
    for (const l of lines) {
      let m;
      if (m = /^ratetest: refused\s*(.*)$/.exec(l)) return { refused: m[1].trim() || "?" };
      if (/^ratetest: (started|cancelled)/.test(l)) return { ok: true };
      if (/^ratetest state=/.test(l)) return { ok: true, running: true };
    }
    return null;
  }
  function playError(lines) {
    for (const l of lines) {
      const m = /^play:\s*(\S+)/.exec(l);
      if (!m) continue;
      if (m[1] === "ESP_OK") return null;
      return { code: m[1], key: PLAY_ERR[m[1]] || "play.err.other" };
    }
    return null;
  }
  var numify, VERDICTS, PLAY_ERR, khz, inZip, isZipEntry;
  var init_fw = __esm({
    "src/js/lib/fw.js"() {
      numify = (o) => {
        for (const k2 of Object.keys(o)) if (/^-?\d+$/.test(o[k2])) o[k2] = +o[k2];
        return o;
      };
      VERDICTS = ["ok", "bad", "untested"];
      PLAY_ERR = {
        ESP_ERR_INVALID_SIZE: "play.err.size",
        ESP_ERR_NOT_FOUND: "play.err.notfound",
        ESP_ERR_TIMEOUT: "play.err.timeout",
        ESP_ERR_INVALID_STATE: "play.err.busy",
        ESP_ERR_NO_MEM: "play.err.nomem",
        ESP_ERR_NOT_SUPPORTED: "play.err.notsup"
      };
      khz = (hz) => String(+(hz / 1e3).toFixed(1));
      inZip = (p) => /\.zip(\/|$)/i.test(p);
      isZipEntry = (name) => /\.zip\/$/i.test(name);
    }
  });

  // src/js/pages/music.js
  function playMsg(err) {
    const m = $("playMsg");
    clearTimeout(msgTimer);
    if (!err) {
      m.textContent = "";
      m.removeAttribute("data-i18n");
      m.removeAttribute("data-i18n-p");
      return;
    }
    setT2(m, err.key, { code: err.code });
    msgTimer = setTimeout(() => playMsg(null), 8e3);
  }
  async function play(path) {
    playMsg(null);
    const r = await send("play " + path);
    playMsg(playError(r));
    refreshStatus();
  }
  function playDirLabel() {
    setT2($("btnPlayDir").lastElementChild, inZip(app.cwd) ? "files.playZip" : "files.playDir");
  }
  async function ls(dir) {
    const prev = app.cwd;
    app.cwd = dir.replace(/\/+$/, "") || "/";
    $("cwd").textContent = app.cwd;
    playDirLabel();
    playMsg(null);
    const lines = await send("ls " + app.cwd);
    const ul = $("list");
    if (app.link !== "on" && !lines.length) return;
    const keep = app.cwd === prev ? ul.scrollTop : 0;
    ul.textContent = "";
    const items = lines.filter((l) => l && l !== "ls: cannot open" && l !== "?").sort((a, b) => b.endsWith("/") - a.endsWith("/") || a.localeCompare(b));
    for (const name of items) {
      const li = document.createElement("li"), isDir = name.endsWith("/"), zip = isZipEntry(name);
      li.tabIndex = 0;
      li.dataset.name = name;
      if (zip) li.classList.add("zip");
      li.textContent = (zip ? "\u{1F4E6} " : isDir ? "\u{1F4C1} " : "\u266A ") + (isDir ? name.slice(0, -1) : name);
      li.onclick = () => isDir ? ls(app.cwd + "/" + name.slice(0, -1)) : play(app.cwd + "/" + name);
      ul.appendChild(li);
    }
    if (!items.length) ul.appendChild(emptyLi("files.empty"));
    ul.scrollTop = keep;
  }
  function initMusic() {
    $("btnUp").onclick = () => {
      const p = app.cwd.split("/");
      if (p.length > 2) {
        p.pop();
        ls(p.join("/"));
      }
    };
    $("btnPlayDir").onclick = () => play(app.cwd);
    segBind("mode", (v) => send("mode " + v));
    segBind("rg", (v) => send("rg " + v));
    $("gapless").onchange = async () => {
      app.gapUntil = Date.now() + 3e3;
      await send("gapless " + ($("gapless").checked ? "on" : "off"));
      app.gapUntil = 0;
      refreshStatus();
    };
    on("status", (state) => {
      segSet("mode", MODES[state.mode] || "folder");
      if (typeof state.rg === "number") segSet("rg", RGS[state.rg] || "off");
      for (const li of $("list").children) li.classList.toggle("cur", !!state.file && li.dataset.name === state.file && state.st !== "idle");
    });
  }
  var msgTimer;
  var init_music = __esm({
    "src/js/pages/music.js"() {
      init_app();
      init_link();
      init_status();
      init_fw();
      msgTimer = 0;
    }
  });

  // src/js/core/bar.js
  function renderVolFor() {
    const outs = ["dac", "bt", "cast"], state = app.state;
    const k2 = state && outs.includes(state.out) ? state.out : outs.includes(app.selOut) ? app.selOut : "";
    const hide = !k2 || app.link !== "on";
    $("volFor").hidden = hide;
    $("cbOut").hidden = hide;
    if (k2) {
      setT2($("volFor"), "vol.for." + k2);
      setIcon($("cbOutIc"), k2);
      setT2($("cbOutTxt"), { dac: "out.dac", bt: "out.btShort", cast: "out.castShort" }[k2]);
      $("cbOut").setAttribute("data-i18n-title", "vol.for." + k2);
      $("cbOut").title = t2("vol.for." + k2);
    }
  }
  function renderVolume() {
    const state = app.state;
    if (!state) return;
    if (typeof state.vmax === "number" && Date.now() > app.vmaxUntil) {
      $("vol").max = state.vmax;
      $("vmax").value = state.vmax;
      $("vmaxv").textContent = state.vmax;
    }
    if (document.activeElement !== $("vol")) {
      $("vol").value = state.vol;
      $("volv").textContent = state.vol;
    }
    if (typeof state.gapless === "number" && Date.now() > app.gapUntil) $("gapless").checked = !!state.gapless;
    renderVolFor();
  }
  function renderTitle() {
    const state = app.state;
    if (!state) {
      setRaw2($("cbTitle"), "\u2014");
      setT2($("cbSub"), "meta.off");
      return;
    }
    $("cbTitle").textContent = state.title || state.file || "\u2014";
    setRaw2($("cbSub"), [known("st.", state.st), state.artist].filter(Boolean).join(" \xB7 "));
  }
  function renderPlay() {
    const playing = !!app.state && app.state.st === "playing";
    setIcon($("btnPlay").firstElementChild, playing ? "pause" : "play");
    setAria($("btnPlay"), app.state ? playing ? "tr.pauseOnly" : "tr.playOnly" : "tr.play");
  }
  function initBar() {
    $("btnPlay").onclick = async () => {
      const state = app.state;
      if (!state) return;
      if (state.st === "playing") await send("pause");
      else if (state.st === "paused") await send("resume");
      else if (state.st === "idle") {
        if (!state.bm) {
          flashHint();
          return;
        }
        const r = await send("resume");
        if (r.some((l) => /nothing to resume/i.test(l))) flashHint();
      } else return;
      refreshStatus();
    };
    $("btnStop").onclick = async () => {
      await send("stop");
      refreshStatus();
    };
    $("btnNext").onclick = async () => {
      await send("next");
      refreshStatus();
    };
    $("btnPrev").onclick = async () => {
      await send("prev");
      refreshStatus();
    };
    $("vol").oninput = () => {
      $("volv").textContent = $("vol").value;
    };
    $("vol").onchange = () => send("vol " + $("vol").value);
    on("status", () => {
      renderTitle();
      renderPlay();
    });
    on("status", () => renderVolume());
    on("linkoff", () => {
      renderTitle();
      renderPlay();
      renderVolFor();
    });
    on("lang", () => {
      renderTitle();
      renderVolFor();
    });
  }
  var setAria;
  var init_bar = __esm({
    "src/js/core/bar.js"() {
      init_app();
      init_link();
      init_status();
      init_now();
      init_icons();
      setAria = (el4, key) => {
        el4.setAttribute("data-i18n-aria", key);
        el4.setAttribute("aria-label", t2(key));
      };
    }
  });

  // src/vendor/tunetap/viz/pixel.js
  function rgb(r, g2, b) {
    return (4278190080 | (b & 255) << 16 | (g2 & 255) << 8 | r & 255) >>> 0;
  }
  function mix(a, b, t6) {
    return rgb(
      Math.trunc((red(a) * (255 - t6) + red(b) * t6) / 255),
      Math.trunc((green(a) * (255 - t6) + green(b) * t6) / 255),
      Math.trunc((blue(a) * (255 - t6) + blue(b) * t6) / 255)
    );
  }
  function tone(c, t6, light) {
    if (light) {
      return rgb(Math.trunc((255 - red(c)) * t6 / 255), Math.trunc((255 - green(c)) * t6 / 255), Math.trunc((255 - blue(c)) * t6 / 255));
    }
    return rgb(Math.trunc(red(c) * t6 / 255), Math.trunc(green(c) * t6 / 255), Math.trunc(blue(c) * t6 / 255));
  }
  function blend(dst, layer, light) {
    if (light) {
      return rgb(Math.max(red(dst) - red(layer), 0), Math.max(green(dst) - green(layer), 0), Math.max(blue(dst) - blue(layer), 0));
    }
    return rgb(Math.min(red(dst) + red(layer), 255), Math.min(green(dst) + green(layer), 255), Math.min(blue(dst) + blue(layer), 255));
  }
  function blit(ctx2d, canvas) {
    const { width: w, height: h } = canvas;
    if (!ctx2d || !w || !h) return;
    let img = images.get(ctx2d);
    if (!img || img.width !== w || img.height !== h) {
      img = ctx2d.createImageData(w, h);
      images.set(ctx2d, img);
    }
    img.data.set(new Uint8ClampedArray(canvas.px32.buffer, canvas.px32.byteOffset, w * h * 4));
    ctx2d.putImageData(img, 0, 0);
  }
  var red, green, blue, Canvas, images;
  var init_pixel = __esm({
    "src/vendor/tunetap/viz/pixel.js"() {
      red = (c) => c & 255;
      green = (c) => c >>> 8 & 255;
      blue = (c) => c >>> 16 & 255;
      Canvas = class {
        // px: an optional Uint32Array of at least width * height pixels; a new one otherwise.
        constructor(width, height, px) {
          this.width = Math.max(0, width | 0);
          this.height = Math.max(0, height | 0);
          this.px32 = px ?? new Uint32Array(this.width * this.height);
          if (this.px32.length < this.width * this.height) throw new RangeError(`a ${width}x${height} canvas needs ${width * height} pixels`);
          this.y0 = 0;
          this.y1 = this.height;
        }
        // The rows y0..y1 are the page's; clamped to the buffer.
        setRows(y0, y1) {
          this.y0 = Math.min(Math.max(y0, 0), this.height);
          this.y1 = Math.min(Math.max(y1, this.y0), this.height);
        }
        inside(x, y) {
          return x >= 0 && x < this.width && y >= this.y0 && y < this.y1;
        }
        // The page's rows, all of one colour.
        clear(c) {
          this.px32.fill(c, this.y0 * this.width, this.y1 * this.width);
        }
        // One pixel.
        px(x, y, c) {
          if (this.inside(x, y)) this.px32[y * this.width + x] = c;
        }
        // Row y whole, as a view into the pixels (chipchill row_mut), if it is one of the page's rows;
        // null otherwise.
        rowMut(y) {
          if (y < this.y0 || y >= this.y1) return null;
          const at = y * this.width;
          return this.px32.subarray(at, at + this.width);
        }
        // A filled rectangle (rgxx box / ui_fill).
        fill(x, y, w, h, c) {
          const x0 = Math.max(x, 0), y0 = Math.max(y, this.y0);
          const x1 = Math.min(x + w, this.width), y1 = Math.min(y + h, this.y1);
          if (x1 <= x0 || y1 <= y0) return;
          for (let yy = y0; yy < y1; yy++) {
            const at = yy * this.width;
            this.px32.fill(c, at + x0, at + x1);
          }
        }
        // A one-pixel outline (ui_frame).
        frame(x, y, w, h, c) {
          this.fill(x, y, w, 1, c);
          this.fill(x, y + h - 1, w, 1, c);
          this.fill(x, y, 1, h, c);
          this.fill(x + w - 1, y, 1, h, c);
        }
        // One row of pixels (hspan).
        hspan(x, y, w, c) {
          if (y < this.y0 || y >= this.y1) return;
          if (x < 0) {
            w += x;
            x = 0;
          }
          if (x + w > this.width) w = this.width - x;
          if (w > 0) {
            const at = y * this.width + x;
            this.px32.fill(c, at, at + w);
          }
        }
        // A filled circle of radius r.
        disc(cx, cy, r, c) {
          for (let y = -r; y <= r; y++) {
            const h = Math.trunc(Math.sqrt(r * r - y * y));
            this.fill(cx - h, cy + y, 2 * h + 1, 1, c);
          }
        }
        // A circle's outline, as 720 points.
        ring(cx, cy, r, c) {
          for (let a = 0; a < 720; a++) {
            const t6 = a * Math.PI / 360;
            this.px(cx + Math.round(r * Math.cos(t6)), cy + Math.round(r * Math.sin(t6)), c);
          }
        }
        // A line of thickness w.
        line(x0, y0, x1, y1, w, c) {
          const dx = x1 - x0, dy = y1 - y0;
          const n = Math.max(Math.abs(dx) > Math.abs(dy) ? Math.abs(dx) : Math.abs(dy), 1);
          for (let i = 0; i <= n; i++) {
            const x = x0 + Math.trunc(dx * i / n), y = y0 + Math.trunc(dy * i / n);
            if (w <= 1) {
              this.px(x, y, c);
            } else if (w === 2) {
              this.px(x - 1, y - 1, c);
              this.px(x, y - 1, c);
              this.px(x - 1, y, c);
              this.px(x, y, c);
            } else {
              const h = Math.trunc(w / 2);
              this.fill(x - h, y - h, w, w, c);
            }
          }
        }
        // Light added to what is there (beams, glows), saturating.
        add(x, y, r, g2, b) {
          if (!this.inside(x, y)) return;
          const i = y * this.width + x;
          const v = this.px32[i];
          this.px32[i] = rgb(Math.min(red(v) + r, 255), Math.min(green(v) + g2, 255), Math.min(blue(v) + b, 255));
        }
        // A layer's value (tone) onto one pixel: added, or subtracted on a light palette.
        blend(x, y, layer, light) {
          if (!this.inside(x, y)) return;
          const i = y * this.width + x;
          this.px32[i] = blend(this.px32[i], layer, light);
        }
        // Every other row of a rectangle darkened by strength (0..255), starting with its second
        // (rg_scanlines).
        scanlines(x, y, w, h, strength) {
          if (x < 0) {
            w += x;
            x = 0;
          }
          if (y < 0) {
            h += y;
            y = 0;
          }
          if (x + w > this.width) w = this.width - x;
          if (y + h > this.height) h = this.height - y;
          const k2 = (v) => Math.trunc(v * (255 - strength) / 255);
          for (let r = y + 1; r < y + h; r += 2) {
            if (r < this.y0 || r >= this.y1) continue;
            for (let i = 0; i < Math.max(w, 0); i++) {
              const at = r * this.width + x + i;
              const v = this.px32[at];
              this.px32[at] = rgb(k2(red(v)), k2(green(v)), k2(blue(v)));
            }
          }
        }
        // A panel with its corners cut and a one-pixel edge, the diagonals two thick (rg_cut_panel).
        cutPanel(x, y, w, h, cut, fill, edge) {
          for (let r = 0; r < h; r++) {
            const l = r < cut ? cut - r : 0;
            const rr = r > h - 1 - cut ? r - (h - 1 - cut) : 0;
            const x0 = x + l, x1 = x + w - 1 - rr;
            if (x1 < x0) continue;
            if (r === 0 || r === h - 1) {
              this.fill(x0, y + r, x1 - x0 + 1, 1, edge);
              continue;
            }
            this.fill(x0 + 1, y + r, x1 - x0 - 1, 1, fill);
            this.fill(x0, y + r, 1, 1, edge);
            this.fill(x1, y + r, 1, 1, edge);
            if (r < cut) this.fill(x0 + 1, y + r, 1, 1, edge);
            if (r > h - 1 - cut) this.fill(x1 - 1, y + r, 1, 1, edge);
          }
        }
      };
      images = /* @__PURE__ */ new WeakMap();
    }
  });

  // src/vendor/tunetap/viz/page.js
  function stage(c) {
    const y0 = c.y0, y1 = c.y1, w = c.width;
    if (y1 - y0 <= w) return [y0, y1];
    const top = y0 + Math.trunc((y1 - y0 - w) / 2);
    return [top, top + w];
  }
  function backdrop(c, t6) {
    c.clear(t6.bg);
    for (let y = 8; y < c.height; y += 16) for (let x = 8; x < c.width; x += 16) c.px(x, y, t6.grid);
  }
  function textScale(s) {
    return Math.max(Math.trunc(s + 0.25), 1);
  }
  var Fit;
  var init_page = __esm({
    "src/vendor/tunetap/viz/page.js"() {
      Fit = class _Fit {
        constructor(num3, den) {
          this.num = Math.max(num3, 1);
          this.den = Math.max(den, 1);
        }
        // num / den; a size of nothing is taken as a pixel.
        static new(num3, den) {
          return new _Fit(num3, den);
        }
        // The smaller of two: what fits both ways.
        static min(a, b) {
          return a.num * b.den <= b.num * a.den ? a : b;
        }
        // A length or an offset of rgxx's, at this size (towards zero).
        i(v) {
          return Math.trunc(v * this.num / this.den);
        }
        // …and at least a pixel, for strokes.
        px(v) {
          return Math.max(this.i(v), 1);
        }
        // Back: a length on the canvas in rgxx's pixels.
        inv(v) {
          return Math.trunc(v * this.den / this.num);
        }
        // As a number to multiply by: exactly 1 at rgxx's size.
        f() {
          return this.num / this.den;
        }
      };
    }
  });

  // src/vendor/tunetap/viz/colour.js
  function palOf(t6, duo1Step, duo2Step) {
    return { bg: t6.bg, panel: t6.panel, fg: t6.fg, a1: t6.cyan, a2: t6.mag, a3: t6.yel, ok: t6.lime, purp: t6.purp, light: !!t6.light, duo1: duo1Step * 10, duo2: duo2Step * 10 };
  }
  function samePal(a, b) {
    if (!a || !b) return false;
    for (const k2 of ["bg", "panel", "fg", "a1", "a2", "a3", "ok", "purp", "light", "duo1", "duo2"]) if (a[k2] !== b[k2]) return false;
    return true;
  }
  function hsv(h, s, v) {
    h = (Math.trunc(h) % 360 + 360) % 360;
    s = F(s);
    v = F(v);
    const c = F(v * s);
    const x = F(c * F(1 - Math.abs(F(F(F(h / 60) % 2) - 1))));
    const m = F(v - c);
    let r = 0, g2 = 0, b = 0;
    switch (Math.trunc(h / 60)) {
      case 0:
        r = c;
        g2 = x;
        break;
      case 1:
        r = x;
        g2 = c;
        break;
      case 2:
        g2 = c;
        b = x;
        break;
      case 3:
        g2 = x;
        b = c;
        break;
      case 4:
        r = x;
        b = c;
        break;
      default:
        r = c;
        b = x;
        break;
    }
    const q = (u) => Math.round(F(F(u + m) * 255));
    return rgb(q(r), q(g2), q(b));
  }
  function through(s, f) {
    for (let i = 0; i < s.length - 1; i++) {
      if (f <= s[i + 1][0]) {
        const t6 = F(F(f - s[i][0]) / F(s[i + 1][0] - s[i][0]));
        const at = (a, b) => Math.trunc(F(a + F((b - a) * t6)));
        return rgb(at(s[i][1], s[i + 1][1]), at(s[i][2], s[i + 1][2]), at(s[i][3], s[i + 1][3]));
      }
    }
    const l = s[s.length - 1];
    return rgb(l[1], l[2], l[3]);
  }
  function lum(c) {
    return Math.trunc((red(c) * 299 + green(c) * 587 + blue(c) * 114) / 1e3);
  }
  function fitLum(c, target) {
    const l = lum(c);
    target = Math.min(Math.max(target, 0), 255);
    if (l === target) return c;
    if (l > target) {
      const k2 = F(target / Math.max(l, 1));
      return rgb(Math.trunc(F(red(c) * k2)), Math.trunc(F(green(c) * k2)), Math.trunc(F(blue(c) * k2)));
    }
    return mix(c, rgb(255, 255, 255), Math.min(Math.max(Math.trunc((target - l) * 255 / Math.max(255 - l, 1)), 0), 255));
  }
  function ramp(p, r, ch, f) {
    f = F(f);
    f = f > 0 ? Math.min(f, 1) : 0;
    r = rampAt(r);
    let c;
    switch (r) {
      case RAMP.FIRE:
        c = through(p.light ? FIRE_LIGHT : FIRE, f);
        break;
      case RAMP.ICE:
        c = through(p.light ? ICE_LIGHT : ICE, f);
        break;
      case RAMP.WINAMP:
        c = through(p.light ? WINAMP_LIGHT : WINAMP, f);
        break;
      // On paper the rainbow and the two hues darken as they louden, their brightness a straight
      // line from pale to ink whatever the hue.
      case RAMP.RAINBOW:
        c = hsv(Math.trunc(F(280 - F(280 * f))), 0.85, 1);
        if (p.light) c = fitLum(c, paperLum(f));
        break;
      case RAMP.DUO:
        c = mix(hsv(p.duo1, 0.85, 1), hsv(p.duo2, 0.85, 1), Math.trunc(F(f * 255)));
        if (p.light) c = fitLum(c, paperLum(f));
        break;
      case RAMP.MONO:
        c = p.a1;
        break;
      default:
        if (f < 0.5) c = mix(ch ? p.a1 : p.ok, p.purp, Math.trunc(F(F(f * 2) * 255)));
        else if (f < F(0.8)) c = mix(p.purp, p.a2, Math.trunc(F(F(F(f - 0.5) / F(0.3)) * 255)));
        else c = p.a3;
    }
    const k2 = F(lum(p.panel) / 240);
    if (p.light && r !== RAMP.PALETTE && k2 < 1) return fitLum(c, Math.trunc(F(lum(c) * k2)));
    return c;
  }
  function map(m, i, n, k2, v) {
    switch (m) {
      case MAP.TIP: {
        const d4 = F(F(v - k2) / TIP);
        return F(1 - Math.min(Math.max(d4, 0), 1));
      }
      case MAP.BAND:
        return n > 1 ? F(i / (n - 1)) : 0;
      case MAP.LEVEL:
        return v;
      default:
        return k2;
    }
  }
  function colour(p, r, m, ch, i, n, k2, v) {
    return ramp(p, r, ch, map(m, i, n, k2, v));
  }
  function lut(p, r, ch) {
    const out = new Uint32Array(256);
    for (let i = 0; i < 256; i++) out[i] = ramp(p, r, ch, F(i / 255));
    return out;
  }
  function lutLoud(p, r, ch) {
    const dark = p.light ? p.panel : p.bg;
    const out = new Uint32Array(256);
    for (let i = 0; i < 256; i++) {
      const f = F(i / 255);
      const up = F(f * 1.5) > 1 ? 1 : F(f * 1.5);
      out[i] = mix(dark, ramp(p, r, ch, f), Math.trunc(F(F(Math.pow(up, F(1.4))) * 255)));
    }
    return out;
  }
  var F, RAMP, RAMP_COUNT, MAP, TIP, rampAt, mapAt, mapFlat, st, FIRE, ICE, WINAMP, FIRE_LIGHT, ICE_LIGHT, WINAMP_LIGHT, paperLum;
  var init_colour = __esm({
    "src/vendor/tunetap/viz/colour.js"() {
      init_pixel();
      F = Math.fround;
      RAMP = Object.freeze({ PALETTE: 0, FIRE: 1, ICE: 2, WINAMP: 3, RAINBOW: 4, DUO: 5, MONO: 6 });
      RAMP_COUNT = 7;
      MAP = Object.freeze({ BOTTOM: 0, TIP: 1, BAND: 2, LEVEL: 3 });
      TIP = F(0.42);
      rampAt = (i) => Number.isInteger(i) && i >= 0 && i < RAMP_COUNT ? i : RAMP.PALETTE;
      mapAt = (i) => i === 1 || i === 2 || i === 3 ? i : MAP.BOTTOM;
      mapFlat = (m) => m === MAP.BAND || m === MAP.LEVEL;
      st = (f, r, g2, b) => Object.freeze([F(f), r, g2, b]);
      FIRE = [st(0, 64, 0, 0), st(0.4, 196, 16, 0), st(0.75, 255, 116, 0), st(1, 255, 226, 96)];
      ICE = [st(0, 0, 16, 72), st(0.45, 0, 110, 220), st(0.8, 90, 230, 255), st(1, 235, 255, 255)];
      WINAMP = [st(0, 24, 176, 40), st(0.55, 230, 220, 30), st(0.8, 255, 140, 0), st(1, 255, 40, 30)];
      FIRE_LIGHT = [st(0, 255, 236, 200), st(0.4, 255, 170, 40), st(0.75, 210, 40, 0), st(1, 90, 0, 0)];
      ICE_LIGHT = [st(0, 230, 245, 255), st(0.45, 90, 180, 255), st(0.8, 0, 90, 200), st(1, 0, 20, 80)];
      WINAMP_LIGHT = [st(0, 170, 230, 130), st(0.55, 190, 170, 0), st(0.8, 220, 100, 0), st(1, 150, 0, 10)];
      paperLum = (f) => Math.trunc(F(225 - F(190 * f)));
    }
  });

  // src/vendor/tunetap/viz/themes.js
  function build(name, colours, light) {
    const t6 = { name, light };
    FIELDS.forEach((f, i) => {
      t6[f] = rgb(colours[i][0], colours[i][1], colours[i][2]);
    });
    return Object.freeze(t6);
  }
  function themeNamed(name) {
    return BY_NAME.get(name) ?? null;
  }
  function parse(s) {
    if (typeof s !== "string") return null;
    s = s.trim().toLowerCase();
    let m = /^#([0-9a-f]{3})$/.exec(s);
    if (m) return [...m[1]].map((h) => parseInt(h + h, 16));
    m = /^#([0-9a-f]{6})$/.exec(s);
    if (m) return [0, 2, 4].map((i) => parseInt(m[1].slice(i, i + 2), 16));
    m = /^rgba?\(\s*(\d+)[\s,]+(\d+)[\s,]+(\d+)/.exec(s);
    if (m) return [1, 2, 3].map((i) => Math.min(255, +m[i]));
    return null;
  }
  function hsvOf([r, g2, b]) {
    const mx = Math.max(r, g2, b), mn = Math.min(r, g2, b), d4 = mx - mn;
    let h = 0;
    if (d4) {
      if (mx === r) h = 60 * ((g2 - b) / d4 % 6);
      else if (mx === g2) h = 60 * ((b - r) / d4 + 2);
      else h = 60 * ((r - g2) / d4 + 4);
    }
    return [(h + 360) % 360, mx ? d4 / mx : 0, mx / 255];
  }
  function themeFromTokens(tokens) {
    const get = (k2) => {
      try {
        if (typeof tokens === "function") return parse(tokens(k2));
        if (tokens && typeof tokens.getPropertyValue === "function") return parse(tokens.getPropertyValue(k2));
      } catch {
      }
      return null;
    };
    const bg = get("--bg");
    if (!bg) return themeNamed("neon");
    const c = (v) => rgb(v[0], v[1], v[2]);
    const light = lum(c(bg)) > 128;
    const base = themeNamed(light ? "paper" : "neon");
    const t6 = { name: "app", light };
    for (const f of FIELDS) t6[f] = base[f];
    t6.bg = c(bg);
    const card = get("--card"), line2 = get("--line"), fg = get("--fg"), muted = get("--muted"), accent = get("--accent");
    if (card) t6.panel = c(card);
    if (line2) t6.grid = c(line2);
    t6.panel2 = mix(t6.panel, t6.grid, 128);
    if (fg) {
      t6.fg = c(fg);
      t6.moon = t6.fg;
    }
    if (muted) t6.dim = c(muted);
    t6.faint = mix(t6.bg, t6.dim, 146);
    if (accent) {
      t6.cyan = c(accent);
      const [h, s0, v] = hsvOf(accent);
      const s = Math.max(s0, 0.5);
      t6.purp = hsv(Math.round(h + 73), s, v);
      t6.mag = hsv(Math.round(h + 140), s, v);
      t6.yel = hsv(Math.round(h + 222), s, v);
      t6.lime = hsv(140, s, v);
      t6.warn = hsv(0, s, v);
    }
    t6.black = light ? t6.grid : mix(t6.bg, rgb(0, 0, 0), 128);
    return Object.freeze(t6);
  }
  var FIELDS, RAW, THEMES, THEME_NAMES, BY_NAME;
  var init_themes = __esm({
    "src/vendor/tunetap/viz/themes.js"() {
      init_pixel();
      init_colour();
      FIELDS = Object.freeze(["bg", "grid", "panel", "panel2", "fg", "dim", "faint", "cyan", "mag", "yel", "lime", "purp", "warn", "moon", "black"]);
      RAW = [
        ["neon", [[7, 5, 18], [34, 22, 66], [16, 12, 36], [30, 22, 64], [228, 234, 255], [120, 110, 172], [52, 40, 96], [0, 230, 255], [255, 40, 160], [255, 212, 40], [90, 255, 140], [150, 80, 255], [255, 64, 64], [235, 232, 210], [4, 3, 12]], false],
        ["miku", [[5, 13, 19], [14, 38, 46], [9, 23, 31], [15, 46, 54], [232, 248, 246], [112, 160, 164], [34, 70, 78], [57, 214, 200], [255, 82, 160], [255, 226, 120], [134, 230, 120], [47, 168, 168], [255, 80, 80], [235, 232, 210], [2, 6, 9]], false],
        ["vapor", [[24, 16, 48], [48, 32, 88], [36, 24, 68], [64, 44, 112], [255, 240, 250], [168, 148, 204], [80, 60, 128], [128, 236, 224], [255, 140, 200], [255, 220, 150], [170, 240, 170], [176, 140, 255], [255, 120, 120], [245, 236, 220], [12, 8, 24]], false],
        ["amber", [[12, 6, 0], [40, 22, 0], [24, 13, 0], [56, 32, 0], [255, 200, 96], [176, 112, 24], [88, 52, 0], [255, 176, 0], [255, 120, 0], [255, 224, 140], [255, 176, 0], [255, 148, 0], [255, 80, 32], [255, 224, 140], [6, 3, 0]], false],
        ["phosphor", [[2, 10, 4], [8, 34, 14], [4, 18, 8], [10, 46, 20], [150, 255, 170], [60, 164, 84], [20, 72, 34], [80, 255, 120], [190, 255, 110], [225, 255, 205], [80, 255, 120], [135, 255, 115], [255, 120, 60], [225, 255, 205], [1, 5, 2]], false],
        ["gameboy", [[155, 188, 15], [139, 172, 15], [139, 172, 15], [124, 160, 20], [6, 34, 8], [30, 68, 30], [60, 100, 20], [15, 56, 15], [48, 98, 48], [15, 56, 15], [15, 56, 15], [48, 98, 48], [15, 56, 15], [48, 98, 48], [139, 172, 15]], true],
        ["paper", [[240, 232, 214], [224, 214, 192], [250, 244, 230], [228, 216, 190], [40, 32, 28], [120, 108, 96], [150, 138, 118], [190, 48, 40], [28, 72, 120], [200, 140, 20], [60, 130, 70], [109, 60, 80], [190, 48, 40], [200, 140, 20], [224, 214, 192]], true],
        // Solarized dark: the colours are Ethan Schoonover's (MIT).
        ["solar", [[0, 43, 54], [16, 70, 84], [7, 54, 66], [16, 70, 84], [238, 232, 213], [131, 148, 150], [88, 110, 117], [42, 161, 152], [211, 54, 130], [181, 137, 0], [133, 153, 0], [108, 113, 196], [220, 50, 47], [253, 246, 227], [0, 30, 38]], false],
        // Gruvbox dark: the colours are Pavel Pertsev's (MIT).
        ["gruv", [[40, 40, 40], [60, 56, 54], [50, 48, 47], [80, 73, 69], [235, 219, 178], [168, 153, 132], [102, 92, 84], [254, 128, 25], [131, 165, 152], [250, 189, 47], [184, 187, 38], [211, 134, 155], [251, 73, 52], [235, 219, 178], [29, 32, 33]], false],
        ["unit", [[14, 8, 24], [44, 26, 72], [24, 14, 42], [48, 28, 80], [232, 240, 224], [140, 120, 176], [64, 44, 100], [140, 230, 60], [255, 140, 40], [255, 220, 80], [140, 230, 60], [150, 90, 230], [255, 60, 60], [235, 232, 210], [6, 3, 12]], false],
        ["ink", [[250, 250, 248], [220, 220, 218], [255, 255, 255], [232, 232, 230], [16, 16, 16], [96, 96, 96], [140, 140, 138], [16, 16, 16], [96, 96, 96], [16, 16, 16], [16, 16, 16], [96, 96, 96], [160, 0, 0], [96, 96, 96], [220, 220, 218]], true],
        ["snes", [[36, 36, 48], [52, 52, 68], [56, 56, 72], [74, 74, 94], [236, 236, 244], [178, 178, 194], [120, 120, 138], [138, 127, 214], [224, 58, 62], [247, 201, 72], [59, 170, 90], [75, 63, 158], [224, 58, 62], [46, 119, 208], [18, 18, 24]], false],
        ["sfc", [[30, 32, 40], [46, 48, 60], [54, 56, 70], [72, 74, 92], [240, 240, 244], [180, 180, 192], [120, 120, 136], [200, 16, 46], [0, 112, 192], [250, 200, 0], [0, 160, 80], [120, 80, 180], [200, 16, 46], [0, 112, 192], [16, 16, 22]], false],
        ["caution", [[22, 16, 34], [40, 30, 60], [36, 28, 54], [52, 40, 76], [240, 236, 248], [170, 160, 190], [110, 100, 130], [250, 140, 0], [140, 255, 80], [255, 220, 0], [140, 255, 80], [120, 60, 200], [255, 60, 60], [180, 120, 255], [10, 6, 16]], false],
        ["aqua", [[244, 250, 250], [224, 236, 236], [255, 255, 255], [236, 247, 246], [30, 40, 44], [110, 124, 128], [170, 186, 186], [30, 150, 142], [255, 100, 160], [255, 200, 40], [60, 170, 90], [140, 100, 200], [220, 60, 60], [80, 120, 200], [220, 232, 232]], true],
        ["cozy", [[250, 242, 226], [236, 222, 200], [255, 252, 246], [242, 230, 206], [70, 50, 40], [140, 116, 100], [190, 170, 150], [120, 84, 60], [240, 150, 150], [255, 204, 80], [120, 160, 90], [160, 120, 160], [210, 90, 80], [110, 130, 170], [236, 222, 200]], true],
        ["mono", [[255, 255, 255], [230, 230, 230], [255, 255, 255], [240, 240, 240], [10, 10, 10], [90, 90, 90], [160, 160, 160], [10, 10, 10], [230, 30, 30], [230, 30, 30], [40, 140, 80], [100, 80, 160], [230, 30, 30], [60, 90, 160], [230, 230, 230]], true],
        ["carbon", [[12, 12, 14], [30, 30, 36], [24, 24, 28], [34, 34, 40], [235, 235, 240], [160, 160, 170], [100, 100, 110], [255, 120, 0], [0, 200, 230], [255, 200, 0], [80, 200, 120], [160, 100, 255], [255, 70, 70], [120, 140, 200], [6, 6, 8]], false]
      ];
      THEMES = Object.freeze(RAW.map(([name, colours, light]) => Object.freeze({ name, colours: Object.freeze(colours.map((c) => Object.freeze(c))), light })));
      THEME_NAMES = Object.freeze(THEMES.map((t6) => t6.name));
      BY_NAME = new Map(THEMES.map((t6) => [t6.name, build(t6.name, t6.colours, t6.light)]));
    }
  });

  // src/vendor/tunetap/viz/cfg.js
  function keyDef(k2) {
    return KEYS[k2] ?? null;
  }
  function plainAt(k2, i) {
    const d4 = KEYS[k2];
    if (!d4 || !Number.isInteger(i) || i < 0 || i >= d4.plain) return null;
    let seen = 0;
    for (let v = 0; v < d4.n; v++) {
      if (v === d4.song) continue;
      if (seen++ === i) return v;
    }
    return null;
  }
  function valueLabelKey(k2, i) {
    const d4 = KEYS[k2];
    return d4?.values?.[i] ?? null;
  }
  function songHash(text) {
    if (text === null || text === void 0 || text === "") return 0;
    let h = 2166136261;
    for (const b of new TextEncoder().encode(String(text))) h = Math.imul((h ^ b) >>> 0, 16777619) >>> 0;
    return h || 1;
  }
  function songRoll(seed, salt, n) {
    let h = (seed >>> 0 ^ Math.imul(salt >>> 0, 2654435769)) >>> 0;
    h = Math.imul(h, 2246822507) >>> 0;
    h = (h ^ h >>> 13) >>> 0;
    h = Math.imul(h, 3266489909) >>> 0;
    h = (h ^ h >>> 16) >>> 0;
    return n ? h % n : 0;
  }
  function pick(k2, seed) {
    const d4 = KEYS[k2];
    const salt = saltOf(k2);
    if (!d4.weights) return songRoll(seed, salt, d4.plain);
    const w = d4.weights.slice(0, d4.plain);
    const roll = songRoll(seed, salt, w.reduce((a, b) => a + b, 0));
    let at = 0, p = 0;
    while (p < d4.plain - 1) {
      at += w[p];
      if (roll < at) break;
      p++;
    }
    return p;
  }
  function definePage(spec) {
    const keys = Object.freeze([...spec.keys]);
    const groups = Object.freeze(spec.groups.map((g2) => Object.freeze({ ...g2, keys: Object.freeze([...g2.keys]) })));
    const presets = spec.presets;
    const copy = (c) => {
      const o = {};
      for (const k2 of keys) o[k2] = c[k2];
      return o;
    };
    const isIdx = (k2, v) => Number.isInteger(v) && v >= 0 && v < KEYS[k2].n;
    const page = {
      id: spec.id,
      keys,
      defaults() {
        const o = {};
        for (const k2 of keys) o[k2] = defaultValue(k2);
        return o;
      },
      plain() {
        const o = {};
        for (const k2 of keys) o[k2] = KEYS[k2].was;
        return o;
      },
      validate(obj) {
        const src = obj && typeof obj === "object" && !Array.isArray(obj) ? obj : {};
        const o = {};
        for (const k2 of keys) o[k2] = isIdx(k2, src[k2]) ? src[k2] : defaultValue(k2);
        return o;
      },
      rows(c) {
        return spec.rows(c);
      },
      // A row that does not exist in this combination carries no value of its own: it takes its
      // first plain value, the one the tour walks it at.
      snapAbsent(c) {
        const o = copy(c);
        const rows = spec.rows(o);
        for (const g2 of groups) for (const k2 of g2.keys) if (!rows.includes(k2)) o[k2] = plainAt(k2, 0) ?? 0;
        return o;
      },
      // The effective choices for the station with this seed: every key at 隨電台 takes one of its
      // own plain values, a salted hash of the seed per key; seed 0 takes the plain look.
      resolve(c, seed) {
        const o = page.plain();
        for (const k2 of keys) {
          const d4 = KEYS[k2];
          let v = c?.[k2];
          if (!isIdx(k2, v)) v = defaultValue(k2);
          if (d4.song < 0 || v !== d4.song) {
            o[k2] = v;
            continue;
          }
          o[k2] = seed ? plainAt(k2, pick(k2, seed >>> 0)) ?? d4.was : d4.was;
        }
        return page.snapAbsent(o);
      },
      // presets: the named ones and one more, 隨電台
      presetCount() {
        return presets.rows.length ? presets.rows.length + 1 : 0;
      },
      presetLabelKey(i) {
        const n = presets.rows.length;
        if (!n || !Number.isInteger(i) || i < 0 || i > n) return null;
        return i === n ? "viz.preset.follow" : presets.rows[i][0];
      },
      applyPreset(c, i) {
        if (page.presetLabelKey(i) === null) return copy(c);
        if (i === presets.rows.length) {
          let o2 = copy(c);
          for (let g2 = 0; g2 < groups.length; g2++) o2 = page.tourSet(o2, g2, page.tourCount(g2));
          return o2;
        }
        const o = copy(c);
        presets.keys.forEach((k2, j) => {
          o[k2] = presets.rows[i][1][j];
        });
        return o;
      },
      // The preset these choices are, or -1 (自訂).
      preset(c) {
        if (!presets.rows.length) return -1;
        let any = false, all2 = true;
        for (const g2 of groups) for (const k2 of g2.keys) {
          if (KEYS[k2].song < 0) continue;
          any = true;
          if (c[k2] !== KEYS[k2].song) all2 = false;
        }
        if (any && all2) return presets.rows.length;
        for (let i = 0; i < presets.rows.length; i++) if (presets.keys.every((k2, j) => c[k2] === presets.rows[i][1][j])) return i;
        return -1;
      },
      // The next preset (from 自訂, the first): { cfg, index }.
      stepPreset(c, dir) {
        const n = page.presetCount();
        if (!n) return { cfg: copy(c), index: -1 };
        const cur = page.preset(c);
        const next = cur < 0 ? 0 : (cur + (dir < 0 ? -1 : 1) + n) % n;
        return { cfg: page.applyPreset(c, next), index: next };
      },
      // tours
      groupCount() {
        return groups.length;
      },
      groupId(g2) {
        return groups[g2]?.id ?? null;
      },
      groupKeys(g2) {
        return groups[g2]?.keys ?? [];
      },
      groupLabelKey(g2) {
        return groups[g2]?.labelKey ?? null;
      },
      // How many combinations the tour has, not counting its 隨電台 stop (which is this index).
      tourCount(g2) {
        return walk(page.plain(), g2, -1).found;
      },
      // Combination pos of the group's tour into the choices, every other key left alone; pos ===
      // the count sets the group's keys to 隨電台. Out of range wraps.
      tourSet(c, g2, pos) {
        if (!groups[g2]) return copy(c);
        const n = page.tourCount(g2);
        pos = (pos % (n + 1) + n + 1) % (n + 1);
        if (pos === n) {
          const o = copy(c);
          for (const k2 of groups[g2].keys) if (KEYS[k2].song >= 0) o[k2] = KEYS[k2].song;
          return o;
        }
        return walk(c, g2, pos).out ?? copy(c);
      },
      // Where these choices stand on the tour: the count when every key of the group follows, the
      // combination's index when all are plain and it exists, -1 for a mixed group.
      tourPos(c, g2) {
        if (!groups[g2]) return -1;
        const ks = groups[g2].keys;
        const song = ks.filter((k2) => KEYS[k2].song >= 0 && c[k2] === KEYS[k2].song).length;
        if (song === ks.length) return page.tourCount(g2);
        if (song > 0) return -1;
        const n = page.tourCount(g2);
        for (let pos = 0; pos < n; pos++) {
          const got = walk(c, g2, pos).out;
          if (got && ks.every((k2) => got[k2] === c[k2])) return pos;
        }
        return -1;
      },
      // One press, from these choices with `shown` being what the page actually draws (the resolved
      // cfg), to the next stop: { cfg, pos }. A mixed group first takes the values that are showing.
      tourStep(c, shown, g2) {
        if (!groups[g2]) return { cfg: copy(c), pos: -1 };
        let o = copy(c);
        let pos = page.tourPos(o, g2);
        if (pos < 0) {
          for (const k2 of groups[g2].keys) o[k2] = shown[k2];
          o = page.snapAbsent(o);
          pos = page.tourPos(o, g2);
        }
        o = page.tourSet(o, g2, pos + 1);
        return { cfg: o, pos: page.tourPos(o, g2) };
      }
    };
    function real(c, g2, digit) {
      const rows = spec.rows(c);
      return groups[g2].keys.every((k2, i) => rows.includes(k2) || digit[i] === 0);
    }
    function walk(base, g2, want) {
      const ks = groups[g2]?.keys ?? [];
      if (!ks.length) return { found: 0, out: null };
      const size = ks.map((k2) => KEYS[k2].plain);
      const raw = size.reduce((a, b) => a * b, 1);
      let found = 0;
      for (let r = 0; r < raw; r++) {
        const c = copy(base);
        let rest = r;
        const digit = size.map((s) => {
          const d4 = rest % s;
          rest = Math.trunc(rest / s);
          return d4;
        });
        ks.forEach((k2, i) => {
          c[k2] = plainAt(k2, digit[i]) ?? 0;
        });
        if (!real(c, g2, digit)) continue;
        if (want === found) return { found, out: c };
        found++;
      }
      return { found, out: null };
    }
    return Object.freeze(page);
  }
  var KEY_ORDER, SHAPE, MAPPING, RAMPS, OFF, WF_LAYOUT, WF_DIR, SC_CHAN, SC_DRAW, XY_DRAW, XY_LAYOUT, ROUTE, HOUR_KEY, WEATHER_KEY, CAR, WEATHER_REACT, TAPE_SHELL, TAPE_LABEL, TAPE_ALPHA, HUE_STEPS, W_HOUR, W_WEATHER, FOLLOW_LABEL, ONOFF, vals, KEYS, saltOf, defaultValue, SPECTRUM, rampRows, WATERFALL, SCOPE, XY, DRIVE, CASSETTE;
  var init_cfg = __esm({
    "src/vendor/tunetap/viz/cfg.js"() {
      KEY_ORDER = Object.freeze([
        "ramp",
        "duo1",
        "duo2",
        "shape",
        "map",
        "peaks",
        "reflect",
        "wf_layout",
        "wf_dir",
        "sc_chan",
        "sc_draw",
        "sc_decay",
        "xy_draw",
        "xy_layout",
        "route",
        "hour",
        "weather",
        "weather_react",
        "car",
        "tape_shell",
        "tape_label",
        "tape_alpha",
        "mini_style",
        "mini_color",
        "voice_view",
        "cover_over",
        "voice_scope"
      ]);
      SHAPE = Object.freeze({ SEG: 0, SOLID: 1, LINE: 2, RADIAL: 3, SONG: 4 });
      MAPPING = Object.freeze({ BOTTOM: 0, TIP: 1, BAND: 2, LEVEL: 3, SONG: 4 });
      RAMPS = Object.freeze({ PALETTE: 0, FIRE: 1, ICE: 2, WINAMP: 3, RAINBOW: 4, DUO: 5, MONO: 6, SONG: 7 });
      OFF = Object.freeze({ NO: 0, YES: 1, SONG: 2 });
      WF_LAYOUT = Object.freeze({ SPLIT: 0, SUM: 1, STACKED: 2, SONG: 3 });
      WF_DIR = Object.freeze({ DOWN: 0, UP: 1, ACROSS: 2, SONG: 3 });
      SC_CHAN = Object.freeze({ OVERLAY: 0, STACKED: 1, MIDSIDE: 2, SONG: 3 });
      SC_DRAW = Object.freeze({ LINE: 0, FILL: 1, DOTS: 2, SONG: 3 });
      XY_DRAW = Object.freeze({ DOTS: 0, LINES: 1, HEAT: 2, SONG: 3 });
      XY_LAYOUT = Object.freeze({ PLAIN: 0, METERS: 1, SONG: 2 });
      ROUTE = Object.freeze({ SONG: 0, SUNSET: 1, CITY: 2, COAST: 3, AURORA: 4, STAGE: 5 });
      HOUR_KEY = Object.freeze({ NIGHT: 0, DUSK: 1, DAWN: 2, DAY: 3, AUTO: 4, SONG: 5 });
      WEATHER_KEY = Object.freeze({ CLEAR: 0, RAIN: 1, SNOW: 2, FOG: 3, SONG: 4 });
      CAR = Object.freeze({ OFF: 0, ON: 1, SONG: 2 });
      WEATHER_REACT = Object.freeze({ OFF: 0, SOFT: 1, STRONG: 2 });
      TAPE_SHELL = Object.freeze({ BLACK: 0, CLEAR: 1, WHITE: 2, PALETTE: 3, SONG: 4 });
      TAPE_LABEL = Object.freeze({ CREAM: 0, PALETTE: 1, SONG: 2 });
      TAPE_ALPHA = Object.freeze({ FULL: 0, A75: 1, A50: 2, A25: 3, SONG: 4 });
      HUE_STEPS = 36;
      W_HOUR = Object.freeze([400, 250, 200, 150]);
      W_WEATHER = Object.freeze([400, 250, 200, 150]);
      FOLLOW_LABEL = "viz.follow";
      ONOFF = ["viz.off", "viz.on", FOLLOW_LABEL];
      vals = (prefix, n) => [...Array.from({ length: n }, (_, i) => `${prefix}.${i}`), FOLLOW_LABEL];
      KEYS = {
        ramp: { n: 8, plain: 7, song: RAMPS.SONG, was: RAMPS.PALETTE, labelKey: "viz.ramp", values: vals("viz.ramp", 7), weights: null },
        duo1: { n: HUE_STEPS, plain: HUE_STEPS, song: -1, was: 32, labelKey: "viz.hue1", values: null, weights: null },
        duo2: { n: HUE_STEPS, plain: HUE_STEPS, song: -1, was: 5, labelKey: "viz.hue2", values: null, weights: null },
        shape: { n: 5, plain: 4, song: SHAPE.SONG, was: SHAPE.SEG, labelKey: "viz.shape", values: vals("viz.shape", 4), weights: null },
        map: { n: 5, plain: 4, song: MAPPING.SONG, was: MAPPING.BOTTOM, labelKey: "viz.map", values: vals("viz.map", 4), weights: null },
        peaks: { n: 3, plain: 2, song: OFF.SONG, was: OFF.YES, labelKey: "viz.peaks", values: ONOFF, weights: null },
        reflect: { n: 3, plain: 2, song: OFF.SONG, was: OFF.NO, labelKey: "viz.reflect", values: ONOFF, weights: null },
        wf_layout: { n: 4, plain: 3, song: WF_LAYOUT.SONG, was: WF_LAYOUT.SPLIT, labelKey: "viz.wf_layout", values: vals("viz.wf_layout", 3), weights: null },
        wf_dir: { n: 4, plain: 3, song: WF_DIR.SONG, was: WF_DIR.DOWN, labelKey: "viz.wf_dir", values: vals("viz.wf_dir", 3), weights: null },
        sc_chan: { n: 4, plain: 3, song: SC_CHAN.SONG, was: SC_CHAN.OVERLAY, labelKey: "viz.sc_chan", values: vals("viz.sc_chan", 3), weights: null },
        sc_draw: { n: 4, plain: 3, song: SC_DRAW.SONG, was: SC_DRAW.LINE, labelKey: "viz.sc_draw", values: vals("viz.sc_draw", 3), weights: null },
        sc_decay: { n: 3, plain: 2, song: OFF.SONG, was: OFF.YES, labelKey: "viz.sc_decay", values: ONOFF, weights: null },
        xy_draw: { n: 4, plain: 3, song: XY_DRAW.SONG, was: XY_DRAW.DOTS, labelKey: "viz.xy_draw", values: vals("viz.xy_draw", 3), weights: null },
        xy_layout: { n: 3, plain: 2, song: XY_LAYOUT.SONG, was: XY_LAYOUT.PLAIN, labelKey: "viz.xy_layout", values: vals("viz.xy_layout", 2), weights: null },
        // 路線's 隨電台 is at 0, where chipchill's 隨歌曲 has been since before the idea had a name.
        route: { n: 6, plain: 5, song: ROUTE.SONG, was: ROUTE.SUNSET, labelKey: "viz.route", values: [FOLLOW_LABEL, "viz.route.1", "viz.route.2", "viz.route.3", "viz.route.4", "viz.route.5"], weights: null },
        hour: { n: 6, plain: 4, song: HOUR_KEY.SONG, was: HOUR_KEY.NIGHT, labelKey: "viz.hour", values: vals("viz.hour", 5), weights: W_HOUR },
        weather: { n: 5, plain: 4, song: WEATHER_KEY.SONG, was: WEATHER_KEY.CLEAR, labelKey: "viz.weather", values: vals("viz.weather", 4), weights: W_WEATHER },
        // 天氣隨音樂 is the owner's taste, not the station's: it has no 隨電台.
        weather_react: { n: 3, plain: 3, song: -1, was: WEATHER_REACT.SOFT, labelKey: "viz.weather_react", values: ["viz.weather_react.0", "viz.weather_react.1", "viz.weather_react.2"], weights: null },
        car: { n: 3, plain: 2, song: CAR.SONG, was: CAR.OFF, labelKey: "viz.car", values: ONOFF, weights: null },
        tape_shell: { n: 5, plain: 4, song: TAPE_SHELL.SONG, was: TAPE_SHELL.BLACK, labelKey: "viz.tape_shell", values: vals("viz.tape_shell", 4), weights: null },
        tape_label: { n: 3, plain: 2, song: TAPE_LABEL.SONG, was: TAPE_LABEL.CREAM, labelKey: "viz.tape_label", values: vals("viz.tape_label", 2), weights: null },
        // The percentages are how much of the plate is left: 不透明 is the label exactly as it was.
        tape_alpha: { n: 5, plain: 4, song: TAPE_ALPHA.SONG, was: TAPE_ALPHA.FULL, labelKey: "viz.tape_alpha", values: vals("viz.tape_alpha", 4), weights: null }
      };
      for (const k2 of Object.keys(KEYS)) Object.freeze(KEYS[k2]);
      saltOf = (k2) => KEY_ORDER.indexOf(k2) + 1;
      defaultValue = (k2) => KEYS[k2].song >= 0 ? KEYS[k2].song : KEYS[k2].was;
      SPECTRUM = definePage({
        id: "spectrum",
        keys: ["shape", "map", "ramp", "peaks", "reflect", "duo1", "duo2"],
        // The keys that are rows in this combination (cfg.rs page_keys): a tube has no caps, a ring no
        // floor, the hues only for the duo ramp.
        rows(c) {
          const l = ["shape", "map"];
          if (c.shape !== SHAPE.LINE) l.push("peaks");
          if (c.shape !== SHAPE.RADIAL) l.push("reflect");
          l.push("ramp");
          if (c.ramp === RAMPS.DUO) l.push("duo1", "duo2");
          return l;
        },
        // The first row is the page as it was before there were choices.
        presets: {
          keys: ["shape", "map", "ramp", "peaks", "reflect"],
          rows: [
            ["viz.spectrum.preset.0", [SHAPE.SEG, MAPPING.BOTTOM, RAMPS.PALETTE, 1, 0]],
            // 經典霓虹
            ["viz.spectrum.preset.1", [SHAPE.SOLID, MAPPING.TIP, RAMPS.FIRE, 1, 1]],
            // 火焰
            ["viz.spectrum.preset.2", [SHAPE.SOLID, MAPPING.TIP, RAMPS.ICE, 1, 1]],
            // 冰焰
            ["viz.spectrum.preset.3", [SHAPE.SEG, MAPPING.BOTTOM, RAMPS.WINAMP, 1, 1]],
            // Winamp
            ["viz.spectrum.preset.4", [SHAPE.SOLID, MAPPING.BAND, RAMPS.RAINBOW, 1, 1]],
            // 彩虹頻段
            ["viz.spectrum.preset.5", [SHAPE.RADIAL, MAPPING.LEVEL, RAMPS.PALETTE, 1, 0]],
            // 脈動
            ["viz.spectrum.preset.6", [SHAPE.LINE, MAPPING.BAND, RAMPS.PALETTE, 0, 1]],
            // 霓虹燈管
            ["viz.spectrum.preset.7", [SHAPE.LINE, MAPPING.BAND, RAMPS.ICE, 0, 1]],
            // 極光
            ["viz.spectrum.preset.8", [SHAPE.SOLID, MAPPING.BAND, RAMPS.PALETTE, 1, 1]]
            // 純色＋倒影
          ]
        },
        // Two groups, each a thing a person can name: the body against its colour. Fastest key first.
        groups: [
          { id: "form", labelKey: "viz.group.form", keys: ["shape", "peaks", "reflect"] },
          // 形體
          { id: "colour", labelKey: "viz.group.colour", keys: ["ramp", "map"] }
          // 顏色
        ]
      });
      rampRows = (c, l) => {
        l.push("ramp");
        if (c.ramp === RAMPS.DUO) l.push("duo1", "duo2");
        return l;
      };
      WATERFALL = definePage({
        id: "waterfall",
        keys: ["wf_layout", "wf_dir", "ramp", "duo1", "duo2"],
        rows(c) {
          return rampRows(c, ["wf_layout", "wf_dir"]);
        },
        presets: {
          keys: ["wf_layout", "wf_dir"],
          rows: [
            ["viz.waterfall.preset.0", [WF_LAYOUT.SPLIT, WF_DIR.DOWN]],
            // 鏡像瀑布
            ["viz.waterfall.preset.1", [WF_LAYOUT.SUM, WF_DIR.DOWN]],
            // 全寬瀑布
            ["viz.waterfall.preset.2", [WF_LAYOUT.STACKED, WF_DIR.DOWN]],
            // 雙層瀑布
            ["viz.waterfall.preset.3", [WF_LAYOUT.SUM, WF_DIR.UP]],
            // 上升
            ["viz.waterfall.preset.4", [WF_LAYOUT.SUM, WF_DIR.ACROSS]],
            // 頻譜圖
            ["viz.waterfall.preset.5", [WF_LAYOUT.STACKED, WF_DIR.ACROSS]]
            // 頻譜圖 L / R
          ]
        },
        // cfg.rs TOURS: the layout against the ramp, fastest key first.
        groups: [
          { id: "layout", labelKey: "viz.group.layout", keys: ["wf_layout", "wf_dir"] },
          // 版面
          { id: "ramp", labelKey: "viz.group.ramp", keys: ["ramp"] }
          // 色帶
        ]
      });
      SCOPE = definePage({
        id: "scope",
        keys: ["sc_chan", "sc_draw", "sc_decay", "ramp", "duo1", "duo2"],
        rows(c) {
          return rampRows(c, ["sc_chan", "sc_draw", "sc_decay"]);
        },
        presets: {
          keys: ["sc_chan", "sc_draw", "sc_decay"],
          rows: [
            ["viz.scope.preset.0", [SC_CHAN.OVERLAY, SC_DRAW.LINE, 1]],
            // 疊加
            ["viz.scope.preset.1", [SC_CHAN.STACKED, SC_DRAW.LINE, 1]],
            // 雙軌
            ["viz.scope.preset.2", [SC_CHAN.MIDSIDE, SC_DRAW.LINE, 1]],
            // Mid / Side
            ["viz.scope.preset.3", [SC_CHAN.OVERLAY, SC_DRAW.FILL, 0]],
            // 填色
            ["viz.scope.preset.4", [SC_CHAN.OVERLAY, SC_DRAW.DOTS, 0]]
            // 點陣
          ]
        },
        // cfg.rs TOURS: how it is drawn against the ramp, fastest key first.
        groups: [
          { id: "draw", labelKey: "viz.group.draw", keys: ["sc_chan", "sc_draw", "sc_decay"] },
          // 畫法
          { id: "ramp", labelKey: "viz.group.ramp", keys: ["ramp"] }
          // 色帶
        ]
      });
      XY = definePage({
        id: "xy",
        keys: ["xy_draw", "xy_layout", "ramp", "duo1", "duo2"],
        rows(c) {
          return rampRows(c, ["xy_draw", "xy_layout"]);
        },
        presets: {
          keys: ["xy_draw", "xy_layout"],
          rows: [
            ["viz.xy.preset.0", [XY_DRAW.DOTS, XY_LAYOUT.PLAIN]],
            // 經典
            ["viz.xy.preset.1", [XY_DRAW.DOTS, XY_LAYOUT.METERS]],
            // 儀表
            ["viz.xy.preset.2", [XY_DRAW.LINES, XY_LAYOUT.METERS]],
            // 向量
            ["viz.xy.preset.3", [XY_DRAW.HEAT, XY_LAYOUT.METERS]]
            // 熱圖
          ]
        },
        // cfg.rs TOURS: how it is drawn against the ramp, fastest key first.
        groups: [
          { id: "draw", labelKey: "viz.group.draw", keys: ["xy_draw", "xy_layout"] },
          // 畫法
          { id: "ramp", labelKey: "viz.group.ramp", keys: ["ramp"] }
          // 色帶
        ]
      });
      DRIVE = definePage({
        id: "drive",
        keys: ["route", "hour", "weather", "weather_react", "car"],
        // Every key is always a row (cfg.rs page_keys): 天氣隨音樂 stays one even on 晴, it is how the
        // owner finds out the weather can hear. The drive has no ramp row.
        rows() {
          return ["route", "hour", "weather", "weather_react", "car"];
        },
        presets: {
          keys: ["hour", "weather", "car"],
          rows: [
            ["viz.drive.preset.0", [HOUR_KEY.NIGHT, WEATHER_KEY.CLEAR, CAR.OFF]],
            // 原版夜色
            // chipchill's 跟著現在 (the clock and the forecast); with no forecast here the weather is
            // what chipchill shows when the platform has none: clear
            ["viz.drive.preset.1", [HOUR_KEY.AUTO, WEATHER_KEY.CLEAR, CAR.ON]],
            // 跟著時鐘
            ["viz.drive.preset.2", [HOUR_KEY.DUSK, WEATHER_KEY.CLEAR, CAR.ON]],
            // 黃昏
            ["viz.drive.preset.3", [HOUR_KEY.DAWN, WEATHER_KEY.RAIN, CAR.ON]],
            // 黎明雨
            ["viz.drive.preset.4", [HOUR_KEY.NIGHT, WEATHER_KEY.SNOW, CAR.ON]],
            // 雪夜
            ["viz.drive.preset.5", [HOUR_KEY.DAWN, WEATHER_KEY.FOG, CAR.ON]]
            // 晨霧
          ]
        },
        // 夜行's sky against its road (cfg.rs TOURS), fastest key first.
        groups: [
          { id: "sky", labelKey: "viz.group.sky", keys: ["weather", "hour"] },
          // 天空
          { id: "road", labelKey: "viz.group.road", keys: ["route", "car"] }
          // 路線
        ]
      });
      CASSETTE = definePage({
        id: "cassette",
        keys: ["tape_shell", "tape_label", "tape_alpha"],
        // Every key is always a row (cfg.rs page_keys); the cassette has no ramp row.
        rows() {
          return ["tape_shell", "tape_label", "tape_alpha"];
        },
        presets: {
          keys: ["tape_shell", "tape_label", "tape_alpha"],
          rows: [
            ["viz.cassette.preset.0", [TAPE_SHELL.BLACK, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]],
            // 經典
            ["viz.cassette.preset.1", [TAPE_SHELL.CLEAR, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]],
            // 透明
            ["viz.cassette.preset.2", [TAPE_SHELL.WHITE, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]],
            // 白色
            ["viz.cassette.preset.3", [TAPE_SHELL.PALETTE, TAPE_LABEL.PALETTE, TAPE_ALPHA.FULL]],
            // 配色
            // (chipchill's 初音, the character label, is left out)
            ["viz.cassette.preset.4", [TAPE_SHELL.CLEAR, TAPE_LABEL.PALETTE, TAPE_ALPHA.A50]]
            // 透視
          ]
        },
        // The shell against the label (cfg.rs TOURS), fastest key first.
        groups: [
          { id: "shell", labelKey: "viz.group.shell", keys: ["tape_shell"] },
          // 外殼
          { id: "label", labelKey: "viz.group.label", keys: ["tape_label", "tape_alpha"] }
          // 標籤
        ]
      });
    }
  });

  // src/vendor/tunetap/viz/registry.js
  function createRegistry() {
    const defs = /* @__PURE__ */ new Map();
    const given = /* @__PURE__ */ new Map();
    const loads = /* @__PURE__ */ new Map();
    return {
      register(def) {
        if (!def || typeof def.id !== "string" || !ID.test(def.id)) throw new TypeError("a plugin needs an id of lower-case letters, digits and dashes");
        if (typeof def.load !== "function") throw new TypeError(`plugin ${def.id}: load must be a function`);
        if (given.get(def.id) === def) return defs.get(def.id);
        if (defs.has(def.id)) throw new Error(`plugin ${def.id} is already registered`);
        const d4 = Object.freeze({ id: def.id, labelKey: def.labelKey ?? `viz.mode.${def.id}`, needsAudio: def.needsAudio !== false, stillWithoutAudio: !!def.stillWithoutAudio, words: def.words === "spectrum" || def.words === "none" ? def.words : "audio", load: def.load });
        defs.set(def.id, d4);
        given.set(def.id, def);
        return d4;
      },
      get: (id) => defs.get(id) ?? null,
      list: () => [...defs.values()],
      // The plugin's module, imported once (a failed import may be tried again).
      load(id) {
        const d4 = defs.get(id);
        if (!d4) return Promise.reject(new Error(`no plugin ${id}`));
        if (!loads.has(id)) {
          const p = Promise.resolve().then(() => d4.load());
          loads.set(id, p);
          p.catch(() => {
            if (loads.get(id) === p) loads.delete(id);
          });
        }
        return loads.get(id);
      }
    };
  }
  var ID, registry;
  var init_registry = __esm({
    "src/vendor/tunetap/viz/registry.js"() {
      ID = /^[a-z][a-z0-9-]*$/;
      registry = createRegistry();
    }
  });

  // src/vendor/tunetap/viz/labels.js
  function wideChar(cp) {
    return cp >= 4352 && cp <= 4447 || cp >= 11904 && cp <= 42191 || cp >= 44032 && cp <= 55203 || cp >= 63744 && cp <= 64255 || cp >= 65072 && cp <= 65103 || cp >= 65280 && cp <= 65376 || cp >= 65504 && cp <= 65510;
  }
  function sameLabels(a, b) {
    if (a === b) return true;
    if (!a || !b || a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) for (const f of FIELDS2) if ((a[i][f] ?? null) !== (b[i][f] ?? null)) return false;
    return true;
  }
  function fitText(text, w, measureW) {
    if (measureW(text) <= w) return text;
    const cps = [...text];
    let lo = 0, hi = cps.length;
    if (measureW(ELLIPSIS) > w) return "";
    while (hi - lo > 1) {
      const mid = lo + hi >> 1;
      if (measureW(cps.slice(0, mid).join("") + ELLIPSIS) <= w) lo = mid;
      else hi = mid;
    }
    return lo === 0 ? ELLIPSIS : cps.slice(0, lo).join("").trimEnd() + ELLIPSIS;
  }
  function contextMeasure(ctx) {
    const cache = /* @__PURE__ */ new Map();
    const unit = (text) => {
      let w = cache.get(text);
      if (w === void 0) {
        try {
          ctx.font = fontOf(MEASURE_PX);
          w = (ctx.measureText(text)?.width ?? 0) / MEASURE_PX;
        } catch {
          w = RGXX_METRICS.width("wide", 1, text) / 16;
        }
        if (!Number.isFinite(w)) w = 0;
        if (cache.size > 256) cache.clear();
        cache.set(text, w);
      }
      return w;
    };
    return { width: (font, scale, text) => Math.ceil(unit(String(text ?? "")) * (font === "wide" ? 16 : 8) * scale) };
  }
  function createLabelLayer({ canvas, ctx }) {
    let last = null;
    let size = "";
    let draws = 0;
    const measure = contextMeasure(ctx);
    function wipe() {
      try {
        ctx.setTransform(1, 0, 0, 1, 0, 0);
        ctx.clearRect(0, 0, canvas.width, canvas.height);
      } catch {
      }
    }
    function paint(labels, pw, ph) {
      wipe();
      const kx = canvas.width / pw, ky = canvas.height / ph;
      if (!(kx > 0) || !(ky > 0)) return;
      for (const l of labels) {
        if (!l || typeof l.text !== "string" || !l.text) continue;
        const hgt = fontHeight(l);
        const font = fontOf(hgt);
        ctx.font = font;
        const mw = (s) => ctx.measureText(s).width;
        let room = pw - l.x;
        if (l.maxW !== null && l.maxW !== void 0) room = Math.min(room, l.maxW);
        if (l.chip !== null && l.chip !== void 0) room -= l.font === "wide" ? 8 : 6;
        if (room <= 0) continue;
        const text = fitText(l.text, room, mw);
        if (!text) continue;
        const tw = Math.min(mw(text), room);
        const [bx, by, bw, bh] = l.chip !== null && l.chip !== void 0 ? l.font === "wide" ? [l.x, l.y - 4, tw + 8, 19] : [l.x, l.y, tw + 6, 11] : [l.x, l.y, tw, hgt];
        ctx.save();
        ctx.setTransform(kx, 0, 0, ky, 0, 0);
        ctx.beginPath();
        ctx.rect(Math.max(bx - 1, 0), Math.max(by - 1, 0), Math.min(bw + 2, pw - Math.max(bx - 1, 0)), Math.min(bh + 2, ph - Math.max(by - 1, 0)));
        ctx.clip();
        let tx = l.x;
        let ty = l.y;
        if (l.chip !== null && l.chip !== void 0) {
          ctx.fillStyle = css(l.chip);
          const pad = 2;
          ctx.beginPath();
          if (typeof ctx.roundRect === "function") ctx.roundRect(bx, by, bw, bh, pad);
          else ctx.rect(bx, by, bw, bh);
          ctx.fill();
          tx = l.x + (l.font === "wide" ? 4 : 3);
          ty = l.font === "wide" ? l.y - 3 : l.y + 2;
        }
        ctx.fillStyle = css(l.colour);
        ctx.textBaseline = "alphabetic";
        ctx.textAlign = "left";
        ctx.fillText(text, tx, ty + hgt * BASE);
        ctx.restore();
      }
      draws++;
    }
    return {
      measure,
      // Draws labels (an array, or null / empty for none) for a pw x ph pixel canvas; returns
      // whether the canvas was drawn again.
      set(labels, pw, ph) {
        const list = Array.isArray(labels) && labels.length ? labels : null;
        const sz = `${canvas.width}x${canvas.height}/${pw}x${ph}`;
        if (!list) {
          if (last === null) return false;
          wipe();
          last = null;
          return true;
        }
        if (last !== null && size === sz && sameLabels(list, last)) return false;
        try {
          paint(list, pw, ph);
        } catch {
          wipe();
        }
        last = list.map((l) => ({ ...l }));
        size = sz;
        return true;
      },
      // Empties the canvas (a new mode, the host stopped); the next set() draws again.
      clear() {
        if (last === null) return;
        wipe();
        last = null;
      },
      // The labels on the canvas now (for tests and the smoke's hook).
      shown: () => last ? last.map((l) => ({ ...l })) : [],
      draws: () => draws
    };
  }
  var FONT_STACK, ELLIPSIS, BASE, fontHeight, RGXX_METRICS, FIELDS2, css, fontOf, MEASURE_PX;
  var init_labels = __esm({
    "src/vendor/tunetap/viz/labels.js"() {
      init_pixel();
      FONT_STACK = 'system-ui, "PingFang TC", "Noto Sans CJK TC", "Microsoft JhengHei", sans-serif';
      ELLIPSIS = "\u2026";
      BASE = 0.85;
      fontHeight = (l) => (l.font === "wide" ? 16 : 8) * (l.scale || 1);
      RGXX_METRICS = Object.freeze({
        width(font, scale, text) {
          let cells = 0;
          for (const ch of String(text ?? "")) cells += font === "wide" && wideChar(ch.codePointAt(0)) ? 2 : 1;
          return 8 * scale * cells;
        }
      });
      FIELDS2 = ["kind", "text", "font", "scale", "x", "y", "colour", "chip", "maxW"];
      css = (c) => `rgb(${red(c)},${green(c)},${blue(c)})`;
      fontOf = (px) => `${px}px ${FONT_STACK}`;
      MEASURE_PX = 64;
    }
  });

  // src/vendor/tunetap/viz/host.js
  function createVizHost(deps = {}) {
    const { doc = null, storage = null, raf, caf, now } = deps;
    const registry2 = deps.registry ?? registry;
    const getTheme = deps.getTheme ?? (() => themeNamed("neon"));
    const getAudio = deps.getAudio ?? (() => null);
    const visible = deps.visible ?? (() => true);
    const isPlaying = deps.playing ?? (() => false);
    const station = deps.station ?? (() => "");
    const reduced = deps.reducedMotion ?? (() => false);
    const icyTitle = deps.title ?? (() => null);
    const stationName = deps.stationName ?? (() => "");
    const playTime = deps.playTime ?? (() => 0);
    const dpr = () => call(() => {
      const d4 = typeof deps.dpr === "function" ? deps.dpr() : deps.dpr;
      return Math.min(Math.max(+d4 || 1, 1), 2);
    }, 1);
    const read2 = (k2) => call(() => storage?.getItem(k2) ?? null, null);
    const write2 = (k2, v) => call(() => {
      storage?.setItem(k2, v);
    }, void 0);
    const listeners2 = /* @__PURE__ */ new Set();
    let on2 = false;
    let modeId = registry2.get(read2(KEY2.mode)) ? read2(KEY2.mode) : registry2.list()[0]?.id ?? null;
    let themeName = read2(KEY2.theme);
    if (themeName !== "app" && !themeNamed(themeName)) themeName = "app";
    let appTheme = null;
    let container = null, canvas = null, ctx = null, pixels = null;
    let textCanvas = null, layer = null;
    let inst = null, instMode = null, cfg = null;
    let failedMode = null, loading = null, token = 0;
    let rafId = null, last = null;
    let tapSeen = null;
    let lastStatus = null;
    let full = false;
    function emit2() {
      lastStatus = status();
      for (const fn of [...listeners2]) call(() => fn(), void 0);
    }
    function emitIfStatusChanged() {
      if (status() !== lastStatus) emit2();
    }
    const theme = () => themeName === "app" ? appTheme ?? (appTheme = call(() => getTheme(), null) ?? themeNamed("neon")) : themeNamed(themeName);
    const seed = () => songHash(call(() => station(), "") || "");
    const def = () => registry2.get(instMode ?? modeId);
    const hidden = () => !!call(() => doc?.hidden, false);
    const conditions = () => on2 && !!canvas && !!call(() => visible(), false) && !hidden() && !!call(() => isPlaying(), false);
    function loadCfg(id) {
      let obj = null;
      try {
        obj = JSON.parse(read2(KEY2.cfg(id)) ?? "null");
      } catch {
        obj = null;
      }
      return call(() => inst.validate(obj), null) ?? call(() => inst.defaults(), {});
    }
    function saveCfg() {
      if (inst) write2(KEY2.cfg(instMode), JSON.stringify(cfg));
    }
    const clearLabels = () => call(() => layer?.clear(), void 0);
    function dropInstance() {
      clearLabels();
      if (!inst) return;
      const i = inst;
      inst = null;
      instMode = null;
      cfg = null;
      tapSeen = null;
      call(() => i.dispose(), void 0);
    }
    function fail(id) {
      dropInstance();
      failedMode = id;
      stopLoop(true);
      emit2();
    }
    function ensure() {
      const id = modeId;
      if (!id || inst && instMode === id || loading?.mode === id || failedMode === id) return;
      const my = ++token;
      const p = registry2.load(id).then((mod) => {
        if (my !== token) return;
        const ni = mod.create({ canvas, ctx2d: ctx, theme: theme(), seed: seed() });
        if (!ni || typeof ni.frame !== "function") throw new Error(`plugin ${id}: no instance`);
        dropInstance();
        inst = ni;
        instMode = id;
        cfg = loadCfg(id);
        if (pixels) call(() => inst.resize(pixels.width, pixels.height, dpr()), void 0);
      }).catch(() => {
        if (my === token) {
          failedMode = id;
          dropInstance();
        }
      }).finally(() => {
        if (my !== token) return;
        loading = null;
        update();
        emit2();
      });
      loading = { mode: id, p };
    }
    function readAudio() {
      if (!inst) return null;
      const tap = call(() => getAudio(), null);
      if (!tap) return null;
      if (def()?.needsAudio === false) {
        if (tap !== tapSeen) {
          call(() => tap.setNeeds?.(inst.needs), void 0);
          tapSeen = tap;
        }
        return null;
      }
      try {
        if (tap !== tapSeen) {
          tap.setNeeds?.(inst.needs);
          tapSeen = tap;
        }
        return tap.read?.(inst.needs) ?? null;
      } catch {
        return null;
      }
    }
    const shown = () => call(() => inst.resolveFollow(cfg, seed()), cfg);
    function words() {
      const icy = call(() => icyTitle(), null);
      const name = String(call(() => stationName(), "") ?? "");
      const t6 = typeof icy === "string" && icy.trim() ? icy.trim() : "";
      return t6 ? { title: t6, artist: name } : { title: name, artist: "" };
    }
    function draw(dt, t6, audio) {
      if (!inst || !pixels) return;
      const id = instMode;
      try {
        const th = theme();
        const sd = seed();
        backdrop(pixels, th);
        const { title, artist } = words();
        const elapsedMs = Math.max(0, +call(() => playTime(), 0) || 0);
        const out = inst.frame({ dt, now: t6, audio, theme: th, cfg: call(() => inst.resolveFollow(cfg, sd), cfg), playing: !!call(() => isPlaying(), false), pixels, full, seed: sd, title, artist, elapsedMs, measure: layer?.measure ?? null });
        blit(ctx, pixels);
        if (layer) call(() => layer.set(out && Array.isArray(out.labels) ? out.labels : null, pixels.width, pixels.height), void 0);
      } catch {
        fail(id);
        return;
      }
      emitIfStatusChanged();
    }
    function startLoop() {
      if (rafId === null) rafId = raf(tick);
    }
    function stopLoop(clear) {
      if (rafId !== null) {
        call(() => caf(rafId), void 0);
        rafId = null;
      }
      last = null;
      if (clear && ctx && canvas) call(() => ctx.clearRect(0, 0, canvas.width, canvas.height), void 0);
      if (clear) clearLabels();
    }
    function tick() {
      rafId = null;
      if (!inst || !conditions()) {
        update();
        return;
      }
      rafId = raf(tick);
      const t6 = now();
      const fps = call(() => reduced(), false) ? FPS_REDUCED : FPS;
      if (last !== null && t6 - last < 1e3 / fps - SLACK_MS) return;
      const dt = last === null ? 1 / fps : (t6 - last) / 1e3;
      last = t6;
      const audio = readAudio();
      if (!audio && def()?.stillWithoutAudio) {
        stopLoop(false);
        draw(REST_S, t6, null);
        return;
      }
      draw(dt, t6, audio);
    }
    function repaint() {
      if (rafId !== null || !inst || !canvas || !on2 || hidden() || !call(() => visible(), false)) return;
      draw(0, now(), readAudio());
    }
    function update() {
      appTheme = null;
      if (on2 && canvas) ensure();
      if (!inst || !conditions()) {
        stopLoop(true);
      } else if (def()?.stillWithoutAudio && !call(() => getAudio(), null)) {
        stopLoop(false);
        draw(REST_S, now(), null);
      } else {
        startLoop();
      }
      emitIfStatusChanged();
    }
    function onVisibility() {
      update();
    }
    function size() {
      const cw = call(() => container.clientWidth, 0) || 0, chh = call(() => container.clientHeight, 0) || 0;
      const w = CANVAS_W;
      const h = cw > 0 && chh > 0 ? Math.min(Math.max(1, Math.round(w * chh / cw)), 4 * w) : Math.round(w * 9 / 16);
      canvas.width = w;
      canvas.height = h;
      pixels = new Canvas(w, h);
      if (textCanvas) {
        const k2 = dpr();
        textCanvas.width = Math.max(1, Math.round((cw > 0 ? cw : w) * k2));
        textCanvas.height = Math.max(1, Math.round((chh > 0 ? chh : h) * k2));
        clearLabels();
      }
      if (inst) call(() => inst.resize(w, h, dpr()), void 0);
    }
    function status() {
      if (!on2) return "off";
      if (failedMode && failedMode === modeId) return "unavailable";
      if (!inst || loading) return "loading";
      if (!conditions()) return "idle";
      return call(() => inst.status(), "ok") || "ok";
    }
    function setMode(id) {
      if (!registry2.get(id)) return;
      if (id === modeId && failedMode !== id) return;
      modeId = id;
      write2(KEY2.mode, id);
      failedMode = null;
      token++;
      loading = null;
      dropInstance();
      stopLoop(false);
      update();
      emit2();
    }
    function stepMode(dir) {
      const list = registry2.list();
      if (!list.length) return;
      const i = Math.max(0, list.findIndex((p) => p.id === modeId));
      setMode(list[(i + dir + list.length) % list.length].id);
    }
    function whenReady() {
      return loading ? loading.p.then(() => loading ? whenReady() : void 0) : Promise.resolve();
    }
    function groups() {
      if (!inst) return [];
      return call(() => inst.groups(), []).map((g2) => ({ id: g2.id, labelKey: g2.labelKey, count: g2.count, pos: call(() => g2.pos(cfg), -1) }));
    }
    function unmount() {
      stopLoop(false);
      token++;
      loading = null;
      dropInstance();
      doc?.removeEventListener?.("visibilitychange", onVisibility);
      call(() => canvas?.remove(), void 0);
      call(() => textCanvas?.remove(), void 0);
      container = null;
      canvas = null;
      ctx = null;
      pixels = null;
      textCanvas = null;
      layer = null;
      emitIfStatusChanged();
    }
    return {
      // Puts the canvas (class viz-canvas; scaled up by CSS, pixelated) and the labels' canvas over it
      // (class viz-labels) into `el` and starts watching.
      mount(el4) {
        if (container) unmount();
        container = el4;
        canvas = doc.createElement("canvas");
        canvas.className = "viz-canvas";
        ctx = call(() => canvas.getContext("2d"), null);
        el4.append(canvas);
        textCanvas = doc.createElement("canvas");
        textCanvas.className = "viz-labels";
        call(() => textCanvas.setAttribute("aria-hidden", "true"), void 0);
        const tctx = call(() => textCanvas.getContext("2d"), null);
        layer = tctx ? createLabelLayer({ canvas: textCanvas, ctx: tctx }) : null;
        el4.append(textCanvas);
        size();
        doc.addEventListener?.("visibilitychange", onVisibility);
        update();
      },
      unmount,
      // Re-measure the container (layout / orientation change).
      resize() {
        if (!canvas) return;
        size();
        repaint();
      },
      update,
      // Loads the current plugin even while the visualizer is off (for the controls).
      prepare() {
        ensure();
      },
      // Settles when the plugin being loaded (if any) is ready or has failed.
      whenReady,
      enabled: () => on2,
      setEnabled(b) {
        on2 = !!b;
        if (!on2) stopLoop(true);
        update();
        emit2();
      },
      mode: () => modeId,
      modes: () => registry2.list().map((p) => ({ id: p.id, labelKey: p.labelKey })),
      setMode,
      nextMode() {
        stepMode(1);
      },
      prevMode() {
        stepMode(-1);
      },
      // The current plugin's controls and choices; null until it is loaded.
      plugin() {
        if (!inst) return null;
        const d4 = registry2.get(instMode);
        return {
          id: instMode,
          labelKey: d4?.labelKey,
          needsAudio: !!d4?.needsAudio,
          // calm: without audio data the plugin keeps animating (the drive), not one resting frame
          calm: !!d4 && !d4.stillWithoutAudio,
          // how the status line speaks of the data (registry.js): 'spectrum' or 'audio'
          words: d4?.words ?? "audio",
          schema: inst.schema ?? [],
          cfg: { ...cfg },
          shown: shown(),
          preset: call(() => inst.presetOf(cfg), -1),
          presets: (inst.presets ?? []).map((p) => ({ id: p.id, labelKey: p.labelKey })),
          groups: groups()
        };
      },
      setCfg(patch) {
        if (!inst) return;
        cfg = call(() => inst.validate({ ...cfg, ...patch }), cfg);
        saveCfg();
        emit2();
        repaint();
      },
      setPreset(id) {
        if (!inst) return;
        cfg = call(() => inst.validate(inst.applyPreset(cfg, id)), cfg);
        saveCfg();
        emit2();
        repaint();
      },
      themeName: () => themeName,
      themes: () => [{ name: "app", labelKey: "viz.theme.app" }, ...THEME_NAMES.map((n) => ({ name: n, labelKey: `viz.theme.${n}` }))],
      setThemeName(name) {
        if (name !== "app" && !themeNamed(name)) return;
        themeName = name;
        appTheme = null;
        write2(KEY2.theme, name);
        emit2();
        repaint();
      },
      // The current plugin's two tour groups and where the choices stand on each (count = 隨電台).
      groups,
      // One press of a group's key (the on-page zones): the next stop, persisted. Returns { pos, count,
      // follow, labelKey, key, args } for a toast — t(key, t(labelKey), ...args.slice(1)) — and `text`
      // already translated when t is given. null for no such group.
      stepGroup(i, t6) {
        const g2 = inst ? call(() => inst.groups(), [])[i] : null;
        if (!g2) return null;
        const r = call(() => g2.step(cfg, shown()), null);
        if (!r) return null;
        cfg = call(() => inst.validate(r.cfg), cfg);
        saveCfg();
        const pos = call(() => g2.pos(cfg), r.pos);
        const follow2 = pos === g2.count;
        const out = { pos, count: g2.count, follow: follow2, labelKey: g2.labelKey, key: follow2 ? "viz.group.follow" : "viz.group.pos", args: follow2 ? [g2.labelKey] : [g2.labelKey, pos + 1, g2.count] };
        if (typeof t6 === "function") out.text = follow2 ? t6(out.key, t6(g2.labelKey)) : t6(out.key, t6(g2.labelKey), pos + 1, g2.count);
        emit2();
        repaint();
        return out;
      },
      // Full screen on/off (the UI's Fullscreen API state); repaints a still frame when stopped.
      full: () => full,
      setFull(b) {
        if (full === !!b) return;
        full = !!b;
        repaint();
      },
      status,
      // The labels on the screen now (tests and the smoke's hook): [{ kind, text, x, y, … }].
      labels: () => call(() => layer?.shown() ?? [], []),
      onChange(fn) {
        listeners2.add(fn);
        return () => listeners2.delete(fn);
      }
    };
  }
  var CANVAS_W, FPS, FPS_REDUCED, REST_S, SLACK_MS, KEY2, call;
  var init_host = __esm({
    "src/vendor/tunetap/viz/host.js"() {
      init_pixel();
      init_page();
      init_themes();
      init_cfg();
      init_registry();
      init_labels();
      CANVAS_W = 480;
      FPS = 30;
      FPS_REDUCED = 15;
      REST_S = 10;
      SLACK_MS = 2;
      KEY2 = { mode: "tunetap.viz.mode", theme: "tunetap.viz.theme", cfg: (id) => `tunetap.viz.cfg.${id}` };
      call = (fn, fallback) => {
        try {
          return fn();
        } catch {
          return fallback;
        }
      };
    }
  });

  // src/vendor/tunetap/viz/bands.js
  function createBands() {
    const two = (n) => [new Float32Array(n), new Float32Array(n)];
    return { level: two(BANDS), peak: two(BANDS), hold: two(BANDS), fine: two(FINE), finePeak: two(FINE), fineHold: two(FINE) };
  }
  function resetBands(b) {
    for (const k2 of ["level", "peak", "hold", "fine", "finePeak", "fineHold"]) {
      b[k2][0].fill(0);
      b[k2][1].fill(0);
    }
  }
  function bandEdges(n, rate, fft = FFT) {
    const e = new Int32Array(n + 1);
    for (let b = 0; b <= n; b++) {
      const f = 40 * Math.pow(16e3 / 40, b / n);
      e[b] = Math.trunc(f * fft / rate + 0.5);
      if (b > 0 && e[b] <= e[b - 1]) e[b] = e[b - 1] + 1;
    }
    return e;
  }
  function dbToLevel(db) {
    const lv = F2((db + DB_CAL - FLOOR_DB) / -FLOOR_DB);
    return lv > 0 ? lv < 1 ? lv : 1 : 0;
  }
  function follow(lv, dt, level, peak, hold, i) {
    const cur = F2(level[i] - F2(FALL_PER_S * dt));
    level[i] = lv > cur ? lv : cur < 0 ? 0 : cur;
    if (level[i] >= peak[i]) {
      peak[i] = level[i];
      hold[i] = PEAK_HOLD_S;
    } else {
      hold[i] = F2(hold[i] - dt);
      if (hold[i] < 0) {
        peak[i] = F2(peak[i] - F2(PEAK_FALL_PER_S * dt));
        if (peak[i] < 0) peak[i] = 0;
      }
    }
  }
  function loudest(db, from, to, fft) {
    let m = -Infinity;
    if (!db) return m;
    const top = Math.min(db.length, fft / 2);
    for (let i = Math.max(from, 1); i < to && i < top; i++) if (db[i] > m) m = db[i];
    return m;
  }
  var F2, BANDS, FINE, FFT, FLOOR_DB, FALL_PER_S, PEAK_HOLD_S, PEAK_FALL_PER_S, DB_CAL, BandAnalyser;
  var init_bands = __esm({
    "src/vendor/tunetap/viz/bands.js"() {
      F2 = Math.fround;
      BANDS = 16;
      FINE = 64;
      FFT = 2048;
      FLOOR_DB = -60;
      FALL_PER_S = 1.5;
      PEAK_HOLD_S = 0.35;
      PEAK_FALL_PER_S = 0.6;
      DB_CAL = 20 * Math.log10(2 / 0.42);
      BandAnalyser = class {
        constructor() {
          this.key = "";
          this.edges = null;
          this.fineEdges = null;
        }
        // freq: { l, r } Float32Array dB per bin (r may be the same array as l: a mono stream);
        // rate: the context's sample rate; fft: the analyser's fftSize; dt: seconds since the last run.
        run(b, freq, rate, fft, dt) {
          const k2 = rate + ":" + fft;
          if (k2 !== this.key) {
            this.edges = bandEdges(BANDS, rate, fft);
            this.fineEdges = bandEdges(FINE, rate, fft);
            this.key = k2;
          }
          dt = F2(dt > 0 ? dt : 0);
          const chans = [freq?.l, freq?.r ?? freq?.l];
          for (let ch = 0; ch < 2; ch++) {
            const db = chans[ch];
            for (let band = 0; band < BANDS; band++) {
              follow(dbToLevel(loudest(db, this.edges[band], this.edges[band + 1], fft)), dt, b.level[ch], b.peak[ch], b.hold[ch], band);
            }
            for (let band = 0; band < FINE; band++) {
              follow(dbToLevel(loudest(db, this.fineEdges[band], this.fineEdges[band + 1], fft)), dt, b.fine[ch], b.finePeak[ch], b.fineHold[ch], band);
            }
          }
        }
      };
    }
  });

  // src/js/lib/viz-tap.js
  function createBandTap(getBytes) {
    const i0 = new Uint8Array(BINS), i1 = new Uint8Array(BINS), w = new Float32Array(BINS);
    const span = Math.log(HI / LO);
    for (let k2 = 0; k2 < BINS; k2++) {
      const f = k2 * RATE / FFT2;
      let p = f > 0 ? Math.log(f / LO) / span * NANO_BANDS - 0.5 : 0;
      p = Math.min(Math.max(p, 0), NANO_BANDS - 1);
      const a = Math.min(Math.floor(p), NANO_BANDS - 2);
      i0[k2] = a;
      i1[k2] = a + 1;
      w[k2] = p - a;
    }
    const db = new Float32Array(BINS);
    const result = { rate: RATE, bins: BINS, freq: { l: db, r: db } };
    const toDb = (b) => b > 0 ? b * 70 / 255 - 70 - DB_CAL : -Infinity;
    return {
      ready: true,
      needs: { freq: "mono", time: "none" },
      setNeeds() {
      },
      read() {
        const v = getBytes();
        for (let k2 = 0; k2 < BINS; k2++) db[k2] = toDb(v[i0[k2]] * (1 - w[k2]) + v[i1[k2]] * w[k2]);
        return result;
      },
      close() {
      }
    };
  }
  var NANO_BANDS, RATE, FFT2, BINS, LO, HI;
  var init_viz_tap = __esm({
    "src/js/lib/viz-tap.js"() {
      init_bands();
      NANO_BANDS = 32;
      RATE = 48e3;
      FFT2 = 2048;
      BINS = FFT2 / 2;
      LO = 40;
      HI = 16e3;
    }
  });

  // src/vendor/tunetap/viz/plugins/spectrum.js
  var spectrum_exports = {};
  __export(spectrum_exports, {
    SpectrumPainter: () => SpectrumPainter,
    create: () => create,
    needs: () => needs
  });
  function lutAt(k2, ch, f) {
    return k2.lut[ch][f <= 0 ? 0 : f >= 1 ? 255 : Math.trunc(F3(f * 255))];
  }
  function fine32(z) {
    const lv = [new Float32Array(FINE32), new Float32Array(FINE32)];
    const pk = [new Float32Array(FINE32), new Float32Array(FINE32)];
    for (let ch = 0; ch < 2; ch++) {
      for (let i = 0; i < FINE32; i++) {
        const a = z.fine[ch][2 * i], b = z.fine[ch][2 * i + 1];
        lv[ch][i] = a > b ? a : b;
        const p = z.finePeak[ch][2 * i], q = z.finePeak[ch][2 * i + 1];
        pk[ch][i] = p > q ? p : q;
      }
    }
    return [lv, pk];
  }
  function shapeSeg(c, v, f, k2, m, peaks, base, top) {
    const avail = c.width - f.i(60), inner = f.i(2), gap = f.i(6);
    const group = Math.trunc((avail + gap) / BANDS);
    const bw = Math.trunc((group - inner - gap) / 2);
    const x0 = f.i(40) + Math.trunc((avail - (BANDS * group - gap)) / 2);
    const seg = f.px(3), step = f.px(5);
    for (let b = 0; b < BANDS; b++) {
      for (let ch = 0; ch < 2; ch++) {
        const lv = v.bands.level[ch][b];
        const h = Math.trunc(F3(lv * top));
        const x = x0 + b * group + ch * (bw + inner);
        for (let yy = 0; yy + seg <= h; yy += step) {
          c.fill(x, base - yy - seg, bw, seg, colour(k2.pal, k2.ramp, m, ch, b, BANDS, F3(yy / top), lv));
        }
        const ph = Math.trunc(F3(v.bands.peak[ch][b] * top));
        if (peaks && ph > 2) c.fill(x, base - ph - f.i(6), bw, f.px(2), v.theme.fg);
      }
    }
    return [x0, BANDS * group - gap];
  }
  function shapeSolid(c, v, f, k2, m, peaks, base, top) {
    const [lv, pk] = fine32(v.bands);
    const pitch = Math.trunc((c.width - f.i(64)) / FINE32);
    const x0 = f.i(44), bw = Math.trunc(pitch * 15 / 18);
    for (let i = 0; i < FINE32; i++) {
      const a = F3(F3(lv[0][i] + lv[1][i]) / 2);
      const p = pk[0][i] > pk[1][i] ? pk[0][i] : pk[1][i];
      const h = Math.trunc(F3(a * top));
      const x = x0 + i * pitch;
      if (mapFlat(m)) {
        c.fill(x, base - h, bw, h, lutAt(k2, 1, map(m, i, FINE32, a, a)));
      } else {
        for (let yy = 0; yy < h; yy++) c.hspan(x, base - 1 - yy, bw, lutAt(k2, 1, map(m, i, FINE32, F3(yy / top), a)));
      }
      const ph = Math.trunc(F3(p * top));
      if (peaks && ph > 2) c.fill(x, base - (ph + f.i(12) > top ? top : ph + f.i(12)), bw, f.px(3), v.theme.fg);
    }
  }
  function shapeLine(c, v, f, k2, m, base, top) {
    const [lv] = fine32(v.bands);
    const x0 = f.i(44), w = c.width - f.i(64), light = k2.pal.light;
    if (w <= 0) return;
    const py = new Int32Array(FINE32), fill = new Uint32Array(FINE32), glow = new Uint32Array(FINE32), core = new Uint32Array(FINE32);
    for (let ch = 1; ch >= 0; ch--) {
      for (let i = 0; i < FINE32; i++) {
        const fv = map(m, i, FINE32, lv[ch][i], lv[ch][i]);
        const col = lutAt(k2, ch, ch ? F3(1 - fv) : fv);
        py[i] = base - Math.trunc(F3(lv[ch][i] * top));
        fill[i] = tone(col, 51, light);
        glow[i] = tone(col, 115, light);
        core[i] = tone(col, 255, light);
      }
      const n = FINE32 - 1;
      for (let x = 0; x < w; x++) {
        const i = Math.trunc(x * n / w);
        const xa = Math.trunc(i * w / n), xb = Math.trunc((i + 1) * w / n);
        const span = Math.max(xb - xa, 1);
        const y = py[i] + Math.trunc((py[i + 1] - py[i]) * (x - xa) / span);
        const yn = py[i] + Math.trunc((py[i + 1] - py[i]) * (x + 1 - xa) / span);
        const lo = Math.min(y, yn) - 1, hi = Math.max(y, yn) + 1;
        for (let yy = lo - 3; yy < base; yy++) {
          const layer = yy < lo || yy > hi && yy <= hi + 3 ? glow[i] : yy <= hi ? core[i] : fill[i];
          c.blend(x0 + x, yy, layer, light);
        }
      }
    }
  }
  function create() {
    const painter = new SpectrumPainter();
    const analyser = new BandAnalyser();
    const bands = createBands();
    let state = "ok";
    let alive = true;
    const groups = Object.freeze(Array.from({ length: SPECTRUM.groupCount() }, (_, g2) => Object.freeze({
      id: SPECTRUM.groupId(g2),
      labelKey: SPECTRUM.groupLabelKey(g2),
      count: SPECTRUM.tourCount(g2),
      pos: (cfg) => SPECTRUM.tourPos(cfg, g2),
      step: (cfg, shown) => SPECTRUM.tourStep(cfg, shown, g2)
    })));
    return {
      needs,
      schema: SCHEMA,
      presets: PRESETS,
      defaults: () => SPECTRUM.defaults(),
      validate: (obj) => SPECTRUM.validate(obj),
      applyPreset: (cfg, id) => SPECTRUM.applyPreset(cfg, id),
      presetOf: (cfg) => SPECTRUM.preset(cfg),
      resolveFollow: (cfg, seed) => SPECTRUM.resolve(cfg, seed),
      groups: () => groups,
      resize() {
        resetBands(bands);
      },
      // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices.
      frame({ dt, audio, theme, cfg, pixels, full }) {
        if (!alive) return;
        const freq = audio?.freq;
        state = freq?.l ? "ok" : "no-data";
        analyser.run(bands, freq?.l ? freq : null, audio?.rate || 48e3, (audio?.bins || 1024) * 2, dt);
        painter.draw(pixels, { bands, cfg, theme, full: !!full });
      },
      status: () => alive ? state : "off",
      bands: () => bands,
      dispose() {
        alive = false;
      }
    };
  }
  var F3, FINE32, SPOKES, DEPTH, PI32, needs, SpectrumPainter, option, selectOf, hueOf, SCHEMA, PRESETS;
  var init_spectrum = __esm({
    "src/vendor/tunetap/viz/plugins/spectrum.js"() {
      init_pixel();
      init_colour();
      init_page();
      init_cfg();
      init_bands();
      F3 = Math.fround;
      FINE32 = FINE / 2;
      SPOKES = 2 * FINE32;
      DEPTH = 58;
      PI32 = F3(Math.PI);
      needs = Object.freeze({ freq: "stereo", time: "none" });
      SpectrumPainter = class {
        constructor() {
          this.look = null;
          this.cs = new Float32Array(SPOKES);
          this.sn = new Float32Array(SPOKES);
          for (let i = 0; i < SPOKES; i++) {
            const a = -Math.PI / 2 + (i + 0.5) / SPOKES * 2 * Math.PI;
            this.cs[i] = Math.cos(a);
            this.sn[i] = Math.sin(a);
          }
          this.alpha = new Int32Array(DEPTH);
          for (let r = 0; r < DEPTH; r++) this.alpha[r] = Math.trunc(256 * 0.38 * Math.pow(1 - r / DEPTH, 1.5));
        }
        lookOf(v) {
          const pal = palOf(v.theme, v.cfg.duo1, v.cfg.duo2);
          const ramp2 = rampAt(v.cfg.ramp);
          if (!(this.look && this.look.ramp === ramp2 && samePal(this.look.pal, pal))) {
            this.look = { pal, ramp: ramp2, lut: [lut(pal, ramp2, 0), lut(pal, ramp2, 1)] };
          }
          return this.look;
        }
        // The bands round a ring, L clockwise down the right, R down the left.
        radial(c, v, f, m, peaks) {
          const [lv, pk] = fine32(v.bands);
          const [y0, y1] = stage(c);
          const cx = Math.trunc(c.width / 2), cy = y0 + Math.trunc((y1 - y0) / 2) - 1;
          const short = Math.min(c.width, y1 - y0);
          const r0 = Math.trunc(short * 16 / 100), rmax = Math.trunc(short * 24 / 100);
          const floor = Math.trunc(short * 3 / 100);
          const { cs, sn } = this;
          const k2 = this.lookOf(v);
          for (let i = 0; i < SPOKES; i++) {
            const ch = i >= FINE32 ? 1 : 0;
            const j = ch ? SPOKES - 1 - i : i;
            const a = lv[ch][j];
            const ra = r0 + f.i(6), rb = r0 + f.i(6) + floor + Math.trunc(F3(a * (rmax - floor)));
            for (let r = ra; r < rb; r++) {
              const up = F3((r - ra) / (rb - ra));
              const col = lutAt(k2, ch, map(m, j, FINE32, F3(a * up), a));
              const arc = F3(F3(r * PI32) / SPOKES);
              const half = Math.max(F3(arc * F3(F3(0.9) - F3(F3(0.45) * up))), 0.5);
              for (let o = -half; o <= half; o = F3(o + 1)) {
                c.px(cx + Math.trunc(F3(F3(cs[i] * r) - F3(sn[i] * o))), cy + Math.trunc(F3(F3(sn[i] * r) + F3(cs[i] * o))), col);
              }
            }
            if (peaks) {
              const rp = Math.min(r0 + f.i(8) + Math.trunc(F3(pk[ch][j] * rmax)) + f.i(10), r0 + f.i(8) + rmax);
              c.fill(cx + Math.trunc(F3(cs[i] * rp)) - f.i(2), cy + Math.trunc(F3(sn[i] * rp)) - f.i(2), f.px(4), f.px(4), v.theme.fg);
            }
          }
          const big = Math.trunc(short * 12 / 100);
          c.disc(cx, cy, big, v.theme.panel);
          c.ring(cx, cy, r0, v.theme.cyan);
        }
        // A fading mirror image under the baseline: the rows above it, flipped, blended in less and less.
        reflect(c, f, x, w, base) {
          const [y0, y1] = stage(c);
          const px = c.px32, width = c.width;
          const depth = Math.max(f.i(DEPTH), 1), gap = f.i(3);
          for (let r = 0; r < depth; r++) {
            if (base - 1 - r < y0 || base + gap + r >= y1) break;
            const src = (base - 1 - r) * width + x, dst = (base + gap + r) * width + x;
            const a = this.alpha[Math.trunc(r * DEPTH / depth)];
            for (let i = 0; i < w; i++) {
              const d4 = px[dst + i], s = px[src + i];
              let rr = d4 & 255, g2 = d4 >>> 8 & 255, b = d4 >>> 16 & 255;
              rr += ((s & 255) - rr) * a >> 8;
              g2 += ((s >>> 8 & 255) - g2) * a >> 8;
              b += ((s >>> 16 & 255) - b) * a >> 8;
              px[dst + i] = (4278190080 | b << 16 | g2 << 8 | rr) >>> 0;
            }
          }
        }
        draw(c, v) {
          const t6 = v.theme;
          const [y0, y1] = stage(c);
          const width = c.width;
          const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 358));
          const l = (n) => f.i(n);
          const m = mapAt(v.cfg.map);
          const peaks = v.cfg.peaks !== 0 || !!v.full;
          const sh = v.cfg.shape;
          if (sh === SHAPE.RADIAL) {
            this.radial(c, v, f, m, peaks);
            return;
          }
          const refl = v.cfg.reflect !== 0;
          const base = refl ? y1 - l(72) : y1 - l(42);
          const top = base - y0 - (refl ? l(16) : l(26));
          if (top <= 0) return;
          const dash = f.px(3), gap = f.px(6);
          for (let db = 0; db < 4; db++) {
            const y = base - top + Math.trunc(db * top / 4);
            for (let x = l(40); x < width - l(20); x += gap) c.fill(x, y, dash, 1, t6.faint);
          }
          const k2 = this.lookOf(v);
          if (sh === SHAPE.SOLID) shapeSolid(c, v, f, k2, m, peaks, base, top);
          else if (sh === SHAPE.LINE) shapeLine(c, v, f, k2, m, base, top);
          else shapeSeg(c, v, f, k2, m, peaks, base, top);
          if (refl) {
            c.fill(l(40), base + 1, width - l(60), 1, mix(t6.bg, t6.faint, 180));
            this.reflect(c, f, l(40), width - l(60), base);
          }
        }
      };
      option = (k2, i) => ({ value: i, labelKey: valueLabelKey(k2, i) });
      selectOf = (k2) => ({ key: k2, type: "select", labelKey: keyDef(k2).labelKey, options: Array.from({ length: keyDef(k2).n }, (_, i) => option(k2, i)), showWhen: (cfg) => SPECTRUM.rows(cfg).includes(k2) });
      hueOf = (k2) => ({ key: k2, type: "range", labelKey: keyDef(k2).labelKey, min: 0, max: keyDef(k2).n - 1, step: 1, format: "hue", showWhen: (cfg) => SPECTRUM.rows(cfg).includes(k2) });
      SCHEMA = Object.freeze([selectOf("shape"), selectOf("map"), selectOf("peaks"), selectOf("reflect"), selectOf("ramp"), hueOf("duo1"), hueOf("duo2")]);
      PRESETS = Object.freeze(Array.from({ length: SPECTRUM.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: SPECTRUM.presetLabelKey(i), cfg: Object.freeze(SPECTRUM.applyPreset(SPECTRUM.plain(), i)) })));
    }
  });

  // src/vendor/tunetap/viz/anim.js
  function take(acc, dt, stepS, cap, keep) {
    if (stepS <= 0 || cap <= 0) return 0;
    if (dt > 0) acc.v = F4(acc.v + dt);
    if (acc.v < stepS) return 0;
    let n = Math.max(Math.trunc(F4(acc.v / stepS)), 1);
    if (n > cap || !keep) {
      n = Math.min(n, cap);
      acc.v = 0;
    } else {
      acc.v = F4(acc.v - F4(n * stepS));
    }
    return n;
  }
  function steps(acc, dt, stepS, cap) {
    return take(acc, dt, F4(stepS), cap, true);
  }
  function decay(perStep, n) {
    if (n <= 0) return 1;
    if (n === 1) return F4(perStep);
    return F4(Math.pow(F4(perStep), n));
  }
  var F4, STEP_S, STEP_CAP;
  var init_anim = __esm({
    "src/vendor/tunetap/viz/anim.js"() {
      F4 = Math.fround;
      STEP_S = F4(1 / 30);
      STEP_CAP = 30;
    }
  });

  // src/vendor/tunetap/viz/drive-env.js
  function hourAt(m) {
    if (m >= 5 * 60 && m < 7 * 60) return HOUR.DAWN;
    if (m >= 7 * 60 && m < 17 * 60) return HOUR.DAY;
    if (m >= 17 * 60 && m < 19 * 60) return HOUR.DUSK;
    return HOUR.NIGHT;
  }
  function tint(h) {
    return TINTS[Number.isInteger(h) && h >= 0 && h < HOUR.AUTO ? h : HOUR.NIGHT];
  }
  function minuteOfDay(d4) {
    if (!d4 || typeof d4.getHours !== "function") return null;
    const m = d4.getHours() * 60 + d4.getMinutes();
    return Number.isFinite(m) ? m : null;
  }
  var HOUR, WEATHER, TINTS;
  var init_drive_env = __esm({
    "src/vendor/tunetap/viz/drive-env.js"() {
      HOUR = Object.freeze({ NIGHT: 0, DUSK: 1, DAWN: 2, DAY: 3, AUTO: 4 });
      WEATHER = Object.freeze({ CLEAR: 0, RAIN: 1, SNOW: 2, FOG: 3 });
      TINTS = Object.freeze([
        Object.freeze({ top: [0, 0, 0], horizon: [0, 0, 0], amount: 0, stars: 100 }),
        Object.freeze({ top: [40, 10, 70], horizon: [255, 120, 60], amount: 140, stars: 40 }),
        Object.freeze({ top: [30, 40, 90], horizon: [255, 190, 120], amount: 140, stars: 40 }),
        Object.freeze({ top: [70, 140, 230], horizon: [200, 230, 250], amount: 191, stars: 0 })
      ]);
    }
  });

  // src/vendor/tunetap/viz/drive-react.js
  var F5, SLOW, HI_SLOW, KICK_SLOW, GUST, GLINT, KICK_DEV, KICK_A, KICK_B, KICK_MARGIN_MIN, KICK_ARM, KICK_FLOOR, GUST_GAP, GUST_FULL, HI_RISE, HI_FLOOR, sane, React;
  var init_drive_react = __esm({
    "src/vendor/tunetap/viz/drive-react.js"() {
      init_anim();
      F5 = Math.fround;
      SLOW = F5(0.983);
      HI_SLOW = F5(0.95);
      KICK_SLOW = F5(0.97);
      GUST = F5(0.92);
      GLINT = F5(0.62);
      KICK_DEV = F5(0.97);
      KICK_A = F5(0.03);
      KICK_B = F5(12);
      KICK_MARGIN_MIN = F5(0.02);
      KICK_ARM = F5(0.5);
      KICK_FLOOR = F5(0.12);
      GUST_GAP = F5(0.25);
      GUST_FULL = F5(0.3);
      HI_RISE = F5(1.22);
      HI_FLOOR = F5(0.05);
      sane = (v, fallback) => Number.isFinite(v) ? F5(v) : fallback;
      React = class {
        constructor() {
          this.slow = 0;
          this.hiSlow = 0;
          this.kickSlow = 0;
          this.kickDev = 0;
          this.devPeak = 0;
          this.gust = 0;
          this.glint = 0;
          this.since = 0;
          this.acc = { v: 0 };
          this.armed = false;
          this.fires = 0;
          this.heard = false;
        }
        // How far above its average the kick has to come to count as a hit, just now.
        kickMargin() {
          let m = F5(KICK_A * this.kickSlow);
          const b = F5(KICK_B * this.kickDev);
          if (b > m) m = b;
          return m > KICK_MARGIN_MIN ? m : KICK_MARGIN_MIN;
        }
        // One frame of dt seconds, with this frame's bass (0..1: the band loudness itself, not the drive's
        // peak-held kick), the overall loudness and the loudness of the top bands. Not moving (paused)
        // freezes all of it. A NaN or an infinity in any of them is ignored.
        step(dt, moving, bass, loud, high) {
          dt = sane(dt, 0);
          bass = sane(bass, this.kickSlow);
          loud = sane(loud, this.slow);
          high = sane(high, this.hiSlow);
          if (!this.heard) {
            this.slow = loud;
            this.hiSlow = high;
            this.kickSlow = bass;
            this.armed = true;
            this.since = GUST_GAP;
            this.heard = true;
          }
          if (!moving) return;
          this.since = F5(this.since + dt);
          const off = F5(bass - this.kickSlow);
          const n = steps(this.acc, dt, STEP_S, STEP_CAP);
          if (n > 0) {
            let k2 = decay(SLOW, n);
            this.slow = F5(this.slow + F5(F5(loud - this.slow) * F5(1 - k2)));
            k2 = decay(HI_SLOW, n);
            this.hiSlow = F5(this.hiSlow + F5(F5(high - this.hiSlow) * F5(1 - k2)));
            k2 = decay(KICK_SLOW, n);
            this.kickSlow = F5(this.kickSlow + F5(off * F5(1 - k2)));
            k2 = decay(KICK_DEV, n);
            this.kickDev = F5(this.kickDev + F5(F5(this.devPeak - this.kickDev) * F5(1 - k2)));
            this.devPeak = 0;
            this.gust = F5(this.gust * decay(GUST, n));
            this.glint = F5(this.glint * decay(GLINT, n));
          }
          const margin = this.kickMargin();
          const rise = F5(bass - this.kickSlow);
          if (rise <= F5(margin * KICK_ARM)) this.armed = true;
          if (this.armed && bass > KICK_FLOOR && rise > margin && this.since >= GUST_GAP) {
            const strength = F5(F5(0.5) + F5(F5(0.5) * (rise > GUST_FULL ? 1 : F5(rise / GUST_FULL))));
            if (strength > this.gust) this.gust = strength;
            this.armed = false;
            this.since = 0;
            this.fires++;
          }
          if (this.armed && off > this.devPeak) this.devPeak = off;
          if (high > HI_FLOOR && high > F5(this.hiSlow * HI_RISE)) this.glint = 1;
        }
      };
    }
  });

  // src/vendor/tunetap/viz/plugins/drive.js
  var drive_exports = {};
  __export(drive_exports, {
    DrivePainter: () => DrivePainter,
    HOUR_KEY: () => HOUR_KEY,
    ROUTE: () => ROUTE,
    create: () => create2,
    needs: () => needs2
  });
  function loudness(lv, lo, hi) {
    let s = 0;
    for (let b = lo; b <= hi; b++) s = F6(s + F6(lv[0][b] + lv[1][b]));
    return F6(s / (2 * (hi - lo + 1)));
  }
  function roadside(c, lay, route, side, z, hz, y1, glow) {
    const k2 = 1 / z;
    const y = hz + Math.trunc((y1 - hz) * 1.2 * k2) - lay.l(6);
    if (y <= hz + lay.l(2) || y > y1 + lay.l(40)) return;
    const x = lay.cx + d(side * lay.l(330) * (y - hz), y1 - hz);
    const h = Math.trunc(120 * lay.s * k2);
    const w = d(h, 12) + 1;
    const dark = rgb(14, 6, 30);
    if (route === COAST) {
      c.fill(x - d(w, 2), y - h, w, h, dark);
      for (let f = 0; f < 5; f++) {
        const a = (f - 2) * 0.55;
        const lean = f < 2 ? -1 : 1;
        c.line(x, y - h, x + Math.trunc(h * 0.45 * Math.sin(a + PI / 2 * lean * 0.8)), y - h + Math.trunc(h * 0.18 * Math.abs(Math.cos(a))) + d(h, 10), w, dark);
      }
    } else if (route === AURORA) {
      for (let r = 0; r < h; r++) {
        const half = r % (d(h, 3) + 1) * w + d(r, 4);
        c.fill(x - d(half, 2), y - h + r, half + 1, 1, r < 2 ? rgb(56, 76, 92) : rgb(10, 24, 30));
      }
    } else {
      const arm = d(side * h, 5);
      c.fill(x - d(w, 2), y - h, w, h, dark);
      c.fill(x - arm, y - h, d(h, 5) + 1, w, dark);
      c.fill(x - arm - w, y - h + w, w * 2 + 1, w + 1, route === CITY ? rgb(120, 255, 255) : rgb(255, 200, 120));
      if (glow) {
        const r2 = w * w * 6;
        for (let gy = -w * 2; gy <= w * 3; gy++) {
          for (let gx = -w * 2; gx <= w * 3; gx++) if (gx * gx + gy * gy < r2) c.add(x - arm + gx, y - h + w + gy, 24, 20, 8);
        }
      }
    }
  }
  function musicStars(c, v, x, y, w, h, n, seed) {
    if (w <= 0 || h <= 0) return;
    const t6 = v.theme, lv = v.bands.level;
    const dim = t6.light ? mix(t6.black, WHITE, 90) : t6.faint;
    const white = t6.light ? WHITE : t6.fg;
    for (let i = 0; i < n; i++) {
      seed = lcg(seed);
      const sx = x + (seed >>> 8) % w;
      seed = lcg(seed);
      const sy = y + (seed >>> 8) % h;
      const b = (seed >>> 16) % BANDS;
      const l = bandOf(lv, b);
      const bright = F6(l * 255) > 255 ? 255 : F6(60 + F6(l * 195));
      const col = mix(dim, (seed >>> 12) % 2 ? t6.cyan : white, Math.trunc(bright));
      if (l > F055) {
        c.fill(sx - 1, sy, 3, 1, col);
        c.fill(sx, sy - 1, 1, 3, col);
      } else {
        c.px(sx, sy, col);
      }
    }
  }
  function stripedSun(c, v, lay, hz, rd) {
    const r = lay.l(rd);
    const lv = v.bands.level, t6 = v.theme;
    for (let y = -r; y < 0; y++) {
      const h = Math.trunc(Math.sqrt(r * r - y * y));
      const yd = lay.fit.inv(y);
      if (yd > d(-rd, 2) && -yd % 12 < d(d(rd, 2) + yd, 8) + Math.trunc(F6(bandOf(lv, d(-yd, 12) % BANDS) * 5))) continue;
      c.fill(lay.cx - h, hz + y, 2 * h, 1, mix(t6.yel, t6.mag, d((y + r) * 255, r)));
    }
  }
  function stageCrowd(c, lay, w, hz, clockS, kick) {
    const teal = rgb(57, 214, 200), pink = rgb(232, 48, 140);
    const left = Math.min(lay.dx(0), 0), span = Math.max(lay.dx(w), 640) - left;
    const people = d(90 * span, 640);
    const up = Math.trunc(F6(kick * F6(6 * lay.s)));
    const l6 = lay.l(6), thick = lay.fit.px(2);
    for (let i = 0; i < people; i++) {
      const r = Math.imul(i, 2654435761) >>> 0;
      const px0 = lay.x(left + r % span), row2 = (r >>> 12) % 3;
      const py0 = hz + lay.l(8 + row2 * 7);
      c.disc(px0, py0, lay.l(4 + row2), rgb(8, 6, 16));
      if ((r >>> 20) % 5 === 0) continue;
      const ang = 0.5 * Math.sin(clockS * 3 + i) + ((r >>> 8) % 2 ? 0.2 : -0.2);
      const len = lay.l(10 + row2 * 2);
      const col = (r >>> 16) % 2 ? teal : pink;
      const x1 = px0 + Math.trunc(Math.sin(ang) * len);
      const y1 = py0 - l6 - up - Math.trunc(Math.cos(ang) * len);
      c.line(px0, py0 - l6 - up, x1, y1, thick, col);
      c.add(x1, y1, 60, 60, 60);
    }
  }
  function screenColour(v, f, own) {
    const r = rampAt(v.cfg.ramp);
    if (r === RAMP.PALETTE) return own;
    const pal = palOf(v.theme, v.cfg.duo1 ?? 32, v.cfg.duo2 ?? 5);
    pal.light = false;
    return ramp(pal, r, 1, f);
  }
  function hearAmp(cfg, soft) {
    if (cfg.weather_react === WEATHER_REACT.STRONG) return 1;
    if (cfg.weather_react === WEATHER_REACT.SOFT) return soft;
    return 0;
  }
  function hearSwing(slow) {
    const s = F6(F6(slow - 0.5) / HEAR_SPAN);
    return s < -1 ? -1 : s > 1 ? 1 : s;
  }
  function weatherHear(cfg, r, kick, seed) {
    const w = { n: 300, lr: 60, lg: 70, lb: 95, gust: 0, glint: 0, grow: false, fogA: 150 };
    const amp = hearAmp(cfg, AMP_RAIN), gentle = hearAmp(cfg, AMP_GENTLE);
    if (amp <= 0) return w;
    const s = hearSwing(r.slow);
    const swing = s > 0 ? F6(F6(0.4) * s) : F6(F6(0.6) * s);
    const count = F6(F6(300 * F6(1 + F6(amp * swing))) + F6(F6(amp * 120) * kick));
    w.n = Math.min(Math.max(Math.trunc(count), 120), 420);
    const flare = F6(F6(amp * F6(0.55)) * kick);
    w.lr += Math.trunc(F6(60 * flare));
    w.lg += Math.trunc(F6(70 * flare));
    w.lb += Math.trunc(F6(95 * flare));
    w.gust = Math.trunc(F6(F6(gentle * 14) * r.gust)) * (seed >>> 3 & 1 ? 1 : -1);
    w.glint = Math.trunc(F6(F6(255 * amp) * r.glint));
    w.grow = cfg.weather_react === WEATHER_REACT.STRONG && r.glint > F06;
    w.fogA = 150 + Math.trunc(F6(F6(F6(150 * gentle) * F6(0.625)) * s));
    return w;
  }
  function weatherLayer(c, lay, kind, clockS, y0, hz, y1, w) {
    const width = c.width, h = y1 - y0;
    if (h <= 0 || width <= 0) return;
    const more = (n) => width === 640 && h === 404 ? n : Math.trunc(n * width * h / (640 * 404));
    if (kind === WEATHER.RAIN) {
      const n = more(w.n);
      for (let i = 0; i < n; i++) {
        const r = Math.imul(i + 1, 2654435761) >>> 0;
        const len = 6 + (r >>> 28);
        const x = r % (width + 40);
        const fall = u32(clockS * ((420 + (r >>> 24 & 63)) * lay.s));
        const y = y0 + ((r >>> 8) % h + fall | 0) % h;
        for (let k2 = 0; k2 < len && y + k2 < y1; k2++) c.add(x - d(k2, 3), y + k2, w.lr, w.lg, w.lb);
      }
    } else if (kind === WEATHER.SNOW) {
      const flake = rgb(240, 244, 255);
      const glinting = w.glint !== 0 ? mix(flake, WHITE, w.glint) : flake;
      const gust = lay.l(w.gust);
      const n = more(260);
      for (let i = 0; i < n; i++) {
        const r = Math.imul(i + 1, 2654435761) >>> 0;
        const big = (r >>> 27 & 3) === 0 ? 1 : 0;
        const fall = u32(clockS * ((28 + (r >>> 24 & 31)) * lay.s));
        const y = y0 + ((r >>> 8) % h + fall | 0) % h;
        let x = r % width + Math.trunc(6 * lay.s * Math.sin(clockS * 0.9 + i));
        const lit = big !== 0 && w.glint !== 0;
        const grow = big !== 0 && w.grow ? 1 : 0;
        x += big !== 0 ? d(gust * 3, 5) : gust;
        if (y + big < y1) {
          const s = lay.fit.px(1 + big + grow);
          c.fill(x, y, s, s, lit ? glinting : flake);
        }
      }
    } else if (kind === WEATHER.FOG) {
      const reach = lay.l(50);
      for (let y = hz - reach; y < hz + lay.l(70) && y < y1; y++) {
        const dd = y < hz ? hz - y : d((y - hz) * 5, 7);
        const a = reach > 0 ? d(w.fogA * (reach - dd), reach) : 0;
        if (a <= 0 || y < y0) continue;
        const row2 = c.rowMut(y);
        if (!row2) continue;
        for (let i = 0; i < row2.length; i++) {
          const p = row2[i];
          let r = p & 255, g2 = p >>> 8 & 255, b = p >>> 16 & 255;
          r += (120 - r) * a >> 8;
          g2 += (130 - g2) * a >> 8;
          b += (160 - b) * a >> 8;
          row2[i] = rgb(r, g2, b);
        }
      }
    }
  }
  function envHour(cfg, v) {
    if (cfg.hour !== HOUR.AUTO) return cfg.hour;
    return v.minuteOfDay === null || v.minuteOfDay === void 0 ? HOUR.NIGHT : hourAt(v.minuteOfDay);
  }
  function envWeather(cfg) {
    const w = cfg.weather;
    return w === WEATHER.RAIN || w === WEATHER.SNOW || w === WEATHER.FOG ? w : WEATHER.CLEAR;
  }
  function create2(args = {}) {
    const clock = typeof args.clock === "function" ? args.clock : () => /* @__PURE__ */ new Date();
    const painter = new DrivePainter();
    const analyser = new BandAnalyser();
    const bands = createBands();
    let seed = args.seed >>> 0;
    let playedS = 0;
    let state = "no-data";
    let alive = true;
    const groups = Object.freeze(Array.from({ length: DRIVE.groupCount() }, (_, g2) => Object.freeze({
      id: DRIVE.groupId(g2),
      labelKey: DRIVE.groupLabelKey(g2),
      count: DRIVE.tourCount(g2),
      pos: (cfg) => DRIVE.tourPos(cfg, g2),
      step: (cfg, shown) => DRIVE.tourStep(cfg, shown, g2)
    })));
    return {
      needs: needs2,
      schema: SCHEMA2,
      presets: PRESETS2,
      defaults: () => DRIVE.defaults(),
      validate: (obj) => DRIVE.validate(obj),
      applyPreset: (cfg, id) => DRIVE.applyPreset(cfg, id),
      presetOf: (cfg) => DRIVE.preset(cfg),
      resolveFollow: (cfg, s) => DRIVE.resolve(cfg, s),
      groups: () => groups,
      resize() {
      },
      // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices; playing: the
      // scene's clock and road run only while the player plays.
      frame({ dt, audio, theme, cfg, playing, pixels, seed: s }) {
        if (!alive) return;
        if (s !== void 0) seed = s >>> 0;
        dt = F6(Number.isFinite(dt) && dt > 0 ? dt : 0);
        const freq = audio?.freq;
        const live = !!freq?.l;
        state = live ? "ok" : "no-data";
        if (live) analyser.run(bands, freq, audio.rate || 48e3, (audio.bins || 1024) * 2, dt);
        else resetBands(bands);
        const moving = !!playing;
        if (moving) playedS += dt;
        const shown = live ? cfg : { ...cfg, ...CALM };
        const minute = shown.hour === HOUR.AUTO ? minuteOfDay((() => {
          try {
            return clock();
          } catch {
            return null;
          }
        })()) : null;
        painter.draw(pixels, { bands, cfg: shown, theme, dt, moving, seed, posMs: Math.trunc(playedS * 1e3), minuteOfDay: minute });
      },
      status: () => alive ? state : "off",
      dispose() {
        alive = false;
      }
    };
  }
  var F6, d, lcg, u32, PI, SUNSET, CITY, COAST, AURORA, STAGE, WHITE, BLACK, F045, F055, F06, F04, F09, needs2, Lay, bandOf, AMP_RAIN, AMP_GENTLE, HEAR_SPAN, DrivePainter, option2, selectOf2, SCHEMA2, PRESETS2, CALM;
  var init_drive = __esm({
    "src/vendor/tunetap/viz/plugins/drive.js"() {
      init_pixel();
      init_colour();
      init_page();
      init_anim();
      init_cfg();
      init_bands();
      init_drive_env();
      init_drive_react();
      F6 = Math.fround;
      d = (a, b) => Math.trunc(a / b);
      lcg = (s) => Math.imul(s, 1103515245) + 12345 >>> 0;
      u32 = (v) => v > 0 ? v < 4294967295 ? Math.trunc(v) : 4294967295 : 0;
      PI = Math.PI;
      SUNSET = 0;
      CITY = 1;
      COAST = 2;
      AURORA = 3;
      STAGE = 4;
      WHITE = rgb(255, 255, 255);
      BLACK = rgb(0, 0, 0);
      F045 = F6(0.45);
      F055 = F6(0.55);
      F06 = F6(0.6);
      F04 = F6(0.4);
      F09 = F6(0.9);
      needs2 = Object.freeze({ freq: "mono", time: "none" });
      Lay = class {
        constructor(fit, cx, top, sky) {
          this.fit = fit;
          this.s = fit.f();
          this.cx = cx;
          this.top = top;
          this.sky = sky;
        }
        x(x) {
          return this.cx + this.fit.i(x - 320);
        }
        l(v) {
          return this.fit.i(v);
        }
        dx(x) {
          return 320 + this.fit.inv(x - this.cx);
        }
        dxf(x) {
          return 320 + (x - this.cx) / this.s;
        }
      };
      bandOf = (lv, b) => F6(F6(lv[0][b] + lv[1][b]) / 2);
      AMP_RAIN = F6(0.58);
      AMP_GENTLE = F6(0.4);
      HEAR_SPAN = F6(0.2);
      DrivePainter = class {
        constructor() {
          this.road = 0;
          this.clockS = 0;
          this.kick = 0;
          this.kickAcc = { v: 0 };
          this.react = new React();
          this.meteorList = Array.from({ length: 4 }, () => ({ x: 0, y: 0, t: 0 }));
          this.lastBass = 0;
          this.beam = 0;
        }
        // Shooting stars on the kicks, somewhere in the upper sky.
        meteors(c, v, lay, bass, y0, h, sky, salt) {
          if (v.moving && bass > 0.5 && this.lastBass <= 0.5) {
            const m = this.meteorList.find((q) => q.t <= 0);
            if (m) {
              const r = Math.imul((salt >>> 0) + (Math.trunc(v.posMs / 97) >>> 0) >>> 0, 2654435761) >>> 0;
              m.x = F6(lay.x(40 + (r >>> 8) % 480));
              const span = h > 16 ? h - 16 : 1;
              m.y = F6((y0 + 8 >>> 0) + (r >>> 20) % span >>> 0);
              m.t = 1;
            }
          }
          this.lastBass = bass;
          const run = F6(180 * lay.s), tail = Math.max(lay.l(46), 2);
          const head = v.theme.light ? WHITE : v.theme.fg;
          for (const m of this.meteorList) {
            if (!(m.t > 0)) continue;
            const dd = F6(F6(1 - m.t) * run);
            for (let k2 = 0; k2 < tail; k2 += 2) {
              c.px(Math.trunc(F6(F6(m.x + dd) - k2)), Math.trunc(F6(m.y + F6(F6(dd - k2) * F04))), mix(head, sky, d(k2 * 255, tail)));
            }
            m.t = F6(m.t - F6(v.dt * 1.5));
          }
        }
        draw(c, v) {
          const y0 = c.y0, y1 = c.y1, width = c.width;
          const t6 = v.theme, cfg = v.cfg, lv = v.bands.level;
          const dt = F6(v.dt > 0 ? v.dt : 0);
          const vv = { ...v, dt };
          const moving = !!v.moving;
          const bass = loudness(lv, 0, 3);
          this.kick = bass > this.kick ? bass : F6(this.kick * decay(F09, steps(this.kickAcc, dt, STEP_S, STEP_CAP)));
          const loud = loudness(lv, 0, 15);
          if (moving) {
            this.road += dt * (0.8 + 2.5 * loud);
            this.clockS += dt;
          }
          this.react.step(dt, moving, bass, loud, loudness(lv, 12, 15));
          const route = Math.min(Math.max((cfg.route | 0) - 1, 0), 4);
          const kick = this.kick;
          const ph = y1 - y0;
          const fit = Fit.min(Fit.new(width, 640), Fit.new(ph, 404));
          const fh = fit.i(404);
          const hz = y0 + ph - (fh - d(fh * 16, 25));
          const sky = hz - y0;
          if (sky <= 0 || width <= 0) return;
          const lay = new Lay(fit, d(width, 2), hz - d(fh * 16, 25), d(fh * 16, 25));
          const cx = lay.cx, l = (n) => lay.l(n);
          const band = (b) => bandOf(lv, b);
          let top, bottom;
          if (route === CITY) {
            top = rgb(4, 2, 16);
            bottom = rgb(46, 12, 80);
          } else if (route === COAST) {
            top = rgb(3, 5, 20);
            bottom = rgb(28, 40, 92);
          } else if (route === AURORA) {
            top = rgb(2, 4, 14);
            bottom = rgb(10, 26, 46);
          } else if (route === STAGE) {
            top = rgb(4, 2, 12);
            bottom = rgb(34, 10, 54);
          } else {
            top = rgb(8, 4, 24);
            bottom = rgb(130, 24, 110);
          }
          const weather = envWeather(cfg);
          const carOn = cfg.car === CAR.ON;
          const tint2 = tint(envHour(cfg, v));
          if (tint2.amount !== 0) {
            const tt = tint2.top, th = tint2.horizon;
            top = mix(top, rgb(tt[0], tt[1], tt[2]), tint2.amount);
            bottom = mix(bottom, rgb(th[0], th[1], th[2]), tint2.amount);
          }
          if (weather === WEATHER.RAIN || weather === WEATHER.FOG) {
            top = mix(top, BLACK, 64);
            bottom = mix(bottom, rgb(40, 44, 60), 64);
          }
          for (let y = y0; y < hz; y++) c.fill(0, y, width, 1, mix(top, bottom, d((y - y0) * 255, sky)));
          const wideSky = route === AURORA || route === COAST;
          const starRows = wideSky ? d(sky * 3, 4) : d(sky, 2), starBase = wideSky ? 110 : 50;
          const clearSky = weather === WEATHER.CLEAR ? 2 : 1;
          const stars = d(d(d(starBase * tint2.stars, 100) * clearSky, 2) * width, 640);
          musicStars(c, vv, 0, y0, width, starRows, stars, 5 + route);
          if (route === SUNSET) this.meteors(c, vv, lay, bass, y0, d(sky, 3), top, v.seed >>> 0);
          let grid = t6.mag;
          if (route === SUNSET) {
            const step = Math.max(l(4), 1), bar = Math.max(l(3), 1);
            for (let x = 0; x < width; x += step) {
              const b = Math.min(Math.max(d(lay.dx(x) * BANDS, 640), 0), BANDS - 1);
              const gh = l(Math.trunc(F6(band(b) * 80)));
              for (let k2 = 0; k2 < gh; k2 += 2) c.fill(x, hz - l(30) - k2, bar, 1, mix(t6.mag, top, 60 + d(lay.fit.inv(k2) * 190, 80)));
            }
            stripedSun(c, vv, lay, hz, 96 + Math.trunc(F6(kick * 16)));
            for (let x = 0; x < width; x += 2) {
              const fx = lay.dxf(x) + this.road * 6;
              let m = Math.trunc(26 + 12 * Math.sin(fx * 0.013) + 7 * Math.sin(fx * 0.041 + 2));
              const dx = lay.dx(x);
              if (dx > 200 && dx < 440) m = d(m, 2);
              c.fill(x, hz - l(m), 2, l(m), rgb(24, 8, 44));
            }
            for (let x = 0; x < width; x += 2) {
              const fx = lay.dxf(x) + this.road * 16;
              let m = Math.trunc(18 + 14 * Math.sin(fx * 0.021) + 8 * Math.sin(fx * 0.057 + 1));
              const dx = lay.dx(x);
              if (dx > 240 && dx < 400) m = d(m, 3);
              c.fill(x, hz - l(m), 2, l(m), rgb(36, 12, 60));
            }
          } else if (route === CITY) {
            const my = lay.top + d(lay.sky, 4);
            c.disc(lay.x(500), my, l(30), mix(bottom, t6.moon, 40));
            c.disc(lay.x(500), my, l(24), t6.moon);
            c.disc(lay.x(508), my - l(6), l(5), mix(t6.moon, BLACK, 30));
            const l0 = lv[0], win = Math.max(l(3), 1);
            const block = (rs2, x) => {
              rs2.r = lcg(rs2.r);
              const bw = 24 + (rs2.r >>> 16) % 40;
              let bh = 40 + (rs2.r >>> 8) % 90;
              if (x > 260 && x < 380) bh = d(bh, 2);
              c.fill(lay.x(x), hz - l(bh), lay.x(x + bw - 2) - lay.x(x), l(bh), rgb(16, 8, 36));
              for (let up = bh - 6; up > 4; up -= 7) {
                for (let wx = x + 4; wx < x + bw - 6; wx += 6) {
                  rs2.r = lcg(rs2.r);
                  const lit = (rs2.r >>> 20) % 5 === 0;
                  const flick = (rs2.r >>> 12) % 16 < Math.trunc(F6(l0[(rs2.r >>> 8) % 16] * 12));
                  if (lit || flick) c.fill(lay.x(wx), hz - l(up), win, win, flick ? t6.cyan : rgb(255, 190, 90));
                }
              }
              return bw;
            };
            let rs = { r: 12345 };
            for (let x = 0; lay.x(x) < width; ) x += block(rs, x);
            rs = { r: 54321 };
            for (let x = 0; lay.x(x) > 0; ) {
              const probe = lcg(rs.r);
              x -= 24 + (probe >>> 16) % 40;
              block(rs, x);
            }
            const tx = lay.x(150), tw = l(56), th = lay.sky - l(58);
            c.fill(tx, hz - th, tw, th, rgb(20, 10, 44));
            c.fill(tx + d(tw, 2) - l(1), hz - th - l(40), l(3), l(40), rgb(40, 30, 70));
            const tip = kick > F045 ? t6.warn : mix(t6.warn, BLACK, 150);
            c.fill(tx + d(tw, 2) - l(3), hz - th - l(44), l(7), l(5), tip);
            if (kick > F045) {
              const reach = l(12), num3 = lay.fit.i(1e3), den = 1e3;
              const lim = 140 * num3 * num3;
              for (let gy = -reach; gy <= reach; gy++) {
                for (let gx = -reach; gx <= reach; gx++) {
                  if ((gx * gx + gy * gy) * den * den < lim) c.add(tx + d(tw, 2) + gx, hz - th - l(42) + gy, 40, 6, 10);
                }
              }
            }
            for (let b = 0; b < BANDS; b++) {
              const bx = tx + l(4 + b * 3), bh = l(6 + Math.trunc(F6(band(b) * 70)));
              const own = mix(t6.cyan, t6.mag, d(b * 255, BANDS));
              c.fill(bx, hz - th + l(90) - bh, Math.max(l(2), 1), bh, screenColour(vv, F6(b / (BANDS - 1)), own));
            }
            c.frame(tx + l(2), hz - th + l(14), tw - l(4), l(80), t6.mag);
            for (let wy = 104; hz - th + l(wy) < hz - l(6); wy += 9) {
              c.fill(tx + l(8), hz - th + l(wy), tw - l(16), Math.max(l(2), 1), mix(t6.cyan, BLACK, 120));
            }
            grid = t6.cyan;
          } else if (route === COAST) {
            const mx = lay.x(220), my = lay.top + d(lay.sky, 3);
            const reach = l(60);
            for (let gy = -reach; gy <= reach; gy++) {
              const dy = lay.fit.inv(gy);
              for (let gx = -reach; gx <= reach; gx += 2) {
                const dx = lay.fit.inv(gx);
                const d22 = dx * dx + dy * dy;
                if (d22 < 3600 && d22 > 900) {
                  const f = d(3600 - d22, 200);
                  c.add(mx + gx, my + gy, d(f, 2), d(f, 2), f);
                  c.add(mx + gx + 1, my + gy, d(f, 2), d(f, 2), f);
                }
              }
            }
            c.disc(mx, my, l(28), t6.moon);
            c.disc(mx - l(8), my - l(6), l(6), mix(t6.moon, BLACK, 25));
            c.disc(mx + l(9), my + l(8), l(4), mix(t6.moon, BLACK, 25));
            for (let k2 = 0; k2 < 20; k2++) {
              const y = hz - l(40 - 2 * k2), yn = hz - l(38 - 2 * k2);
              c.fill(0, y, width, yn - y, mix(rgb(8, 12, 40), rgb(20, 30, 70), d(k2 * 255, 20)));
              const b = band(k2 % BANDS);
              const w = l(16 + (40 - 2 * k2) * 3 + Math.trunc(F6(b * 120)));
              c.fill(mx - d(w, 2) + Math.trunc(6 * lay.s * Math.sin(this.road * 3 + y)), y, w, 1, mix(rgb(150, 160, 200), t6.fg, Math.trunc(F6(b * 255))));
            }
            const lx = lay.x(548), ly = hz - l(40);
            for (let y = ly - l(6); y < hz; y++) c.fill(lx - l(44) + (y - ly) * 2, y, l(150), 1, rgb(14, 6, 30));
            c.fill(lx - l(5), ly - l(52), l(11), l(46), rgb(230, 220, 230));
            c.fill(lx - l(5), ly - l(40), l(11), l(6), rgb(255, 60, 90));
            c.fill(lx - l(5), ly - l(24), l(11), l(6), rgb(255, 60, 90));
            c.fill(lx - l(7), ly - l(60), l(15), l(8), rgb(40, 20, 60));
            this.beam += dt * (moving ? 0.9 + kick * 2.5 : 0);
            const a = this.beam;
            const facing = Math.abs(Math.sin(a));
            const bright = 40 + Math.trunc(facing * 70 + kick * 40);
            const lamp = ly - l(57);
            const reachB = l(330), from = l(6);
            for (let r = -3; r <= 3; r++) {
              const ang = a + r * 0.025;
              const dx = Math.cos(ang), dy = Math.sin(ang) * 0.12;
              const side = 4 - Math.abs(r);
              for (let dd = from; dd < reachB; dd++) {
                const f = d(d(bright * (reachB - dd), reachB) * side, 4);
                if (f > 2) c.add(lx + Math.trunc(dx * dd), lamp + Math.trunc(dy * dd), f, d(f * 9, 10), d(f * 6, 10));
              }
            }
            const glow = l(6);
            for (let gy = -glow; gy <= glow; gy++) {
              const dy = lay.fit.inv(gy);
              for (let gx = -glow; gx <= glow; gx++) {
                const dx = lay.fit.inv(gx);
                if (dx * dx + dy * dy < 40) c.add(lx + gx, lamp + gy, 90, 80, 50);
              }
            }
          } else if (route === AURORA) {
            const tt = this.clockS * 0.35;
            const turn = Math.trunc(tt * 3);
            for (let x = 0; x < width; x += 2) {
              const fx = lay.dxf(x);
              const wave = 18 * Math.sin(fx * 0.011 + tt) + 10 * Math.sin(fx * 0.027 - tt * 1.7);
              const base = lay.top + d(lay.sky, 2) + Math.trunc(wave * lay.s);
              const bi = ((d(lay.dx(x) * BANDS, 640) + turn) % BANDS + BANDS) % BANDS;
              const h = Math.max(l(30 + Math.trunc(F6(band(bi) * 110))), 1);
              for (let k2 = 0; k2 < h; k2++) {
                let f = d((h - k2) * 120, h);
                if (k2 < 6) f = d(f * (k2 + 1), 7);
                const fk = d(f * k2, h + 1);
                const g2 = f, r = d(fk * 2, 3), b = d(f, 2) + d(fk, 2);
                c.add(x, base - k2, r, g2, b);
                c.add(x + 1, base - k2, d(r, 2), d(g2, 2), d(b, 2));
              }
            }
            for (let x = 0; x < width; x += 2) {
              const fx = lay.dxf(x) + this.road * 5;
              const a = fx / 190 - Math.floor(fx / 190);
              const b = fx / 53 + 0.3 - Math.floor(fx / 53 + 0.3);
              const t1 = 1 - Math.abs(a * 2 - 1), t22 = 1 - Math.abs(b * 2 - 1);
              const m = Math.trunc(18 + 62 * t1 * t1 + 12 * t22);
              c.fill(x, hz - l(m), 2, l(m), rgb(18, 26, 40));
              if (m > 56) c.fill(x, hz - l(m), 2, l(d(m - 56, 2) + 1), rgb(74, 96, 122));
            }
            grid = rgb(80, 200, 220);
          } else {
            const teal = rgb(57, 214, 200), pink = rgb(232, 48, 140);
            const sx0 = lay.x(150), sx1 = lay.x(490), floorY = hz - l(18), tr = lay.top + d(lay.sky, 5);
            const reach = l(420), from = l(4), stop = hz + l(30), down = tr + l(8);
            const bright = 26 + Math.trunc(F6(kick * 40));
            for (let li = 0; li < 6; li++) {
              const lx = sx0 + l(20) + d(li * (sx1 - sx0 - l(40)), 5);
              const sign = li % 2 ? 1 : -1;
              const a = PI / 2 + 0.55 * Math.sin(this.clockS * (0.7 + 0.13 * li) + li * 1.3) * sign;
              const cr = li % 3 === 0 ? 57 : li % 3 === 1 ? 232 : 150;
              const cg = li % 3 === 0 ? 214 : li % 3 === 1 ? 48 : 80;
              const cb = li % 3 === 0 ? 200 : li % 3 === 1 ? 140 : 255;
              const ca = Math.cos(a), sa = Math.sin(a);
              for (let dd = from; dd < reach; dd++) {
                const half = d(dd, 14);
                const bx = lx + Math.trunc(ca * dd);
                const by2 = down + Math.trunc(sa * dd);
                if (by2 > stop) break;
                const f = d(bright * (reach - dd), reach);
                const ar = d(cr * f, 255), ag = d(cg * f, 255), ab = d(cb * f, 255);
                for (let w = -half; w <= half; w += 2) c.add(bx + w, by2, ar, ag, ab);
              }
            }
            c.fill(sx0 - l(10), tr, sx1 - sx0 + l(20), l(4), rgb(60, 60, 80));
            for (let x = sx0 - l(10); x < sx1 + l(10); x += Math.max(l(8), 1)) c.line(x, tr, x + l(8), tr + l(4), 1, rgb(40, 40, 56));
            c.fill(sx0 - l(10), tr, l(4), floorY - tr, rgb(50, 50, 70));
            c.fill(sx1 + l(6), tr, l(4), floorY - tr, rgb(50, 50, 70));
            for (let li = 0; li < 6; li++) {
              c.fill(sx0 + l(16) + d(li * (sx1 - sx0 - l(40)), 5), tr + l(4), l(9), l(6), kick > F045 ? t6.fg : rgb(120, 120, 140));
            }
            const wx = lay.x(210), ww = l(220), wy = tr + l(20);
            const wh = floorY - l(62) - wy;
            const whn = wh !== 0 ? wh : 1;
            const step = Math.max(l(3), 1), seg = Math.max(l(2), 1);
            c.fill(wx - l(3), wy - l(3), ww + l(6), wh + l(6), rgb(10, 8, 20));
            const barW = d(ww - l(4), BANDS) - l(2);
            for (let b = 0; b < BANDS; b++) {
              const bh = Math.trunc(F6(band(b) * wh));
              const bx = wx + l(2) + d(b * (ww - l(4)), BANDS);
              for (let yy = 0; yy + seg <= bh; yy += step) {
                const own = mix(teal, pink, d(yy * 255, whn));
                c.fill(bx, wy + wh - yy - seg, barW, seg, screenColour(vv, F6(yy / whn), own));
              }
            }
            c.fill(sx0 - l(20), floorY, sx1 - sx0 + l(40), l(18), rgb(24, 18, 40));
            c.fill(sx0 - l(20), floorY, sx1 - sx0 + l(40), l(2), mix(teal, BLACK, 60));
            const sway = Math.trunc(3 * lay.s * Math.sin(this.clockS * 4));
            const hop = Math.trunc(F6(kick * F6(3 * lay.s)));
            const cx0 = cx + sway, fy = floorY - hop;
            for (const side of [-1, 1]) {
              for (let k2 = 0; k2 < 34; k2++) {
                const tx = cx0 + side * l(6 + d(k2, 5)) + Math.trunc(side * 2 * lay.s * Math.sin(this.clockS * 5 + k2 * 0.2));
                c.fill(tx - l(1), fy - l(52 - k2), l(3), Math.max(l(1), 1), teal);
              }
            }
            const body = rgb(20, 16, 30);
            c.disc(cx0, fy - l(50), l(7), body);
            c.fill(cx0 - l(9), fy - l(55), l(3), l(3), pink);
            c.fill(cx0 + l(7), fy - l(55), l(3), l(3), pink);
            c.fill(cx0 - l(6), fy - l(42), l(13), l(18), body);
            c.fill(cx0 - l(5), fy - l(24), l(4), l(24), body);
            c.fill(cx0 + l(2), fy - l(24), l(4), l(24), body);
            c.line(cx0 + l(6), fy - l(38), cx0 + l(14), fy - l(48) - hop * 3, l(3), body);
            grid = pink;
          }
          c.fill(0, hz, width, y1 - hz, route === AURORA ? rgb(6, 14, 22) : rgb(10, 3, 24));
          const fr = this.road - Math.floor(this.road);
          for (let i = 0; i < 14; i++) {
            const z = i + 1 - fr;
            const y = hz + Math.trunc((y1 - hz) * 1.2 / z) - l(6);
            if (y > hz && y < y1) c.fill(0, y, width, 1, mix(grid, t6.bg, 60 + Math.trunc(z * 10)));
          }
          const rule = mix(grid, t6.bg, 80);
          const spread = 80 * lay.s;
          const lines = Math.max(12, Math.trunc(width / 2 / spread) + 1);
          const span = y1 - hz;
          for (let i = -lines; i <= lines; i++) {
            for (let y = hz + 2; y < y1; y += 2) c.px(cx + Math.trunc(i * 50 * (y - hz) / span * 1.6 * lay.s), y, rule);
          }
          if (route === STAGE) stageCrowd(c, lay, width, hz, this.clockS, kick);
          const sfr = this.road * 0.5 - Math.floor(this.road * 0.5);
          for (let i = 7; i >= 0; i--) {
            const z = (i + 1 - sfr) * 2;
            roadside(c, lay, route, -1, z, hz, y1, route === CITY);
            roadside(c, lay, route, 1, z, hz, y1, route === CITY);
          }
          const by = y1 - l(56) + (moving ? Math.trunc(2 * lay.s * Math.sin(this.road * 12)) : 0);
          c.fill(cx - l(50), by, l(100), l(24), rgb(30, 20, 50));
          c.fill(cx - l(42), by - l(18), l(84), l(20), rgb(50, 30, 80));
          const glass = carOn ? route === CITY ? t6.mag : t6.cyan : route === CITY ? rgb(200, 60, 200) : rgb(20, 180, 220);
          c.fill(cx - l(35), by - l(15), l(70), l(11), glass);
          const kk = Math.min(Math.trunc(F6(kick * 255)), 255);
          const tail = carOn ? mix(mix(t6.mag, BLACK, 110), t6.mag, kk) : mix(rgb(160, 20, 50), rgb(255, 90, 120), kk);
          if (kick > F045) {
            for (let gy = -l(6); gy <= l(14); gy++) {
              for (let gx = -l(8); gx <= l(30); gx++) {
                c.add(cx - l(48) + gx, by + l(4) + gy, 30, 0, 8);
                c.add(cx + l(26) + gx, by + l(4) + gy, 30, 0, 8);
              }
            }
          }
          c.fill(cx - l(48), by + l(6), l(22), l(7), tail);
          c.fill(cx + l(26), by + l(6), l(22), l(7), tail);
          c.fill(cx - l(58), by + l(20), l(22), l(14), rgb(8, 6, 12));
          c.fill(cx + l(36), by + l(20), l(22), l(14), rgb(8, 6, 12));
          const hear = weatherHear(cfg, this.react, kick, v.seed >>> 0);
          weatherLayer(c, lay, weather, this.clockS, y0, hz, y1, hear);
        }
      };
      option2 = (k2, i) => ({ value: i, labelKey: valueLabelKey(k2, i) });
      selectOf2 = (k2) => ({ key: k2, type: "select", labelKey: keyDef(k2).labelKey, options: Array.from({ length: keyDef(k2).n }, (_, i) => option2(k2, i)) });
      SCHEMA2 = Object.freeze(DRIVE.keys.map(selectOf2));
      PRESETS2 = Object.freeze(Array.from({ length: DRIVE.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: DRIVE.presetLabelKey(i), cfg: Object.freeze(DRIVE.applyPreset(DRIVE.plain(), i)) })));
      CALM = Object.freeze({ weather_react: WEATHER_REACT.OFF });
    }
  });

  // src/vendor/tunetap/viz/tables.js
  function buildTables(pal, ramp2, serial) {
    const r = rampAt(ramp2);
    const t6 = { pal, ramp: ramp2, serial, plain: [lut(pal, r, 0), lut(pal, r, 1)], loud: [lutLoud(pal, r, 0), lutLoud(pal, r, 1)], glow: [new Uint32Array(256), new Uint32Array(256)] };
    for (let ch = 0; ch < 2; ch++) {
      for (let g2 = 0; g2 < 256; g2++) {
        const c = ramp(pal, r, ch, F7(F7(0.35) + F7(F7(F7(0.65) * g2) / 255)));
        t6.glow[ch][g2] = tone(c, 64 + Math.trunc(191 * g2 / 255), pal.light);
      }
    }
    return t6;
  }
  var F7, TableCache;
  var init_tables = __esm({
    "src/vendor/tunetap/viz/tables.js"() {
      init_colour();
      F7 = Math.fround;
      TableCache = class {
        constructor() {
          this.t = null;
          this.serial = 0;
        }
        // The tables for this frame's theme and the choices' ramp and hues: the same ones as last frame
        // unless those changed. Building them allocates, which is setting a page up, not drawing it.
        now(theme, cfg) {
          const pal = palOf(theme, cfg.duo1, cfg.duo2);
          const ramp2 = cfg.ramp;
          if (!(this.t && this.t.ramp === ramp2 && samePal(this.t.pal, pal))) {
            this.serial++;
            this.t = buildTables(pal, ramp2, this.serial);
          }
          return this.t;
        }
      };
    }
  });

  // src/vendor/tunetap/viz/plugins/waterfall.js
  var waterfall_exports = {};
  __export(waterfall_exports, {
    WaterfallPainter: () => WaterfallPainter,
    create: () => create3,
    lanes: () => lanes,
    needs: () => needs3
  });
  function lanes(layout, dir, pw, ph, f) {
    if (dir !== WF_DIR.ACROSS) {
      if (layout === WF_LAYOUT.SUM) return [lane(0, 0, pw, ph, 2, 64, false, false), NONE, 1];
      if (layout === WF_LAYOUT.STACKED) return [lane(0, 0, pw, d2(ph, 2), 0, 64, false, false), lane(0, d2(ph, 2), pw, d2(ph, 2), 1, 64, false, false), 2];
      return [lane(0, 0, d2(pw, 2), ph, 0, 32, true, true), lane(d2(pw, 2), 0, d2(pw, 2), ph, 1, 32, false, true), 2];
    }
    const band = Math.max(d2(ph - f.i(16), 64), 0);
    const y = d2(ph - 64 * band, 2);
    if (layout === WF_LAYOUT.SUM) return [lane(0, y, pw, 64 * band, 2, 64, false, false), NONE, 1];
    return [lane(0, y, pw, 32 * band, 0, 32, false, true), lane(0, y + 32 * band, pw, 32 * band, 1, 32, layout === WF_LAYOUT.SPLIT, true), 2];
  }
  function value(z, l, cell) {
    const b = l.flip ? l.n - 1 - cell : cell;
    let v0, v1;
    if (l.half) {
      const a0 = z.fine[0][2 * b], c0 = z.fine[0][2 * b + 1];
      const a1 = z.fine[1][2 * b], c1 = z.fine[1][2 * b + 1];
      v0 = a0 > c0 ? a0 : c0;
      v1 = a1 > c1 ? a1 : c1;
    } else {
      v0 = z.fine[0][b];
      v1 = z.fine[1][b];
    }
    if (l.ch === 2) return F8(F8(v0 + v1) / 2);
    return l.ch ? v1 : v0;
  }
  function advance(px, pw, line2, z, l, dir, t6) {
    const lut2 = t6.loud[l.ch === 0 ? 0 : 1];
    if (dir === WF_DIR.ACROSS) {
      const ch = d2(l.h, l.n);
      if (l.w > line2) {
        for (let y = l.y; y < l.y + l.h; y++) {
          const row2 = y * pw + l.x;
          px.copyWithin(row2 + line2, row2, row2 + (l.w - line2));
        }
      }
      const w = Math.min(line2, l.w);
      for (let cell = 0; cell < l.n; cell++) {
        const c = lut2[byte(value(z, l, cell))];
        for (let k2 = 0; k2 < ch; k2++) {
          const at = (l.y + l.h - 1 - cell * ch - k2) * pw + l.x;
          px.fill(c, at, at + Math.max(w, 0));
        }
      }
      return;
    }
    if (l.h < line2) return;
    const cw = d2(l.w, l.n);
    if (dir === WF_DIR.DOWN) {
      for (let y = l.y + l.h - 1; y >= l.y + line2; y--) {
        const to = y * pw + l.x, from = (y - line2) * pw + l.x;
        px.copyWithin(to, from, from + l.w);
      }
    } else {
      for (let y = l.y; y < l.y + l.h - line2; y++) {
        const to = y * pw + l.x, from = (y + line2) * pw + l.x;
        px.copyWithin(to, from, from + l.w);
      }
    }
    const first2 = dir === WF_DIR.DOWN ? l.y : l.y + l.h - line2;
    const cells = Math.max(cw - 1, 0);
    for (let k2 = 0; k2 < line2; k2++) {
      const row2 = (first2 + k2) * pw + l.x;
      px.fill(lut2[0], row2, row2 + Math.max(l.w, 0));
      for (let cell = 0; cell < l.n; cell++) {
        const at = row2 + cell * cw;
        px.fill(lut2[byte(value(z, l, cell))], at, at + cells);
      }
    }
  }
  function create3() {
    const painter = new WaterfallPainter();
    const analyser = new BandAnalyser();
    const bands = createBands();
    let state = "no-data";
    let alive = true;
    const groups = Object.freeze(Array.from({ length: WATERFALL.groupCount() }, (_, g2) => Object.freeze({
      id: WATERFALL.groupId(g2),
      labelKey: WATERFALL.groupLabelKey(g2),
      count: WATERFALL.tourCount(g2),
      pos: (cfg) => WATERFALL.tourPos(cfg, g2),
      step: (cfg, shown) => WATERFALL.tourStep(cfg, shown, g2)
    })));
    return {
      needs: needs3,
      schema: SCHEMA3,
      presets: PRESETS3,
      defaults: () => WATERFALL.defaults(),
      validate: (obj) => WATERFALL.validate(obj),
      applyPreset: (cfg, id) => WATERFALL.applyPreset(cfg, id),
      presetOf: (cfg) => WATERFALL.preset(cfg),
      resolveFollow: (cfg, seed) => WATERFALL.resolve(cfg, seed),
      groups: () => groups,
      // The picture is made again at the next frame when its size changed (WaterfallPainter.draw).
      resize() {
      },
      // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices; playing: the picture
      // scrolls only while the player plays.
      frame({ dt, audio, theme, cfg, playing, pixels }) {
        if (!alive) return;
        dt = F8(Number.isFinite(dt) && dt > 0 ? dt : 0);
        const freq = audio?.freq;
        const live = !!freq?.l;
        state = live ? "ok" : "no-data";
        if (live) {
          analyser.run(bands, freq, audio.rate || 48e3, (audio.bins || 1024) * 2, dt);
        } else {
          resetBands(bands);
          painter.reset();
        }
        painter.draw(pixels, { bands, cfg, theme, dt, moving: !!playing });
      },
      status: () => alive ? state : "off",
      painter: () => painter,
      dispose() {
        alive = false;
      }
    };
  }
  var F8, d2, TICK, needs3, lane, NONE, byte, WaterfallPainter, option3, selectOf3, hueOf2, SCHEMA3, PRESETS3;
  var init_waterfall = __esm({
    "src/vendor/tunetap/viz/plugins/waterfall.js"() {
      init_page();
      init_anim();
      init_tables();
      init_cfg();
      init_bands();
      F8 = Math.fround;
      d2 = (a, b) => Math.trunc(a / b);
      TICK = F8(0.06);
      needs3 = Object.freeze({ freq: "stereo", time: "none" });
      lane = (x, y, w, h, ch, n, flip, half) => ({ x, y, w, h, ch, n, flip, half });
      NONE = lane(0, 0, 0, 0, 0, 0, false, false);
      byte = (v) => {
        const i = Math.trunc(F8(v * 255));
        return i > 0 ? i < 255 ? i : 255 : 0;
      };
      WaterfallPainter = class {
        constructor() {
          this.pic = new Uint32Array(0);
          this.size = [0, 0, 0];
          this.tables = new TableCache();
          this.drawn = null;
          this.acc = { v: 0 };
          this.lines = 0;
        }
        // Start a new picture on the next draw (the silent rest without audio data).
        reset() {
          this.drawn = null;
        }
        draw(c, v) {
          const th = v.theme;
          const y0 = c.y0, y1 = c.y1, width = c.width;
          const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 358));
          const l = (n) => f.i(n);
          const wx = l(32), py = y0 + l(4);
          const pw = Math.max(width - 2 * wx, 0), ph = Math.max(y1 - y0 - l(22), 0), line2 = f.px(4);
          const sz = this.size;
          if (sz[0] !== pw || sz[1] !== ph || sz[2] !== line2) {
            this.pic = new Uint32Array(pw * ph);
            this.size = [pw, ph, line2];
            this.drawn = null;
          }
          const t6 = this.tables.now(th, v.cfg);
          const layout = v.cfg.wf_layout, dir = v.cfg.wf_dir;
          const dr = this.drawn;
          if (!dr || dr[0] !== layout || dr[1] !== dir || dr[2] !== t6.serial) {
            this.pic.fill(t6.loud[1][0]);
            this.drawn = [layout, dir, t6.serial];
            this.acc.v = 0;
            this.lines = 0;
          }
          const [l0, l1, nLanes] = lanes(layout, dir, pw, ph, f);
          const lines = d2(dir === WF_DIR.ACROSS ? pw : ph, line2);
          if (v.moving && lines > 0) {
            const n = steps(this.acc, v.dt, TICK, d2(ph, line2));
            for (let i = 0; i < n; i++) {
              advance(this.pic, pw, line2, v.bands, l0, dir, t6);
              if (nLanes > 1) advance(this.pic, pw, line2, v.bands, l1, dir, t6);
            }
            this.lines += n;
          }
          for (let y = 0; y < ph && py + y < y1; y++) {
            const row2 = c.rowMut(py + y);
            if (!row2) continue;
            const from = y * pw;
            const n = Math.min(pw, Math.max(row2.length - wx, 0));
            row2.set(this.pic.subarray(from, from + n), wx);
          }
          const tick = (k2) => d2(k2 * 1e3, 60) * line2;
          const tk0 = f.px(4), tk1 = f.px(1);
          if (dir === WF_DIR.ACROSS) {
            for (let k2 = 1; tick(k2) < pw; k2++) c.fill(wx + tick(k2), py + ph + l(1), tk1, tk0, th.dim);
          } else {
            for (let k2 = 1; tick(k2) < ph; k2++) c.fill(wx + pw + l(2), dir === WF_DIR.DOWN ? py + tick(k2) : py + ph - tick(k2), tk0, tk1, th.dim);
          }
        }
      };
      option3 = (k2, i) => ({ value: i, labelKey: valueLabelKey(k2, i) });
      selectOf3 = (k2) => ({ key: k2, type: "select", labelKey: keyDef(k2).labelKey, options: Array.from({ length: keyDef(k2).n }, (_, i) => option3(k2, i)), showWhen: (cfg) => WATERFALL.rows(cfg).includes(k2) });
      hueOf2 = (k2) => ({ key: k2, type: "range", labelKey: keyDef(k2).labelKey, min: 0, max: keyDef(k2).n - 1, step: 1, format: "hue", showWhen: (cfg) => WATERFALL.rows(cfg).includes(k2) });
      SCHEMA3 = Object.freeze([selectOf3("wf_layout"), selectOf3("wf_dir"), selectOf3("ramp"), hueOf2("duo1"), hueOf2("duo2")]);
      PRESETS3 = Object.freeze(Array.from({ length: WATERFALL.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: WATERFALL.presetLabelKey(i), cfg: Object.freeze(WATERFALL.applyPreset(WATERFALL.plain(), i)) })));
    }
  });

  // src/vendor/tunetap/viz/plugins/cassette.js
  var cassette_exports = {};
  __export(cassette_exports, {
    C90_MS: () => C90_MS,
    CassettePainter: () => CassettePainter,
    contrast: () => contrast,
    create: () => create4,
    labelNow: () => labelNow,
    needs: () => needs4,
    shellNow: () => shellNow,
    streamView: () => streamView,
    time: () => time
  });
  function rrectIn(yy, h, r) {
    const bite = (dd) => r - i32(Math.sqrt(r * r - dd * dd));
    if (yy < r) return bite(r - yy);
    if (yy > h - 1 - r) return bite(yy - (h - 1 - r));
    return 0;
  }
  function rrect(c, x, y, w, h, r, col) {
    for (let yy = 0; yy < h; yy++) {
      const bite = rrectIn(yy, h, r);
      c.fill(x + bite, y + yy, w - 2 * bite, 1, col);
    }
  }
  function screw(c, f, cx, cy) {
    c.disc(cx, cy, f.i(7), rgb(150, 150, 160));
    c.disc(cx, cy, f.i(5), rgb(110, 110, 122));
    c.fill(cx - f.i(4), cy, f.i(9), f.px(1), rgb(50, 50, 60));
    c.fill(cx, cy - f.i(4), f.px(1), f.i(9), rgb(50, 50, 60));
  }
  function shellNow(i, t6) {
    const k2 = { ...SHELLS[Number.isInteger(i) && i >= 0 && i < 4 ? i : TAPE_SHELL.BLACK] };
    if (k2.clear) {
      k2.body = mix(t6.bg, rgb(190, 200, 215), 72);
      k2.head = mix(t6.bg, rgb(190, 200, 215), 100);
      k2.glass = mix(k2.body, rgb(214, 222, 232), 150);
    }
    if (k2.palette) {
      k2.rim = mix(t6.cyan, t6.bg, 90);
      k2.body = t6.panel2;
      k2.head = t6.panel;
      k2.frame = t6.faint;
    }
    return k2;
  }
  function labelNow(i, t6) {
    const k2 = { ...LABELS[Number.isInteger(i) && i >= 0 && i < LABELS.length ? i : TAPE_LABEL.CREAM] };
    if (k2.palette) {
      k2.paper = t6.light ? t6.panel : t6.fg;
      k2.ink = t6.light ? t6.fg : t6.bg;
      k2.rule = mix(k2.paper, k2.ink, 50);
      k2.note = t6.light ? t6.cyan : mix(t6.mag, t6.black, 90);
      k2.stripe = [t6.mag, t6.yel, t6.cyan];
    }
    return k2;
  }
  function contrast(a, b) {
    const lin = (v) => {
      const c = v / 255;
      return c <= 0.03928 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4);
    };
    const lum2 = (c) => 0.2126 * lin(red(c)) + 0.7152 * lin(green(c)) + 0.0722 * lin(blue(c));
    const la = lum2(a), lb = lum2(b);
    return la > lb ? (la + 0.05) / (lb + 0.05) : (lb + 0.05) / (la + 0.05);
  }
  function writing(lb, eff) {
    if (contrast(lb.ink, eff) >= 4.5) return INK;
    if (contrast(lb.paper, eff) >= 4.5) return SWAP;
    return BACK;
  }
  function time(ms) {
    const s = Math.trunc(ms / 1e3);
    return `${Math.trunc(s / 60)}:${String(s % 60).padStart(2, "0")}`;
  }
  function streamView(elapsedMs) {
    const e = Number.isFinite(elapsedMs) && elapsedMs > 0 ? Math.trunc(elapsedMs) : 0;
    return { posMs: e % C90_MS, lenMs: C90_MS, tag: "C-90", counter: time(e) };
  }
  function create4() {
    const painter = new CassettePainter();
    let alive = true;
    const groups = Object.freeze(Array.from({ length: CASSETTE.groupCount() }, (_, g2) => Object.freeze({
      id: CASSETTE.groupId(g2),
      labelKey: CASSETTE.groupLabelKey(g2),
      count: CASSETTE.tourCount(g2),
      pos: (cfg) => CASSETTE.tourPos(cfg, g2),
      step: (cfg, shown) => CASSETTE.tourStep(cfg, shown, g2)
    })));
    return {
      needs: needs4,
      schema: SCHEMA4,
      presets: PRESETS4,
      defaults: () => CASSETTE.defaults(),
      validate: (obj) => CASSETTE.validate(obj),
      applyPreset: (cfg, id) => CASSETTE.applyPreset(cfg, id),
      presetOf: (cfg) => CASSETTE.preset(cfg),
      resolveFollow: (cfg, seed) => CASSETTE.resolve(cfg, seed),
      groups: () => groups,
      // Laid out from the canvas each frame (chipchill's any size): nothing to keep.
      resize() {
      },
      // title / artist: what the label says (host.js maps the station's name and its ICY title);
      // elapsedMs: the play time; playing: the reels turn (by dt) and the lamp is lit; measure: the
      // platform's text widths (labels.js), rgxx's metrics when not given. Audio is not used.
      frame({ dt, theme, cfg, playing, pixels, title = "", artist = "", elapsedMs = 0, measure = null }) {
        if (!alive) return void 0;
        dt = F9(Number.isFinite(dt) && dt > 0 ? dt : 0);
        const labels = painter.draw(pixels, { theme, cfg, title: String(title ?? ""), artist: String(artist ?? ""), ...streamView(elapsedMs), dt, moving: !!playing, measure: measure ?? RGXX_METRICS });
        return { labels };
      },
      status: () => alive ? "ok" : "off",
      painter: () => painter,
      dispose() {
        alive = false;
      }
    };
  }
  var F9, d3, i32, round, needs4, C90_MS, SHELLS, LABELS, Plate, INK, SWAP, BACK, PLATE_A, CassettePainter, option4, selectOf4, SCHEMA4, PRESETS4;
  var init_cassette = __esm({
    "src/vendor/tunetap/viz/plugins/cassette.js"() {
      init_pixel();
      init_page();
      init_labels();
      init_cfg();
      F9 = Math.fround;
      d3 = (a, b) => Math.trunc(a / b);
      i32 = (v) => Number.isNaN(v) ? 0 : Math.trunc(Math.min(Math.max(v, -2147483648), 2147483647));
      round = (v) => v < 0 ? -Math.round(-v) : Math.round(v);
      needs4 = Object.freeze({ freq: "none", time: "none" });
      C90_MS = 90 * 60 * 1e3;
      SHELLS = Object.freeze([
        { rim: rgb(70, 68, 86), body: rgb(34, 32, 42), head: rgb(46, 44, 58), frame: rgb(90, 86, 110), glass: rgb(22, 20, 30), palette: false, clear: false },
        // body, head and glass: the night seen through smoke
        { rim: rgb(150, 156, 172), body: 0, head: 0, frame: rgb(170, 176, 190), glass: 0, palette: false, clear: true },
        { rim: rgb(176, 176, 188), body: rgb(226, 226, 232), head: rgb(204, 204, 214), frame: rgb(150, 150, 162), glass: rgb(22, 20, 30), palette: false, clear: false },
        { rim: 0, body: 0, head: 0, frame: 0, glass: rgb(22, 20, 30), palette: true, clear: false }
      ]);
      LABELS = Object.freeze([
        { paper: rgb(238, 230, 204), ink: rgb(36, 34, 70), rule: rgb(200, 190, 170), note: rgb(170, 60, 90), stripe: [rgb(255, 64, 150), rgb(255, 172, 40), rgb(0, 196, 226)], side: "A", palette: false },
        { paper: 0, ink: 0, rule: 0, note: 0, stripe: [0, 0, 0], side: "A", palette: true }
      ]);
      Plate = class {
        constructor(o) {
          Object.assign(this, o);
        }
        // One row: the shell's blend, with the packs' blend where the discs filled them.
        row(c, x, y, w, onShell, onPack) {
          let cur = x;
          const end = x + w;
          for (let side = 0; side < 2; side++) {
            if (!this.packs) break;
            const r = side !== 0 ? this.rr : this.rl;
            const dy = y - this.hubY;
            if (dy < -r || dy > r) continue;
            const cx = side !== 0 ? this.hr : this.hl;
            const half = i32(Math.sqrt(r * r - dy * dy));
            const a = Math.max(cx - half, cur), b = Math.min(cx + half + 1, end);
            if (b <= a) continue;
            if (a > cur) c.fill(cur, y, a - cur, 1, onShell);
            c.fill(a, y, b - a, 1, onPack);
            cur = b;
          }
          if (cur < end) c.fill(cur, y, end - cur, 1, onShell);
        }
        fill(c, x, y, w, h, col) {
          if (this.a >= 255) {
            c.fill(x, y, w, h, col);
            return;
          }
          const onShell = mix(this.shell, col, this.a), onPack = mix(this.pack, col, this.a);
          for (let yy = y; yy < y + h; yy++) this.row(c, x, yy, w, onShell, onPack);
        }
        rrect(c, x, y, w, h, r, col) {
          if (this.a >= 255) {
            rrect(c, x, y, w, h, r, col);
            return;
          }
          const onShell = mix(this.shell, col, this.a), onPack = mix(this.pack, col, this.a);
          for (let yy = 0; yy < h; yy++) {
            const bite = rrectIn(yy, h, r);
            this.row(c, x + bite, y + yy, w - 2 * bite, onShell, onPack);
          }
        }
      };
      INK = 0;
      SWAP = 1;
      BACK = 2;
      PLATE_A = [255, 191, 128, 64];
      CassettePainter = class {
        constructor() {
          this.al = 0;
          this.ar = 0;
        }
        draw(c, v) {
          const labels = [];
          const m = v.measure ?? RGXX_METRICS;
          const th = v.theme;
          const title = v.title ?? "", artist = v.artist ?? "";
          const y0 = c.y0, y1 = c.y1, width = c.width;
          const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 404));
          const l = (k2) => f.i(k2);
          const ts = textScale(f.f());
          const mm = 5.9 * f.f();
          const w = i32(100.4 * mm), h = i32(63.8 * mm);
          const x = d3(width - w, 2), y = y0 + d3(y1 - y0 - h, 2);
          const mid = d3(width, 2);
          const hubY = y + i32(27.5 * mm);
          const hl = mid - i32(21.25 * mm), hr = mid + i32(21.25 * mm);
          const rmin = 11 * mm, rmax = 24 * mm;
          const lenMs = v.lenMs ?? 0, posMs = v.posMs ?? 0;
          const p = lenMs <= 0 ? 0 : Math.min(Math.max(F9(F9(posMs) / F9(lenMs)), 0), 1);
          const rl = Math.sqrt(rmin * rmin + (rmax * rmax - rmin * rmin) * F9(1 - p));
          const rr = Math.sqrt(rmin * rmin + (rmax * rmax - rmin * rmin) * p);
          if (v.moving) {
            const dt28 = F9(F9(v.dt) * 28);
            this.al += dt28 * mm / rl;
            this.ar += dt28 * mm / rr;
          }
          const sk = shellNow(v.cfg.tape_shell, th);
          const lb = labelNow(v.cfg.tape_label, th);
          const shell = sk.body, shell2 = sk.head, tape = rgb(96, 62, 38);
          rrect(c, x, y, w, h, l(10), sk.rim);
          rrect(c, x + l(2), y + l(2), w - l(4), h - l(4), l(9), shell);
          c.fill(x + i32(8 * mm), y, i32(5 * mm), i32(3 * mm), th.bg);
          c.fill(x + w - i32(13 * mm), y, i32(5 * mm), i32(3 * mm), th.bg);
          if (sk.clear) {
            c.disc(hl, hubY, i32(rl), mix(shell, tape, 190));
            c.disc(hr, hubY, i32(rr), mix(shell, tape, 190));
          }
          const lx = x + i32(5 * mm), ly = y + i32(5 * mm), lw = w - i32(10 * mm), lh = i32(42 * mm);
          const ai = v.cfg.tape_alpha;
          const pl = new Plate({ a: PLATE_A[Number.isInteger(ai) && ai >= 0 && ai < 4 ? ai : TAPE_ALPHA.FULL], shell, pack: shell, packs: false, hl, hr, hubY, rl: i32(rl), rr: i32(rr) });
          if (sk.clear) {
            pl.pack = mix(shell, tape, 190);
            pl.packs = true;
          }
          const eff = pl.a >= 255 ? lb.paper : mix(shell, lb.paper, pl.a);
          const wr = pl.a >= 255 ? INK : writing(lb, eff);
          const back = wr === BACK;
          const ink = wr === SWAP ? lb.paper : lb.ink;
          let note = lb.note;
          const rule = lb.rule;
          if (pl.a < 255 && wr !== BACK && contrast(note, eff) < 4.5) {
            const lift = mix(lb.note, ink, 150);
            note = contrast(lift, eff) >= 4.5 ? lift : ink;
          }
          pl.rrect(c, lx, ly, lw, lh, l(6), lb.paper);
          const sy = ly + i32(11 * mm);
          pl.fill(c, lx, sy, lw, f.px(5), lb.stripe[0]);
          pl.fill(c, lx, sy + l(5), lw, f.px(5), lb.stripe[1]);
          pl.fill(c, lx, sy + l(10), lw, f.px(5), lb.stripe[2]);
          c.fill(lx + l(10), ly + l(10), l(30), l(30), lb.ink);
          const sideScale = lb.side.length > 1 ? ts : 2 * ts;
          const sideW = m.width("wide", sideScale, lb.side);
          labels.push({ kind: "str", text: lb.side, font: "wide", scale: sideScale, x: lx + l(10) + d3(l(30) - sideW, 2), y: sideScale === 2 * ts ? ly + l(9) : ly + l(17), colour: lb.paper, chip: null, maxW: null });
          const tx = lx + l(52), tw = lw - l(52) - l(110);
          pl.fill(c, tx, ly + l(44), tw + l(100), f.px(1), rule);
          if (back) c.fill(tx - l(6), ly + l(8), lx + lw - l(6) - (tx - l(6)), l(36), lb.paper);
          if (m.width("wide", 2 * ts, title) <= tw) {
            labels.push({ kind: "title", text: title, font: "wide", scale: 2 * ts, x: tx, y: ly + l(10), colour: ink, chip: null, maxW: null });
          } else {
            labels.push({ kind: "title", text: title, font: "wide", scale: ts, x: tx, y: ly + l(22), colour: ink, chip: null, maxW: tw + l(100) });
          }
          const tag = v.tag ?? `C-${time(lenMs)}`;
          const tagX = lx + lw - l(12) - m.width("hud", 2 * ts, tag);
          labels.push({ kind: "short", text: tag, font: "hud", scale: 2 * ts, x: tagX, y: ly + l(16), colour: note, chip: null, maxW: null });
          const wx = hl - i32(9 * mm), wx2 = hr + i32(9 * mm), wy = hubY - i32(8 * mm), wh = i32(16 * mm);
          rrect(c, wx - l(3), wy - l(3), wx2 - wx + l(6), wh + l(6), d3(wh, 2) + l(3), sk.frame);
          rrect(c, wx, wy, wx2 - wx, wh, d3(wh, 2), sk.glass);
          const pr = d3(wh, 2);
          const rli = i32(rl), rri = i32(rr), layer = rgb(120, 82, 52);
          for (let yy = wy; yy < wy + wh; yy++) {
            const dy = yy + 0.5 - (wy + pr);
            if (Math.abs(dy) > pr) continue;
            const half = i32(Math.sqrt(pr * pr - dy * dy));
            const a = wx + pr - half, b = wx2 - pr + half;
            for (let side = 0; side < 2; side++) {
              const r = side !== 0 ? rr : rl;
              if (Math.abs(dy) >= r) continue;
              const cx = side !== 0 ? hr : hl;
              const hh = i32(Math.sqrt(r * r - dy * dy));
              const x0 = Math.max(cx - hh, a), x1 = Math.min(cx + hh, b);
              if (x1 > x0) c.fill(x0, yy, x1 - x0, 1, tape);
            }
            for (let k2 = 1; k2 < 4; k2++) {
              for (let side = 0; side < 2; side++) {
                const r = (side !== 0 ? rri : rli) - k2 * l(9);
                if (r <= 0 || Math.abs(dy) > r) continue;
                const cx = side !== 0 ? hr : hl;
                const hh = i32(round(Math.sqrt(r * r - dy * dy)));
                for (const e of [-1, 1]) {
                  const qx = cx + e * hh;
                  if (qx >= a && qx < b) c.px(qx, yy, layer);
                }
              }
            }
            if ((yy - wy & 1) !== 0) c.scanlines(a, yy - 1, b - a, 2, 35);
          }
          for (let side = 0; side < 2; side++) {
            const cx = side !== 0 ? hr : hl;
            const ang = side !== 0 ? this.ar : this.al;
            c.disc(cx, hubY, i32(7.5 * mm), sk.glass);
            c.disc(cx, hubY, i32(6 * mm), rgb(236, 234, 228));
            for (let t6 = 0; t6 < 6; t6++) {
              const an = ang + t6 * Math.PI / 3;
              const kx = cx + i32(round(4.2 * mm * Math.cos(an)));
              const ky = hubY + i32(round(4.2 * mm * Math.sin(an)));
              c.fill(kx - l(3), ky - l(3), l(7), l(7), rgb(60, 58, 74));
            }
            c.disc(cx, hubY, i32(2.3 * mm), rgb(40, 38, 52));
          }
          const now = v.counter ?? time(posMs);
          pl.fill(c, lx + l(10), ly + lh - l(14), lw - l(20), f.px(1), rule);
          const nowW = m.width("wide", ts, now);
          if (back) c.fill(lx + l(6), ly + lh - l(34), lw - l(12), l(20), lb.paper);
          labels.push({ kind: "artist", text: artist, font: "wide", scale: ts, x: lx + l(12), y: ly + lh - l(32), colour: ink, chip: null, maxW: lw - l(150) });
          labels.push({ kind: "short", text: now, font: "wide", scale: ts, x: lx + lw - l(12) - nowW, y: ly + lh - l(32), colour: note, chip: null, maxW: null });
          const ty = y + h - i32(11 * mm), tt = i32(64 * mm), tb = i32(76 * mm);
          const rows = y + h - l(2) - ty;
          for (let r = 0; r < rows; r++) {
            const ww = tt + d3((tb - tt) * r, rows);
            c.fill(mid - d3(ww, 2), ty + r, ww, 1, shell2);
          }
          const oy = y + h - i32(6 * mm);
          const hole = rgb(12, 10, 16);
          c.fill(mid - i32(6 * mm), oy - l(4), i32(12 * mm), i32(6 * mm), hole);
          c.fill(mid - i32(2.5 * mm), oy - l(2), i32(5 * mm), i32(3 * mm), rgb(200, 180, 140));
          for (const side of [-1, 1]) {
            c.disc(mid + side * i32(20 * mm), oy, i32(1.6 * mm), hole);
            c.fill(mid + side * i32(29 * mm) - l(6), oy - l(5), l(12), l(10), hole);
          }
          c.fill(mid - d3(tb, 2) + l(8), y + h - l(9), tb - l(16), l(3), tape);
          screw(c, f, x + l(12), y + l(12));
          screw(c, f, x + w - l(13), y + l(12));
          screw(c, f, x + l(12), y + h - l(13));
          screw(c, f, x + w - l(13), y + h - l(13));
          screw(c, f, mid, ty + l(10));
          if (v.moving) c.disc(x + w - l(30), ty + l(16), l(5), rgb(255, 50, 60));
          return labels;
        }
      };
      option4 = (k2, i) => ({ value: i, labelKey: valueLabelKey(k2, i) });
      selectOf4 = (k2) => ({ key: k2, type: "select", labelKey: keyDef(k2).labelKey, options: Array.from({ length: keyDef(k2).n }, (_, i) => option4(k2, i)), showWhen: (cfg) => CASSETTE.rows(cfg).includes(k2) });
      SCHEMA4 = Object.freeze([selectOf4("tape_shell"), selectOf4("tape_label"), selectOf4("tape_alpha")]);
      PRESETS4 = Object.freeze(Array.from({ length: CASSETTE.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: CASSETTE.presetLabelKey(i), cfg: Object.freeze(CASSETTE.applyPreset(CASSETTE.plain(), i)) })));
    }
  });

  // src/js/lib/viz.js
  function t3(key, ...args) {
    return I18N.t(key, args);
  }
  function rgbaOf(s) {
    const m = NUM.exec(s || "");
    if (!m) return null;
    if (m[1]) {
      const h = m[1].length === 3 ? [...m[1]].map((c) => c + c).join("") : m[1];
      return [0, 2, 4].map((i) => parseInt(h.slice(i, i + 2), 16)).concat(1);
    }
    return [+m[2], +m[3], +m[4], m[5] === void 0 ? 1 : +m[5]];
  }
  function nanoTheme(root) {
    const cs = getComputedStyle(root);
    const css2 = (k2) => cs.getPropertyValue(k2).trim();
    const map2 = { "--muted": "--mut", "--accent": "--acc" };
    return themeFromTokens((k2) => {
      if (k2 === "--line") {
        const l = rgbaOf(css2("--line")), c = rgbaOf(css2("--card"));
        if (!l || !c) return "";
        const a = l[3];
        return `rgb(${[0, 1, 2].map((i) => Math.round(l[i] * a + c[i] * (1 - a))).join(",")})`;
      }
      return css2(map2[k2] || k2);
    });
  }
  function createNanoViz({ getBytes }) {
    let state = null, polledAt = 0;
    const mq = (q) => {
      try {
        return window.matchMedia(q);
      } catch {
        return null;
      }
    };
    const reducedMq = mq("(prefers-reduced-motion: reduce)");
    const tap = createBandTap(getBytes);
    const store = {
      // the host's keys are tunetap.viz.*; Nano keeps its own under nsn.viz.*
      getItem: (k2) => {
        try {
          return localStorage.getItem(k2.replace(/^tunetap\.viz\./, "nsn.viz."));
        } catch {
          return null;
        }
      },
      setItem: (k2, v) => {
        try {
          localStorage.setItem(k2.replace(/^tunetap\.viz\./, "nsn.viz."), v);
        } catch {
        }
      }
    };
    const host = createVizHost({
      doc: document,
      storage: store,
      raf: (fn) => window.requestAnimationFrame(fn),
      caf: (id) => window.cancelAnimationFrame(id),
      now: () => performance.now(),
      getTheme: () => nanoTheme(document.documentElement),
      getAudio: () => tap,
      visible: () => !document.hidden,
      playing: () => state?.st === "playing",
      station: () => state ? state.file || state.title || "" : "",
      title: () => state?.title ? String(state.title) : null,
      stationName: () => baseName(state?.file || state?.title),
      // the box reports the position once a second; the cassette's counter runs on between the reports
      playTime: () => state ? (+state.pos || 0) + (state.st === "playing" ? Math.min(2e3, performance.now() - polledAt) : 0) : 0,
      reducedMotion: () => !!reducedMq?.matches,
      dpr: () => window.devicePixelRatio || 1,
      registry
    });
    mq("(prefers-color-scheme: dark)")?.addEventListener?.("change", () => host.update());
    return {
      host,
      t: t3,
      setState(s) {
        state = s;
        polledAt = performance.now();
        host.update();
      }
    };
  }
  var NUM, baseName;
  var init_viz = __esm({
    "src/js/lib/viz.js"() {
      init_host();
      init_registry();
      init_themes();
      init_viz_tap();
      init_i18n();
      if (!registry.get("spectrum")) {
        registry.register({ id: "spectrum", labelKey: "viz.mode.spectrum", needsAudio: true, stillWithoutAudio: true, words: "spectrum", load: () => Promise.resolve().then(() => (init_spectrum(), spectrum_exports)) });
        registry.register({ id: "drive", labelKey: "viz.mode.drive", needsAudio: true, stillWithoutAudio: false, load: () => Promise.resolve().then(() => (init_drive(), drive_exports)) });
        registry.register({ id: "waterfall", labelKey: "viz.mode.waterfall", needsAudio: true, stillWithoutAudio: true, load: () => Promise.resolve().then(() => (init_waterfall(), waterfall_exports)) });
        registry.register({ id: "cassette", labelKey: "viz.mode.cassette", needsAudio: false, stillWithoutAudio: false, words: "none", load: () => Promise.resolve().then(() => (init_cassette(), cassette_exports)) });
      }
      NUM = /^\s*(?:#([0-9a-f]{3}|[0-9a-f]{6})|rgba?\(\s*([\d.]+)[\s,]+([\d.]+)[\s,]+([\d.]+)(?:[\s,/]+([\d.]+))?)/i;
      baseName = (s) => String(s || "").replace(/\.[A-Za-z0-9]{1,5}$/, "");
    }
  });

  // src/js/lib/viz-ui.js
  function mountViz({ viz, stage: stage2, modeRow, options }) {
    const { host, t: t6 } = viz;
    const prev = el("button", { type: "button", className: "viz-btn", "aria-label": t6("viz.prev"), "data-i18n-aria": "viz.prev", on: { click: () => host.prevMode() } }, "\u25C0");
    const next = el("button", { type: "button", className: "viz-btn", "aria-label": t6("viz.next"), "data-i18n-aria": "viz.next", on: { click: () => host.nextMode() } }, "\u25B6");
    const name = el("span", { className: "viz-mode-name", role: "status" });
    modeRow.replaceChildren(prev, name, next);
    let builtFor = null;
    let controls = [];
    let modeSel = null, presetSel = null, themeSel = null;
    const note = el("div", { className: "mut viz-note", role: "status" });
    const option5 = (value2, label, selected, disabled = false) => {
      const o = el("option", { value: String(value2), disabled }, label);
      o.selected = !!selected;
      return o;
    };
    const field = (id, label, input, ...extra) => {
      input.id = id;
      return el("div", { className: "viz-field" }, el("label", { htmlFor: id }, label), input, ...extra);
    };
    const modeLabel = () => {
      const m = host.modes().find((x) => x.id === host.mode());
      return m ? t6(m.labelKey) : "";
    };
    function build2() {
      const p = host.plugin();
      controls = [];
      presetSel = null;
      const parts = [];
      modeSel = el("select", { className: "viz-select" }, ...host.modes().map((m) => option5(m.id, t6(m.labelKey), m.id === host.mode())));
      modeSel.addEventListener("change", () => host.setMode(modeSel.value));
      parts.push(field("vz-mode", t6("viz.mode"), modeSel));
      if (!p) {
        parts.push(el("div", { className: "mut" }, host.status() === "unavailable" ? t6("viz.unavailable") : t6("common.loading")));
      } else {
        if (p.presets.length) {
          presetSel = el("select", { className: "viz-select" }, ...p.presets.map((x) => option5(x.id, t6(x.labelKey), x.id === p.preset)), option5(-1, t6("viz.preset.custom"), p.preset === -1, true));
          presetSel.addEventListener("change", () => host.setPreset(Number(presetSel.value)));
          parts.push(field("vz-preset", t6("viz.preset"), presetSel));
        }
        for (const s of p.schema) {
          const id = `vz-${s.key}`;
          let input, out = null;
          if (s.type === "select") {
            input = el("select", { className: "viz-select" }, ...(s.options ?? []).map((o) => option5(o.value, t6(o.labelKey), false)));
            input.addEventListener("change", () => host.setCfg({ [s.key]: Number(input.value) }));
          } else if (s.type === "range") {
            input = el("input", { type: "range", className: "viz-range", min: s.min, max: s.max, step: s.step ?? 1 });
            out = el("output", { className: "viz-range-out", htmlFor: id });
            input.addEventListener("input", () => host.setCfg({ [s.key]: Number(input.value) }));
          } else if (s.type === "toggle") {
            input = el("input", { type: "checkbox", className: "viz-toggle" });
            input.addEventListener("change", () => host.setCfg({ [s.key]: input.checked ? 1 : 0 }));
          } else continue;
          const w = field(id, t6(s.labelKey), input, ...out ? [out] : []);
          controls.push({ wrap: w, el: input, schema: s, out });
          parts.push(w);
        }
        themeSel = el("select", { className: "viz-select" }, ...host.themes().map((x) => option5(x.name, t6(x.labelKey), x.name === host.themeName())));
        themeSel.addEventListener("change", () => host.setThemeName(themeSel.value));
        parts.push(field("vz-theme", t6("viz.theme"), themeSel));
        const gs = host.groups();
        if (gs.length) {
          parts.push(el("div", { className: "row", style: "margin-top:.6rem" }, ...gs.map((g2, i) => el("button", {
            type: "button",
            on: { click: () => {
              const r = host.stepGroup(i, t6);
              if (r?.text) setText(note, r.text);
            } }
          }, "\u27F3 " + t6(g2.labelKey)))));
        }
      }
      parts.push(note);
      options.replaceChildren(...parts);
      builtFor = p ? p.id : null;
      refresh();
    }
    function refresh() {
      const p = host.plugin();
      if (modeSel && modeSel.value !== host.mode()) modeSel.value = host.mode();
      if (!p) return;
      if (presetSel) {
        const v = String(p.preset);
        if (presetSel.value !== v) presetSel.value = v;
      }
      if (themeSel && themeSel.value !== host.themeName()) themeSel.value = host.themeName();
      for (const c of controls) {
        const v = p.cfg[c.schema.key];
        c.wrap.hidden = typeof c.schema.showWhen === "function" ? !c.schema.showWhen(p.cfg) : false;
        if (c.schema.type === "toggle") c.el.checked = !!v;
        else if (c.el.value !== String(v)) c.el.value = String(v);
        if (c.out) c.out.textContent = c.schema.format === "hue" ? `${Number(v) * 10}\xB0` : String(v);
      }
    }
    function sync() {
      setText(name, modeLabel());
      const p = host.plugin();
      if ((p ? p.id : null) !== builtFor || !modeSel) build2();
      else refresh();
    }
    I18N.onChange(() => {
      note.textContent = "";
      modeSel = null;
      sync();
    });
    host.mount(stage2);
    host.onChange(sync);
    host.setEnabled(true);
    sync();
    try {
      new ResizeObserver(() => host.resize()).observe(stage2);
    } catch {
      window.addEventListener("resize", () => host.resize());
    }
    return { sync };
  }
  var el, setText;
  var init_viz_ui = __esm({
    "src/js/lib/viz-ui.js"() {
      init_i18n();
      el = (tag, props = {}, ...kids) => {
        const e = document.createElement(tag);
        for (const [k2, v] of Object.entries(props)) {
          if (k2 === "on") for (const [ev, fn] of Object.entries(v)) e.addEventListener(ev, fn);
          else if (k2 in e && k2 !== "list") e[k2] = v;
          else e.setAttribute(k2, v);
        }
        e.append(...kids);
        return e;
      };
      setText = (e, v) => {
        if (e.textContent !== v) e.textContent = v;
      };
    }
  });

  // src/js/pages/now.js
  function flashHint() {
    hintUntil = Date.now() + 4e3;
    renderMeta();
    setTimeout(renderMeta, 4100);
  }
  function renderMeta() {
    const state = app.state;
    if (!state) return;
    const mmss = (ms) => {
      const s = Math.max(0, Math.round(ms / 1e3));
      return Math.floor(s / 60) + ":" + String(s % 60).padStart(2, "0");
    };
    const where = { dac: t2("out.dac"), bt: t2("out.btShort"), cast: t2("out.castShort") }[state.out];
    setRaw2($("meta"), known("st.", state.st) + (state.fmt ? " \xB7 " + state.fmt + " " + state.rate / 1e3 + " kHz" : "") + (where ? " \xB7 " + where : "") + (state.n > 1 ? ` \xB7 ${state.trk}/${state.n}` : "") + (state.len > 0 ? " \xB7 " + mmss(state.pos) + " / " + mmss(state.len) : ""));
    setRaw2($("timerv"), state.sleep > 0 ? t2("timer.left", { m: Math.ceil(state.sleep / 60) }) : "");
    const idle = state.st === "idle", hint = $("idleHint");
    if (idle && !state.bm && Date.now() < hintUntil) setT2(hint, "now.pickHint");
    else if (idle && state.bm) setT2(hint, "now.resumeHint");
    else setRaw2(hint, "");
  }
  function setOut(v) {
    app.selOut = v;
    segSet("out", v);
    if (v === "cast") loadCast();
    renderCast();
    renderVolFor();
  }
  function parseCast(r) {
    const l = r.find((x) => x.startsWith("cast:"));
    if (!l) return null;
    const on2 = /^cast: on\b/.test(l), num3 = (k2) => {
      const m = new RegExp("\\b" + k2 + "=(\\d+)").exec(l);
      return m ? +m[1] : 0;
    };
    const sm = /\bstreams=(\d+)/.exec(l);
    return { on: on2, und: num3("underflows"), err: num3("send_err"), streams: sm ? [...sm[1]].filter((c) => c === "1").length : 0 };
  }
  async function loadCast() {
    const r = await send("cast");
    if (app.link !== "on") return;
    const c = parseCast(r);
    if (c) {
      castRep = c;
      renderCast();
    }
  }
  function renderCast() {
    const state = app.state;
    const show = app.selOut === "cast" && app.link === "on";
    $("castInfo").hidden = !show;
    $("dCast").className = "dot" + (state && state.out === "cast" || castRep && castRep.on && app.link === "on" ? " on" : "");
    emit("cast");
    if (!show) return;
    let txt;
    if (castErr) txt = t2("cast.err", { e: castErr });
    else if (!castRep) txt = t2("cast.wait");
    else if (!castRep.on) txt = t2("cast.off");
    else {
      txt = castRep.streams >= 2 ? t2("cast.on") : t2("cast.part", { n: castRep.streams });
      if (castRep.und || castRep.err) txt += " " + t2("cast.counts", { u: castRep.und, e: castRep.err });
    }
    setRaw2($("castSt"), txt);
  }
  async function loadOut() {
    const r = await send("out");
    if (app.link !== "on") return;
    const m = (r.find((l) => /^out \w+ \(playing on /.test(l)) || "").match(/^out (\w+)/);
    if (m) {
      castErr = "";
      setOut(m[1]);
    }
  }
  async function chooseOut(v) {
    castErr = "";
    setOut(v);
    const r = await send("out " + v);
    const starting = r.some((l) => /reconnects in a moment/.test(l)) || v === "cast" && !r.length && app.link !== "on";
    if (starting) {
      app.castStarting = true;
      setLinkUI();
      for (const t0 = Date.now(); app.link === "on" && Date.now() - t0 < 5e3; ) await sleep(100);
      const ok = await waitLink();
      app.castStarting = false;
      setLinkUI();
      if (ok) {
        await loadOut();
        refreshStatus();
        loadCast();
      }
      return;
    }
    if (app.link !== "on") return;
    const bad = r.find((l) => /^out \w+: /.test(l));
    if (bad) {
      castErr = bad.replace(/^out \w+: /, "");
      await loadOut();
      renderCast();
    } else {
      const m = r.find((l) => /^out \w+ \(playing on /.test(l));
      if (m) setOut(m.match(/^out (\w+)/)[1]);
    }
    refreshStatus();
    if (v === "cast") loadCast();
  }
  function mountVisual() {
    try {
      const viz = createNanoViz({ getBytes: () => app.target });
      app.viz = viz;
      if (DEMO) {
        const m = new URLSearchParams(location.search).get("viz");
        if (m) viz.host.setMode(m);
      }
      mountViz({ viz, stage: $("vizStage"), modeRow: $("vizMode"), options: $("vizOpts") });
      if (app.state) viz.setState(app.state);
    } catch (e) {
      log("viz: " + e);
      setT2($("vizStage"), "viz.needHttp");
      $("vizStage").style.cssText = "display:flex;align-items:center;justify-content:center;text-align:center;padding:1rem;color:var(--mut);font-size:.9rem";
      setRaw2($("vizOpts"), "");
    }
  }
  function initNow() {
    mountVisual();
    for (const b of $("out").children) b.onclick = () => chooseOut(b.dataset.v);
    segBind("timer", (v) => send("timer " + v).then(refreshStatus));
    setInterval(() => {
      if (app.link === "on" && app.selOut === "cast" && !document.hidden && !app.scanBusy) loadCast();
    }, 4e3);
    document.addEventListener("visibilitychange", () => {
      if (!document.hidden && app.link === "on" && app.selOut === "cast") loadCast();
    });
    on("status", (state) => {
      if (app.viz) app.viz.setState(state);
      $("file").textContent = state.title || state.file || "\u2014";
      $("file").title = state.title && state.file ? state.file : "";
      const tags = [state.artist, state.album].filter(Boolean).join(" \u2014 ");
      $("tags").textContent = tags;
      $("tags").hidden = !tags;
    });
    on("status", (state) => {
      renderMeta();
      $("prog").style.width = state.len > 0 ? Math.min(100, 100 * state.pos / state.len) + "%" : "0";
      $("dDac").className = "dot" + (state.dac ? " on" : "");
      $("dBt").className = "dot" + (state.bt ? " on" : "");
      renderCast();
      if (!(state.sleep > 0)) segSet("timer", "0");
    });
    on("linkoff", () => {
      hintUntil = 0;
      setRaw2($("idleHint"), "");
      setT2($("meta"), "meta.off");
      if (app.viz) app.viz.setState(null);
      renderCast();
    });
    on("lang", () => {
      renderMeta();
      renderCast();
    });
  }
  var hintUntil, castRep, castErr;
  var init_now = __esm({
    "src/js/pages/now.js"() {
      init_app();
      init_link();
      init_status();
      init_bar();
      init_viz();
      init_viz_ui();
      hintUntil = 0;
      castRep = null;
      castErr = "";
    }
  });

  // src/js/lib/eq.js
  function coeffs(type, fc, gainDb, q, fs) {
    fs = fs || FS;
    const w0 = 2 * Math.PI * clamp(fc, 1, fs * 0.49) / fs, cw = Math.cos(w0), sw = Math.sin(w0);
    const A = Math.pow(10, gainDb / 40), al = sw / (2 * Math.max(q, 1e-3));
    let b0, b1, b2, a0, a1, a2;
    if (type === 1) {
      const s = 2 * Math.sqrt(A) * al;
      b0 = A * (A + 1 - (A - 1) * cw + s);
      b1 = 2 * A * (A - 1 - (A + 1) * cw);
      b2 = A * (A + 1 - (A - 1) * cw - s);
      a0 = A + 1 + (A - 1) * cw + s;
      a1 = -2 * (A - 1 + (A + 1) * cw);
      a2 = A + 1 + (A - 1) * cw - s;
    } else if (type === 2) {
      const s = 2 * Math.sqrt(A) * al;
      b0 = A * (A + 1 + (A - 1) * cw + s);
      b1 = -2 * A * (A - 1 + (A + 1) * cw);
      b2 = A * (A + 1 + (A - 1) * cw - s);
      a0 = A + 1 - (A - 1) * cw + s;
      a1 = 2 * (A - 1 - (A + 1) * cw);
      a2 = A + 1 - (A - 1) * cw - s;
    } else {
      b0 = 1 + al * A;
      b1 = -2 * cw;
      b2 = 1 - al * A;
      a0 = 1 + al / A;
      a1 = -2 * cw;
      a2 = 1 - al / A;
    }
    return [b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0];
  }
  function magDb(c, f, fs) {
    const w = 2 * Math.PI * f / (fs || FS), c1 = Math.cos(w), s1 = Math.sin(w), c2 = Math.cos(2 * w), s2 = Math.sin(2 * w);
    const nr = c[0] + c[1] * c1 + c[2] * c2, ni = -(c[1] * s1 + c[2] * s2);
    const dr = 1 + c[3] * c1 + c[4] * c2, di = -(c[3] * s1 + c[4] * s2);
    return 10 * Math.log10((nr * nr + ni * ni) / (dr * dr + di * di));
  }
  function response(bands, preamp, freqs, fs) {
    const cs = bands.map((b) => coeffs(b.type, b.fc, b.gain, b.q, fs));
    return freqs.map((f) => cs.reduce((s, c) => s + magDb(c, f, fs), preamp || 0));
  }
  function parseAutoEq(text) {
    const out = { preamp: null, bands: [], skipped: 0, overflow: 0 };
    let valid = 0;
    for (const raw of String(text || "").split(/\r?\n/)) {
      const line2 = raw.trim();
      let m = /^preamp\s*:\s*([-+]?\d+(?:[.,]\d+)?)/i.exec(line2);
      if (m) {
        out.preamp = clamp(num(m[1]), LIM.preamp[0], LIM.preamp[1]);
        continue;
      }
      m = /^filter\s*\d*\s*:\s*(on|off)\s+([a-z]+)\b(.*)$/i.exec(line2);
      if (!m) continue;
      const type = TYPES[m[2].toUpperCase()], rest = m[3];
      const fc = /\bfc\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest), gain = /\bgain\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest), q = /\bq\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest);
      if (m[1].toLowerCase() === "off" || type === void 0 || !fc) {
        out.skipped++;
        continue;
      }
      valid++;
      if (out.bands.length >= MAX_BANDS) {
        out.overflow++;
        continue;
      }
      out.bands.push({
        type,
        fc: clamp(num(fc[1]), LIM.fc[0], LIM.fc[1]),
        gain: gain ? clamp(num(gain[1]), LIM.gain[0], LIM.gain[1]) : 0,
        q: q ? clamp(num(q[1]), LIM.q[0], LIM.q[1]) : 0.7
      });
    }
    return out;
  }
  function setup(cv) {
    const dpr = globalThis.devicePixelRatio || 1, w = cv.clientWidth, h = cv.clientHeight;
    if (!w || !h) return null;
    const pw = Math.round(w * dpr), ph = Math.round(h * dpr);
    if (cv.width !== pw) cv.width = pw;
    if (cv.height !== ph) cv.height = ph;
    const x = cv.getContext("2d");
    x.setTransform(dpr, 0, 0, dpr, 0, 0);
    x.clearRect(0, 0, w, h);
    const cs = globalThis.getComputedStyle(cv), col = (n) => cs.getPropertyValue(n).trim() || "#888";
    return { x, w, h, line: col("--line"), mut: col("--mut"), acc: col("--acc"), fg: col("--fg") };
  }
  function drawResponse(cv, bands, preamp) {
    const s = setup(cv);
    if (!s) return;
    const { x, w, h } = s, L = 30, R = 6, T = 8, B = 14, pw = w - L - R, ph = h - T - B;
    const N = Math.max(40, Math.round(pw / 2)), fa = Math.log(20), fb = Math.log(2e4);
    const freqs = [];
    for (let i = 0; i < N; i++) freqs.push(Math.exp(fa + (fb - fa) * i / (N - 1)));
    const db = response(bands, preamp, freqs);
    const peak = Math.max(0, ...db.map(Math.abs)), rng = [6, 12, 18, 24, 36].find((r) => r >= peak) || 48, rngv = Math.max(12, rng);
    const X = (f) => L + pw * (Math.log(f) - fa) / (fb - fa), Y = (d4) => T + ph * (0.5 - clamp(d4, -rngv, rngv) / (2 * rngv));
    x.font = "10px system-ui, sans-serif";
    x.lineWidth = 1;
    x.strokeStyle = s.line;
    x.fillStyle = s.mut;
    x.textBaseline = "middle";
    x.textAlign = "right";
    for (const d4 of [-rngv, -rngv / 2, 0, rngv / 2, rngv]) {
      const y = Math.round(Y(d4)) + 0.5;
      x.globalAlpha = d4 === 0 ? 1 : 0.5;
      x.beginPath();
      x.moveTo(L, y);
      x.lineTo(L + pw, y);
      x.stroke();
      x.globalAlpha = 1;
      if (d4 === 0 || Math.abs(d4) === rngv) x.fillText((d4 > 0 ? "+" : "") + d4, L - 4, y);
    }
    x.textAlign = "center";
    x.textBaseline = "alphabetic";
    for (const [f, lab] of [[50], [100, "100"], [200], [500], [1e3, "1k"], [2e3], [5e3], [1e4, "10k"]]) {
      const px = Math.round(X(f)) + 0.5;
      x.globalAlpha = lab ? 0.8 : 0.4;
      x.beginPath();
      x.moveTo(px, T);
      x.lineTo(px, T + ph);
      x.stroke();
      x.globalAlpha = 1;
      if (lab) x.fillText(lab, px, h - 2);
    }
    x.beginPath();
    freqs.forEach((f, i) => {
      const px = X(f), py = Y(db[i]);
      i ? x.lineTo(px, py) : x.moveTo(px, py);
    });
    x.strokeStyle = s.acc;
    x.lineWidth = 2;
    x.lineJoin = "round";
    x.stroke();
    x.lineTo(X(2e4), Y(0));
    x.lineTo(X(20), Y(0));
    x.closePath();
    x.fillStyle = s.acc;
    x.globalAlpha = 0.14;
    x.fill();
    x.globalAlpha = 1;
    x.fillStyle = s.acc;
    x.strokeStyle = globalThis.getComputedStyle(cv).getPropertyValue("--card").trim() || "#fff";
    x.lineWidth = 1.5;
    for (const b of bands) {
      const f = clamp(b.fc, 20, 2e4), d4 = response(bands, preamp, [f])[0];
      x.beginPath();
      x.arc(X(f), Y(d4), 3.5, 0, 2 * Math.PI);
      x.fill();
      x.stroke();
    }
  }
  function drawSpark(cv, arr, min, max) {
    const s = setup(cv);
    if (!s || arr.length < 1) return;
    const { x, w, h } = s, P = 3;
    let lo = min, hi = max;
    if (lo === null || hi === null) {
      lo = Math.min(...arr);
      hi = Math.max(...arr);
      if (hi - lo < 1) {
        const c = (hi + lo) / 2;
        lo = c - 0.5;
        hi = c + 0.5;
      }
      const pad = (hi - lo) * 0.1;
      lo -= pad;
      hi += pad;
    }
    const X = (i) => arr.length < 2 ? w : P + (w - 2 * P) * i / (arr.length - 1), Y = (v) => h - P - (h - 2 * P) * clamp((v - lo) / (hi - lo), 0, 1);
    x.strokeStyle = s.line;
    x.lineWidth = 1;
    x.beginPath();
    x.moveTo(0, h - 0.5);
    x.lineTo(w, h - 0.5);
    x.stroke();
    x.beginPath();
    arr.forEach((v, i) => {
      i ? x.lineTo(X(i), Y(v)) : x.moveTo(X(i), Y(v));
    });
    x.strokeStyle = s.acc;
    x.lineWidth = 1.5;
    x.lineJoin = "round";
    x.stroke();
    x.lineTo(X(arr.length - 1), h);
    x.lineTo(X(0), h);
    x.closePath();
    x.fillStyle = s.acc;
    x.globalAlpha = 0.13;
    x.fill();
    x.globalAlpha = 1;
  }
  var FS, MAX_BANDS, LIM, clamp, TYPES, num, EQ;
  var init_eq = __esm({
    "src/js/lib/eq.js"() {
      FS = 48e3;
      MAX_BANDS = 10;
      LIM = { fc: [20, 2e4], gain: [-24, 24], q: [0.1, 10], preamp: [-24, 6] };
      clamp = (x, lo, hi) => Math.min(hi, Math.max(lo, x));
      TYPES = { PK: 0, PEQ: 0, LS: 1, LSC: 1, HS: 2, HSC: 2 };
      num = (s) => parseFloat(String(s).replace(",", "."));
      EQ = { FS, MAX_BANDS, LIM, clamp, coeffs, magDb, response, parseAutoEq, drawResponse, drawSpark };
    }
  });

  // src/js/pages/fx.js
  function fxLabels() {
    for (const e of document.querySelectorAll("[data-fxapp]")) e.textContent = fxAppName(e.dataset.fxapp, e.dataset.fb);
    for (const e of document.querySelectorAll("[data-fxp]")) {
      e.textContent = fxParName(e.dataset.fxp);
      e.nextElementSibling.setAttribute("aria-label", e.textContent);
    }
  }
  async function loadFx() {
    const lines = await send("fx params"), body = $("fxBody");
    if (app.link !== "on" && !lines.length) return;
    const oldTa = body.querySelector(".eqimp textarea"), oldOpen = body.querySelector(".eqimp") && body.querySelector(".eqimp").open, oldText = oldTa ? oldTa.value : "";
    body.textContent = "";
    const hasBands = lines.some((l) => l.startsWith("APP fxb equalizer "));
    let box = null, rows = null;
    eq = null;
    for (const l of lines) {
      const p = l.split(" ");
      if (p[0] !== "APP") continue;
      const app2 = p[2];
      if (p[1] === "fxb") {
        if (app2 !== "equalizer" || !eq) continue;
        const i = +p[3];
        if (!(i >= 0 && i < EQ.MAX_BANDS)) continue;
        eq.bands[i] = { type: Math.min(2, Math.max(0, +p[4] | 0)), fc: +p[5], gain: +p[6], q: +p[7] };
        continue;
      }
      if (p[1] !== "fxp") continue;
      if (p[3] === "on") {
        box = document.createElement("div");
        box.className = "fxapp";
        const lab = document.createElement("label"), c = document.createElement("input");
        c.type = "checkbox";
        c.checked = p[4] === "1";
        rows = document.createElement("div");
        rows.hidden = !c.checked;
        const myRows = rows;
        c.onchange = () => {
          myRows.hidden = !c.checked;
          send("fx on " + app2 + " " + (c.checked ? 1 : 0)).then(eqDraw);
        };
        const nm = document.createElement("span");
        nm.dataset.fxapp = app2;
        nm.dataset.fb = p.slice(5).join(" ");
        lab.append(c, nm);
        box.append(lab, rows);
        body.appendChild(box);
        if (app2 === "equalizer" && hasBands) eq = { rows, preamp: 0, count: 0, bands: Array.from({ length: EQ.MAX_BANDS }, () => ({ type: 0, fc: 1e3, gain: 0, q: 1 })) };
      } else if (rows) {
        const par = p[3], lo = +p[4], hi = +p[5], cur = +p[7];
        if (eq && eq.rows === rows && par === "count") {
          eq.count = Math.min(EQ.MAX_BANDS, Math.max(0, Math.round(cur)));
          continue;
        }
        const r = document.createElement("div");
        r.className = "fxp";
        const nm = document.createElement("span");
        nm.className = "mut";
        nm.dataset.fxp = par;
        const sl = document.createElement("input");
        sl.type = "range";
        sl.min = lo;
        sl.max = hi;
        sl.step = (hi - lo) / 200;
        sl.value = cur;
        const v = document.createElement("span");
        v.className = "mut";
        const show = (x) => {
          v.textContent = String(+(+x).toPrecision(3));
        };
        show(cur);
        const isPre = eq && eq.rows === rows && par === "preamp_db";
        if (isPre) {
          eq.preamp = cur;
          eq.preSl = sl;
          eq.preShow = show;
        }
        sl.oninput = () => {
          show(sl.value);
          if (isPre) {
            eq.preamp = +sl.value;
            eqDraw();
          }
        };
        sl.onchange = () => send("fx set " + app2 + " " + par + " " + sl.value);
        r.append(nm, sl, v);
        rows.appendChild(r);
      }
    }
    if (eq) {
      eqBuild();
      const ta = body.querySelector(".eqimp textarea");
      if (ta && oldText) {
        ta.value = oldText;
        ta.parentElement.open = true;
      } else if (oldOpen) body.querySelector(".eqimp").open = true;
    }
    if (!body.children.length) {
      const sp = document.createElement("span");
      sp.className = "mut";
      setT2(sp, "fx.none");
      body.appendChild(sp);
    }
    fxLabels();
  }
  function eqAria(e, key, n) {
    e.setAttribute("data-i18n-aria", key);
    e.setAttribute("data-i18n-p", JSON.stringify({ n }));
    e.setAttribute("aria-label", t2(key, { n }));
  }
  function eqDraw() {
    if (eq && eq.cv && app.route === "settings/fx") EQ.drawResponse(eq.cv, eq.bands.slice(0, eq.count), eq.preamp);
  }
  async function eqSendBand(i) {
    for (const f of ["type", "fc", "gain", "q"]) await sendFast(`fx set equalizer b${i}_${eqFieldName[f]} ${eq.bands[i][f]}`);
  }
  function eqBandRow(i) {
    const b = eq.bands[i], row2 = eqEl("div", "eqb"), head = eqEl("div", "eqh");
    const idx = eqEl("strong");
    idx.textContent = i + 1;
    const sel = eqEl("select", "viz-select");
    EQ_TYPES.forEach((k2, v) => {
      const o = eqEl("option");
      o.value = v;
      setT2(o, k2);
      sel.appendChild(o);
    });
    sel.value = b.type;
    eqAria(sel, "eq.type.aria", i + 1);
    sel.onchange = () => {
      b.type = +sel.value;
      eqDraw();
      eqSend(i, "type");
    };
    const rm = eqEl("button", "rm ctl");
    rm.textContent = "\u2715";
    eqAria(rm, "eq.remove", i + 1);
    rm.onclick = () => eqRemove(i);
    head.append(idx, sel, rm);
    row2.appendChild(head);
    for (const o of EQ_FIELDS) {
      const wrap = eqEl("div", "eqf"), lab = eqEl("span", "mut"), r = eqEl("input"), nu = eqEl("input");
      setT2(lab, o.lab);
      r.type = "range";
      r.min = o.rmin;
      r.max = o.rmax;
      r.step = o.rstep;
      r.value = o.pos(b[o.f]);
      eqAria(r, o.aria, i + 1);
      nu.type = "number";
      nu.min = o.lo;
      nu.max = o.hi;
      nu.step = o.step;
      nu.value = b[o.f];
      nu.inputMode = "decimal";
      eqAria(nu, o.num, i + 1);
      r.oninput = () => {
        b[o.f] = o.round(Math.min(o.hi, Math.max(o.lo, o.val(+r.value))));
        nu.value = b[o.f];
        eqDraw();
      };
      r.onchange = () => eqSend(i, o.f);
      nu.oninput = () => {
        const x = parseFloat(nu.value);
        if (isFinite(x)) {
          b[o.f] = Math.min(o.hi, Math.max(o.lo, x));
          r.value = o.pos(b[o.f]);
          eqDraw();
        }
      };
      nu.onchange = () => {
        const x = parseFloat(nu.value);
        if (isFinite(x)) {
          b[o.f] = Math.min(o.hi, Math.max(o.lo, x));
          eqSend(i, o.f);
        }
        nu.value = b[o.f];
        r.value = o.pos(b[o.f]);
        eqDraw();
      };
      wrap.append(lab, r, nu);
      row2.appendChild(wrap);
    }
    return row2;
  }
  function eqRenderBands() {
    eq.list.textContent = "";
    for (let i = 0; i < eq.count; i++) eq.list.appendChild(eqBandRow(i));
    if (!eq.count) {
      const e = eqEl("div", "mut");
      setT2(e, "eq.empty");
      eq.list.appendChild(e);
    }
    setT2(eq.title, "eq.bands.title", { n: eq.count, max: EQ.MAX_BANDS });
    eq.addBtn.disabled = !app.connected || eq.count >= EQ.MAX_BANDS;
    eqDraw();
  }
  async function eqAdd() {
    if (eq.count >= EQ.MAX_BANDS) return;
    const i = eq.count++;
    eq.bands[i] = { type: 0, fc: 1e3, gain: 0, q: 1 };
    eqRenderBands();
    await eqSendBand(i);
    await eqSendCount();
  }
  async function eqRemove(k2) {
    for (let j = k2; j < eq.count - 1; j++) eq.bands[j] = { ...eq.bands[j + 1] };
    eq.count--;
    eqRenderBands();
    for (let j = k2; j < eq.count; j++) await eqSendBand(j);
    await eqSendCount();
    await loadFx();
  }
  async function eqReset() {
    eq.count = 0;
    eq.preamp = 0;
    if (eq.preSl) {
      eq.preSl.value = 0;
      eq.preShow(0);
    }
    eqRenderBands();
    await eqSendCount();
    await send("fx set equalizer preamp_db 0");
  }
  async function eqImport(text, btn, note) {
    const r = EQ.parseAutoEq(text);
    if (!r.bands.length) {
      eqNote = { key: "eq.imp.none" };
      setT2(note, eqNote.key);
      return;
    }
    btn.disabled = true;
    setT2(note, "eq.imp.busy");
    await sendFast("fx set equalizer preamp_db " + (r.preamp === null ? 0 : r.preamp));
    for (let i = 0; i < r.bands.length; i++) {
      const b = r.bands[i];
      await sendFast(`fx set equalizer b${i}_type ${b.type}`);
      await sendFast(`fx set equalizer b${i}_fc ${b.fc}`);
      await sendFast(`fx set equalizer b${i}_gain_db ${b.gain}`);
      await sendFast(`fx set equalizer b${i}_q ${b.q}`);
    }
    await sendFast("fx set equalizer count " + r.bands.length);
    await sendFast("fx on equalizer 1");
    eqNote = r.overflow ? { key: "eq.imp.over", p: { n: r.bands.length, m: r.skipped, max: EQ.MAX_BANDS, k: r.overflow } } : { key: "eq.imp.done", p: { n: r.bands.length, m: r.skipped } };
    await loadFx();
    loadNight();
  }
  function eqBuild() {
    const rows = eq.rows;
    eq.cv = eqEl("canvas", "eqg");
    eq.cv.setAttribute("role", "img");
    eq.cv.setAttribute("data-i18n-aria", "eq.graph.aria");
    eq.cv.setAttribute("aria-label", t2("eq.graph.aria"));
    if (window.ResizeObserver) new ResizeObserver(() => eqDraw()).observe(eq.cv);
    const bar = eqEl("div", "eqbar");
    eq.title = eqEl("strong");
    eq.addBtn = eqEl("button", "ctl");
    setT2(eq.addBtn, "eq.add");
    eq.addBtn.onclick = eqAdd;
    const rs = eqEl("button", "ctl");
    setT2(rs, "eq.reset");
    rs.onclick = eqReset;
    bar.append(eq.title, eq.addBtn, rs);
    eq.list = eqEl("div");
    const det = eqEl("details", "eqimp"), sum = eqEl("summary"), ta = eqEl("textarea"), ib = eqEl("button", "ctl"), note = eqEl("div", "mut eqnote");
    setT2(sum, "eq.imp.title");
    ta.rows = 6;
    ta.spellcheck = false;
    ta.placeholder = "Preamp: -6.1 dB\nFilter 1: ON PK Fc 105 Hz Gain -3.2 dB Q 0.70";
    ta.setAttribute("data-i18n-aria", "eq.imp.aria");
    ta.setAttribute("aria-label", t2("eq.imp.aria"));
    setT2(ib, "eq.imp.btn");
    ib.onclick = () => eqImport(ta.value, ib, note);
    if (eqNote) {
      setT2(note, eqNote.key, eqNote.p);
      det.open = true;
    }
    det.append(sum, ta, ib, note);
    rows.append(eq.cv, bar, eq.list, det);
    eqRenderBands();
  }
  function initFx() {
    $("fxOff").onclick = async () => {
      await send("fx off");
      loadFx();
      loadNight();
    };
    const notes = () => {
      const state = app.state;
      $("fxCast").hidden = !(app.link === "on" && (app.selOut === "cast" || state && state.out === "cast"));
      $("fxShed").hidden = !(app.link === "on" && state && state.fxshed);
    };
    on("cast", notes);
    on("status", notes);
    on("lang", fxLabels);
    window.addEventListener("resize", eqDraw);
    if (window.matchMedia) window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", eqDraw);
  }
  var fxAppName, fxParName, EQ_TYPES, eq, eqNote, eqEl, eqFieldName, eqSend, eqSendCount, EQ_FIELDS, fxOnCount;
  var init_fx = __esm({
    "src/js/pages/fx.js"() {
      init_app();
      init_i18n();
      init_link();
      init_eq();
      init_night();
      fxAppName = (app2, fb) => I18N.has("fx.app." + app2) ? t2("fx.app." + app2) : fb || app2;
      fxParName = (par) => I18N.has("fx.p." + par) ? t2("fx.p." + par) : par;
      EQ_TYPES = ["eq.type.pk", "eq.type.ls", "eq.type.hs"];
      eq = null;
      eqNote = null;
      eqEl = (tag, cls) => {
        const e = document.createElement(tag);
        if (cls) e.className = cls;
        return e;
      };
      eqFieldName = { type: "type", fc: "fc", gain: "gain_db", q: "q" };
      eqSend = (i, f) => send(`fx set equalizer b${i}_${eqFieldName[f]} ${eq.bands[i][f]}`);
      eqSendCount = () => sendFast("fx set equalizer count " + eq.count);
      EQ_FIELDS = [
        {
          f: "fc",
          lab: "eq.fc",
          aria: "eq.fc.aria",
          num: "eq.fcnum.aria",
          lo: 20,
          hi: 2e4,
          step: 1,
          rmin: 0,
          rmax: 1e3,
          rstep: 1,
          pos: (v) => 1e3 * Math.log(v / 20) / Math.log(1e3),
          val: (p) => 20 * Math.pow(1e3, p / 1e3),
          round: (v) => v < 100 ? Math.round(v * 10) / 10 : Math.round(v)
        },
        {
          f: "gain",
          lab: "eq.gain",
          aria: "eq.gain.aria",
          num: "eq.gainnum.aria",
          lo: -24,
          hi: 24,
          step: 0.1,
          rmin: -24,
          rmax: 24,
          rstep: 0.1,
          pos: (v) => v,
          val: (p) => p,
          round: (v) => Math.round(v * 10) / 10
        },
        {
          f: "q",
          lab: "eq.q",
          aria: "eq.q.aria",
          num: "eq.qnum.aria",
          lo: 0.1,
          hi: 10,
          step: 0.05,
          rmin: 0,
          rmax: 1e3,
          rstep: 1,
          pos: (v) => 1e3 * Math.log(v / 0.1) / Math.log(100),
          val: (p) => 0.1 * Math.pow(100, p / 1e3),
          round: (v) => Math.round(v * 100) / 100
        }
      ];
      fxOnCount = () => {
        const b = $("fxBody").querySelectorAll(".fxapp input[type=checkbox]");
        return b.length ? [...b].filter((c) => c.checked).length : null;
      };
    }
  });

  // src/js/pages/night.js
  async function loadNight() {
    const r = await send("night"), l = r.find((x) => x.startsWith("night:")) || "";
    if (!l) return;
    const on2 = /night: on/.test(l);
    segSet("night", on2 ? "on" : "off");
    const m = l.match(/schedule (\d\d:\d\d)-(\d\d:\d\d)/);
    nightRep = { on: on2, sched: m ? [m[1], m[2]] : null };
    if (m) setT2($("nightv"), "night.sched", { from: m[1], to: m[2] });
    else setT2($("nightv"), "night.nosched");
    if (m) {
      $("nFrom").value = m[1];
      $("nTo").value = m[2];
    }
  }
  function initNight() {
    segBind("night", (v) => send("night " + v).then(() => {
      loadNight();
      loadFx();
    }));
    $("btnNight").onclick = async () => {
      if ($("nFrom").value && $("nTo").value) {
        await send(`night auto ${$("nFrom").value} ${$("nTo").value}`);
        loadNight();
      }
    };
    $("btnNightOff").onclick = async () => {
      await send("night auto off");
      loadNight();
    };
  }
  var nightRep, nightInfo;
  var init_night = __esm({
    "src/js/pages/night.js"() {
      init_app();
      init_link();
      init_fx();
      nightRep = null;
      nightInfo = () => nightRep;
    }
  });

  // src/vendor/tunetap/radio/route.js
  function isUuid(s) {
    return typeof s === "string" && UUID_RE.test(s);
  }
  function normalizeStreamUrl(u) {
    if (typeof u !== "string") return null;
    const s = u.trim();
    if (!s || s.length > MAX_URL || BAD_CHARS.test(s)) return null;
    const explicit = /^[a-z][a-z0-9+.-]*:(?!\d+(?:[/?#]|$))/i.test(s);
    if (explicit && !/^https:\/\//i.test(s)) return null;
    const full = explicit ? s : "https://" + s.replace(/^\/\//, "");
    try {
      const p = new URL(full);
      if (p.protocol !== "https:" || !p.hostname || p.username || p.password) return null;
      return p.href;
    } catch {
      return null;
    }
  }
  function stationId(url) {
    const s = normalizeStreamUrl(url) ?? String(url);
    let a = 2166136261;
    let b = 16777619;
    for (let i = 0; i < s.length; i++) {
      const c = s.charCodeAt(i);
      a = Math.imul(a ^ c, 16777619) >>> 0;
      b = Math.imul(b ^ c, 2246822507) >>> 0;
      b ^= b >>> 13;
    }
    return "u-" + a.toString(16).padStart(8, "0") + (b >>> 0).toString(16).padStart(8, "0");
  }
  var UUID_RE, MAX_URL, BAD_CHARS;
  var init_route = __esm({
    "src/vendor/tunetap/radio/route.js"() {
      UUID_RE = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i;
      MAX_URL = 2e3;
      BAD_CHARS = /[\u0000- \u007f]/;
    }
  });

  // src/vendor/tunetap/radio/rb.js
  function httpUrl(s) {
    if (typeof s !== "string" || s.length > MAX_URL_LEN || /[\u0000- \u007f]/.test(s)) return null;
    try {
      const u = new URL(s);
      return (u.protocol === "https:" || u.protocol === "http:") && u.hostname ? s : null;
    } catch {
      return null;
    }
  }
  function streamKey(s) {
    return String(s).trim().replace(/^[a-z][a-z0-9+.-]*:\/\//i, "").replace(/\/+$/, "").replace(/^[^/?#]+/, (h) => h.toLowerCase());
  }
  function candidates(st2) {
    if (!st2 || typeof st2 !== "object") return [];
    const out = [];
    for (const c of [st2.url_resolved, st2.url]) {
      const u = httpUrl(c);
      if (u && !out.includes(u)) out.push(u);
    }
    return out;
  }
  function toItem(station) {
    if (!station || typeof station !== "object" || Array.isArray(station)) return null;
    if (!isUuid(station.stationuuid)) return null;
    const name = str(station.name, 120).trim();
    if (!name) return null;
    const all2 = candidates(station);
    const urls = [...all2.filter((u) => u.startsWith("https:")), ...all2.filter((u) => !u.startsWith("https:"))].slice(0, MAX_URLS);
    if (!urls.length) return null;
    const fav = httpUrl(station.favicon);
    return {
      id: station.stationuuid,
      name,
      urls,
      icon: fav && fav.startsWith("https:") ? fav : "",
      tags: str(station.tags, 200),
      country: str(station.country, 60),
      added: 0
    };
  }
  function createRb({ fetch, storage, setTimeout: setT4 = globalThis.setTimeout, clearTimeout: clearT = globalThis.clearTimeout } = {}) {
    const read2 = (k2) => {
      try {
        return storage.getItem(k2) || "";
      } catch {
        return "";
      }
    };
    const write2 = (k2, v) => {
      try {
        storage.setItem(k2, v);
      } catch {
      }
    };
    let last = read2(LAST_KEY);
    if (!isHost(last)) last = "";
    let hosts = BUILTIN.slice();
    let discovered = false;
    let failed = [];
    async function getJson(host, path, outer) {
      const ctl = new AbortController();
      const t6 = setT4(() => ctl.abort(), TIMEOUT_MS);
      const onOuter = () => ctl.abort();
      if (outer) {
        if (outer.aborted) ctl.abort();
        else outer.addEventListener("abort", onOuter);
      }
      try {
        const r = await fetch(`https://${host}/json/${path}`, { signal: ctl.signal });
        if (!r || !r.ok) throw new Error("http");
        return await r.json();
      } finally {
        clearT(t6);
        if (outer) outer.removeEventListener("abort", onOuter);
      }
    }
    async function discover(host) {
      if (discovered) return;
      discovered = true;
      try {
        const list = await getJson(host, "servers");
        if (!Array.isArray(list)) return;
        const names = [...new Set(list.map((x) => x && typeof x.name === "string" ? x.name.toLowerCase() : "").filter(isHost))];
        if (!names.length) return;
        hosts = ["all.api.radio-browser.info", ...names.filter((h) => h !== "all.api.radio-browser.info")];
      } catch {
      }
    }
    async function call2(path, { persist = true } = {}) {
      const fresh = [...last ? [last] : [], ...hosts].filter((h, i, a) => a.indexOf(h) === i && !failed.includes(h));
      const order = [...fresh, ...failed].slice(0, MAX_TRIES);
      const dl = new AbortController();
      const d4 = setT4(() => dl.abort(), DEADLINE_MS);
      try {
        for (const h of order) {
          if (dl.signal.aborted) break;
          try {
            const j = await getJson(h, path, dl.signal);
            failed = [];
            if (h !== last) {
              last = h;
              if (persist) write2(LAST_KEY, h);
            }
            discover(h);
            return j;
          } catch {
            failed = [...failed.filter((x) => x !== h), h];
            if (h === last) {
              last = "";
              if (persist) write2(LAST_KEY, "");
            }
          }
        }
        throw new Error("rb-unreachable");
      } finally {
        clearT(d4);
      }
    }
    return {
      async search(p = {}) {
        const q = new URLSearchParams();
        for (const k2 of ["name", "tag", "language"]) {
          if (typeof p[k2] === "string" && p[k2]) q.set(k2, p[k2].slice(0, 100));
        }
        if (typeof p.country === "string" && p.country) {
          if (/^[a-z]{2}$/i.test(p.country)) q.set("countrycode", p.country.toUpperCase());
          else q.set("country", p.country.slice(0, 100));
        }
        const order = ORDERS.has(p.order) ? p.order : "clickcount";
        q.set("order", order);
        if (REVERSE.has(order)) q.set("reverse", "true");
        const off = Number.isInteger(p.offset) && p.offset > 0 ? p.offset : 0;
        const lim = Number.isInteger(p.limit) && p.limit > 0 ? Math.min(p.limit, 200) : 50;
        q.set("offset", String(off));
        q.set("limit", String(lim));
        q.set("hidebroken", "true");
        const r = await call2("stations/search?" + q);
        return Array.isArray(r) ? r.filter((x) => x && typeof x === "object") : [];
      },
      async byUuid(uuid) {
        if (!isUuid(uuid)) return null;
        const r = await call2("stations/byuuid/" + uuid);
        const s = Array.isArray(r) ? r[0] : null;
        return s && typeof s === "object" ? s : null;
      },
      // Lookup by stream URL (the NFC page; user-initiated only). One request, no filters. Exact match
      // (trailing slash and http/https ignored) wins, else the first answer. null: nothing found or bad URL.
      async byUrl(url) {
        if (!httpUrl(url)) return null;
        const r = await call2("stations/byurl?" + new URLSearchParams({ url }), { persist: false });
        const list = Array.isArray(r) ? r.filter((x) => x && typeof x === "object" && !Array.isArray(x)) : [];
        const key = streamKey(url);
        return list.find((s) => [s.url_resolved, s.url].some((c) => typeof c === "string" && streamKey(c) === key)) || list[0] || null;
      },
      async lists(kind) {
        if (!LIST_KINDS.has(kind)) throw new Error("bad-kind");
        const r = await call2(`${kind}?hidebroken=true&order=stationcount&reverse=true`);
        if (!Array.isArray(r)) return [];
        return r.filter((x) => x && typeof x.name === "string" && x.name).map((x) => ({ name: x.name, stationcount: Number.isFinite(x.stationcount) ? x.stationcount : 0 }));
      },
      hosts: () => hosts.slice(),
      last: () => last
    };
  }
  var BUILTIN, HOST_RE, LAST_KEY, TIMEOUT_MS, MAX_TRIES, DEADLINE_MS, MAX_URLS, MAX_URL_LEN, ORDERS, REVERSE, LIST_KINDS, isHost, str;
  var init_rb = __esm({
    "src/vendor/tunetap/radio/rb.js"() {
      init_route();
      BUILTIN = [
        "all.api.radio-browser.info",
        "de1.api.radio-browser.info",
        "de2.api.radio-browser.info",
        "nl1.api.radio-browser.info",
        "at1.api.radio-browser.info",
        "fi1.api.radio-browser.info"
      ];
      HOST_RE = /^[a-z0-9-]+(\.[a-z0-9-]+)*\.radio-browser\.info$/i;
      LAST_KEY = "tunetap.rb.last";
      TIMEOUT_MS = 8e3;
      MAX_TRIES = 3;
      DEADLINE_MS = 15e3;
      MAX_URLS = 3;
      MAX_URL_LEN = 500;
      ORDERS = /* @__PURE__ */ new Set(["name", "votes", "clickcount", "clicktrend", "bitrate", "lastcheckok", "random"]);
      REVERSE = /* @__PURE__ */ new Set(["votes", "clickcount", "clicktrend", "bitrate"]);
      LIST_KINDS = /* @__PURE__ */ new Set(["tags", "countries", "languages"]);
      isHost = (h) => typeof h === "string" && HOST_RE.test(h);
      str = (v, max) => typeof v === "string" ? v.slice(0, max) : "";
    }
  });

  // src/vendor/tunetap/radio/find-model.js
  var PAGE, hasMore, isSubmitKey;
  var init_find_model = __esm({
    "src/vendor/tunetap/radio/find-model.js"() {
      init_rb();
      PAGE = 30;
      hasMore = (rawCount) => rawCount >= PAGE;
      isSubmitKey = (e) => e?.key === "Enter" && !e.isComposing && e.keyCode !== 229;
    }
  });

  // src/vendor/tunetap/radio/regions.js
  var COUNTRIES, GENRES;
  var init_regions = __esm({
    "src/vendor/tunetap/radio/regions.js"() {
      COUNTRIES = [
        { code: "TW", tw: "\u53F0\u7063", cn: "\u53F0\u6E7E", en: "Taiwan" },
        { code: "HK", tw: "\u9999\u6E2F", cn: "\u9999\u6E2F", en: "Hong Kong" },
        { code: "MO", tw: "\u6FB3\u9580", cn: "\u6FB3\u95E8", en: "Macau" },
        { code: "CN", tw: "\u4E2D\u570B", cn: "\u4E2D\u56FD", en: "China" },
        { code: "JP", tw: "\u65E5\u672C", cn: "\u65E5\u672C", en: "Japan" },
        { code: "KR", tw: "\u97D3\u570B", cn: "\u97E9\u56FD", en: "South Korea" },
        { code: "KP", tw: "\u5317\u97D3", cn: "\u671D\u9C9C", en: "North Korea" },
        { code: "MN", tw: "\u8499\u53E4", cn: "\u8499\u53E4", en: "Mongolia" },
        { code: "US", tw: "\u7F8E\u570B", cn: "\u7F8E\u56FD", en: "United States" },
        { code: "CA", tw: "\u52A0\u62FF\u5927", cn: "\u52A0\u62FF\u5927", en: "Canada" },
        { code: "MX", tw: "\u58A8\u897F\u54E5", cn: "\u58A8\u897F\u54E5", en: "Mexico" },
        { code: "BR", tw: "\u5DF4\u897F", cn: "\u5DF4\u897F", en: "Brazil" },
        { code: "AR", tw: "\u963F\u6839\u5EF7", cn: "\u963F\u6839\u5EF7", en: "Argentina" },
        { code: "CL", tw: "\u667A\u5229", cn: "\u667A\u5229", en: "Chile" },
        { code: "CO", tw: "\u54E5\u502B\u6BD4\u4E9E", cn: "\u54E5\u4F26\u6BD4\u4E9A", en: "Colombia" },
        { code: "PE", tw: "\u79D8\u9B6F", cn: "\u79D8\u9C81", en: "Peru" },
        { code: "VE", tw: "\u59D4\u5167\u745E\u62C9", cn: "\u59D4\u5185\u745E\u62C9", en: "Venezuela" },
        { code: "EC", tw: "\u5384\u74DC\u591A", cn: "\u5384\u74DC\u591A\u5C14", en: "Ecuador" },
        { code: "BO", tw: "\u73BB\u5229\u7DAD\u4E9E", cn: "\u73BB\u5229\u7EF4\u4E9A", en: "Bolivia" },
        { code: "PY", tw: "\u5DF4\u62C9\u572D", cn: "\u5DF4\u62C9\u572D", en: "Paraguay" },
        { code: "UY", tw: "\u70CF\u62C9\u572D", cn: "\u4E4C\u62C9\u572D", en: "Uruguay" },
        { code: "CU", tw: "\u53E4\u5DF4", cn: "\u53E4\u5DF4", en: "Cuba" },
        { code: "DO", tw: "\u591A\u660E\u5C3C\u52A0", cn: "\u591A\u7C73\u5C3C\u52A0", en: "Dominican Republic" },
        { code: "PR", tw: "\u6CE2\u591A\u9ECE\u5404", cn: "\u6CE2\u591A\u9ECE\u5404", en: "Puerto Rico" },
        { code: "JM", tw: "\u7259\u8CB7\u52A0", cn: "\u7259\u4E70\u52A0", en: "Jamaica" },
        { code: "CR", tw: "\u54E5\u65AF\u5927\u9ECE\u52A0", cn: "\u54E5\u65AF\u8FBE\u9ECE\u52A0", en: "Costa Rica" },
        { code: "PA", tw: "\u5DF4\u62FF\u99AC", cn: "\u5DF4\u62FF\u9A6C", en: "Panama" },
        { code: "GT", tw: "\u74DC\u5730\u99AC\u62C9", cn: "\u5371\u5730\u9A6C\u62C9", en: "Guatemala" },
        { code: "GB", tw: "\u82F1\u570B", cn: "\u82F1\u56FD", en: "United Kingdom" },
        { code: "IE", tw: "\u611B\u723E\u862D", cn: "\u7231\u5C14\u5170", en: "Ireland" },
        { code: "FR", tw: "\u6CD5\u570B", cn: "\u6CD5\u56FD", en: "France" },
        { code: "DE", tw: "\u5FB7\u570B", cn: "\u5FB7\u56FD", en: "Germany" },
        { code: "IT", tw: "\u7FA9\u5927\u5229", cn: "\u610F\u5927\u5229", en: "Italy" },
        { code: "ES", tw: "\u897F\u73ED\u7259", cn: "\u897F\u73ED\u7259", en: "Spain" },
        { code: "PT", tw: "\u8461\u8404\u7259", cn: "\u8461\u8404\u7259", en: "Portugal" },
        { code: "NL", tw: "\u8377\u862D", cn: "\u8377\u5170", en: "Netherlands" },
        { code: "BE", tw: "\u6BD4\u5229\u6642", cn: "\u6BD4\u5229\u65F6", en: "Belgium" },
        { code: "LU", tw: "\u76E7\u68EE\u5821", cn: "\u5362\u68EE\u5821", en: "Luxembourg" },
        { code: "CH", tw: "\u745E\u58EB", cn: "\u745E\u58EB", en: "Switzerland" },
        { code: "AT", tw: "\u5967\u5730\u5229", cn: "\u5965\u5730\u5229", en: "Austria" },
        { code: "SE", tw: "\u745E\u5178", cn: "\u745E\u5178", en: "Sweden" },
        { code: "NO", tw: "\u632A\u5A01", cn: "\u632A\u5A01", en: "Norway" },
        { code: "DK", tw: "\u4E39\u9EA5", cn: "\u4E39\u9EA6", en: "Denmark" },
        { code: "FI", tw: "\u82AC\u862D", cn: "\u82AC\u5170", en: "Finland" },
        { code: "IS", tw: "\u51B0\u5CF6", cn: "\u51B0\u5C9B", en: "Iceland" },
        { code: "EE", tw: "\u611B\u6C99\u5C3C\u4E9E", cn: "\u7231\u6C99\u5C3C\u4E9A", en: "Estonia" },
        { code: "LV", tw: "\u62C9\u812B\u7DAD\u4E9E", cn: "\u62C9\u8131\u7EF4\u4E9A", en: "Latvia" },
        { code: "LT", tw: "\u7ACB\u9676\u5B9B", cn: "\u7ACB\u9676\u5B9B", en: "Lithuania" },
        { code: "PL", tw: "\u6CE2\u862D", cn: "\u6CE2\u5170", en: "Poland" },
        { code: "CZ", tw: "\u6377\u514B", cn: "\u6377\u514B", en: "Czechia" },
        { code: "SK", tw: "\u65AF\u6D1B\u4F10\u514B", cn: "\u65AF\u6D1B\u4F10\u514B", en: "Slovakia" },
        { code: "HU", tw: "\u5308\u7259\u5229", cn: "\u5308\u7259\u5229", en: "Hungary" },
        { code: "RO", tw: "\u7F85\u99AC\u5C3C\u4E9E", cn: "\u7F57\u9A6C\u5C3C\u4E9A", en: "Romania" },
        { code: "BG", tw: "\u4FDD\u52A0\u5229\u4E9E", cn: "\u4FDD\u52A0\u5229\u4E9A", en: "Bulgaria" },
        { code: "GR", tw: "\u5E0C\u81D8", cn: "\u5E0C\u814A", en: "Greece" },
        { code: "HR", tw: "\u514B\u7F85\u57C3\u897F\u4E9E", cn: "\u514B\u7F57\u5730\u4E9A", en: "Croatia" },
        { code: "RS", tw: "\u585E\u723E\u7DAD\u4E9E", cn: "\u585E\u5C14\u7EF4\u4E9A", en: "Serbia" },
        { code: "SI", tw: "\u65AF\u6D1B\u7DAD\u5C3C\u4E9E", cn: "\u65AF\u6D1B\u6587\u5C3C\u4E9A", en: "Slovenia" },
        { code: "MT", tw: "\u99AC\u723E\u4ED6", cn: "\u9A6C\u8033\u4ED6", en: "Malta" },
        { code: "CY", tw: "\u8CFD\u666E\u52D2\u65AF", cn: "\u585E\u6D66\u8DEF\u65AF", en: "Cyprus" },
        { code: "TR", tw: "\u571F\u8033\u5176", cn: "\u571F\u8033\u5176", en: "Turkey" },
        { code: "RU", tw: "\u4FC4\u7F85\u65AF", cn: "\u4FC4\u7F57\u65AF", en: "Russia" },
        { code: "UA", tw: "\u70CF\u514B\u862D", cn: "\u4E4C\u514B\u5170", en: "Ukraine" },
        { code: "BY", tw: "\u767D\u4FC4\u7F85\u65AF", cn: "\u767D\u4FC4\u7F57\u65AF", en: "Belarus" },
        { code: "GE", tw: "\u55AC\u6CBB\u4E9E", cn: "\u683C\u9C81\u5409\u4E9A", en: "Georgia" },
        { code: "AM", tw: "\u4E9E\u7F8E\u5C3C\u4E9E", cn: "\u4E9A\u7F8E\u5C3C\u4E9A", en: "Armenia" },
        { code: "AZ", tw: "\u4E9E\u585E\u62DC\u7136", cn: "\u963F\u585E\u62DC\u7586", en: "Azerbaijan" },
        { code: "KZ", tw: "\u54C8\u85A9\u514B", cn: "\u54C8\u8428\u514B\u65AF\u5766", en: "Kazakhstan" },
        { code: "UZ", tw: "\u70CF\u8332\u5225\u514B", cn: "\u4E4C\u5179\u522B\u514B\u65AF\u5766", en: "Uzbekistan" },
        { code: "IN", tw: "\u5370\u5EA6", cn: "\u5370\u5EA6", en: "India" },
        { code: "PK", tw: "\u5DF4\u57FA\u65AF\u5766", cn: "\u5DF4\u57FA\u65AF\u5766", en: "Pakistan" },
        { code: "BD", tw: "\u5B5F\u52A0\u62C9", cn: "\u5B5F\u52A0\u62C9\u56FD", en: "Bangladesh" },
        { code: "LK", tw: "\u65AF\u91CC\u862D\u5361", cn: "\u65AF\u91CC\u5170\u5361", en: "Sri Lanka" },
        { code: "NP", tw: "\u5C3C\u6CCA\u723E", cn: "\u5C3C\u6CCA\u5C14", en: "Nepal" },
        { code: "AF", tw: "\u963F\u5BCC\u6C57", cn: "\u963F\u5BCC\u6C57", en: "Afghanistan" },
        { code: "ID", tw: "\u5370\u5C3C", cn: "\u5370\u5EA6\u5C3C\u897F\u4E9A", en: "Indonesia" },
        { code: "MY", tw: "\u99AC\u4F86\u897F\u4E9E", cn: "\u9A6C\u6765\u897F\u4E9A", en: "Malaysia" },
        { code: "SG", tw: "\u65B0\u52A0\u5761", cn: "\u65B0\u52A0\u5761", en: "Singapore" },
        { code: "TH", tw: "\u6CF0\u570B", cn: "\u6CF0\u56FD", en: "Thailand" },
        { code: "VN", tw: "\u8D8A\u5357", cn: "\u8D8A\u5357", en: "Vietnam" },
        { code: "PH", tw: "\u83F2\u5F8B\u8CD3", cn: "\u83F2\u5F8B\u5BBE", en: "Philippines" },
        { code: "MM", tw: "\u7DEC\u7538", cn: "\u7F05\u7538", en: "Myanmar" },
        { code: "KH", tw: "\u67EC\u57D4\u5BE8", cn: "\u67EC\u57D4\u5BE8", en: "Cambodia" },
        { code: "LA", tw: "\u5BEE\u570B", cn: "\u8001\u631D", en: "Laos" },
        { code: "AU", tw: "\u6FB3\u6D32", cn: "\u6FB3\u5927\u5229\u4E9A", en: "Australia" },
        { code: "NZ", tw: "\u7D10\u897F\u862D", cn: "\u65B0\u897F\u5170", en: "New Zealand" },
        { code: "IR", tw: "\u4F0A\u6717", cn: "\u4F0A\u6717", en: "Iran" },
        { code: "IQ", tw: "\u4F0A\u62C9\u514B", cn: "\u4F0A\u62C9\u514B", en: "Iraq" },
        { code: "IL", tw: "\u4EE5\u8272\u5217", cn: "\u4EE5\u8272\u5217", en: "Israel" },
        { code: "JO", tw: "\u7D04\u65E6", cn: "\u7EA6\u65E6", en: "Jordan" },
        { code: "LB", tw: "\u9ECE\u5DF4\u5AE9", cn: "\u9ECE\u5DF4\u5AE9", en: "Lebanon" },
        { code: "SY", tw: "\u6558\u5229\u4E9E", cn: "\u53D9\u5229\u4E9A", en: "Syria" },
        { code: "SA", tw: "\u6C99\u70CF\u5730\u963F\u62C9\u4F2F", cn: "\u6C99\u7279\u963F\u62C9\u4F2F", en: "Saudi Arabia" },
        { code: "AE", tw: "\u963F\u62C9\u4F2F\u806F\u5408\u5927\u516C\u570B", cn: "\u963F\u62C9\u4F2F\u8054\u5408\u914B\u957F\u56FD", en: "United Arab Emirates" },
        { code: "KW", tw: "\u79D1\u5A01\u7279", cn: "\u79D1\u5A01\u7279", en: "Kuwait" },
        { code: "QA", tw: "\u5361\u9054", cn: "\u5361\u5854\u5C14", en: "Qatar" },
        { code: "OM", tw: "\u963F\u66FC", cn: "\u963F\u66FC", en: "Oman" },
        { code: "BH", tw: "\u5DF4\u6797", cn: "\u5DF4\u6797", en: "Bahrain" },
        { code: "YE", tw: "\u8449\u9580", cn: "\u4E5F\u95E8", en: "Yemen" },
        { code: "EG", tw: "\u57C3\u53CA", cn: "\u57C3\u53CA", en: "Egypt" },
        { code: "MA", tw: "\u6469\u6D1B\u54E5", cn: "\u6469\u6D1B\u54E5", en: "Morocco" },
        { code: "DZ", tw: "\u963F\u723E\u53CA\u5229\u4E9E", cn: "\u963F\u5C14\u53CA\u5229\u4E9A", en: "Algeria" },
        { code: "TN", tw: "\u7A81\u5C3C\u897F\u4E9E", cn: "\u7A81\u5C3C\u65AF", en: "Tunisia" },
        { code: "NG", tw: "\u5948\u53CA\u5229\u4E9E", cn: "\u5C3C\u65E5\u5229\u4E9A", en: "Nigeria" },
        { code: "GH", tw: "\u8FE6\u7D0D", cn: "\u52A0\u7EB3", en: "Ghana" },
        { code: "SN", tw: "\u585E\u5167\u52A0\u723E", cn: "\u585E\u5185\u52A0\u5C14", en: "Senegal" },
        { code: "CI", tw: "\u8C61\u7259\u6D77\u5CB8", cn: "\u79D1\u7279\u8FEA\u74E6", en: "Cote d'Ivoire" },
        { code: "CM", tw: "\u5580\u9EA5\u9686", cn: "\u5580\u9EA6\u9686", en: "Cameroon" },
        { code: "KE", tw: "\u80AF\u4E9E", cn: "\u80AF\u5C3C\u4E9A", en: "Kenya" },
        { code: "ET", tw: "\u8863\u7D22\u6BD4\u4E9E", cn: "\u57C3\u585E\u4FC4\u6BD4\u4E9A", en: "Ethiopia" },
        { code: "TZ", tw: "\u5766\u5C1A\u5C3C\u4E9E", cn: "\u5766\u6851\u5C3C\u4E9A", en: "Tanzania" },
        { code: "UG", tw: "\u70CF\u5E72\u9054", cn: "\u4E4C\u5E72\u8FBE", en: "Uganda" },
        { code: "ZA", tw: "\u5357\u975E", cn: "\u5357\u975E", en: "South Africa" },
        { code: "ZW", tw: "\u8F9B\u5DF4\u5A01", cn: "\u6D25\u5DF4\u5E03\u97E6", en: "Zimbabwe" },
        { code: "ZM", tw: "\u5C1A\u6BD4\u4E9E", cn: "\u8D5E\u6BD4\u4E9A", en: "Zambia" },
        { code: "MZ", tw: "\u83AB\u4E09\u6BD4\u514B", cn: "\u83AB\u6851\u6BD4\u514B", en: "Mozambique" },
        { code: "AO", tw: "\u5B89\u54E5\u62C9", cn: "\u5B89\u54E5\u62C9", en: "Angola" }
      ];
      GENRES = [
        { id: "pop", tw: "\u6D41\u884C", cn: "\u6D41\u884C", en: "Pop" },
        { id: "rock", tw: "\u6416\u6EFE", cn: "\u6447\u6EDA", en: "Rock" },
        { id: "jazz", tw: "\u7235\u58EB", cn: "\u7235\u58EB", en: "Jazz" },
        { id: "classical", tw: "\u53E4\u5178", cn: "\u53E4\u5178", en: "Classical" },
        { id: "news", tw: "\u65B0\u805E", cn: "\u65B0\u95FB", en: "News" },
        { id: "talk", tw: "\u8AC7\u8A71", cn: "\u8C08\u8BDD", en: "Talk" },
        { id: "electronic", tw: "\u96FB\u5B50", cn: "\u7535\u5B50", en: "Electronic" },
        { id: "dance", tw: "\u821E\u66F2", cn: "\u821E\u66F2", en: "Dance" },
        { id: "hip hop", tw: "\u563B\u54C8", cn: "\u563B\u54C8", en: "Hip hop" },
        { id: "country", tw: "\u9109\u6751", cn: "\u4E61\u6751", en: "Country" },
        { id: "folk", tw: "\u6C11\u8B20", cn: "\u6C11\u8C23", en: "Folk" },
        { id: "blues", tw: "\u85CD\u8ABF", cn: "\u84DD\u8C03", en: "Blues" },
        { id: "reggae", tw: "\u96F7\u9B3C", cn: "\u96F7\u9B3C", en: "Reggae" },
        { id: "metal", tw: "\u91D1\u5C6C", cn: "\u91D1\u5C5E", en: "Metal" },
        { id: "ambient", tw: "\u6C1B\u570D", cn: "\u6C1B\u56F4", en: "Ambient" },
        { id: "chillout", tw: "\u653E\u9B06", cn: "\u653E\u677E", en: "Chillout" },
        { id: "lounge", tw: "\u6C99\u767C\u97F3\u6A02", cn: "\u6C99\u53D1\u97F3\u4E50", en: "Lounge" },
        { id: "oldies", tw: "\u61F7\u820A", cn: "\u6000\u65E7", en: "Oldies" },
        { id: "80s", tw: "80 \u5E74\u4EE3", cn: "80 \u5E74\u4EE3", en: "80s" },
        { id: "90s", tw: "90 \u5E74\u4EE3", cn: "90 \u5E74\u4EE3", en: "90s" },
        { id: "top 40", tw: "\u6392\u884C\u699C", cn: "\u6392\u884C\u699C", en: "Top 40" },
        { id: "r&b", tw: "R&B", cn: "R&B", en: "R&B" },
        { id: "soul", tw: "\u9748\u9B42\u6A02", cn: "\u7075\u9B42\u4E50", en: "Soul" },
        { id: "latin", tw: "\u62C9\u4E01", cn: "\u62C9\u4E01", en: "Latin" },
        { id: "world", tw: "\u4E16\u754C\u97F3\u6A02", cn: "\u4E16\u754C\u97F3\u4E50", en: "World" },
        { id: "sports", tw: "\u9AD4\u80B2", cn: "\u4F53\u80B2", en: "Sports" },
        { id: "religion", tw: "\u5B97\u6559", cn: "\u5B97\u6559", en: "Religion" },
        { id: "kids", tw: "\u5152\u7AE5", cn: "\u513F\u7AE5", en: "Kids" },
        { id: "college", tw: "\u6821\u5712", cn: "\u6821\u56ED", en: "College" },
        { id: "public radio", tw: "\u516C\u5171\u5EE3\u64AD", cn: "\u516C\u5171\u5E7F\u64AD", en: "Public radio" },
        { id: "culture", tw: "\u6587\u5316", cn: "\u6587\u5316", en: "Culture" },
        { id: "mandopop", tw: "\u83EF\u8A9E\u6D41\u884C", cn: "\u534E\u8BED\u6D41\u884C", en: "Mandopop" },
        { id: "cantopop", tw: "\u7CB5\u8A9E\u6D41\u884C", cn: "\u7CA4\u8BED\u6D41\u884C", en: "Cantopop" },
        { id: "j-pop", tw: "\u65E5\u672C\u6D41\u884C", cn: "\u65E5\u672C\u6D41\u884C", en: "J-pop" },
        { id: "k-pop", tw: "\u97D3\u570B\u6D41\u884C", cn: "\u97E9\u56FD\u6D41\u884C", en: "K-pop" },
        { id: "anime", tw: "\u52D5\u6F2B", cn: "\u52A8\u6F2B", en: "Anime" }
      ];
    }
  });

  // src/js/lib/radio.js
  function boxUrl(urls) {
    const ok = (urls || []).filter((u) => typeof u === "string" && /^https?:\/\/[^\s|]+$/i.test(u) && !isM3u8(u));
    return ok.find((u) => u.startsWith("http:")) || ok[0] || null;
  }
  function toRow(station) {
    const item = toItem(station);
    if (!item) return null;
    const url = boxUrl(item.urls);
    const codec = typeof station.codec === "string" ? station.codec.replace(/[^A-Za-z0-9.+ -]/g, "").slice(0, 12) : "";
    const bitrate = Number.isInteger(station.bitrate) && station.bitrate > 0 && station.bitrate < 1e5 ? station.bitrate : 0;
    const hls = station.hls === 1 || station.hls === true || !url;
    const pl = url && /\.(pls|m3u)$/i.test(new URL(url).pathname);
    return { item, url, codec, bitrate, hls, maybe: !hls && (!CODEC_OK.test(codec) || pl), scheme: url ? url.split(":")[0] : "" };
  }
  function cleanName(name, fallback) {
    let s = String(name || "").replace(/[|\u0000-\u001f\u007f-\u009f]/g, " ").trim();
    let out = "", n = 0;
    for (const ch of s) {
      const b = bytes(ch);
      if (n + b > 63) break;
      out += ch;
      n += b;
    }
    out = out.trim();
    return out || (fallback ? cleanName(fallback) : "");
  }
  function mountRadio({ favBox, findBox, send: send2, refresh, connected }) {
    const rb = createRb({ fetch: window.fetch.bind(window), storage: memStorage });
    let saved2 = [];
    const savedByKey = () => new Map(saved2.map((s) => [streamKey(s.url), s]));
    async function play2(url, note) {
      if (!connected()) {
        say(note, "rn.needConn", null, true);
        return;
      }
      say(note, "rn.sent");
      await send2("radio " + url);
      say(note, null);
      refresh();
    }
    const favNote = el2("div", { class: "mut rnote", role: "status" });
    const favUl = el2("ul", { class: "rlist" });
    const file = el2("input", { type: "file", hidden: true, accept: ".json,application/json" });
    const bExport = k(el2("button", { type: "button", disabled: true }), "rn.export");
    const bImport = k(el2("button", { type: "button" }), "rn.import");
    favBox.textContent = "";
    favBox.append(favUl, el2("div", { class: "row", style: "margin-top:.5rem" }, bExport, bImport, file), favNote);
    function renderSaved(empty) {
      favUl.textContent = "";
      for (const s of saved2) {
        const main = el2("button", { type: "button", class: "rmain" }, el2("span", { class: "rname" }, s.name), el2("span", { class: "rmeta" }, s.url));
        main.onclick = () => {
          if (!connected()) return say(favNote, "rn.needConn", null, true);
          play2("" + s.n, favNote);
        };
        const del = el2("button", { type: "button", class: "star" }, "\u2715");
        ka(del, "aria", "rn.removeAria", { name: s.name });
        ka(del, "title", "rn.removeTitle");
        del.onclick = () => removeAt(s.n);
        favUl.append(el2("li", { class: "rrow" }, main, del));
      }
      if (!saved2.length) favUl.append(k(el2("li", { class: "mut rempty" }), empty || "rn.empty"));
      bExport.disabled = !saved2.length;
      renderResults();
    }
    async function reload() {
      if (!connected()) {
        saved2 = [];
        renderSaved("fav.hint");
        return;
      }
      const r = await send2("radio list");
      saved2 = [];
      for (const l of r) {
        const m = l.match(/^(\d+) (.*) (https?:\/\/\S+)\s*$/);
        if (m) saved2.push({ n: +m[1], name: m[2], url: m[3] });
      }
      renderSaved();
    }
    async function removeAt(n) {
      const r = await send2("radio del " + n);
      const ok = r.some((l) => l.startsWith("radio del: ok"));
      await reload();
      const line2 = r.find((l) => l.startsWith("radio del"));
      say(favNote, ok ? "rn.removed" : "rn.removeFailed", ok ? null : { why: line2 || { t: "rn.noReply" } }, !ok);
    }
    async function addStation(name, url) {
      if (!connected()) return { ok: false, key: "rn.needConn" };
      if (!urlOk(url)) return { ok: false, key: bytes(url) > 255 ? "rn.urlLong" : "rn.urlBad" };
      let host = url;
      try {
        host = new URL(url).hostname;
      } catch {
      }
      const nm = cleanName(name, host);
      const r = await send2(`radio add ${nm}|${url}`);
      const l = r.find((x) => x.startsWith("radio add:")) || "";
      const m = l.match(/^radio add:\s*(\d+)/);
      if (m) return { ok: true, n: +m[1], key: "rn.saved" };
      if (!l) return { ok: false, key: "rn.devNoReply" };
      const why = l.replace(/^radio add:\s*/, ""), key = "rn.add." + slug(why);
      return { ok: false, key: I18N.has(key) ? key : "rn.add.other", params: { why }, full: why === "list full" };
    }
    bExport.onclick = () => {
      const items = saved2.map((s) => ({ id: stationId(s.url), name: s.name, urls: [s.url], icon: "", tags: "", country: "", added: Date.now() }));
      const url = URL.createObjectURL(new Blob([JSON.stringify(items, null, 2)], { type: "application/json" }));
      const a = el2("a", { href: url, hidden: true });
      a.download = "tunetap-favorites.json";
      document.body.append(a);
      a.click();
      a.remove();
      setTimeout(() => URL.revokeObjectURL(url), 1e3);
      say(favNote, "rn.exported", { n: items.length });
    };
    bImport.onclick = () => {
      if (!connected()) return say(favNote, "rn.needConn", null, true);
      file.value = "";
      file.click();
    };
    file.onchange = async () => {
      const f = file.files && file.files[0];
      if (!f) return;
      if (f.size > 1024 * 1024) return say(favNote, "rn.fileBig", null, true);
      let arr;
      try {
        arr = JSON.parse(await f.text());
        if (!Array.isArray(arr) || arr.length > 1e3) throw 0;
      } catch {
        return say(favNote, "rn.badFile", null, true);
      }
      let added = 0, refused = 0, why = null;
      for (let i = 0; i < arr.length; i++) {
        const it = arr[i], url = it && typeof it === "object" ? boxUrl(Array.isArray(it.urls) ? it.urls : []) : null;
        if (!url || typeof it.name !== "string") {
          refused++;
          continue;
        }
        say(favNote, "rn.importing", { i: i + 1, n: arr.length });
        const r = await addStation(it.name, url);
        if (r.ok) added++;
        else {
          refused++;
          why = { t: r.key, p: r.params };
          if (r.full) {
            refused += arr.length - i - 1;
            break;
          }
        }
      }
      await reload();
      say(favNote, why ? "rn.importDoneWhy" : "rn.importDone", { added, refused, why }, refused > 0 && !added);
    };
    const q = ka(ka(el2("input", { type: "search", maxLength: 100, autocomplete: "off" }), "placeholder", "rn.q"), "aria", "rn.q");
    const named = (value2, o) => {
      const e = el2("option", { value: value2 }, nameOf(o));
      e._nm = o;
      return e;
    };
    const country = ka(el2("select"), "aria", "rn.region");
    country.append(k(el2("option", { value: "" }), "rn.regionAll"), ...COUNTRIES.map((c) => named(c.code, c)));
    const tag = ka(el2("select"), "aria", "rn.genre");
    tag.append(k(el2("option", { value: "" }), "rn.genreAll"), ...GENRES.map((g2) => named(g2.id, g2)));
    const codec = ka(el2("select"), "aria", "rn.codec");
    codec.append(k(el2("option", { value: "" }), "rn.codecAll"), el2("option", { value: "mp3" }, "MP3"), el2("option", { value: "aac" }, "AAC"), el2("option", { value: "ogg" }, "Ogg"));
    const kbps = ka(el2("select"), "aria", "rn.kbps");
    kbps.append(k(el2("option", { value: "0" }), "rn.kbpsAll"), el2("option", { value: "64" }, "\u2265 64 kbps"), el2("option", { value: "128" }, "\u2265 128 kbps"), el2("option", { value: "192" }, "\u2265 192 kbps"));
    const go = k(el2("button", { type: "button", class: "primary" }), "rn.search");
    const showHls = el2("input", { type: "checkbox", id: "showHls" });
    const status = el2("div", { class: "mut rnote", role: "status" });
    const ul = el2("ul", { class: "rlist" });
    const more = k(el2("button", { type: "button", hidden: true }), "rn.more");
    findBox.textContent = "";
    findBox.append(
      el2("div", { class: "row" }, el2("div", { class: "grow", style: "display:flex" }, q), go),
      el2("div", { class: "rfilters" }, country, tag, codec, kbps),
      el2("label", { class: "rchk", for: "showHls" }, showHls, k(el2("span"), "rn.hlsToo")),
      status,
      ul,
      more
    );
    I18N.onChange(() => {
      for (const sel of [country, tag]) for (const o of sel.options) if (o._nm) o.textContent = nameOf(o._nm);
    });
    function rowEl(r) {
      const { item } = r;
      const sv = r.url ? savedByKey().get(streamKey(r.url)) : null;
      const meta = [item.country, r.codec, r.bitrate ? r.bitrate + " kbps" : "", r.scheme].filter(Boolean).join(" \xB7 ");
      const main = el2(
        "button",
        { type: "button", class: "rmain", disabled: r.hls },
        el2("span", { class: "rname" }, item.name),
        meta ? el2("span", { class: "rmeta" }, meta) : null
      );
      if (r.hls) {
        ka(main, "title", "rn.hlsTitle");
        main.append(k(el2("span", { class: "rtag bad" }), "rn.hlsTag"));
      } else {
        if (r.maybe) main.append(k(el2("span", { class: "rtag" }), "rn.maybeTag"));
        if (sv) main.append(k(el2("span", { class: "rtag ok" }), "rn.savedTag"));
      }
      main.onclick = () => play2(r.url, status);
      const star = el2("button", { type: "button", class: "star", disabled: r.hls, "aria-pressed": String(!!sv) }, sv ? "\u2605" : "\u2606");
      ka(star, "aria", sv ? "rn.unsaveAria" : "rn.saveAria", { name: item.name });
      star.onclick = async () => {
        star.disabled = true;
        if (sv) {
          await removeAt(sv.n);
          return;
        }
        const res = await addStation(item.name, r.url);
        await reload();
        if (res.ok) say(status, "rn.savedOne", { name: cleanName(item.name) });
        else say(status, res.key, res.params, true);
      };
      return el2("li", { class: "rrow" }, main, star);
    }
    let rows = [], rawCount = 0, lastCount = 0, loading = false, failed = false, token = 0, searched = false, hiddenHls = 0;
    const seen = /* @__PURE__ */ new Set();
    const codecOf = (r) => {
      const c = r.codec.toLowerCase();
      return c.startsWith("aac") ? "aac" : c === "mp3" ? "mp3" : c === "ogg" || c === "vorbis" || c === "opus" ? "ogg" : c;
    };
    const passes = (r) => (showHls.checked || !r.hls) && (!codec.value || codecOf(r) === codec.value) && (!r.bitrate ? +kbps.value === 0 : r.bitrate >= +kbps.value);
    function renderResults() {
      const vis = rows.filter(passes);
      ul.textContent = "";
      for (const r of vis) ul.append(rowEl(r));
      hiddenHls = rows.filter((r) => r.hls && !showHls.checked).length;
      more.hidden = !hasMore(lastCount) || failed || !rows.length;
      more.disabled = loading;
      if (loading && !rows.length) say(status, "rn.searching");
      else if (failed) say(status, "rn.rbFail", null, true);
      else if (searched && !vis.length) say(status, rows.length ? "rn.noMatchHid" : "rn.noMatch", { n: rows.length });
      else if (searched) say(status, hiddenHls ? "rn.resultsHls" : "rn.results", { n: vis.length, h: hiddenHls });
      else say(status, null);
    }
    async function load(append) {
      const my = ++token;
      if (!append) {
        rows = [];
        rawCount = 0;
        lastCount = 0;
        seen.clear();
      }
      searched = true;
      failed = false;
      loading = true;
      renderResults();
      const p = { order: "clickcount", limit: PAGE, offset: append ? rawCount : 0 };
      if (q.value.trim()) p.name = q.value.trim();
      if (country.value) p.country = country.value;
      if (tag.value) p.tag = tag.value;
      try {
        const res = await rb.search(p);
        if (my !== token) return;
        rawCount += res.length;
        lastCount = res.length;
        for (const st2 of res) {
          const r = toRow(st2);
          if (r && !seen.has(r.item.id)) {
            seen.add(r.item.id);
            rows.push(r);
          }
        }
        loading = false;
      } catch {
        if (my !== token) return;
        loading = false;
        failed = true;
      }
      renderResults();
    }
    go.onclick = () => load(false);
    q.onkeydown = (e) => {
      if (isSubmitKey(e)) {
        e.preventDefault();
        load(false);
      }
    };
    more.onclick = () => {
      if (!loading) load(true);
    };
    for (const s of [country, tag]) s.onchange = () => load(false);
    for (const s of [codec, kbps, showHls]) s.onchange = renderResults;
    renderSaved("fav.hint");
    if (connected()) reload();
    return {
      reload,
      // the direct-URL box: save the typed address (name = host) to the card. Returns { ok, key, params } for the page's setT().
      async addUrl(url) {
        const r = await addStation("", url);
        if (r.ok) await reload();
        return r;
      }
    };
  }
  var mem, memStorage, CODEC_OK, isM3u8, bytes, urlOk, el2, t4, NAME_KEY, nameOf, k, ka, say, slug;
  var init_radio = __esm({
    "src/js/lib/radio.js"() {
      init_rb();
      init_find_model();
      init_regions();
      init_route();
      init_i18n();
      mem = /* @__PURE__ */ new Map();
      memStorage = { getItem: (k2) => mem.get(k2) ?? null, setItem: (k2, v) => {
        mem.set(k2, v);
      } };
      CODEC_OK = /^(mp3|aac\+?|ogg|vorbis|opus|unknown|)$/i;
      isM3u8 = (u) => {
        try {
          return new URL(u).pathname.toLowerCase().endsWith(".m3u8");
        } catch {
          return false;
        }
      };
      bytes = (s) => new TextEncoder().encode(s).length;
      urlOk = (u) => /^https?:\/\/[^\s|\u0000-\u001f]+$/i.test(u) && bytes(u) <= 255;
      el2 = (tag, props = {}, ...kids) => {
        const e = document.createElement(tag);
        for (const [k2, v] of Object.entries(props)) {
          if (k2 === "class") e.className = v;
          else if (k2 in e) e[k2] = v;
          else e.setAttribute(k2, v);
        }
        for (const c of kids) if (c != null) e.append(c);
        return e;
      };
      t4 = I18N.t;
      NAME_KEY = { "zh-TW": "tw", "zh-CN": "cn", en: "en" };
      nameOf = (o) => o[NAME_KEY[I18N.lang()]] || o.en;
      k = (e, key, params) => {
        I18N.setT(e, key, params);
        return e;
      };
      ka = (e, kind, key, params) => {
        e.setAttribute("data-i18n-" + kind, key);
        if (params) e.setAttribute("data-i18n-p", JSON.stringify(params));
        e.setAttribute(kind === "aria" ? "aria-label" : kind, t4(key, params));
        return e;
      };
      say = (node, key, params, bad) => {
        if (key) I18N.setT(node, key, params);
        else I18N.setRaw(node, "");
        node.classList.toggle("bad", !!bad);
      };
      slug = (s) => String(s).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
    }
  });

  // src/js/pages/radio.js
  function renderRstate() {
    const s = app.state;
    if (s) setRaw2($("rstate"), s.rstate ? known("rs.", s.rstate) : t2("radio.stateNone"));
  }
  function initRadio() {
    try {
      app.radio = mountRadio({ favBox: $("favBox"), findBox: $("findBox"), send, refresh: refreshStatus, connected: () => !!app.chCmd });
    } catch (e) {
      log("radio: " + e);
      setT2($("findBox"), "viz.needHttp");
      setRaw2($("favBox"), "");
    }
    $("btnRadio").onclick = async () => {
      const u = radioUrl();
      if (u) {
        setRaw2($("radioNote"), "");
        await send("radio " + u);
        refreshStatus();
      }
    };
    $("btnRadioFav").onclick = async () => {
      const u = radioUrl();
      if (!u) return;
      if (!app.radio) {
        setT2($("radioNote"), "url.noFav");
        return;
      }
      const r = await app.radio.addUrl(u);
      setT2($("radioNote"), r.key, r.params);
      $("radioNote").classList.toggle("bad", !r.ok);
    };
    on("status", renderRstate);
    on("lang", renderRstate);
  }
  var loadStations, radioUrl;
  var init_radio2 = __esm({
    "src/js/pages/radio.js"() {
      init_app();
      init_link();
      init_status();
      init_radio();
      loadStations = () => app.radio ? app.radio.reload() : Promise.resolve();
      radioUrl = () => {
        const u = $("radioUrl").value.trim();
        if (/^https?:\/\/[^\s]+$/i.test(u)) return u;
        setT2($("radioNote"), "url.bad");
        return "";
      };
    }
  });

  // src/js/pages/ambient.js
  async function loadScenes() {
    const r = await send("scene"), ul = $("scenes");
    if (app.link !== "on" && !r.length) return;
    ul.textContent = "";
    for (const l of r) {
      const m = l.match(/^(\d+) (.+)$/);
      if (!m) continue;
      const li = document.createElement("li");
      li.tabIndex = 0;
      li.textContent = m[2];
      li.onclick = () => send("scene " + m[1]).then(refreshStatus);
      ul.appendChild(li);
    }
    if (!ul.children.length) ul.appendChild(emptyLi("scenes.none"));
  }
  function initAmbient() {
    for (const c of ["white", "pink", "brown", "blue", "violet", "grey", "green", "velvet"]) {
      const b = document.createElement("button");
      b.className = "ctl";
      b.disabled = true;
      setT2(b, "noise." + c);
      b.onclick = () => send("noise " + c).then(refreshStatus);
      $("noise").appendChild(b);
    }
    $("btBed").onclick = async () => {
      await send("bedtime 30");
      refreshStatus();
      loadNight();
      loadFx();
    };
    $("btWake").onclick = async () => {
      await send("wake");
      refreshStatus();
      loadNight();
      loadFx();
    };
  }
  var init_ambient = __esm({
    "src/js/pages/ambient.js"() {
      init_app();
      init_link();
      init_status();
      init_night();
      init_fx();
    }
  });

  // src/js/pages/keys.js
  function buildKeys() {
    const grid = $("keyGrid");
    for (const b of KEY_BTNS) {
      const row2 = document.createElement("div");
      row2.className = "keyrow";
      const h = document.createElement("strong");
      setT2(h, "keys.btn." + b);
      row2.appendChild(h);
      const cells = document.createElement("div");
      cells.className = "keycells";
      for (const g2 of KEY_GS) {
        const lab = document.createElement("label"), sp = document.createElement("span"), sel = document.createElement("select");
        setT2(sp, "keys.g." + g2);
        sel.className = "viz-select ctl";
        sel.disabled = true;
        sel.setAttribute("data-i18n-aria", "keys.sel");
        sel.setAttribute("data-i18n-p", JSON.stringify({ b: { t: "keys.btn." + b }, g: { t: "keys.g." + g2 } }));
        sel.setAttribute("aria-label", t2("keys.sel", { b: { t: "keys.btn." + b }, g: { t: "keys.g." + g2 } }));
        for (const [a, k2] of Object.entries(KEY_ACTS)) {
          const o = document.createElement("option");
          o.value = a;
          setT2(o, k2);
          sel.appendChild(o);
        }
        sel.onchange = () => keysCmd(`keys set ${b} ${g2} ${sel.value}`);
        keySel[b + g2] = sel;
        lab.append(sp, sel);
        cells.appendChild(lab);
      }
      row2.appendChild(cells);
      grid.appendChild(row2);
    }
    for (const btn of $("keyPresets").children) btn.onclick = () => keysCmd("keys preset " + btn.dataset.n);
  }
  function parseKeys(r) {
    const map2 = {};
    for (const l of r) {
      const m = /^keys ([cpm])((?: [123h]=\S+)+)\s*$/.exec(l);
      if (!m) continue;
      map2[m[1]] = {};
      for (const x of m[2].trim().split(/\s+/)) map2[m[1]][x[0]] = x.slice(2);
    }
    const last = r.find((l) => /^keys last\b/.test(l)) || "";
    const lm = /^keys last ([cpm])([123h]) (\S+) (\d+) s ago/.exec(last);
    return { map: KEY_BTNS.every((b) => map2[b]) ? map2 : null, last: lm ? { b: lm[1], g: lm[2], a: lm[3], s: +lm[4] } : last ? "none" : null };
  }
  function renderKeyLast(last) {
    if (last === null) return;
    if (last === "none") {
      setT2($("keyLast"), "keys.last.none");
      return;
    }
    setT2($("keyLast"), "keys.last.some", {
      b: keyName("keys.btn.", last.b, true),
      g: keyName("keys.g.", last.g, true),
      a: last.a in KEY_ACTS ? { t: KEY_ACTS[last.a] } : last.a,
      s: last.s
    });
  }
  function renderKeys(p) {
    if (p.map) {
      keyMap = p.map;
      for (const b of KEY_BTNS) for (const g2 of KEY_GS) {
        const sel = keySel[b + g2], v = p.map[b][g2] || "none";
        for (const o of [...sel.options]) if (o.dataset.extra) o.remove();
        if (!(v in KEY_ACTS)) {
          const o = document.createElement("option");
          o.value = v;
          o.dataset.extra = "1";
          setRaw2(o, v);
          sel.appendChild(o);
        }
        sel.value = v;
      }
      let match = 0;
      for (const n of [1, 2, 3]) if (KEY_BTNS.every((b) => KEY_GS.every((g2, i) => p.map[b][g2] === KEY_PRESETS[n][b].split(" ")[i]))) match = n;
      for (const btn of $("keyPresets").children) btn.classList.toggle("on", +btn.dataset.n === match);
      $("keyCustom").hidden = !!match;
      keyMatch = match;
    }
    renderKeyLast(p.last);
  }
  async function keysCmd(cmd) {
    const r = await send(cmd);
    if (!r.length) return;
    const p = parseKeys(r);
    if (!p.map) {
      log("keys: " + (r[0] || ""));
      loadKeys();
      return;
    }
    renderKeys(p);
  }
  async function loadKeys() {
    const r = await send("keys");
    if (!r.length) return;
    renderKeys(parseKeys(r));
  }
  async function pollKeysLast() {
    if (keyPollBusy || app.scanBusy || app.link !== "on" || document.hidden || app.route !== "settings/keys") return;
    keyPollBusy = true;
    try {
      const r = await send("keys");
      if (r.length) renderKeyLast(parseKeys(r).last);
    } finally {
      keyPollBusy = false;
    }
  }
  function initKeys() {
    buildKeys();
    setInterval(pollKeysLast, 2e3);
  }
  var KEY_BTNS, KEY_GS, KEY_ACTS, KEY_PRESETS, keyMap, keySel, keyPollBusy, keyMatch, keyName, keysMatch;
  var init_keys = __esm({
    "src/js/pages/keys.js"() {
      init_app();
      init_link();
      KEY_BTNS = ["c", "p", "m"];
      KEY_GS = ["1", "2", "3", "h"];
      KEY_ACTS = { none: "keys.a.none", play: "keys.a.play", next: "keys.a.next", prev: "keys.a.prev", "vol+": "keys.a.volup", "vol-": "keys.a.voldn", stop: "keys.a.stop" };
      KEY_PRESETS = {
        // what `keys preset N` loads (firmware); only used to highlight the matching button
        1: { c: "play next prev none", p: "none none none none", m: "none none none none" },
        2: { c: "none none none none", p: "vol+ next none vol+", m: "vol- prev none vol-" },
        3: { c: "play next prev none", p: "vol+ none none vol+", m: "vol- none none vol-" }
      };
      keyMap = null;
      keySel = {};
      keyPollBusy = false;
      keyMatch = null;
      keyName = (pre, v, known2) => known2 ? { t: pre + v } : v;
      keysMatch = () => keyMatch;
    }
  });

  // src/js/lib/lrc.js
  function cutBytes(s, max) {
    if (utf8Len2(s) <= max) return s;
    let out = "", n = 0;
    for (const ch of s) {
      const b = utf8Len2(ch);
      if (n + b > max) break;
      out += ch;
      n += b;
    }
    return out;
  }
  function parseLrc(text) {
    const raw = [];
    let offset = 0;
    for (const src of text.split(/\r\n|\n|\r/)) {
      let rest = src, times = [];
      for (; ; ) {
        const m = /^\s*\[([^\]]*)\]/.exec(rest);
        if (!m) break;
        rest = rest.slice(m[0].length);
        const t6 = TIME.exec(m[1]);
        if (t6) {
          const f = t6[3] === void 0 ? 0 : t6[3].length === 1 ? +t6[3] * 100 : t6[3].length === 2 ? +t6[3] * 10 : +t6[3];
          times.push((+t6[1] * 60 + +t6[2]) * 1e3 + f);
        } else {
          const o = OFFSET.exec(m[1]);
          if (o) offset = parseInt(o[1], 10);
        }
      }
      if (!times.length) continue;
      const txt = rest.replace(WORD, "").trim();
      for (const ms of times) raw.push({ ms, text: txt });
    }
    const lines = raw.map((l) => ({ ms: l.ms - offset, text: l.text })).sort((a, b) => a.ms - b.ms);
    let truncated = false, bytes2 = 0;
    const out = [];
    for (const l of lines) {
      if (out.length >= MAX_LINES) {
        truncated = true;
        break;
      }
      let t6 = cutBytes(l.text, LINE_BYTES);
      if (t6 !== l.text) truncated = true;
      const b = utf8Len2(t6);
      if (bytes2 + b > TEXT_BYTES) {
        truncated = true;
        break;
      }
      bytes2 += b;
      out.push({ ms: l.ms, text: t6 });
    }
    return { lines: out, truncated };
  }
  function findLine(lines, posMs) {
    let lo = 0, hi = lines.length;
    while (lo < hi) {
      const mid = lo + hi >> 1;
      if (lines[mid].ms <= posMs) lo = mid + 1;
      else hi = mid;
    }
    return lo - 1;
  }
  function decodeLrc(bytes2) {
    if (bytes2.length >= 3 && bytes2[0] === 239 && bytes2[1] === 187 && bytes2[2] === 191) bytes2 = bytes2.subarray(3);
    for (const enc2 of ["utf-8", "big5", "gbk"]) {
      try {
        return new TextDecoder(enc2, { fatal: true }).decode(bytes2);
      } catch {
      }
    }
    return new TextDecoder("gbk").decode(bytes2);
  }
  function parseReply(replyLines) {
    if (replyLines.some((l) => l.startsWith("lrc none"))) return null;
    const b = replyLines.find((l) => l.startsWith("lrc begin"));
    if (!b) return null;
    const want = parseInt(b.slice(9), 10) || 0;
    const chunks = [];
    for (const l of replyLines) if (l.startsWith("lrc+ ")) {
      try {
        const s = atob(l.slice(5).trim());
        const u = new Uint8Array(s.length);
        for (let i = 0; i < s.length; i++) u[i] = s.charCodeAt(i);
        chunks.push(u);
      } catch {
        return null;
      }
    }
    const all2 = new Uint8Array(chunks.reduce((n, c) => n + c.length, 0));
    let o = 0;
    for (const c of chunks) {
      all2.set(c, o);
      o += c.length;
    }
    if (!all2.length || want && all2.length < want && !replyLines.includes("lrc end")) return null;
    const r = parseLrc(decodeLrc(all2.subarray(0, MAX_FILE)));
    return r.lines.length ? r : null;
  }
  var MAX_FILE, MAX_LINES, TEXT_BYTES, LINE_BYTES, TIME, OFFSET, WORD, utf8Len2;
  var init_lrc = __esm({
    "src/js/lib/lrc.js"() {
      MAX_FILE = 65536;
      MAX_LINES = 400;
      TEXT_BYTES = 24576;
      LINE_BYTES = 160;
      TIME = /^(\d{1,3}):(\d{2})(?:[.:](\d{1,3}))?$/;
      OFFSET = /^offset:\s*([+-]?\d+)\s*$/i;
      WORD = /<\d{1,3}:\d{2}(?:[.:]\d{1,3})?>/g;
      utf8Len2 = (s) => new TextEncoder().encode(s).length;
    }
  });

  // src/js/pages/lyrics.js
  async function fetchLrc(key) {
    const reply = await send("lrc");
    if (app.link !== "on" && !reply.length) {
      if (key === lrcKey) lrcKey = "";
      return;
    }
    if (key !== lrcKey) return;
    setLyrics(parseReply(reply));
  }
  function setLyrics(res) {
    lyrLines = res ? res.lines : [];
    lyrCur = -2;
    const ul = $("lyrList");
    ul.textContent = "";
    for (const l of lyrLines) {
      const li = document.createElement("li");
      li.textContent = l.text || "\xA0";
      ul.appendChild(li);
    }
    $("lyr3").hidden = !lyrLines.length;
    if (!lyrLines.length) setT2($("lyrNote"), "lyr.none");
    else if (res.truncated) setT2($("lyrNote"), "lyr.truncated");
    else setRaw2($("lyrNote"), "");
    tickLyrics();
  }
  function lyrScroll() {
    const ul = $("lyrList"), li = ul.children[lyrCur];
    if (li && $("p-lyrics").getClientRects().length) ul.scrollTop = li.offsetTop - ul.clientHeight / 2 + li.clientHeight / 2;
  }
  function tickLyrics() {
    if (!lyrLines.length) return;
    let pos = posRef.pos + (posRef.playing ? performance.now() - posRef.at : 0);
    if (app.state && app.state.len > 0) pos = Math.min(pos, app.state.len);
    const i = findLine(lyrLines, pos);
    if (i === lyrCur) return;
    const ul = $("lyrList");
    if (ul.children[lyrCur]) ul.children[lyrCur].classList.remove("cur");
    lyrCur = i;
    if (ul.children[i]) ul.children[i].classList.add("cur");
    const t6 = (k2) => lyrLines[k2] ? lyrLines[k2].text : "";
    const d4 = $("lyr3").children;
    d4[0].textContent = t6(i - 1);
    d4[1].textContent = t6(i);
    d4[2].textContent = t6(i + 1);
    lyrScroll();
  }
  function initLyrics() {
    setInterval(tickLyrics, 200);
    on("status", (state) => {
      posRef = { pos: state.pos || 0, at: performance.now(), playing: state.st === "playing" };
      const key = state.len > 0 && state.st !== "idle" ? (state.file || "") + "#" + state.trk : "";
      if (key !== lrcKey) {
        lrcKey = key;
        setLyrics(null);
        if (key) fetchLrc(key);
      }
    });
    on("endlink", () => {
      lrcKey = "";
      setLyrics(null);
    });
  }
  var lrcKey, posRef, lyrLines, lyrCur;
  var init_lyrics = __esm({
    "src/js/pages/lyrics.js"() {
      init_app();
      init_link();
      init_lrc();
      lrcKey = "";
      posRef = { pos: 0, at: 0, playing: false };
      lyrLines = [];
      lyrCur = -2;
    }
  });

  // src/js/pages/diag.js
  function pushHist(st2) {
    const add = (a, v) => {
      a.push(v);
      if (a.length > HIST_N) a.shift();
    };
    if (Array.isArray(st2.cpu) && st2.cpu[0] >= 0 && st2.cpu[1] >= 0) {
      add(HIST.c0, st2.cpu[0]);
      add(HIST.c1, st2.cpu[1]);
    }
    if (typeof st2.mem === "number") add(HIST.mem, st2.mem);
  }
  function drawSparks() {
    if (app.route !== "settings/diag") return;
    const rows = [
      ["spc0", HIST.c0, 0, 100, (v) => Math.round(v) + "%"],
      ["spc1", HIST.c1, 0, 100, (v) => Math.round(v) + "%"],
      ["spm", HIST.mem, null, null, (v) => Math.round(v / 1024) + " KB"]
    ];
    for (const [id, arr, lo, hi, fmt] of rows) {
      const row2 = $(id);
      row2.hidden = arr.length === 0;
      if (!arr.length) continue;
      row2.lastElementChild.textContent = fmt(arr[arr.length - 1]);
      EQ.drawSpark(row2.querySelector("canvas"), arr, lo, hi);
    }
  }
  function renderSys() {
    const state = app.state;
    if (!state) return;
    const cpu = Array.isArray(state.cpu) && state.cpu[0] >= 0 && state.cpu[1] >= 0 ? " \xB7 " + t2("sys.cpu", { a: Math.round(state.cpu[0]), b: Math.round(state.cpu[1]) }) : "";
    setRaw2($("sysv"), t2("sys.line", { wifi: t2(state.wifi ? "sys.wifiOn" : "sys.wifiOff"), kb: Math.round(state.mem / 1024), n: state.und }) + cpu);
  }
  function initDiag() {
    $("buildStamp").textContent = BUILD;
    on("status", (state) => {
      pushHist(state);
      renderSys();
      drawSparks();
    });
    on("lang", renderSys);
    window.addEventListener("resize", drawSparks);
    if (window.matchMedia) window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", drawSparks);
  }
  var BUILD, HIST, HIST_N;
  var init_diag = __esm({
    "src/js/pages/diag.js"() {
      init_app();
      init_eq();
      BUILD = true ? "be9d5d02" : "dev";
      HIST = { c0: [], c1: [], mem: [] };
      HIST_N = 120;
    }
  });

  // src/js/pages/set-output.js
  async function readStatus() {
    if (busy) return;
    busy = true;
    try {
      const lines = await send("ratetest status");
      if (!lines.length && app.link !== "on") return;
      const r = parseRatetest(lines);
      if (r) {
        rt = r;
        if (msg && msg.key !== "rt.unsupported") msg = null;
      } else if (lines.length) msg = { key: "rt.unsupported" };
      renderRt();
    } finally {
      busy = false;
    }
    schedule();
  }
  function schedule() {
    clearTimeout(timer);
    timer = 0;
    if (rt && rt.state === "running" && here()) timer = setTimeout(() => {
      timer = 0;
      if (rt && rt.state === "running" && here()) readStatus();
    }, 1e3);
  }
  function refusedMsg(why) {
    const k2 = "rt.refused." + String(why).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
    return t2(k2) !== k2 ? { key: k2 } : { key: "rt.refused.other", p: { why } };
  }
  function renderRt() {
    const running = !!rt && rt.state === "running", on_ = app.link === "on";
    $("rtStart").disabled = !on_ || running;
    $("rtCancel").disabled = !on_ || !running;
    const st2 = $("rtState"), bar = $("rtBar"), tbl = $("rtTable"), mx = $("rtMax");
    const rates = rt ? rt.rates : [];
    if (msg) setT2(st2, msg.key, msg.p);
    else if (!rt) setT2(st2, "rt.state.none");
    else if (running) {
      const done = rates.filter((r) => r.verdict !== "untested" || r.set !== "na").length;
      if (rt.now) setT2(st2, "rt.state.running", { rate: khz(rt.now), done: Math.min(done, rates.length), n: rates.length });
      else setT2(st2, "rt.state.running0");
    } else if (rt.state === "failed") {
      if (rt.reason) setT2(st2, "rt.state.failedWhy", { why: rt.reason });
      else setT2(st2, "rt.state.failed");
    } else if (rt.state === "done" || rt.state === "cancelled") setT2(st2, "rt.state." + rt.state);
    else setT2(st2, rates.length ? "rt.state.idle" : "rt.state.none");
    st2.dataset.state = msg ? "refused" : rt ? rt.state : "none";
    bar.hidden = !running || !rates.length;
    if (!bar.hidden) bar.firstElementChild.style.width = Math.round(100 * rates.filter((r) => r.set !== "na" || r.verdict !== "untested").length / rates.length) + "%";
    tbl.hidden = !rates.length;
    const body = $("rtBody");
    body.textContent = "";
    for (const r of rates) {
      const tr = document.createElement("tr"), c1 = document.createElement("td"), c2 = document.createElement("td"), c3 = document.createElement("td");
      c1.textContent = khz(r.rate) + " kHz";
      const tag = document.createElement("span");
      tag.className = "rtag " + r.verdict;
      setT2(tag, "rt.v." + r.verdict);
      c2.appendChild(tag);
      if (r.verdict === "bad") {
        const w = document.createElement("div");
        w.className = "mut";
        if (r.set === "fail") setT2(w, "rt.why.setfail");
        else setT2(w, "rt.why.xfer", { n: r.xferBad });
        c2.appendChild(w);
      }
      c3.textContent = r.fps ? r.fps + " Hz" : "\u2014";
      tr.append(c1, c2, c3);
      body.appendChild(tr);
    }
    const fin = rt && !running && (rt.state === "done" || rt.max);
    mx.hidden = !fin;
    if (fin) {
      if (rt.max) setT2(mx, "rt.max", { r: khz(rt.max) });
      else setT2(mx, "rt.max.none");
    }
  }
  async function start() {
    msg = null;
    const r = parseRatetestReply(await send("ratetest"));
    if (r && r.refused) {
      msg = refusedMsg(r.refused);
      renderRt();
      return;
    }
    if (!r) {
      msg = { key: "rt.unsupported" };
      renderRt();
      return;
    }
    rt = rt && rt.state === "running" ? rt : { state: "running", dac: "", now: 0, reason: "", rates: [], max: null };
    renderRt();
    refreshStatus();
    readStatus();
  }
  async function cancel() {
    const r = parseRatetestReply(await send("ratetest cancel"));
    msg = r && r.refused ? refusedMsg(r.refused) : null;
    await readStatus();
    refreshStatus();
  }
  function loadRatetest() {
    msg = null;
    return readStatus();
  }
  function initOutput() {
    $("vmax").oninput = () => {
      const v = +$("vmax").value;
      app.vmaxUntil = Date.now() + 3e3;
      $("vmaxv").textContent = v;
      $("vol").max = v;
      if (+$("vol").value > v) {
        $("vol").value = v;
        $("volv").textContent = v;
      }
    };
    $("vmax").onchange = async () => {
      app.vmaxUntil = Date.now() + 3e3;
      await send("vollimit " + $("vmax").value);
      app.vmaxUntil = 0;
      refreshStatus();
    };
    segBind("dacmax", async (v) => {
      dacUntil = Date.now() + 3e3;
      await send("dacmax " + (v === "0" ? "auto" : v));
      dacUntil = 0;
      refreshStatus();
    });
    $("rtStart").onclick = start;
    $("rtCancel").onclick = cancel;
    on("status", (state) => {
      if (typeof state.dacmax === "number" && Date.now() > dacUntil) segSet("dacmax", state.dacmax);
    });
    on("route", () => {
      if (!here()) {
        clearTimeout(timer);
        timer = 0;
      }
    });
    on("linkoff", () => {
      clearTimeout(timer);
      timer = 0;
      rt = null;
      msg = null;
      renderRt();
    });
    on("lang", renderRt);
    renderRt();
  }
  var here, rt, msg, timer, busy, dacUntil;
  var init_set_output = __esm({
    "src/js/pages/set-output.js"() {
      init_app();
      init_link();
      init_status();
      init_fw();
      here = () => app.route === "settings/output" && app.link === "on";
      rt = null;
      msg = null;
      timer = 0;
      busy = false;
      dacUntil = 0;
    }
  });

  // src/js/pages/fw.js
  async function loadFw() {
    const v = await send("ver");
    if (app.link !== "on" && !v.length) return;
    const h = await send("hw");
    if (app.link !== "on" && !h.length) return;
    ver = parseVer(v);
    hw = parseHw(h);
    loaded = true;
    render();
    emit("fw");
  }
  function line(parts, cls) {
    const d4 = document.createElement("div");
    if (cls) d4.className = cls;
    parts.forEach((p, i) => {
      if (i) d4.append(" \xB7 ");
      const s = document.createElement("span");
      if (p.key) setT2(s, p.key, p.p);
      else s.textContent = p.raw;
      d4.appendChild(s);
    });
    return d4;
  }
  function row(label, lines) {
    const d4 = document.createElement("div");
    d4.className = "kv";
    const k2 = document.createElement("span");
    k2.className = "mut";
    setT2(k2, label);
    const v = document.createElement("div");
    v.className = "kvv";
    v.append(...lines);
    d4.append(k2, v);
    return d4;
  }
  function sdRow(sd) {
    const spi = sd.mode === "spi", w = sd.width === void 0 ? 4 : sd.width;
    const pins = spi ? [["CLK", sd.clk], ["MOSI", sd.mosi], ["MISO", sd.miso], ["CS", sd.cs]] : [["CLK", sd.clk], ["CMD", sd.cmd], ...[0, 1, 2, 3].slice(0, w === 1 ? 1 : 4).map((i) => ["D" + i, sd["d" + i]])];
    return row("card.title", [
      line([spi ? { key: "hw.mode.spi" } : { key: "hw.mode.sdmmc", p: { width: w } }]),
      line([{ raw: pins.filter((p) => p[1] !== void 0).map((p) => p[0] + " " + p[1]).join(" \xB7 ") }], "pins"),
      line([word("hw.src.", sd.src, "default|custom"), word("hw.sdstate.", sd.state, "mounted|none")], "meta")
    ]);
  }
  function ledRow(l) {
    return row("hw.led", l.on === 0 ? [line([{ key: "hw.led.disabled" }])] : [line([{ key: "hw.led.line", p: { gpio: l.gpio, order: String(l.order || "").toUpperCase() } }]), line([word("hw.src.", l.src, "default|custom")], "meta")]);
  }
  function render() {
    const box = $("hwBox");
    box.textContent = "";
    $("hwPending").hidden = !(hw && hw.pending > 0);
    if (hw && (hw.sd || hw.led)) {
      if (hw.sd) box.appendChild(sdRow(hw.sd));
      if (hw.led) box.appendChild(ledRow(hw.led));
    } else {
      const d4 = document.createElement("div");
      d4.className = "mut";
      if (loaded) setT2(d4, "hw.none");
      else d4.textContent = "\u2014";
      box.appendChild(d4);
    }
    if (ver) {
      setRaw2($("fwVer"), ver.version);
      if (ver.idf || ver.chip || ver.built) setT2($("fwDetail"), "diag.fw.detail", { idf: ver.idf || "?", chip: ver.chip || "?", built: ver.built || "?" });
      else setRaw2($("fwDetail"), "");
    } else {
      if (loaded) setT2($("fwVer"), "diag.fw.none");
      else setRaw2($("fwVer"), "\u2014");
      setRaw2($("fwDetail"), "");
    }
    const off = ledDisabled() === true;
    $("ledOff").hidden = !off;
    $("ledCard").hidden = off;
  }
  function initFw() {
    on("linkoff", () => {
      ver = null;
      hw = null;
      loaded = false;
      render();
      emit("fw");
    });
    on("linkui", renderFlashHint);
    on("linkoff", renderFlashHint);
    on("route", renderFlashHint);
    on("lang", render);
    render();
    renderFlashHint();
  }
  var ver, hw, loaded, ledDisabled, word, renderFlashHint;
  var init_fw2 = __esm({
    "src/js/pages/fw.js"() {
      init_app();
      init_link();
      init_fw();
      ver = null;
      hw = null;
      loaded = false;
      ledDisabled = () => hw && hw.led ? hw.led.on === 0 : null;
      word = (prefix, v, fallbackOnly) => new RegExp("^(" + fallbackOnly + ")$").test(v) ? { key: prefix + v } : { raw: v || "" };
      renderFlashHint = () => {
        $("flashHint").hidden = !(app.link === "off" && (app.route === "now" || app.route === "settings"));
      };
    }
  });

  // src/js/core/session.js
  function loadOnce() {
    app.ui.time.sync();
    app.ui.wifi.load();
    loadOut();
    loadNight();
    loadStations();
    loadScenes();
    loadFx();
    loadKeys();
    app.ui.card.load();
    loadFw();
  }
  async function afterConnectLoads() {
    await ls(app.cwd);
    loadOnce();
  }
  function initSession() {
    on("route", (key, was) => {
      if (key === was) return;
      const f = ENTER[key];
      if (!f) return;
      if (!app.chCmd && !DRAW_ONLY.has(key)) {
        if (key === "settings/fx") eqDraw();
        return;
      }
      f();
    });
  }
  var ENTER, DRAW_ONLY;
  var init_session = __esm({
    "src/js/core/session.js"() {
      init_app();
      init_music();
      init_now();
      init_night();
      init_radio2();
      init_ambient();
      init_fx();
      init_keys();
      init_lyrics();
      init_diag();
      init_set_output();
      init_fw2();
      ENTER = {
        "now/lyrics": () => {
          lyrScroll();
        },
        "radio": () => loadStations(),
        "ambient": () => loadScenes(),
        "settings/night": () => loadNight(),
        "settings/output": () => loadRatetest(),
        "settings/fx": () => {
          eqDraw();
          return loadFx();
        },
        "settings/diag": () => {
          drawSparks();
        },
        "settings/keys": () => loadKeys(),
        "settings/card": () => app.ui.card.load(),
        "settings/wifi": () => app.ui.wifi.load(),
        "settings/time": () => app.ui.time.load()
      };
      DRAW_ONLY = /* @__PURE__ */ new Set(["now/lyrics", "settings/diag"]);
    }
  });

  // src/js/core/link.js
  function settleWaiters(ok) {
    const w = linkWaiters;
    linkWaiters = [];
    for (const r of w) r(ok);
  }
  function onEvt(ev) {
    evtBuf += dec.decode(ev.target.value);
    let i;
    while ((i = evtBuf.indexOf("\n")) >= 0) {
      const line2 = evtBuf.slice(0, i);
      evtBuf = evtBuf.slice(i + 1);
      log("< " + line2);
      if (pending) {
        pending.lines.push(line2);
        if (pending.until) {
          if (pending.until.test(line2)) finish();
        } else {
          clearTimeout(pending.t);
          pending.t = setTimeout(finish, 250);
        }
      }
    }
  }
  function onSpec(e) {
    const v = e.target.value;
    if (v.byteLength >= 34) for (let i = 0; i < 32; i++) app.target[i] = v.getUint8(2 + i);
  }
  function finish() {
    const p = pending;
    pending = null;
    if (p) {
      clearTimeout(p.t);
      p.resolve(p.lines);
    }
    pump();
  }
  function pump() {
    if (pending || !queue.length || !app.chCmd) return;
    const p = pending = queue.shift();
    p.lines = [];
    log("> " + logCmd(p.cmd));
    app.chCmd.writeValueWithResponse(enc.encode(p.cmd)).then(() => {
      if (pending !== p) return;
      if (p.fast) finish();
      else p.t = setTimeout(finish, p.until ? p.ms : 1500);
    }).catch((e) => {
      log("write failed: " + e);
      if (pending === p) finish();
    });
  }
  function enqueue(cmd, fast, until, ms) {
    return new Promise((resolve) => {
      if (app.link === "off" && !app.chCmd) {
        resolve([]);
        return;
      }
      queue.push({ cmd, resolve, fast, until, ms });
      pump();
    });
  }
  function dropCommands() {
    const p = pending;
    pending = null;
    if (p) {
      clearTimeout(p.t);
      p.resolve(p.lines || []);
    }
    const q = queue;
    queue = [];
    for (const c of q) c.resolve([]);
  }
  function log(s) {
    const p = $("log");
    p.textContent = (p.textContent + s + "\n").split("\n").slice(-80).join("\n");
    p.scrollTop = p.scrollHeight;
  }
  async function realGattSetup() {
    const srv = await app.dev.gatt.connect();
    const svc = await srv.getPrimaryService(SVC);
    const cmd = await svc.getCharacteristic(CMD);
    app.chEvt = await svc.getCharacteristic(EVT);
    app.chSpec = await svc.getCharacteristic(SPEC);
    app.chEvt.removeEventListener("characteristicvaluechanged", onEvt);
    app.chEvt.addEventListener("characteristicvaluechanged", onEvt);
    await app.chEvt.startNotifications();
    app.chSpec.removeEventListener("characteristicvaluechanged", onSpec);
    app.chSpec.addEventListener("characteristicvaluechanged", onSpec);
    await app.chSpec.startNotifications();
    evtBuf = "";
    app.chCmd = cmd;
  }
  async function connect() {
    if (!DEMO && !navigator.bluetooth) {
      $("warn").style.display = "block";
      return;
    }
    try {
      if (!DEMO) app.dev = await navigator.bluetooth.requestDevice({ filters: [{ services: [SVC] }], optionalServices: [SVC] });
      app.dev.addEventListener("gattserverdisconnected", onDisconnected);
      app.userDisc = false;
      await gattSetup();
      await afterConnect();
    } catch (e) {
      log("connect: " + e);
      app.userDisc = true;
      app.chCmd = null;
      try {
        app.dev && app.dev.gatt.disconnect();
      } catch {
      }
      setConn(false);
    }
  }
  async function afterConnect() {
    setConn(true);
    settleWaiters(true);
    pump();
    clearInterval(pollTimer);
    pollTimer = setInterval(refreshStatus, 1e3);
    await refreshStatus();
    await afterConnectLoads();
  }
  function onDisconnected() {
    clearInterval(pollTimer);
    app.chCmd = null;
    evtBuf = "";
    dropCommands();
    if (app.link === "reconnecting") return;
    if (app.userDisc || app.link === "off") {
      endLink();
      return;
    }
    app.link = "reconnecting";
    app.target.fill(0);
    setLinkUI();
    reconnectLoop();
  }
  function endLink() {
    rcTok++;
    app.link = "off";
    app.castStarting = false;
    emit("endlink");
    settleWaiters(false);
    dropCommands();
    setConn(false);
  }
  async function reconnectLoop() {
    const tok = ++rcTok, t0 = Date.now();
    for (let n = 0; ; n++) {
      const d4 = n < RC_DELAYS.length ? RC_DELAYS[n] : RC_EVERY;
      if (d4) await sleep(d4);
      if (tok !== rcTok || app.link !== "reconnecting") return;
      if (Date.now() - t0 > RC_MAX) {
        log("reconnect: giving up");
        app.userDisc = true;
        try {
          app.dev.gatt.disconnect();
        } catch {
        }
        endLink();
        return;
      }
      try {
        await gattSetup();
      } catch (e) {
        app.chCmd = null;
        log("reconnect " + (n + 1) + ": " + e);
        continue;
      }
      if (tok !== rcTok || app.link !== "reconnecting") return;
      log("reconnected");
      await afterConnect();
      return;
    }
  }
  function setConn(on2) {
    app.link = on2 ? "on" : "off";
    setLinkUI();
  }
  function setLinkUI() {
    const on2 = app.link === "on", rc = app.link === "reconnecting";
    app.connected = on2;
    $("dConn").className = "dot" + (on2 ? " on" : rc ? " warn" : "");
    setT2($("connTxt"), app.castStarting && app.link !== "off" ? "conn.castStarting" : on2 ? "conn.on" : rc ? "conn.reconnecting" : "conn.off");
    setT2($("btnConnect"), on2 ? "btn.disconnect" : rc ? "btn.cancel" : "btn.connect");
    for (const el4 of document.querySelectorAll(".ctl")) el4.disabled = !on2;
    if (app.link === "off") {
      app.target.fill(0);
      app.state = null;
      emit("linkoff");
    }
    emit("linkui");
  }
  function initLink() {
    $("btnConnect").onclick = () => {
      if (app.link === "reconnecting") {
        app.userDisc = true;
        try {
          app.dev.gatt.disconnect();
        } catch {
        }
        endLink();
        return;
      }
      if (app.link === "on" && app.dev && app.dev.gatt.connected) {
        app.userDisc = true;
        app.dev.gatt.disconnect();
      } else connect();
    };
  }
  var UUID, SVC, CMD, EVT, SPEC, dec, enc, pollTimer, rcTok, RC_DELAYS, RC_EVERY, RC_MAX, linkWaiters, waitLink, pending, queue, evtBuf, send, sendFast, sendUntil, LOG_REDACT, logCmd, gattSetup, setGattSetup;
  var init_link = __esm({
    "src/js/core/link.js"() {
      init_app();
      init_wifi();
      init_status();
      init_session();
      UUID = (n) => `7f2a000${n}-a1c4-4e53-9a31-6e73616e6f00`;
      SVC = UUID(1);
      CMD = UUID(2);
      EVT = UUID(3);
      SPEC = UUID(4);
      dec = new TextDecoder();
      enc = new TextEncoder();
      pollTimer = null;
      rcTok = 0;
      RC_DELAYS = [0, 500, 1e3, 2e3, 4e3];
      RC_EVERY = 5e3;
      RC_MAX = 6e4;
      linkWaiters = [];
      waitLink = () => app.link === "on" ? Promise.resolve(true) : app.link === "off" ? Promise.resolve(false) : new Promise((r) => linkWaiters.push(r));
      pending = null;
      queue = [];
      evtBuf = "";
      send = (cmd) => enqueue(cmd, false);
      sendFast = (cmd) => enqueue(cmd, true);
      sendUntil = (cmd, re, ms) => enqueue(cmd, false, re, ms || 1e4);
      LOG_REDACT = [redact];
      logCmd = (c) => LOG_REDACT.reduce((s, f) => f(s), c);
      gattSetup = realGattSetup;
      setGattSetup = (fn) => {
        gattSetup = fn;
      };
    }
  });

  // src/js/demo/box.js
  var box_exports = {};
  __export(box_exports, {
    startDemo: () => startDemo
  });
  function startDemo() {
    const fake = {
      st: "playing",
      fmt: "FLAC",
      rate: 48e3,
      pos: 79e3,
      len: 241e3,
      vol: 42,
      trk: 3,
      n: 12,
      sub: 1,
      subn: 1,
      sleep: 1500,
      mode: 0,
      und: 0,
      out: "dac",
      dac: 1,
      bt: 0,
      wifi: 1,
      mem: 101e3,
      title: "",
      rstate: "",
      file: "03 - Night Drive.flac",
      bm: 0,
      cpu: [-1, -1],
      artist: "Demo Artist",
      album: "Harbour Lights",
      rg: 0,
      gapless: 0,
      vmax: 100,
      dacmax: 0,
      fxshed: 0
    };
    fake.fxshed = new URLSearchParams(location.search).get("shed") === "1" ? 1 : 0;
    fake.title = "Night Drive";
    if (new URLSearchParams(location.search).get("tags") === "long") {
      fake.title = "Variations on a Theme by Somebody Else, Op. 123 No. 4 (Live at the Grand Auditorium, Remastered 2024 Deluxe Edition)";
      fake.artist = "The Extraordinarily Long-Named Philharmonic Orchestra & Chamber Choir featuring Supercalifragilisticexpialidocious_Soloist";
      fake.album = "Complete_Works_Volume_07_1999-2024_Anniversary_Collection";
    }
    let tick = 0;
    const q = new URLSearchParams(location.search);
    let fakeOutSel = "auto", castOn = false, castLoaded = false, castFirst = true, castFrames = 0, castTry = q.get("cast") === "fail" ? 1 : 0;
    const castBad = q.get("cast") === "bad" ? 1 : q.get("cast") === "half" ? 2 : 0;
    const fakeVols = { dac: 42, bt: 30, cast: 55 };
    const volKey = () => ["dac", "bt", "cast"].includes(fake.out) ? fake.out : fakeOutSel === "bt" ? "bt" : "dac";
    const fakeOut = () => {
      fake.out = fakeOutSel === "cast" ? "cast" : fakeOutSel === "bt" ? "none" : "dac";
      fake.dac = fake.out === "dac" ? 1 : 0;
      fake.bt = 0;
      fake.vol = Math.min(fakeVols[volKey()], fake.vmax);
    };
    fakeOut();
    const dl = [];
    const fakeDev = {
      addEventListener: (ev, fn) => {
        if (ev === "gattserverdisconnected" && !dl.includes(fn)) dl.push(fn);
      },
      gatt: {
        connected: true,
        connect: () => new Promise((res) => setTimeout(() => {
          fakeDev.gatt.connected = true;
          res({});
        }, 600)),
        disconnect: () => {
          if (!fakeDev.gatt.connected) return;
          fakeDev.gatt.connected = false;
          setTimeout(() => dl.forEach((f) => f({ target: fakeDev })), 0);
        }
      }
    };
    function fakeDrop() {
      if (!fakeDev.gatt.connected) return;
      fakeDev.gatt.connected = false;
      dl.forEach((f) => f({ target: fakeDev }));
    }
    const dropAt = +q.get("drop");
    if (dropAt > 0) setTimeout(fakeDrop, dropAt * 1e3);
    const DEMO_LRC = "[ti:Night Drive]\r\n[ar:Demo]\r\n[00:00.00]\r\n[00:12.50]\u591C\u8272\u6162\u6162\u843D\u5728\u57CE\u5E02\u7684\u908A\u7DE3\r\n[00:21.00]\u8DEF\u71C8\u4E00\u76DE\u4E00\u76DE\u4EAE\u8D77\u4F86\r\n[00:30.40]\u6536\u97F3\u6A5F\u8F15\u8F15\u5531\u8457\u8001\u6B4C\r\n[00:41.00]<00:41.00>\u6211<00:41.50>\u5011<00:42.00>\u4E0D\u8D95\u6642\u9593\r\n[00:52.00]\u6D77\u98A8\u7A7F\u904E\u534A\u958B\u7684\u8ECA\u7A97\r\n[01:03.20]\u628A\u4ECA\u5929\u7684\u75B2\u5026\u90FD\u5E36\u8D70\r\n[01:14.00]\r\n[01:19.00]\u9060\u65B9\u7684\u71C8\u5854\u9583\u8457\u5FAE\u5149\r\n[01:30.00]\u50CF\u5728\u8AAA\u4E00\u5207\u90FD\u6703\u597D\u7684\r\n[01:41.50]\u518D\u958B\u4E00\u6BB5\u5C31\u5230\u5BB6\u4E86\r\n[01:52.00]\u665A\u5B89\uFF0C\u57CE\u5E02\r\n[02:05.00]";
    const stations = [{ name: "SomaFM Groove Salad", url: "http://ice1.somafm.com/groovesalad-128-mp3" }, { name: "Radio Paradise", url: "http://stream.radioparadise.com/mp3-192" }, { name: "\u53E4\u5178\u97F3\u6A02\u96FB\u53F0", url: "https://example.com/classical.aac" }];
    const FX = [
      ["loudness", "Loudness", 1, ["quieter_db", 0, 40, 20, 20]],
      ["bass", "Bass", 0, ["amount", 0, 1, 0.5, 0.5]],
      ["equalizer", "Equalizer", 1, ["preamp_db", -24, 6, 0, -3], ["count", 0, 10, 0, 3]],
      ["crossfeed", "Crossfeed", 0, ["feed_db", 0, 15, 4.5, 4.5]],
      ["compressor", "Compressor", 1, ["threshold_db", -60, 0, -24, -24], ["ratio", 1, 20, 2.5, 2.5]],
      ["limiter", "Limiter", 1, ["ceiling_db", -12, 0, -1, -1]]
    ];
    const EQB = [[1, 80, 4, 0.7], [0, 1e3, -3, 1], [2, 8e3, 2, 0.7], ...Array.from({ length: 7 }, () => [0, 1e3, 0, 1])];
    const fxLines = () => FX.flatMap(([id, name, on2, ...ps]) => [
      `APP fxp ${id} on ${on2} ${name}`,
      ...ps.map(([n, lo, hi, d4, c]) => `APP fxp ${id} ${n} ${lo} ${hi} ${d4} ${c}`),
      ...id === "equalizer" ? EQB.map((b, i) => `APP fxb equalizer ${i} ${b.join(" ")}`) : []
    ]);
    const fxCmd = (a) => {
      if (a[1] === "off") {
        for (const x of FX) x[2] = 0;
        return [];
      }
      const app2 = FX.find((x) => x[0] === a[2]);
      if (!app2) return ["fx: unknown app"];
      if (a[1] === "on") {
        app2[2] = +a[3];
        return [];
      }
      if (a[1] === "set") {
        const m = /^b(\d)_(type|fc|gain_db|q)$/.exec(a[3]);
        if (app2[0] === "equalizer" && m) {
          EQB[+m[1]][["type", "fc", "gain_db", "q"].indexOf(m[2])] = +a[4];
          return [];
        }
        const ps = app2.slice(3).find((x) => x[0] === a[3]);
        if (ps) {
          ps[4] = +a[4];
          return [];
        }
      }
      return ["fx: bad command"];
    };
    const KP = {
      1: ["play next prev none", "none none none none", "none none none none"],
      2: ["none none none none", "vol+ next none vol+", "vol- prev none vol-"],
      3: ["play next prev none", "vol+ none none vol+", "vol- none none vol-"]
    };
    const keyMap2 = { c: {}, p: {}, m: {} }, keyLoad = (n) => ["c", "p", "m"].forEach((b, i) => KP[n][i].split(" ").forEach((a, j) => {
      keyMap2[b]["123h"[j]] = a;
    }));
    keyLoad(3);
    let keyLastAt = 0, keyLastG = null;
    setInterval(() => {
      keyLastG = "cpm"[Math.floor(Math.random() * 3)] + "123h"[Math.floor(Math.random() * 4)];
      keyLastAt = Date.now();
    }, 7e3);
    const keysReply = (cmd) => {
      let m;
      if (m = /^keys preset ([123])$/.exec(cmd)) keyLoad(+m[1]);
      else if (m = /^keys set (\S+) (\S+) (\S+)$/.exec(cmd)) {
        if (!keyMap2[m[1]] || !"123h".includes(m[2]) || m[2].length !== 1 || !["none", "play", "next", "prev", "vol+", "vol-", "stop"].includes(m[3])) return ["keys set <c|p|m> <1|2|3|h> <none|play|next|prev|vol+|vol-|stop>"];
        keyMap2[m[1]][m[2]] = m[3];
      } else if (cmd !== "keys") return ["keys: unknown"];
      const line2 = (b) => `keys ${b} ` + [..."123h"].map((g2) => `${g2}=${keyMap2[b][g2]}`).join(" ");
      return [
        line2("c"),
        line2("p"),
        line2("m"),
        keyLastG ? `keys last ${keyLastG} ${keyMap2[keyLastG[0]][keyLastG[1]]} ${Math.floor((Date.now() - keyLastAt) / 1e3)} s ago` : "keys last none"
      ];
    };
    let cardSt = q.get("card") === "bad" ? "bad" : q.get("card") === "none" ? "none" : "ok", cardName = "SL16G", cardFs = "FAT32";
    const ctest = { run: null, fail: "", raw: null, file: null, basis: "", t0: 0 };
    const CT_FMTS = [["WAV 384k/32", 4.61], ["FLAC 384k/24", 2.6], ["WAV 192k/24", 1.15], ["WAV 96k/24", 0.58], ["FLAC 96k/24", 0.33], ["MP3 320k", 0.04]];
    const ctClock = () => {
      const d4 = /* @__PURE__ */ new Date(), z = (n) => String(n).padStart(2, "0");
      return `${d4.getFullYear()}-${z(d4.getMonth() + 1)}-${z(d4.getDate())}T${z(d4.getHours())}:${z(d4.getMinutes())}`;
    };
    const ctFinish = () => {
      const r = ctest.run;
      ctest.run = null;
      if (q.get("testfail")) {
        ctest.fail = r + " " + q.get("testfail");
        return;
      }
      if (r === "raw") ctest.raw = { avg16: 3.62, slow16: 3.4, max64: 4.1, worst_us: 5230, time: ctClock() };
      else ctest.file = { write: 2.91, read: 3.05, verify: 1, time: ctClock() };
      ctest.basis = ctest.file ? "file" : "raw";
    };
    const ctReply = (cmd) => {
      const a = cmd.split(" ");
      if (ctest.run && Date.now() - ctest.t0 >= 6e3) ctFinish();
      if (a[1] === "raw" || a[1] === "file") {
        if (cardSt === "none") return ["cardtest: refused no card"];
        if (cardSt === "bad") return ["cardtest: refused card unreadable"];
        if (ctest.run) return ["cardtest: refused already running"];
        if (q.get("refuse")) return ["cardtest: refused " + q.get("refuse")];
        if (fake.st !== "idle") {
          fake.bm = fake.len > 0 ? 1 : 0;
          fake.st = "idle";
        }
        ctest.run = a[1];
        ctest.t0 = Date.now();
        ctest.fail = "";
        return ["cardtest: started " + a[1]];
      }
      if (a[1] === "cancel") {
        if (!ctest.run) return ["cardtest: refused not running"];
        ctest.run = null;
        return ["cardtest: cancelled"];
      }
      const out = [ctest.run ? `cardtest state running ${ctest.run} ${Math.min(99, Math.floor((Date.now() - ctest.t0) / 60))}%` : "cardtest state idle"];
      if (ctest.fail) out.push("cardtest fail " + ctest.fail);
      const kvs = (o) => Object.entries(o).map(([k2, v]) => `${k2}=${v}`).join(" ");
      if (ctest.raw) out.push("cardtest raw " + kvs(ctest.raw));
      if (ctest.file) out.push("cardtest file " + kvs(ctest.file));
      if (ctest.basis) out.push(`cardtest basis ${ctest.basis} speed=${(ctest.basis === "file" ? ctest.file.read : ctest.raw.slow16).toFixed(2)}`);
      const marks = [["raw", ctest.raw ? ctest.raw.slow16 - 0.01 : 0], ["file", ctest.file ? ctest.file.read : 0]].sort((x, y) => y[1] - x[1]);
      const ref = ctest.basis ? ctest.basis === "file" ? ctest.file.read : ctest.raw.slow16 : 0;
      const rows = CT_FMTS.map(([n, need]) => ({ n, need, v: need }));
      for (const [n, sp] of marks) {
        const i = rows.findIndex((x) => x.need <= sp);
        rows.splice(i < 0 ? rows.length : i, 0, { mark: n, sp });
      }
      for (const x of rows) {
        if (x.mark) out.push(`cardtest mark ${x.mark} speed=${x.sp.toFixed(2)}`);
        else out.push(`cardtest fmt "${x.n}" need=${x.need.toFixed(2)} grade=${!ref ? "unknown" : ref >= x.need * 1.5 ? "ok" : ref >= x.need ? "marginal" : "slow"}`);
      }
      return out;
    };
    let cf = null, cfRun = 0, cfFail = q.get("fmtfail");
    const rnd4 = () => String(1e3 + Math.floor(Math.random() * 9e3));
    const cfRefuse = (why) => {
      cf = null;
      return ["cardfmt: refused " + why];
    };
    const cfReply = (cmd) => {
      const a = cmd.split(" ");
      if (a[1] === "status") {
        if (cfRun && Date.now() >= cfRun) {
          cfRun = 0;
          if (cfFail) return ["cardfmt state failed " + cfFail];
          cardSt = "ok";
          cardName = "NIGHTSTAND";
          cardFs = "exFAT";
          return ["cardfmt state done"];
        }
        return [cfRun ? "cardfmt state running" : cf ? "cardfmt state pending" : "cardfmt state idle"];
      }
      if (a[1] === "cancel") {
        cf = null;
        return ["cardfmt: cancelled"];
      }
      if (cardSt === "none") return cfRefuse("no card");
      if (cardSt === "ok") return cfRefuse("the card is readable; formatting is only for a card that cannot be read");
      if (cfRun) return ["cardfmt: refused already running"];
      if (!a[1]) {
        cf = { step: 1, code: rnd4(), exp: Date.now() + 3e4 };
        return ['cardfmt plan fs=exFAT label="NIGHTSTAND" size=15931539456', "cardfmt warn everything on the card is erased, including the Wi-Fi settings file", `cardfmt: confirm ${cf.code} within 30 s`];
      }
      if (!cf) return cfRefuse("no request");
      if (Date.now() > cf.exp) return cfRefuse("too late");
      if (a[1] !== cf.code) return cfRefuse("wrong code");
      if (cf.step === 1) {
        let c2;
        do
          c2 = rnd4();
        while (c2 === cf.code);
        cf = { step: 2, code: c2, exp: Date.now() + 3e4 };
        return [`cardfmt: confirm again ${c2} within 30 s`];
      }
      cf = null;
      cfRun = Date.now() + 5e3;
      return ["cardfmt: started"];
    };
    const cardReply = () => cardSt === "ok" ? [`card state=mounted size=15931539456 free=15908798464 fs=${cardFs} name="${cardName}" serial=78EA244F fmt=idle`] : cardSt === "bad" ? [`card state=unreadable size=15931539456 free=? fs=? name="" serial=78EA244F fmt=${cfRun ? "running" : "idle"}`] : ["card state=none fmt=idle"];
    const wf = q.get("wifi") === "none" ? { state: "no credentials", ssid: "", ip: "0.0.0.0", rssi: 0, src: "none", until: 0, next: null } : { state: "connected", ssid: "HomeNet-5G", ip: "192.0.2.23", rssi: -58, src: "card", until: 0, next: null };
    const wfCard = wf.src === "card" ? wf.ssid : "";
    const wfTick = () => {
      if (wf.next && Date.now() >= wf.until) {
        Object.assign(wf, wf.next);
        wf.next = null;
      }
      fake.wifi = wf.state === "connected" ? 1 : 0;
    };
    const wfEsc = (x) => x.replace(/[\\"]/g, "\\$&");
    const SCAN = [[-48, "secured", "HomeNet-5G"], [-61, "open", 'Cafe "Blue" Guest'], [-72, "secured", "Neighbor_2.4G"], [-80, "secured", "\u8A2A\u5BA2\u5C08\u7528\u7DB2\u8DEF"], [-86, "open", "Office-Guest"]];
    const tm = { src: q.get("clock") === "none" ? "none" : "ntp", skew: 0, tz: q.get("tz") || "CST-8", up0: Date.now() };
    const tmUnix = () => tm.src === "none" ? Math.floor((Date.now() - tm.up0) / 1e3) + 12 : Math.floor((Date.now() + tm.skew) / 1e3);
    const tmOff = () => {
      const m = /^[A-Za-z]{3,}([+-]?)(\d{1,2})(?::(\d\d))?/.exec(tm.tz);
      return m ? -((m[1] === "-" ? -1 : 1) * (+m[2] * 60 + +(m[3] || 0))) : 0;
    };
    const tmLocal = () => new Date((tmUnix() + tmOff() * 60) * 1e3).toISOString().slice(0, 19);
    const tmZoneOk = (z) => /^[\x21-\x7e]{1,47}$/.test(z);
    const timeReply = (cmd) => {
      let m;
      if (cmd === "time") {
        const o = tmOff(), a = Math.abs(o);
        return [`time ${tmUnix()} ${tmLocal()}${o < 0 ? "-" : "+"}${String(Math.floor(a / 60)).padStart(2, "0")}:${String(a % 60).padStart(2, "0")} src=${tm.src} tz="${tm.tz}"`];
      }
      if (m = /^time set (\d+)(?: (\S+))?$/.exec(cmd)) {
        if (m[2] && !tmZoneOk(m[2])) return ["time set: refused bad zone"];
        if (m[2]) tm.tz = m[2];
        if (tm.src === "ntp") return ["time set: zone only, clock is from ntp"];
        tm.src = "client";
        tm.skew = +m[1] * 1e3 - Date.now();
        return ["time set: ok"];
      }
      if (m = /^time tz (\S+)$/.exec(cmd)) {
        if (!tmZoneOk(m[1])) return ["time tz: refused bad zone"];
        tm.tz = m[1];
        return ["time tz: ok"];
      }
      return ["time: unknown"];
    };
    const wifiReply = (cmd) => {
      wfTick();
      let m;
      if (cmd === "wifi") return [`wifi ${wf.state} ip=${wf.ip} rssi=${wf.rssi} time=${tm.src === "none" ? "(unsynced) " : ""}${tmLocal().replace("T", " ")} src=${wf.src} ssid="${wfEsc(wf.ssid)}"`];
      if (cmd === "wifi scan") {
        const aps = SCAN.map(([r, k2, n]) => `wifi ap ${r} ${k2} "${wfEsc(n)}"`);
        return { chunks: [[2e3, aps], [2400, [`wifi scan: ${aps.length} found`]]] };
      }
      if (m = /^wifi set (.*)$/s.exec(cmd)) {
        const i = m[1].indexOf("|");
        if (i < 0) return ["wifi set: refused usage: wifi set <ssid>|<password>"];
        const ssid = m[1].slice(0, i), pw = m[1].slice(i + 1), sb = enc.encode(ssid).length, pb = enc.encode(pw).length;
        if (/[\x00-\x1f\x7f]/.test(m[1])) return ["wifi set: refused control characters"];
        if (sb < 1 || sb > 32) return ["wifi set: refused ssid must be 1-32 bytes"];
        if (pb && (pb < 8 || pb > 63)) return ["wifi set: refused password must be empty or 8-63 bytes"];
        Object.assign(wf, {
          state: "connecting",
          ssid,
          ip: "0.0.0.0",
          rssi: 0,
          src: "nvs",
          until: Date.now() + 3e3,
          next: pw === "wrongpass1" ? { state: "wrong password" } : ssid === "Nowhere" ? { state: "not found" } : { state: "connected", ip: "192.0.2.57", rssi: -52 }
        });
        return ["wifi set: ok, connecting"];
      }
      if (cmd === "wifi forget") {
        wf.next = null;
        Object.assign(wf, wfCard ? { state: "connected", ssid: wfCard, ip: "192.0.2.23", rssi: -58, src: "card" } : { state: "no credentials", ssid: "", ip: "0.0.0.0", rssi: 0, src: "none" });
        return ["wifi forget: ok"];
      }
      if (cmd === "wifi off") {
        wf.next = null;
        wf.state = "off";
        wf.ip = "0.0.0.0";
        wf.rssi = 0;
        return ["wifi: off"];
      }
      if (cmd === "wifi on") {
        if (wf.state === "off") Object.assign(wf, wf.ssid ? { state: "connected", ip: "192.0.2.23", rssi: -58 } : { state: "no credentials" });
        return ["wifi: on"];
      }
      return ["wifi: unknown"];
    };
    const hwv = (q.get("hw") || "").split(",");
    const hwReply = () => [
      hwv.includes("spi") ? `hw sd clk=18 mosi=19 miso=20 cs=21 src=custom state=${cardSt === "ok" ? "mounted" : cardSt === "none" ? "none" : "failed"} mode=spi` : `hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=${cardSt === "ok" ? "mounted" : cardSt === "none" ? "none" : "failed"} mode=sdmmc`,
      ...hwv.includes("spi") ? [] : ["hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any"],
      `hw led gpio=60 order=grb on=${hwv.includes("ledoff") ? 0 : 1} src=default`,
      "hw reserved 26,27,28,29,30,31,32,33,34,35,36,37",
      "hw gpios 0-61",
      `hw pending ${hwv.includes("pending") ? 1 : 0}`
    ];
    const RT_RATES = [44100, 48e3, 96e3, 192e3, 384e3], RT_STEP = 1600;
    const rtb = { state: "idle", t0: 0, n: 0, dac: "06cb:1594" };
    const rtTick = () => {
      if (rtb.state === "running") {
        rtb.n = Math.min(RT_RATES.length, Math.floor((Date.now() - rtb.t0) / RT_STEP));
        if (rtb.n >= RT_RATES.length) rtb.state = "done";
      }
    };
    const rtRow = (r, i) => {
      if (i >= rtb.n) return `ratetest rate=${r} set=na clock=0 xfer_ok=0 xfer_bad=0 fps=0 verdict=untested`;
      if (r === 384e3) return `ratetest rate=${r} set=fail clock=192000 xfer_ok=0 xfer_bad=0 fps=0 verdict=bad`;
      if (r === 192e3) return `ratetest rate=${r} set=ok clock=${r} xfer_ok=611 xfer_bad=139 fps=${r - 1} verdict=bad`;
      return `ratetest rate=${r} set=ok clock=${r} xfer_ok=${Math.round(r / 128)} xfer_bad=0 fps=${r - 1} verdict=ok`;
    };
    const rtReply = (cmd) => {
      rtTick();
      if (cmd === "ratetest cancel") {
        if (rtb.state !== "running") return ["ratetest: refused not running"];
        rtb.state = "cancelled";
        return ["ratetest: cancelled"];
      }
      if (cmd === "ratetest" || cmd === "ratetest start") {
        if (rtb.state !== "running") {
          if (q.get("dac") === "none") return ["ratetest: refused no dac"];
          if (ctest.run) return ["ratetest: refused card test running"];
          if (fake.st !== "idle") {
            fake.bm = fake.len > 0 ? 1 : 0;
            fake.st = "idle";
          }
          Object.assign(rtb, { state: "running", t0: Date.now(), n: 0 });
          return ["ratetest: started"];
        }
      } else if (cmd !== "ratetest status") return ["ratetest [status|cancel]"];
      const out = [`ratetest state=${rtb.state} dac=${rtb.dac}` + (rtb.state === "running" ? ` now=${RT_RATES[Math.min(rtb.n, RT_RATES.length - 1)]}` : "")];
      if (rtb.state !== "idle") {
        RT_RATES.forEach((r, i) => out.push(rtRow(r, i)));
        if (rtb.state !== "running") out.push("ratetest max=" + (rtb.n >= RT_RATES.length ? 96e3 : 0));
      }
      return out;
    };
    const TREE = {
      "/sdcard/packs": ["nes.zip/", "Loose Folder/", "08 - Bonus Track.flac"],
      "/sdcard/packs/nes.zip": ["01 - Title Theme.flac", "02 - Overworld.flac", "03 - Dungeon.flac", "99 - Full Soundtrack (huge).wav"]
    };
    const lsReply = (cmd) => TREE[cmd.slice(3).trim().replace(/\/+$/, "")] || ["packs/", "Albums/", "Chiptune/", "01 - Intro.flac", "02 - Harbour Lights.flac", "03 - Night Drive.flac", "04 - Slow Rain.mp3"];
    const playReply = (cmd) => {
      rtTick();
      if (rtb.state === "running") return ["play: ESP_ERR_INVALID_STATE"];
      if (/nes\.zip\/.*huge/i.test(cmd)) return ["play: ESP_ERR_INVALID_SIZE"];
      if (/missing/i.test(cmd)) return ["play: ESP_ERR_NOT_FOUND"];
      fake.st = "playing";
      fake.bm = 0;
      return ["play: ESP_OK"];
    };
    const reply = (cmd) => {
      if (cmd === "ver") return ["ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83"];
      if (cmd === "hw") return hwReply();
      if (cmd === "ratetest" || cmd.startsWith("ratetest ")) return rtReply(cmd);
      if (cmd === "dacmax" || cmd.startsWith("dacmax ")) {
        const a = cmd.slice(7);
        if (a === "auto") fake.dacmax = 0;
        else if ([48e3, 96e3, 192e3, 384e3].includes(+a)) fake.dacmax = +a;
        return [fake.dacmax ? "dacmax " + fake.dacmax : "dacmax auto"];
      }
      if (cmd === "wifi" || cmd.startsWith("wifi ")) return wifiReply(cmd);
      if (cmd === "time" || cmd.startsWith("time ")) return timeReply(cmd);
      if (cmd === "gapless" || cmd.startsWith("gapless ")) {
        if (cmd.endsWith(" on")) fake.gapless = 1;
        else if (cmd.endsWith(" off")) fake.gapless = 0;
        return ["gapless " + (fake.gapless ? "on" : "off")];
      }
      if (cmd === "vollimit" || cmd.startsWith("vollimit ")) {
        const n = Math.round(+cmd.slice(9));
        if (cmd.length > 8 && n >= 0 && n <= 100) {
          fake.vmax = n;
          for (const k2 in fakeVols) fakeVols[k2] = Math.min(fakeVols[k2], n);
          fake.vol = Math.min(fake.vol, n);
        }
        return ["vollimit " + fake.vmax];
      }
      if (cmd.startsWith("vol ")) {
        const n = Math.max(0, Math.min(fake.vmax, Math.round(+cmd.slice(4)) || 0));
        fakeVols[volKey()] = n;
        fake.vol = n;
        return ["vol " + n];
      }
      if (cmd === "st") {
        wfTick();
        if (fake.st === "playing") fake.pos = (fake.pos + 1e3) % fake.len;
        const n = tick++;
        fake.cpu = n === 0 ? [-1, -1] : [Math.round(10 + 6 * Math.sin(n / 5) + 4 * Math.random()), Math.round(18 + 10 * Math.sin(n / 3.3 + 1) + 5 * Math.random())];
        fake.mem = 101e3 - n * 700 % 9e3 + Math.round(300 * Math.random());
        if (ctest.run && Date.now() - ctest.t0 >= 6e3) ctFinish();
        rtTick();
        fake.card = cardSt;
        return [JSON.stringify(fake)];
      }
      if (cmd === "card") return cardReply();
      if (cmd === "cardtest" || cmd.startsWith("cardtest ")) return ctReply(cmd);
      if (cmd === "cardfmt" || cmd.startsWith("cardfmt ")) return cfReply(cmd);
      if (cmd.startsWith("rg ")) {
        const m = cmd.slice(3);
        fake.rg = Math.max(0, ["off", "track", "album"].indexOf(m));
        return ["rg: " + m];
      }
      if (cmd === "pause") {
        if (fake.st === "playing") fake.st = "paused";
        return [];
      }
      if (cmd === "stop") {
        if (fake.st !== "idle") {
          fake.bm = fake.len > 0 ? 1 : 0;
          fake.st = "idle";
        }
        return [];
      }
      if (cmd === "resume") {
        if (fake.st === "paused") {
          fake.st = "playing";
          return [];
        }
        if (fake.st === "idle" && fake.bm) {
          fake.st = "playing";
          fake.bm = 0;
          return ["resumed /sdcard/" + fake.file];
        }
        return ["nothing to resume"];
      }
      if (cmd.startsWith("play ")) return playReply(cmd);
      if (cmd === "fx params") return fxLines();
      if (cmd.startsWith("fx ")) return fxCmd(cmd.split(" "));
      if (cmd.startsWith("ls")) return lsReply(cmd);
      if (cmd === "out" || cmd.startsWith("out ")) {
        const v = cmd.slice(4);
        if (v === "cast") {
          if (castOn) return ["out cast (playing on cast)"];
          if (castTry === 1) {
            castTry++;
            return ["out cast: ESP_ERR_NO_MEM"];
          }
          castOn = true;
          castLoaded = true;
          fakeOutSel = "cast";
          fakeOut();
          fake.dac = 0;
          if (castFirst) {
            castFirst = false;
            setTimeout(() => fakeDrop(), 350);
            return ["out cast: starting, the Bluetooth control link reconnects in a moment"];
          }
          return ["out cast (playing on cast)"];
        }
        if (["auto", "dac", "bt"].includes(v)) {
          castOn = false;
          fakeOutSel = v;
          fakeOut();
        }
        return ["out " + fakeOutSel + " (playing on " + fake.out + ")"];
      }
      if (cmd === "cast") return [castOn ? `cast: on "NightStand Nano" frames=${castFrames += 100} underflows=${castBad ? 4 : 0} lc3_us_per_ch=2310 send_err=${castBad ? 2 : 0} gap_max_us=1840 streams=${castBad === 2 ? "10" : "11"}` : "cast: off" + (castLoaded ? " (LE Audio stays loaded until reboot)" : "")];
      if (cmd === "keys" || cmd.startsWith("keys ")) return keysReply(cmd);
      if (cmd === "night") return ["night: off, schedule 22:30-06:30"];
      if (cmd === "radio list") return stations.length ? stations.map((x, i) => `${i + 1} ${x.name} ${x.url}`) : ["radio: no stations"];
      if (cmd.startsWith("radio add ")) {
        const [name, url] = cmd.slice(10).split("|");
        if (!/^https?:\/\//.test(url || "")) return ["radio add: refused"];
        let i = stations.findIndex((x) => x.url === url);
        if (i < 0) {
          if (stations.length >= 40) return ["radio add: list full"];
          stations.push({ name, url });
          i = stations.length - 1;
        }
        return ["radio add: " + (i + 1)];
      }
      if (cmd.startsWith("radio del ")) {
        const n = +cmd.slice(10);
        if (n >= 1 && n <= stations.length) {
          stations.splice(n - 1, 1);
          return ["radio del: ok"];
        }
        return ["radio del: failed"];
      }
      if (cmd === "lrc") {
        const b = enc.encode(DEMO_LRC), out = ["lrc begin " + b.length];
        for (let i = 0; i < b.length; i += 90) out.push("lrc+ " + btoa(String.fromCharCode(...b.subarray(i, i + 90))));
        return out.concat("lrc end");
      }
      if (/^radio \d+$/.test(cmd) || /^radio https?:/.test(cmd)) {
        fake.rstate = "buffering";
        return ["ok"];
      }
      if (cmd === "scene") return ["1 Cooling Summer Nap", "2 Soothing Colored Noise", "3 Campfire Rain"];
      return ["ok"];
    };
    const fakeCh = { writeValueWithResponse: (b) => {
      if (!fakeDev.gatt.connected) return Promise.reject(new Error("GATT Server is disconnected"));
      const c = dec.decode(b), res = reply(c);
      const emit2 = (lines, ms) => setTimeout(() => {
        if (fakeDev.gatt.connected) onEvt({ target: { value: enc.encode(lines.join("\n") + "\n") } });
      }, ms);
      if (Array.isArray(res)) {
        if (res.length) emit2(res, 5);
      } else for (const [ms, lines] of res.chunks) emit2(lines, ms);
      return Promise.resolve();
    } };
    app.dev = fakeDev;
    setGattSetup(async () => {
      if (!app.dev.gatt.connected) await app.dev.gatt.connect();
      app.chCmd = fakeCh;
    });
    setInterval(() => {
      const t6 = performance.now() / 1e3, beat = Math.pow(0.5 + 0.5 * Math.sin(t6 * 7.5), 3);
      for (let i = 0; i < 32; i++) {
        const tilt = 150 * Math.exp(-i / 14), bump = 70 * Math.exp(-Math.pow((i - (16 + 12 * Math.sin(t6 * 0.7))) / 3, 2));
        const low = i < 8 ? 60 * beat : 0, spark = i > 20 ? 25 * Math.random() : 15 * Math.random();
        app.target[i] = Math.max(0, Math.min(255, Math.round(55 + tilt * (0.7 + 0.3 * Math.sin(t6 * 3 + i)) + bump + low + spark)));
      }
    }, 33);
    connect();
  }
  var init_box = __esm({
    "src/js/demo/box.js"() {
      init_app();
      init_link();
    }
  });

  // src/js/main.js
  init_i18n();
  init_app();
  init_icons();
  init_link();
  init_now();
  init_lyrics();
  init_diag();

  // src/js/pages/device.js
  init_app();
  init_link();

  // src/js/lib/card.js
  init_i18n();
  var I = I18N;
  var t5 = I.t;
  var setT3 = I.setT;
  var setRaw3 = I.setRaw;
  var TTL = 30;
  var el3 = (tag, cls, text) => {
    const e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text !== void 0) e.textContent = text;
    return e;
  };
  var elT = (tag, cls, key, params) => {
    const e = el3(tag, cls);
    setT3(e, key, params);
    return e;
  };
  function kv2(s) {
    const o = {}, re = /(\w+)=(?:"([^"]*)"|(\S+))/g;
    let m;
    while (m = re.exec(s)) o[m[1]] = m[2] !== void 0 ? m[2] : m[3];
    return o;
  }
  var num2 = (v) => typeof v === "string" && /^\d+(\.\d+)?$/.test(v) ? +v : null;
  function parseCard(lines) {
    const l = lines.find((x) => /^card state=/.test(x));
    if (!l) return null;
    const o = kv2(l);
    return { state: o.state, size: num2(o.size), free: num2(o.free), fs: o.fs && o.fs !== "?" ? o.fs : "", name: o.name || "", fmt: o.fmt || "idle" };
  }
  function parseTest(lines) {
    const r = { seen: false, running: false, kind: "", pct: 0, fail: null, raw: null, file: null, basis: null, rows: [] };
    for (const l of lines) {
      let m;
      if (m = /^cardtest state (\w+)(?: (raw|file) (\d+)%)?/.exec(l)) {
        r.seen = true;
        r.running = m[1] === "running";
        if (m[2]) {
          r.kind = m[2];
          r.pct = +m[3];
        }
      } else if (m = /^cardtest fail (raw|file) (.+)$/.exec(l)) r.fail = { kind: m[1], why: m[2].trim() };
      else if (m = /^cardtest (raw|file) (\S.*)$/.exec(l)) r[m[1]] = kv2(m[2]);
      else if (m = /^cardtest basis (raw|file) speed=([\d.]+)/.exec(l)) r.basis = { kind: m[1], speed: +m[2] };
      else if (m = /^cardtest mark (raw|file) speed=([\d.]+)/.exec(l)) r.rows.push({ mark: m[1], speed: +m[2] });
      else if (m = /^cardtest fmt "([^"]*)"\s*(.*)$/.exec(l)) {
        const o = kv2(m[2]);
        r.rows.push({ name: m[1], need: o.need, grade: o.grade || "unknown" });
      }
    }
    return r;
  }
  function fmtBytes(n) {
    if (n === null) return null;
    if (n >= 1e12) return (n / 1e12).toFixed(2) + " TB";
    if (n >= 1e9) return (n / 1e9).toFixed(2) + " GB";
    if (n >= 1e6) return Math.round(n / 1e6) + " MB";
    return Math.round(n / 1e3) + " KB";
  }
  var mbs = (v) => v === void 0 || v === null || v === "" || isNaN(+v) ? "?" : (+v).toFixed(2);
  var when = (s) => s ? String(s).replace("T", " ") : "";
  var reasonKey = (r) => "card.reason." + String(r).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
  var reasonP = (r) => I.has(reasonKey(r)) ? { t: reasonKey(r) } : String(r);
  function fmtWhy(r) {
    if (/readable/i.test(r)) return { t: "card.fmt.why.readable" };
    if (/wrong|mismatch|invalid|bad code|incorrect/i.test(r)) return { t: "card.fmt.why.wrong" };
    if (/late|expire|time/i.test(r)) return { t: "card.fmt.why.late" };
    return reasonP(r);
  }
  function mount2(o) {
    const { box, banner, send: send2, getState, isOn, tabOpen } = o;
    const S = { card: null, test: null, testMsg: null, fmt: { phase: "idle", plan: null, code: "", deadline: 0, msg: null, result: null } };
    let loading = null, lastCard = null, testTimer = 0, fmtTimer = 0, cdTimer = 0, polling = false, fmtPolling = false, fmtBusy = false, fmtSig = "";
    const factsEl = el3("div", "cfacts"), testEl = el3("div"), fmtEl = el3("div");
    box.textContent = "";
    box.append(factsEl, testEl, fmtEl);
    const bn = { text: el3("span", "grow"), btn: elT("button", "", "card.warn.open") };
    banner.textContent = "";
    banner.append(bn.text, bn.btn);
    bn.btn.onclick = () => o.openTab();
    function kind() {
      const st2 = getState();
      if (st2 && st2.card) return st2.card;
      if (!st2 && !isOn()) return "";
      return S.card ? { mounted: "ok", unreadable: "bad", none: "none" }[S.card.state] || "" : "";
    }
    function renderBanner() {
      const st2 = getState(), k2 = st2 && st2.card;
      const show = !!st2 && (k2 === "none" || k2 === "bad");
      banner.hidden = !show;
      if (show) setT3(bn.text, k2 === "none" ? "card.warn.none" : "card.warn.bad");
    }
    function renderFacts() {
      const k2 = kind(), c = S.card;
      factsEl.textContent = "";
      if (!isOn() && !getState()) {
        factsEl.append(elT("div", "mut", "card.wait"));
        return;
      }
      if (k2 === "none") {
        factsEl.append(elT("div", "cstate", "card.state.none"));
        return;
      }
      if (k2 === "bad") {
        factsEl.append(elT("div", "cstate bad", "card.state.bad"));
        if (c && c.state === "unreadable" && (c.size !== null || c.name)) {
          const d4 = el3("div", "mut"), size = c.size !== null ? fmtBytes(c.size) : { t: "card.unknown" };
          if (c.name) setT3(d4, "card.facts.bad", { size, name: c.name });
          else if (c.size !== null) setRaw3(d4, size);
          factsEl.append(d4);
        }
        return;
      }
      if (k2 === "ok" && c && c.state === "mounted") {
        const d4 = el3("div", "cstate");
        setT3(d4, "card.facts.ok", {
          size: c.size !== null ? fmtBytes(c.size) : { t: "card.checking" },
          free: c.free !== null ? fmtBytes(c.free) : { t: "card.checking" },
          fs: c.fs || { t: "card.checking" },
          name: c.name || { t: "card.noname" }
        });
        factsEl.append(d4);
        return;
      }
      factsEl.append(elT("div", "mut", k2 === "ok" ? "card.checking" : "card.na"));
    }
    const KINDS = { raw: "card.test.raw", file: "card.test.file" };
    function renderTest() {
      testEl.textContent = "";
      if (kind() !== "ok") return;
      const T = S.test, running = !!(T && T.running);
      testEl.append(elT("div", "csub", "card.test.title"));
      const row2 = el3("div", "row");
      for (const kd of ["raw", "file"]) {
        const b = elT("button", "ctl", KINDS[kd]);
        b.dataset.kind = kd;
        b.disabled = !isOn() || running;
        b.onclick = () => startTest(kd);
        row2.append(b);
      }
      testEl.append(row2, elT("div", "mut cnote", "card.test.note"));
      if (running) {
        const p = el3("div", "cprogwrap");
        p.append(elT("div", "cprogtxt", T.kind === "file" ? "card.test.running.file" : "card.test.running.raw", { pct: T.pct }));
        const bar = el3("div", "cprog");
        bar.setAttribute("role", "progressbar");
        bar.setAttribute("aria-valuemin", "0");
        bar.setAttribute("aria-valuemax", "100");
        bar.setAttribute("aria-valuenow", T.pct);
        const i = el3("i");
        i.style.width = T.pct + "%";
        bar.append(i);
        const cb = elT("button", "ctl", "btn.cancel");
        cb.disabled = !isOn();
        cb.onclick = cancelTest;
        const r2 = el3("div", "row");
        bar.style.flex = "1 1 8rem";
        r2.append(bar, cb);
        p.append(r2);
        testEl.append(p);
      }
      if (S.testMsg) {
        const m = elT("div", "cmsg" + (S.testMsg.bad ? " bad" : ""), S.testMsg.key, S.testMsg.params);
        m.setAttribute("role", "status");
        testEl.append(m);
      }
      if (T && T.fail && !running) {
        const m = elT("div", "cmsg bad", "card.test.fail", { kind: { t: KINDS[T.fail.kind] }, why: reasonP(T.fail.why) });
        testEl.append(m);
      }
      if (T && !running) renderResults(T);
    }
    function group(titleKey, time2) {
      const gp = el3("div", "cgrp"), h = el3("div", "cgh");
      h.append(elT("strong", "", titleKey));
      if (time2) h.append(el3("span", "mut", when(time2)));
      gp.append(h);
      return gp;
    }
    function kvRows(gp, rows) {
      const d4 = el3("div", "ckv");
      for (const [k2, v] of rows) {
        d4.append(elT("span", "", k2));
        const s = el3("span");
        if (typeof v === "string") s.textContent = v;
        else setT3(s, v.t);
        d4.append(s);
      }
      gp.append(d4);
    }
    function renderResults(T) {
      const res = el3("div", "cres");
      if (T.raw) {
        const gp = group("card.test.raw", T.raw.time), r = T.raw;
        kvRows(gp, [
          ["card.res.read3", `${mbs(r.avg16)} / ${mbs(r.slow16)} / ${mbs(r.max64)} MB/s`],
          ["card.res.worst", num2(r.worst_us) !== null ? (num2(r.worst_us) / 1e3).toFixed(1) + " ms" : "?"]
        ]);
        res.append(gp);
      }
      if (T.file) {
        const gp = group("card.test.file", T.file.time), f = T.file;
        kvRows(gp, [["card.res.write", mbs(f.write) + " MB/s"], ["card.res.read", mbs(f.read) + " MB/s"], ["card.res.verify", { t: f.verify === "1" ? "card.res.pass" : "card.res.nopass" }]]);
        res.append(gp);
      }
      const fmts = T.rows.filter((x) => x.name !== void 0);
      if (fmts.length) {
        const gp = el3("div", "cgrp");
        gp.append(el3("div", "cgh"));
        gp.firstChild.append(elT("strong", "", "card.ladder.title"));
        if (T.basis && T.basis.speed > 0) gp.append(elT("div", "mut cnote", "card.ladder.basis", { what: { t: KINDS[T.basis.kind] } }));
        for (const r of T.rows) {
          if (r.mark) {
            if (!(r.speed > 0)) continue;
            const m = el3("div", "cmark" + (T.basis && T.basis.kind === r.mark ? " basis" : ""));
            m.append(elT("span", "", r.mark === "raw" ? "card.ladder.markRaw" : "card.ladder.markFile", { v: r.speed.toFixed(2) }));
            gp.append(m);
          } else {
            const l = el3("div", "cl");
            l.append(el3("span", "cln", r.name));
            const need = elT("span", "mut", "card.ladder.need", { v: mbs(r.need) });
            const g2 = ["ok", "marginal", "slow"].includes(r.grade) ? r.grade : "unknown";
            const b = elT("span", "gb " + g2, "card.grade." + g2);
            l.append(need, b);
            gp.append(l);
          }
        }
        gp.append(elT("div", "mut cnote help", "card.ladder.hint"));
        res.append(gp);
      }
      testEl.append(res);
    }
    async function startTest(kd) {
      S.testMsg = null;
      const r = await send2("cardtest " + kd);
      const line2 = r.find((l) => /^cardtest:/.test(l)) || "";
      let m;
      if (/^cardtest: started/.test(line2)) S.test = Object.assign(S.test || parseTest([]), { running: true, kind: kd, pct: 0, fail: null });
      else if (m = /^cardtest: refused\s*(.*)$/.exec(line2)) S.testMsg = { key: "card.test.refused", params: { why: reasonP(m[1].trim() || "?") }, bad: true };
      else S.testMsg = { key: "card.test.noreply", bad: true };
      renderTest();
      schedule2();
      if (S.test && S.test.running) pollTest();
    }
    async function cancelTest() {
      const r = await send2("cardtest cancel");
      S.testMsg = r.some((l) => /cancelled/.test(l)) ? { key: "card.test.cancelled" } : null;
      await pollTest();
    }
    async function pollTest() {
      if (polling) return;
      polling = true;
      try {
        const p = parseTest(await send2("cardtest"));
        if (p.seen) {
          S.test = p;
        }
      } finally {
        polling = false;
      }
      renderTest();
      schedule2();
    }
    function schedule2() {
      const want = isOn() && !document.hidden && tabOpen() && S.test && S.test.running;
      if (want && !testTimer) testTimer = setInterval(() => {
        if (!isOn() || document.hidden || !tabOpen()) {
          schedule2();
          return;
        }
        pollTest();
      }, 1e3);
      else if (!want && testTimer) {
        clearInterval(testTimer);
        testTimer = 0;
      }
      const fwant = isOn() && S.fmt.phase === "running";
      if (fwant && !fmtTimer) fmtTimer = setInterval(pollFmt, 1e3);
      else if (!fwant && fmtTimer) {
        clearInterval(fmtTimer);
        fmtTimer = 0;
      }
    }
    const F10 = S.fmt;
    function fmtVisible() {
      return kind() === "bad" || F10.phase === "running" || F10.phase === "result";
    }
    function setPhase(phase, extra) {
      Object.assign(F10, { phase, plan: F10.plan, code: "", deadline: 0, msg: null, result: null }, extra || {});
      clearInterval(cdTimer);
      cdTimer = 0;
      if (phase === "plan1" || phase === "plan2") {
        F10.deadline = Date.now() + TTL * 1e3;
        cdTimer = setInterval(tickCountdown, 250);
      }
      if (phase === "idle" && !extra) F10.plan = null;
      renderFmt(true);
      schedule2();
    }
    function tickCountdown() {
      const s = Math.ceil((F10.deadline - Date.now()) / 1e3), sp = fmtEl.querySelector(".ccount");
      if (s <= 0) {
        send2("cardfmt cancel");
        setPhase("idle", { msg: { key: "card.fmt.expired", bad: true } });
        return;
      }
      if (sp) setT3(sp, "card.fmt.countdown", { s });
    }
    function renderFmt(force) {
      const vis = fmtVisible();
      const sig = JSON.stringify([vis, F10.phase, F10.plan, F10.code, F10.msg, F10.result, isOn()]);
      if (!force && sig === fmtSig) return;
      fmtSig = sig;
      fmtEl.textContent = "";
      if (!vis) return;
      const wrap = el3("div", "cfmt" + (F10.phase === "plan2" ? " final" : ""));
      wrap.setAttribute("role", "group");
      fmtEl.append(wrap);
      if (F10.phase === "idle") {
        wrap.append(elT("div", "csub nomt", "card.fmt.title"), elT("div", "", "card.fmt.explain"));
        if (F10.msg) {
          const m = elT("div", "cmsg" + (F10.msg.bad ? " bad" : ""), F10.msg.key, F10.msg.params);
          m.setAttribute("role", "status");
          wrap.append(m);
        }
        const b = elT("button", "ctl danger", "card.fmt.start");
        b.style.marginTop = ".7rem";
        b.disabled = !isOn();
        b.onclick = startFmt;
        wrap.append(b);
      } else if (F10.phase === "plan1" || F10.phase === "plan2") {
        const second = F10.phase === "plan2";
        wrap.append(elT("div", "csub nomt", second ? "card.fmt.s2.title" : "card.fmt.s1.title"));
        if (F10.plan) wrap.append(elT("div", "", "card.fmt.plan", { fs: F10.plan.fs || "?", label: F10.plan.label || "?", size: F10.plan.size !== null ? fmtBytes(F10.plan.size) : "?" }));
        wrap.append(elT("div", "cwarn", "card.fmt.warn"));
        wrap.append(elT("div", "", second ? "card.fmt.s2.how" : "card.fmt.s1.how"));
        const code = el3("div", "ccode", F10.code);
        code.setAttribute("aria-label", t5("card.fmt.code.aria"));
        code.setAttribute("data-i18n-aria", "card.fmt.code.aria");
        wrap.append(code);
        const inp = el3("input");
        inp.type = "text";
        inp.inputMode = "numeric";
        inp.maxLength = 4;
        inp.autocomplete = "off";
        inp.spellcheck = false;
        inp.className = "ccodein";
        inp.setAttribute("data-i18n-aria", "card.fmt.input.aria");
        inp.setAttribute("aria-label", t5("card.fmt.input.aria"));
        inp.setAttribute("data-i18n-placeholder", "card.fmt.input.ph");
        inp.setAttribute("placeholder", t5("card.fmt.input.ph"));
        inp.oninput = () => {
          const v = inp.value.replace(/\D/g, "").slice(0, 4);
          if (v !== inp.value) inp.value = v;
          ok.disabled = !isOn() || fmtBusy || !v;
        };
        const ok = elT("button", "ctl " + (second ? "danger" : "primary"), second ? "card.fmt.s2.go" : "card.fmt.confirm");
        ok.disabled = true;
        const go = () => {
          const v = inp.value;
          if (v && !fmtBusy && isOn()) confirmFmt(v);
        };
        ok.onclick = go;
        inp.onkeydown = (e) => {
          if (e.key === "Enter") {
            e.preventDefault();
            go();
          }
        };
        const r1 = el3("div", "row");
        inp.style.flex = "1 1 7rem";
        r1.append(inp, ok);
        wrap.append(r1);
        const r2 = el3("div", "row cfoot");
        const cd = el3("span", "mut ccount grow");
        setT3(cd, "card.fmt.countdown", { s: Math.max(0, Math.ceil((F10.deadline - Date.now()) / 1e3)) });
        const cb = elT("button", "ctl", "btn.cancel");
        cb.disabled = !isOn();
        cb.onclick = cancelFmt;
        r2.append(cd, cb);
        wrap.append(r2);
        if (!second) setTimeout(() => {
          if (inp.isConnected && tabOpen()) {
            try {
              inp.focus({ preventScroll: true });
            } catch {
            }
          }
        }, 0);
      } else if (F10.phase === "running") {
        wrap.append(elT("div", "csub nomt", "card.fmt.title"), elT("div", "", "card.fmt.running"));
        const bar = el3("div", "cprog ind");
        bar.setAttribute("role", "progressbar");
        bar.append(el3("i"));
        wrap.append(bar);
      } else if (F10.phase === "result") {
        const okr = F10.result && F10.result.ok;
        const m = F10.result && F10.result.why ? elT("div", "cmsg bad", "card.fmt.failed", { why: reasonP(F10.result.why) }) : elT("div", "cmsg" + (okr ? " good" : " bad"), okr ? "card.fmt.done" : "card.fmt.failed0");
        m.setAttribute("role", "status");
        const b = elT("button", "ctl", "card.fmt.dismiss");
        b.style.marginTop = ".6rem";
        b.onclick = () => {
          setPhase("idle");
          renderAll();
        };
        wrap.append(m, b);
      }
    }
    const fmtLine = (r) => r.find((l) => /^cardfmt:/.test(l)) || "";
    function fmtReply(r, fromStep) {
      const l = fmtLine(r);
      let m;
      if (m = /^cardfmt: confirm again (\d+)/.exec(l)) return setPhase("plan2", { code: m[1] });
      if (m = /^cardfmt: confirm (\d+)/.exec(l)) return setPhase("plan1", { code: m[1] });
      if (/^cardfmt: started/.test(l)) return setPhase("running");
      if (m = /^cardfmt: refused\s*(.*)$/.exec(l)) return setPhase("idle", { msg: { key: "card.fmt.refused", params: { why: fmtWhy(m[1].trim()) }, bad: true } });
      setPhase("idle", { msg: { key: "card.fmt.noreply", bad: true } });
    }
    async function startFmt() {
      if (fmtBusy) return;
      fmtBusy = true;
      try {
        const r = await send2("cardfmt");
        const pl = r.find((l) => /^cardfmt plan\b/.test(l));
        if (pl) {
          const k2 = kv2(pl);
          F10.plan = { fs: k2.fs, label: k2.label, size: num2(k2.size) };
        } else F10.plan = null;
        fmtReply(r, 0);
      } finally {
        fmtBusy = false;
      }
    }
    async function confirmFmt(v) {
      fmtBusy = true;
      const was = F10.phase;
      for (const e of fmtEl.querySelectorAll("button,input")) e.disabled = true;
      try {
        const r = await send2("cardfmt " + v);
        fmtReply(r, was === "plan1" ? 1 : 2);
      } finally {
        fmtBusy = false;
      }
    }
    async function cancelFmt() {
      fmtBusy = true;
      clearInterval(cdTimer);
      cdTimer = 0;
      try {
        await send2("cardfmt cancel");
      } finally {
        fmtBusy = false;
      }
      setPhase("idle", { msg: { key: "card.fmt.cancelled" } });
    }
    async function pollFmt() {
      if (fmtPolling || !isOn()) return;
      fmtPolling = true;
      try {
        const r = await send2("cardfmt status");
        const m = /^cardfmt state (\w+)(?:\s+(.*))?$/.exec(r.find((l) => /^cardfmt state\b/.test(l)) || "");
        if (m && F10.phase === "running") {
          if (m[1] === "done") {
            setPhase("result", { result: { ok: true } });
            load();
          } else if (m[1] === "failed") {
            setPhase("result", { result: { ok: false, why: (m[2] || "").trim() } });
            load();
          } else if (m[1] === "idle") {
            setPhase("idle");
            load();
          }
        }
      } finally {
        fmtPolling = false;
      }
    }
    function renderAll() {
      renderBanner();
      renderFacts();
      renderTest();
      renderFmt();
      schedule2();
    }
    function load() {
      if (loading) return loading;
      loading = (async () => {
        try {
          if (!isOn()) return;
          const c = parseCard(await send2("card"));
          if (c) S.card = c;
          lastCard = (getState() || {}).card || lastCard;
          if (S.card && S.card.state === "mounted") {
            const p = parseTest(await send2("cardtest"));
            if (p.seen) S.test = p;
          }
          if (F10.phase === "running" || S.card && (S.card.state === "unreadable" || S.card.fmt === "running")) {
            const r = await send2("cardfmt status");
            const m = /^cardfmt state (\w+)/.exec(r.find((l) => /^cardfmt state\b/.test(l)) || "");
            if (m && m[1] === "running" && F10.phase !== "running") setPhase("running");
          }
        } finally {
          loading = null;
          renderAll();
        }
      })();
      return loading;
    }
    function onState(st2) {
      const k2 = st2 ? st2.card : null;
      if (st2 && k2 !== lastCard) {
        lastCard = k2;
        if (isOn()) load();
      }
      if ((F10.phase === "plan1" || F10.phase === "plan2") && k2 !== "bad") {
        clearInterval(cdTimer);
        cdTimer = 0;
        Object.assign(F10, { phase: "idle", plan: null, code: "", msg: null });
      }
      renderAll();
    }
    function reset() {
      lastCard = null;
      S.card = null;
      S.test = null;
      S.testMsg = null;
      clearInterval(cdTimer);
      cdTimer = 0;
      Object.assign(F10, { phase: "idle", plan: null, code: "", msg: null, result: null });
      renderAll();
    }
    document.addEventListener("visibilitychange", () => {
      if (!document.hidden && isOn() && tabOpen()) load();
      else schedule2();
    });
    renderAll();
    return { onState, load, reset, rerender: renderAll, info: () => S.card };
  }

  // src/js/pages/device.js
  init_wifi();

  // src/js/lib/time.js
  init_i18n();
  var KEY3 = "nsn.tzsync";
  function buildTz(offsetMin, abbr) {
    const name = /^[A-Za-z]{3,5}$/.test(abbr || "") ? abbr : "UTC";
    const posix = -Math.round(offsetMin);
    const a = Math.abs(posix), h = Math.floor(a / 60), m = a % 60;
    return name + (posix < 0 ? "-" : "") + h + (m ? ":" + String(m).padStart(2, "0") : "");
  }
  function tzString(d4, zone) {
    d4 = d4 || /* @__PURE__ */ new Date();
    let abbr = "";
    try {
      zone = zone || new Intl.DateTimeFormat().resolvedOptions().timeZone;
      const p = new Intl.DateTimeFormat("en-US", { timeZone: zone, timeZoneName: "short" }).formatToParts(d4).find((x) => x.type === "timeZoneName");
      abbr = p ? p.value : "";
    } catch {
    }
    return buildTz(-d4.getTimezoneOffset(), abbr);
  }
  var RE_TIME = /^time (\d+) (\S+) src=(none|client|ntp) tz="(.*)"$/;
  function parseTime(lines) {
    for (const raw of lines || []) {
      const m = RE_TIME.exec(String(raw).trimEnd());
      if (!m) continue;
      const o = /([+-])(\d\d):?(\d\d)$/.exec(m[2]);
      return { unix: +m[1], iso: m[2], src: m[3], tz: m[4].replace(/\\(["\\])/g, "$1"), offsetMin: o ? (o[1] === "-" ? -1 : 1) * (+o[2] * 60 + +o[3]) : 0 };
    }
    return null;
  }
  function fmtLocal(unix, offsetMin) {
    const d4 = new Date((unix + offsetMin * 60) * 1e3);
    return d4.toISOString().slice(0, 19).replace("T", " ");
  }
  var utcLabel = (m) => "UTC" + (m < 0 ? "-" : "+") + String(Math.floor(Math.abs(m) / 60)).padStart(2, "0") + ":" + String(Math.abs(m) % 60).padStart(2, "0");
  function autoOn() {
    try {
      return globalThis.localStorage.getItem(KEY3) !== "0";
    } catch {
      return true;
    }
  }
  function mount3(o) {
    const I2 = I18N, setT4 = I2.setT, setRaw4 = I2.setRaw;
    const { box, send: send2, isOn, tabOpen } = o;
    const el4 = (tag, cls, text) => {
      const e = document.createElement(tag);
      if (cls) e.className = cls;
      if (text !== void 0) e.textContent = text;
      return e;
    };
    const elT2 = (tag, cls, key, params) => {
      const e = el4(tag, cls);
      setT4(e, key, params);
      return e;
    };
    let T = null, at = 0, syncing = null, busy2 = false, msg2 = null;
    const clockV = el4("span"), srcV = el4("span"), zoneV = el4("span");
    const kv3 = el4("div", "ckv");
    kv3.append(elT2("span", "", "time.k.now"), clockV, elT2("span", "", "time.k.src"), srcV, elT2("span", "", "time.k.zone"), zoneV);
    const noClock = elT2("div", "mut cnote", "time.none.note");
    noClock.hidden = true;
    const syncBtn = elT2("button", "ctl", "time.sync");
    const chk = el4("input", "ctl");
    chk.type = "checkbox";
    chk.checked = autoOn();
    const chkLab = el4("label", "rchk");
    chkLab.append(chk, elT2("span", "", "time.auto"));
    const msgEl = el4("div", "cmsg");
    msgEl.setAttribute("role", "status");
    const row2 = el4("div", "row");
    row2.style.marginTop = ".6rem";
    row2.append(syncBtn);
    box.textContent = "";
    box.append(kv3, noClock, row2, chkLab, msgEl);
    function setMsg(key, params, kind) {
      msg2 = key;
      msgEl.className = "cmsg" + (kind ? " " + kind : "");
      if (key) setT4(msgEl, key, params);
      else setRaw4(msgEl, "");
    }
    function render3() {
      const on2 = isOn();
      syncBtn.disabled = !on2 || busy2;
      if (!T) {
        setRaw4(clockV, "\u2014");
        setRaw4(srcV, "\u2014");
        setRaw4(zoneV, "\u2014");
        noClock.hidden = true;
        return;
      }
      setT4(srcV, "time.src." + T.src);
      noClock.hidden = T.src !== "none";
      setRaw4(zoneV, utcLabel(T.offsetMin) + (T.tz ? " \xB7 " + T.tz : ""));
      tick();
    }
    function tick() {
      if (!T || T.src === "none") {
        setRaw4(clockV, "\u2014");
        return;
      }
      setRaw4(clockV, fmtLocal(T.unix + Math.floor((performance.now() - at) / 1e3), T.offsetMin));
    }
    setInterval(tick, 1e3);
    async function read2() {
      if (!isOn()) return null;
      const p = parseTime(await send2("time"));
      if (p) {
        T = p;
        at = performance.now();
      }
      render3();
      return p;
    }
    const nowSec = () => Math.floor(Date.now() / 1e3);
    function sync() {
      if (syncing) return syncing;
      syncing = (async () => {
        try {
          const p = await read2();
          if (!p) return;
          const mine = tzString(), auto = chk.checked;
          if (p.src === "none" || p.src === "client") {
            const r = await send2("time set " + nowSec() + (auto ? " " + mine : ""));
            if (!r.some((l) => /^time set: (ok|zone only)/.test(l))) o.log("time set: " + (r[0] || "no reply"));
          } else if (p.src === "ntp" && auto && p.tz !== mine) {
            const r = await send2("time tz " + mine);
            if (!r.some((l) => /^time tz: ok/.test(l))) o.log("time tz: " + (r[0] || "no reply"));
          } else return;
          await read2();
        } finally {
          syncing = null;
        }
      })();
      return syncing;
    }
    async function syncNow() {
      if (busy2 || !isOn()) return;
      busy2 = true;
      render3();
      setMsg(null);
      try {
        const r = await send2("time set " + nowSec() + (chk.checked ? " " + tzString() : ""));
        const line2 = r.find((l) => /^time set:/.test(l)) || "", m = /^time set: refused\s*(.*)$/.exec(line2);
        if (/^time set: ok/.test(line2)) setMsg("time.res.ok", null, "good");
        else if (/^time set: zone only/.test(line2)) setMsg("time.res.zone", null, "good");
        else if (m) setMsg("time.res.refused", { why: m[1] || "?" }, "bad");
        else setMsg("time.res.noreply", null, "bad");
        await read2();
      } finally {
        busy2 = false;
        render3();
      }
    }
    syncBtn.onclick = syncNow;
    chk.onchange = () => {
      try {
        globalThis.localStorage.setItem(KEY3, chk.checked ? "1" : "0");
      } catch {
      }
      if (chk.checked && isOn()) sync();
    };
    function reset() {
      T = null;
      busy2 = false;
      setMsg(null);
      render3();
    }
    setInterval(() => {
      if (isOn() && tabOpen() && !document.hidden && !syncing && !busy2) read2();
    }, 3e4);
    render3();
    return {
      sync,
      load: read2,
      reset,
      rerender: render3,
      info: () => T,
      clock: () => !T || T.src === "none" ? null : fmtLocal(T.unix + Math.floor((performance.now() - at) / 1e3), T.offsetMin)
    };
  }

  // src/js/pages/device.js
  function initDevice() {
    const isOn = () => app.link === "on", getState = () => app.state, here2 = (r) => () => app.route === r;
    app.ui.card = mount2({ box: $("cardBox"), banner: $("cardWarn"), send, getState, isOn, tabOpen: here2("settings/card"), openTab: () => {
      location.hash = "#/settings/card";
    }, log });
    app.ui.wifi = mount({ box: $("wifiBox"), send, sendUntil, getState, isOn, tabOpen: here2("settings/wifi"), onBusy: (b) => {
      app.scanBusy = b;
    } });
    app.ui.time = mount3({ box: $("timeBox"), send, isOn, tabOpen: here2("settings/time"), log });
    on("status", (state) => {
      app.ui.card.onState(state);
      app.ui.wifi.onState(state);
    });
    on("linkoff", () => {
      app.ui.card.reset();
      app.ui.wifi.reset();
      app.ui.time.reset();
    });
    on("lang", () => {
      app.ui.card.rerender();
      app.ui.wifi.rerender();
      app.ui.time.rerender();
    });
  }

  // src/js/main.js
  init_music();
  init_radio2();
  init_ambient();
  init_fx();
  init_night();
  init_keys();
  init_set_output();

  // src/js/pages/set-led.js
  init_app();
  init_link();
  function initLed() {
    $("led").oninput = () => {
      $("ledv").textContent = $("led").value;
    };
    $("led").onchange = () => send("led " + $("led").value);
  }

  // src/js/main.js
  init_fw2();

  // src/js/pages/settings.js
  init_app();
  init_i18n();
  init_fx();
  init_night();
  init_keys();
  init_diag();

  // src/js/core/help.js
  init_app();
  var KEY4 = "nsn.help";
  var read = () => {
    try {
      const v = localStorage.getItem(KEY4);
      return v === "1" ? true : v === "0" ? false : null;
    } catch {
      return null;
    }
  };
  var write = (v) => {
    try {
      localStorage.setItem(KEY4, v ? "1" : "0");
    } catch {
    }
  };
  var all = read() !== false;
  var over = /* @__PURE__ */ new Map();
  var helpAll = () => all;
  function setHelpAll(v) {
    all = v;
    write(v);
    over.clear();
    applyAll();
    $("helpNotice").hidden = true;
    const sw = $("helpSw");
    if (sw) sw.checked = v;
  }
  var pageOn = (key) => over.has(key) ? over.get(key) : all;
  function applyPage(el4) {
    const key = el4.dataset.page, onNow = pageOn(key);
    el4.classList.toggle("help-on", onNow);
    const b = el4.querySelector(".hbtn");
    if (b) b.setAttribute("aria-pressed", String(onNow));
  }
  function applyAll() {
    for (const el4 of document.querySelectorAll(".page")) applyPage(el4);
  }
  function initHelp() {
    applyAll();
    $("helpSw").checked = all;
    $("helpSw").onchange = () => setHelpAll($("helpSw").checked);
    $("helpNotice").hidden = read() !== null;
    $("helpOk").onclick = () => setHelpAll(all);
    $("helpOff").onclick = () => setHelpAll(false);
    for (const el4 of document.querySelectorAll(".page")) {
      const b = el4.querySelector(".hbtn");
      if (b) b.onclick = () => {
        over.set(el4.dataset.page, !pageOn(el4.dataset.page));
        applyPage(el4);
      };
    }
  }

  // src/js/pages/settings.js
  init_fw2();
  init_fw();
  var LANG_NAMES = { en: "English", "zh-TW": "\u7E41\u9AD4\u4E2D\u6587", "zh-CN": "\u7B80\u4F53\u4E2D\u6587" };
  function ent(id, what, level = "") {
    const a = $("ent-" + id), small = a.querySelector("small"), badge = a.querySelector(".badge");
    if (Array.isArray(what)) setT2(small, what[0], what[1]);
    else setRaw2(small, what || "");
    a.classList.toggle("warn", level === "warn");
    a.classList.toggle("bad", level === "bad");
    badge.hidden = !level;
  }
  function render2() {
    const st2 = app.state, on_ = app.link === "on" && !!st2, off = ["conn.off"];
    ent("output", on_ && typeof st2.vmax === "number" ? st2.dacmax > 0 ? ["ent.output.rate", { n: st2.vmax, r: khz(st2.dacmax) }] : ["ent.output", { n: st2.vmax }] : on_ ? null : off);
    const n = fxOnCount();
    ent("fx", !on_ ? off : st2.fxshed ? ["ent.fx.shed"] : n === null ? null : n ? ["ent.fx.n", { n }] : ["ent.fx.none"]);
    const ni = nightInfo();
    ent("night", !on_ ? off : !ni ? null : ["ent.night", { s: { t: ni.on ? "night.on" : "night.off" }, r: ni.sched ? ni.sched[0] + "\u2013" + ni.sched[1] : { t: "night.nosched" } }]);
    const w = app.ui.wifi && app.ui.wifi.info();
    if (!on_) ent("wifi", off);
    else if (!w) ent("wifi", null);
    else if (w.state === "connected") ent("wifi", ["ent.wifi.on", { ssid: w.ssid }]);
    else {
      const k2 = "wifi.state." + w.state.toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
      ent("wifi", I18N.has(k2) ? [k2] : w.state, /wrong|not found|no credentials/i.test(w.state) ? /no credentials/i.test(w.state) ? "warn" : "bad" : "");
    }
    const T = app.ui.time && app.ui.time.info(), clock = app.ui.time && app.ui.time.clock();
    if (!on_) ent("time", off);
    else if (!T) ent("time", null);
    else if (T.src === "none") ent("time", ["time.src.none"], "warn");
    else ent("time", ["ent.time", { t: clock ? clock.slice(11, 16) : "\u2014", src: { t: "time.src." + T.src } }]);
    const ck = st2 && st2.card, c = app.ui.card && app.ui.card.info();
    if (!on_) ent("card", off);
    else if (ck === "none") ent("card", ["ent.card.none"], "warn");
    else if (ck === "bad") ent("card", ["ent.card.bad"], "bad");
    else if (c && c.state === "mounted" && c.free !== null) ent("card", ["ent.card.free", { free: fmtBytes(c.free) }]);
    else ent("card", ck === "ok" ? ["card.checking"] : null);
    const km = keysMatch();
    ent("keys", !on_ ? off : km === null ? null : km ? ["keys.preset." + km] : ["keys.custom"]);
    ent("led", [!on_ ? "conn.off" : ledDisabled() ? "hw.led.disabled" : "ent.led"]);
    const mode = app.viz && app.viz.host.modes().find((m) => m.id === app.viz.host.mode());
    ent("viz", mode ? [mode.labelKey] : null);
    ent("lang", [helpAll() ? "ent.lang.on" : "ent.lang.off", { l: LANG_NAMES[I18N.lang()] }]);
    ent("diag", on_ ? ["ent.diag", { kb: Math.round(st2.mem / 1024), n: st2.und }] : t2("diag.build") + " " + BUILD);
  }
  function initSettings() {
    on("status", render2);
    on("linkoff", render2);
    on("lang", render2);
    on("fw", render2);
    on("route", (key) => {
      if (key === "settings") render2();
    });
    render2();
  }

  // src/js/pages/lang.js
  init_app();
  init_i18n();
  function initLang() {
    const sel = $("langSel");
    sel.value = I18N.pref();
    sel.onchange = () => I18N.setLang(sel.value);
    I18N.onChange(() => {
      sel.value = I18N.pref();
      emit("lang");
    });
  }

  // src/js/main.js
  init_bar();

  // src/js/core/router.js
  init_app();
  var ROUTES = {
    "now": { page: "p-now", nav: "now" },
    "now/lyrics": { page: "p-lyrics", nav: "now", parent: "now" },
    "music": { page: "p-music", nav: "music" },
    "radio": { page: "p-radio", nav: "radio" },
    "ambient": { page: "p-ambient", nav: "ambient" },
    "settings": { page: "p-settings", nav: "settings" },
    ...Object.fromEntries(["output", "fx", "night", "viz", "wifi", "time", "card", "keys", "led", "lang", "diag"].map((id) => ["settings/" + id, { page: "p-" + id, nav: "settings", parent: "settings" }]))
  };
  var DEMO_ALIAS = { files: "music", noise: "ambient", bed: "settings/night", sys: "settings/diag", lyrics: "now/lyrics" };
  function demoRoute(name) {
    if (!name) return "";
    const k2 = DEMO_ALIAS[name] || name;
    return ROUTES[k2] ? k2 : ROUTES["settings/" + k2] ? "settings/" + k2 : "";
  }
  var keyOf = () => {
    const k2 = location.hash.replace(/^#\/?/, "").replace(/\/+$/, "");
    return ROUTES[k2] ? k2 : "now";
  };
  var first = true;
  var prevKey = "";
  function apply() {
    const key = keyOf(), was = app.route;
    if (location.hash !== "#/" + key) history.replaceState(null, "", "#/" + key);
    app.route = key;
    document.body.dataset.route = key;
    for (const [k2, r] of Object.entries(ROUTES)) $(r.page).hidden = k2 !== key;
    for (const a of document.querySelectorAll("#nav [data-nav]")) {
      if (a.dataset.nav === ROUTES[key].nav) a.setAttribute("aria-current", "page");
      else a.removeAttribute("aria-current");
    }
    $("content").scrollTop = 0;
    if (!first && was !== key) {
      const h = $(ROUTES[key].page).querySelector("h2");
      if (h) {
        h.tabIndex = -1;
        h.focus({ preventScroll: true });
      }
    }
    first = false;
    emit("route", key, was);
  }
  function initRouter() {
    for (const a of document.querySelectorAll(".back")) a.addEventListener("click", (e) => {
      if (history.length > 1 && prevKey === ROUTES[keyOf()].parent) {
        e.preventDefault();
        history.back();
      }
    });
    window.addEventListener("hashchange", () => {
      prevKey = app.route;
      apply();
    });
    apply();
  }

  // src/js/main.js
  init_session();
  I18N.init();
  hydrateIcons();
  initLink();
  initNow();
  initLyrics();
  initDiag();
  initDevice();
  initMusic();
  initRadio();
  initAmbient();
  initFx();
  initNight();
  initKeys();
  initOutput();
  initLed();
  initFw();
  initBar();
  initLang();
  initSettings();
  initHelp();
  initSession();
  if (DEMO && !location.hash) {
    const r = demoRoute(new URLSearchParams(location.search).get("demo"));
    if (r) location.hash = "#/" + r;
  }
  initRouter();
  document.addEventListener("keydown", (e) => {
    if ((e.key === "Enter" || e.key === " ") && e.target.tagName === "LI") {
      e.preventDefault();
      e.target.click();
    }
  });
  if (DEMO) Promise.resolve().then(() => (init_box(), box_exports)).then((m) => m.startDemo());
})();
