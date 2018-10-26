const libpurple = require('./build/Release/module');
libpurple.debug.set_enabled(1);
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);
libpurple.helper.setupPurple(
    {
        debugEnabled: 1
    }
);
