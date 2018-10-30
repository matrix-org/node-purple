const libpurple = require('./build/Debug/module');
console.log(`Libpurple core version:`, libpurple.core.get_version());
console.log(libpurple);

setInterval(() => {
    console.log("node-purple idle output");
}, 4000);

libpurple.helper.setupPurple(
    {
        debugEnabled: 1
    }
);
console.log("Finished setting up purple!");
console.log("Plugin list:", libpurple.plugins.get_protocols());
