var Clay = require('@rebble/clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, function(minified) {
    var clayConfig = this;
    var colorChangeCausesCustom = true;
    var separateSystemBars = true;  // emery layout is slightly different, with uneven gap to action/status bars
    var round = false;  // round layouts' actionbar icons are on the progress ring

    function setColor(messageKey, color) {
        var item = clayConfig.getItemByMessageKey(messageKey);
        if (item) {
            item.set(color);
        }
    }

    function setColorById(id, color) {
        var item = clayConfig.getItemById(id);
        if (item) {
            item.set(color);
        }
    }

    function setTheme() {
        var themeSelector = this;
        const theme = themeSelector.get();
        if (theme == "Custom") {
            // after Custom is picked, show color options permanently
            for (var item of clayConfig.getItemsByType("color")) {
                item.show();
            }
        } else {
            colorChangeCausesCustom = false;
            switch (theme) {
            case "Dark":
                // This should match the defaults in config.json and config.c
                setColor('bgColor',             0x000000);
                setColor('statusBarBgColor',    0x000000);
                setColor('actionBarBgColor',    0x000000);
                setColor('textColor',           0xffffff);
                setColor('statusBarTextColor',  0xffffff);
                setColor('actionBarIconColor',  0xffffff);
                setColorById('col_ringColorEmpty',     0x555555);
                setColorById('col_ringColorRemaining', 0x00ff00);
                setColorById('col_ringColorOvertime',  0xff0000);
                setColorById('bw_ringColorEmpty',      0x555555);
                setColorById('bw_ringColorRemaining',  0xffffff);
                setColorById('bw_ringColorOvertime',   0xffffff);
                setColor('bgColorImage',        0x550000);
                break;
            case "Light":
                setColor('bgColor',             0xffffff);
                setColor('statusBarBgColor',    0xffffff);
                setColor('actionBarBgColor',    0xffffff);
                setColor('textColor',           0x000000);
                setColor('statusBarTextColor',  0x000000);
                setColor('actionBarIconColor',  0x000000);
                setColorById('col_ringColorEmpty',     0xaaaaaa);
                setColorById('col_ringColorRemaining', 0x00ff00);
                setColorById('col_ringColorOvertime',  0xff0000);
                setColorById('bw_ringColorEmpty',      0xffffff);
                setColorById('bw_ringColorRemaining',  0x555555);
                setColorById('bw_ringColorOvertime',   0x000000);
                setColor('bgColorImage',        0xff5555);
                break;
            case "Green":
                setColor('bgColor',             0x005500);
                if (separateSystemBars) {
                    setColor('statusBarBgColor', 0x000000);
                    setColor('actionBarBgColor', 0x000000);
                } else {
                    setColor('statusBarBgColor', 0x005500);
                    setColor('actionBarBgColor', 0x005500);
                }
                setColor('textColor',           0xffffff);
                setColor('statusBarTextColor',  0xaaffaa);
                if (round) {
                    setColor('actionBarIconColor',  0Xffffff);
                } else {
                    setColor('actionBarIconColor',  0xaaffaa);
                }
                setColor('ringColorEmpty',      0x00aa00);
                setColor('ringColorRemaining',  0x55ff00);
                setColor('ringColorOvertime',   0xaa0000);
                setColor('bgColorImage',        0x550000);
                break;
            case "Aqua":
                setColor('bgColor',             0x55aaff);
                if (separateSystemBars) {
                    setColor('statusBarBgColor', 0x00aaaa);
                    setColor('actionBarBgColor', 0x00aaaa);
                } else {
                    setColor('statusBarBgColor', 0x55aaff);
                    setColor('actionBarBgColor', 0x55aaff);
                }
                setColor('textColor',           0xffffff);
                setColor('statusBarTextColor',  0xffffff);
                setColor('actionBarIconColor',  0xffffff);
                setColor('ringColorEmpty',      0x5555AA);
                setColor('ringColorRemaining',  0x00ff00);
                setColor('ringColorOvertime',   0xff00aa);
                setColor('bgColorImage',        0xaa0055);
                break;
            default:
                break;
            }
            colorChangeCausesCustom = true;
        }
    }

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
        if (clayConfig.meta.activeWatchInfo) {
            separateSystemBars = (clayConfig.meta.activeWatchInfo.platform === "emery");
            round = ["chalk", "gabbro"].includes(clayConfig.meta.activeWatchInfo.platform);
        }
        var themeSelector = clayConfig.getItemByMessageKey("theme");
        for (var item of clayConfig.getItemsByType("color")) {
            item.hide();
            item.on("change", function() {  // the "click" event doesn't work
                if (colorChangeCausesCustom) {
                    themeSelector.set("Custom")
                }
            });
        }
        setTheme.call(themeSelector);
        themeSelector.on("change", setTheme);
    });

});
