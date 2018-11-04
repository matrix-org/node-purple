const libpurple = require('./build/Debug/module');
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);

let acct;

setInterval(() => {
    if (acct) {

    }
    libpurple.helper.pollEvents().forEach((ev) => {
        console.log("Got event:", ev);
        if (ev.eventName === "received-im-msg"){
            libpurple.messaging.sendIM(acct.handle, ev.sender, ev.message);
        }
    });
}, 500);

libpurple.helper.setupPurple(
    {
        debugEnabled: 1,
    }
);
console.log("Finished setting up purple!");
//console.log("Plugin list:", libpurple.plugins.get_protocols());
acct = libpurple.accounts.find("halfshot@localhost/", "prpl-jabber");
statusTypes = libpurple.accounts.get_status_types(acct.handle);
console.log("Buddy:", libpurple.buddy.find(acct.handle, "tester@localhost"));
console.log(statusTypes);
//console.log("Acct:", acct);
//console.log("Enabled:", libpurple.accounts.get_enabled(acct.handle));
//libpurple.accounts.set_enabled(acct.handle, true);
//console.log("Enabled:", libpurple.accounts.get_enabled(acct.handle));
//libpurple.accounts.new("TestJabber", "prpl-jabber");
