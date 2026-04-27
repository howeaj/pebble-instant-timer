var Clay = require('@rebble/clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, function(minified) {
    var clayConfig = this;
    var colorChangeCausesCustom = true;

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
                setColor('bgColor',             0x000000);
                setColor('statusBarBgColor',    0x000000);
                setColor('actionBarBgColor',    0x000000);
                setColor('bgColorImage',        0x550000);
                setColor('textColor',           0xffffff);
                setColor('statusBarTextColor',  0xffffff);
                setColor('actionBarIconColor',  0xffffff);
                setColor('ringColorEmpty',      0x555555);
                setColor('ringColorRemaining',  0x00ff00);
                setColorById('ringColorOvertimeColor', 0xff0000);
                setColorById('ringColorOvertimeBw',    0x555555);
                break;
            case "Light":
                setColor('bgColor',             0xffffff);
                setColor('actionBarBgColor',    0xffffff);
                setColor('statusBarBgColor',    0xffffff);
                setColor('bgColorImage',        0xff5555);
                setColor('textColor',           0x000000);
                setColor('statusBarTextColor',  0x000000);
                setColor('actionBarIconColor',  0x000000);
                setColor('ringColorEmpty',      0xaaaaaa);
                setColor('ringColorRemaining',  0x00ff00);
                setColorById('ringColorOvertimeColor', 0xff0000);
                setColorById('ringColorOvertimeBw',    0x555555);
                break;
            case "Green":
                setColor('bgColor',             0x005500);
                setColor('actionBarBgColor',    0x005500);
                setColor('statusBarBgColor',    0x005500);
                setColor('bgColorImage',        0x555500);
                setColor('textColor',           0xaaffaa);
                setColor('statusBarTextColor',  0xaaffaa);
                setColor('actionBarIconColor',  0xaaffaa);
                setColor('ringColorEmpty',      0x55aa55);
                setColor('ringColorRemaining',  0x00ff00);
                setColor('ringColorOvertime',   0xff0000);
                break;
            case "Blue":
                setColor('bgColor',             0x000055);
                setColor('actionBarBgColor',    0x000055);
                setColor('statusBarBgColor',    0x000055);
                setColor('bgColorImage',        0x0000aa);
                setColor('textColor',           0xaaffff);
                setColor('statusBarTextColor',  0xaaffff);
                setColor('actionBarIconColor',  0xaaffff);
                setColor('ringColorEmpty',      0x5555AA);
                setColor('ringColorRemaining',  0x00ff00);
                setColor('ringColorOvertime',   0xff0000);
                break;
            default:
                break;
            }
            colorChangeCausesCustom = true;
        }
    }

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
        var themeSelector = clayConfig.getItemByMessageKey("theme")
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
