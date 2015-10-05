var Cli = require("matrix-appservice-bridge").Cli;
var Bridge = require("matrix-appservice-bridge").Bridge;
var AppServiceRegistration = require("matrix-appservice-bridge").AppServiceRegistration;
var Purple = require('../purple');

var REGISTRATION_PATH = "purple-registration.yaml";
var SCHEMA_PATH = "purple-config-schema.yaml";

function runMatrix(port, config) {
    var userPrefixInfo = getUserPrefixInfo(config);
    var bridgeController = {
        onUserQuery: function(queriedUser) {
            return {}; // auto-provision users with no additonal data
        },

        onEvent: function(request, context) {
            var event = request.getData();
            var room = context.rooms.matrix;
            if (event.type === "m.room.member" &&
                    event.content.membership === "invite") {
                // if they are inviting a virtual user, accept it.
                var invitee = context.targets.matrix;
                var account = getPurpleUserForMatrixUser(invitee, userPrefixInfo);
                if (!account) {
                    return; // invite not for a virtual user.
                }
                console.log("[INVITE] Matched to: %s", JSON.stringify(account));
                console.log(
                    "[INVITE] Accepting invite from %s for %s to %s ",
                    event.user_id, invitee.userId, event.room_id
                );
                var intent = bridge.getIntentFromLocalpart(invitee.localpart);
                intent.join(event.room_id).then(function() {
                    // link up this room for this real matrix user as a 1:1
                    console.log(
                        "[INVITE] %s joined room %s", invitee.userId, event.room_id
                    );
                    room.set("matrix_user", event.user_id);
                    room.set("purple_user", account);
                    return bridge.getRoomStore().setMatrixRoom(room);
                }).done(function() {
                    console.log(
                        "[INVITE] Marked %s as a PM room.", event.room_id
                    );
                }, function(err) {
                    console.error(
                        "[INVITE] %s failed to join room %s",
                        invitee.userId, event.room_id, err
                    );
                });
            }
            else if (event.type === "m.room.message" && room.get("purple_user")) {
                var body = event.content.body;
                var prplUser = room.get("purple_user");
                // send body to purple_user
                console.log("[MSG] Sending message to %s", JSON.stringify(prplUser));
            }
        }
    };

    bridge = new Bridge({
        homeserverUrl: config.homeserver.url,
        domain: config.homeserver.server_name,
        registration: REGISTRATION_PATH,
        controller: bridgeController
    });
    console.log("Matrix-side listening on port %s", port);
    bridge.run(port, config);
    runPurple(config.accounts[0]);
}

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

function getPurpleUserForMatrixUser(matrixUser, userPrefixInfo) {
    var userPrefixes = Object.keys(userPrefixInfo);
    for (var i = 0; i < userPrefixes.length; i++) {
        if (matrixUser.localpart.indexOf(userPrefixes[i]) === 0) {
            return {
                // +1 for the _
                username: matrixUser.localpart.substring(userPrefixes[i].length + 1),
                accountHolder: userPrefixInfo[userPrefixes[i]].username,
                protocol: userPrefixInfo[userPrefixes[i]].protocol
            };
        }
    }
    return null;
}

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
        runMatrix(port, config);
    }
})

cli.run();
