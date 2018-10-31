const libpurple = require('./build/Debug/module');
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);

setInterval(() => {
    libpurple.helper.pollEvents().forEach((ev) => {
        console.log("Got event:", ev);
    });
}, 100);

libpurple.helper.setupPurple(
    {
        debugEnabled: 1,
    }
);
console.log("Finished setting up purple!");
//console.log("Plugin list:", libpurple.plugins.get_protocols());
const acct = libpurple.accounts.find("halfshot@localhost/", "prpl-jabber");
console.log("Acct:", acct);
//console.log("Enabled:", libpurple.accounts.get_enabled(acct.handle));
//libpurple.accounts.set_enabled(acct.handle, true);
//console.log("Enabled:", libpurple.accounts.get_enabled(acct.handle));
//libpurple.accounts.new("TestJabber", "prpl-jabber");
