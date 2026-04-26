var Clay = require('@rebble/clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, function(minified) {
    var clayConfig = this;

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
        // nothing to do
    });
});
