const libpurple = require('./build/Release/module');
console.log(libpurple);
libpurple.debug.set_enabled(1);
console.log(`Libpurple core version:`, libpurple.core.get_version());

function testCreateAndQuit() {
    libpurple.core.init();
    console.log("Created new purple core");
    setTimeout(() => {
        libpurple.core.quit();
        console.log("Quit purple core");
    }, 2000);
}


testCreateAndQuit();
