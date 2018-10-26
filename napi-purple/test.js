const libpurple = require('./build/Release/module');
libpurple.debug.set_enabled(1);
console.log(`Libpurple core version:`, libpurple.core.get_version());

libpurple.helper.setupPurple({

});
