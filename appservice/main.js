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
        var userPrefixInfo = getUserPrefixInfo(cfg);
        Object.keys(userPrefixInfo).forEach(function(prefix) {
            reg.addRegexPattern("users", "@" + prefix + "_.*", true);
        });
        if (Object.keys(userPrefixInfo).length === 0) {
            throw new Error("You must specify at least one account in the config.");
        }
        callback(reg);
    },
    run: function(port, config) {
        var userPrefixInfo = getUserPrefixInfo(config);
        bridge = new Bridge({
            homeserverUrl: config.homeserver.url,
            domain: config.homeserver.server_name,
            registration: REGISTRATION_PATH,

            controller: {
                onUserQuery: function(queriedUser) {
                    return {}; // auto-provision users with no additonal data
                },

                onEvent: function(request, context) {
                    var event = request.getData();
                    if (event.type === "m.room.member" &&
                            event.content.membership === "invite") {
                        // if they are inviting a virtual user, accept it.
                        var invitee = context.targets.matrix;
                        var account = getAccountForMatrixUser(invitee, userPrefixInfo);
                        if (!account) {
                            return; // invite not for a virtual user.
                        }
                        console.log(
                            "[INVITE] Accepting invite from %s for %s to %s ",
                            event.user_id, invitee.userId, event.room_id
                        );
                        var intent = bridge.getIntentFromLocalpart(invitee.localpart);
                        intent.join(event.room_id).done(function() {
                            console.log(
                                "[INVITE] %s joined room %s", invitee.userId, event.room_id
                            );
                        }, function(err) {
                            console.error(
                                "[INVITE] %s failed to join room %s",
                                invitee.userId, event.room_id, err
                            );
                        });
                    }
                }
            }
        });
        console.log("Matrix-side listening on port %s", port);
        bridge.run(port, config);
        runPurple(config.accounts[0]);
    }
})

cli.run();

function runPurple(acc) {
    console.log("Run with : %s (%s)", acc.username, acc.protocol)
    var p = new Purple({
        debug: true
    });
    var Status = require('../lib/status');
    var Account = require('../lib/account');
    var account = new Account(acc.username, acc.protocol);
    account.password = acc.password;

    p.enableAccount(account);

    var s = new Status('Testing App', Status.AVAILABLE);
    p.savedstatus = s;

    p.on('create_conversation', function (conversation) {
        console.log("we have a new conversation", conversation.title, conversation.name);
    });

    p.on('write_conv', function (conv, who, alias, message, flags, time) {
        if (alias !== acc.username && who !== acc.username) {
            conv.send('you said "' + message + '"');
        }
    });
}

function getUserPrefixInfo(cfg) {
    var prefixes = {};
    cfg.accounts.forEach(function(acc) {
        if (cfg.user_prefixes[acc.protocol]) {
            prefixes[cfg.user_prefixes[acc.protocol]] = acc;
        }
        else {
            prefixes[acc.protocol] = acc;
        }
    });
    return prefixes;
}

function getAccountForMatrixUser(matrixUser, userPrefixInfo) {
    var userPrefix = Object.keys(userPrefixInfo).filter(function(prefix) {
        return matrixUser.localpart.indexOf(prefix) === 0;
    });
    return userPrefixInfo[userPrefix];
}