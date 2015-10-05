var Purple = require('../purple');
var read = require("read");
var username, password, protocol;

var p = new Purple({
    debug: true,
});

var Status = require('../lib/status');
var Account = require('../lib/account');

p.protocols.forEach(function(prpl, i) {
    console.log("[%s] %s", i, prpl.name);
});

read({ prompt: "Service number: " }, function(err, srv) {
    if (err) { console.error(err); process.exit(1); }
    protocol = p.protocols[parseInt(srv)].id;
    read({ prompt: "Username: " }, function(err, uname) {
        if (err) { console.error(err); process.exit(1); }
        username = uname;
        read({ prompt: "Password: ", silent: true }, function(err, pword) {
            if (err) { console.error(err); process.exit(1); }
            password = pword;
            main();
        });
    });
});

function main() {
    var account = new Account(username, protocol);
    account.password = password;

    p.enableAccount(account);

    var s = new Status('Testing App', Status.AVAILABLE);
    p.savedstatus = s;

    p.on('create_conversation', function (conversation) {
        console.log("we have a new conversation", conversation.title, conversation.name);
    });

    p.on('write_conv', function (conv, who, alias, message, flags, time) {
        if (alias !== username && who !== username) {
            conv.send('you said "' + message + '"');
        }
    });
}
