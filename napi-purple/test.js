const libpurple = require('./build/Release/module');
libpurple.debug.set_enabled(1);
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);

setInterval(() => {
    console.log("I'm doing stuff!");
}, 100);

libpurple.helper.setupPurple(
    {
        debugEnabled: 1
    }
);
