const libpurple = require('./build/Debug/module');
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);

setTimeout(() => {
    libpurple.core.quit();
}, 2000);

libpurple.helper.setupPurple(
    {
        debugEnabled: 0
    }
);
console.log("Finished setting up purple!");
//console.log("Plugin list:", libpurple.plugins.get_protocols());
libpurple.accounts.new("TestJabber", "prpl-jabber")
