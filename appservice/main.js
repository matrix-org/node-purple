var Cli = require("matrix-appservice-bridge").Cli;
var Bridge = require("matrix-appservice-bridge").Bridge;
var AppServiceRegistration = require("matrix-appservice-bridge").AppServiceRegistration;
var Purple = require('../purple');

var REGISTRATION_PATH = "purple-registration.yaml";
var SCHEMA_PATH = "purple-config-schema.yaml";

var cli = new Cli({
    registrationPath: REGISTRATION_PATH,
    bridgeConfig: {
        affectsRegistration: true,
        schema: SCHEMA_PATH,
        defaults: {
            bot_username: "purplebridge",
            user_prefixes: {}
        }
    },
    generateRegistration: function(reg, callback) {
        var cfg = cli.getConfig();
        reg.setHomeserverToken(AppServiceRegistration.generateToken());
        reg.setAppServiceToken(AppServiceRegistration.generateToken());
        reg.setSenderLocalpart(cfg.bot_username);
        var prefixes = {};
        cfg.accounts.forEach(function(acc) {
            if (cfg.user_prefixes[acc.protocol]) {
                prefixes[cfg.user_prefixes[acc.protocol]] = true;
            }
            else {
                prefixes[acc.protocol] = true;
            }
        });
        Object.keys(prefixes).forEach(function(prefix) {
            reg.addRegexPattern("users", "@" + prefix + "_.*", true);
        });
        callback(reg);
    },
    run: function(port, config) {
        console.log("Run run run %s", config);
        /*
        bridge = new Bridge({
            homeserverUrl: config.homserver.url,
            domain: config.homeserver.server_name,
            registration: REGISTRATION_PATH,

            controller: {
                onUserQuery: function(queriedUser) {
                    return {}; // auto-provision users with no additonal data
                },

                onEvent: function(request, context) {
                    var event = request.getData();
                    if (event.type !== "m.room.message" || !event.content || event.room_id !== ROOM_ID) {
                        return;
                    }
                    requestLib({
                        method: "POST",
                        json: true,
                        uri: SLACK_WEBHOOK_URL,
                        body: {
                            username: event.user_id,
                            text: event.content.body
                        }
                    }, function(err, res) {
                        if (err) {
                            console.log("HTTP Error: %s", err);
                        }
                        else {
                            console.log("HTTP %s", res.statusCode);
                        }
                    });
                }
            }
        });
        console.log("Matrix-side listening on port %s", port);
        bridge.run(port, config); */
    }
})

cli.run();

function runPurple() {
    var p = new Purple({
        debug: true,
    });
    var Status = require('../lib/status');
    var Account = require('../lib/account');
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
