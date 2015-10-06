var Cli = require("matrix-appservice-bridge").Cli;
var Bridge = require("matrix-appservice-bridge").Bridge;
var AppServiceRegistration = require("matrix-appservice-bridge").AppServiceRegistration;
var Purple = require('../purple');

var REGISTRATION_PATH = "purple-registration.yaml";
var SCHEMA_PATH = "purple-config-schema.yaml";

var accountsByUserId = {
    //user_id: AccountInfo
};

function AccountInfo(accConfig, prplInst) {
    this.protocol = accConfig.protocol;
    this.username = accConfig.username;
    this.password = accConfig.accConfig;
    this.user_id = accConfig.user_id;
    this.prpl = prplInst || null;
}

function UserPrefixMapper(config) {
    var self = this;
    this.protocolToPrefix = {};
    this.prefixToProtocol = {};
    var prefixSet = {};
    config.accounts.forEach(function(acc) {
        var prefix = config.user_prefixes[acc.protocol] || acc.protocol;
        prefixSet[prefix] = true;
        self.protocolToPrefix[acc.protocol] = prefix;
        self.prefixToProtocol[prefix] = acc.protocol;
    });
    this.prefixes = Object.keys(prefixSet);
}

UserPrefixMapper.prototype.getUserLocalpartFromPurpleId = function(protocol, username) {
    return this.protocolToPrefix[protocol] + "_" + username;
};

UserPrefixMapper.prototype.getPurpleInfoFromUserLocalpart = function(localpart) {
    for (var i = 0; i < this.prefixes.length; i++) {
        if (localpart.indexOf(this.prefixes[i]) === 0) {
            return {
                // +1 for the _
                username: localpart.substring(this.prefixes[i].length + 1),
                protocol: this.prefixToProtocol[this.prefixes[i]]
            };
        }
    }
};


function runMatrix(port, config) {
    var mapper = new UserPrefixMapper(config);
    // run purple conns and set globals
    config.accounts.forEach(function(a) {
        var prpl = runPurple(a, mapper);
        accountsByUserId[a.user_id] = new AccountInfo(a, prpl);
        prpl.onMessage = function(conv, who, alias, message, protocol) {
            var prefix = mapper.protocolToPrefix[protocol];
            var intent = bridge.getIntentFromLocalpart(prefix + "_" + who);
            console.log(
                "[PRPL-MSG] from=%s protocol=%s (conn_behalf_of=%s)",
                who, protocol, a.user_id
            );
            // get all rooms between this purple+matrix user
            bridge.getRoomStore().getMatrixRooms({
                "extras.purple_username": who,
                "extras.purple_protocol": protocol,
                "extras.matrix_user": a.user_id
            }).done(function(rooms) {
                console.log(
                    "[PRPL-MSG] Sending msg from %s to %s rooms", who, rooms.length
                );
                var content = mapMessageToMatrix(protocol, message);
                rooms.forEach(function(r) {
                    intent.sendEvent(r.roomId, "m.room.message", content);
                });
                
            }, function(e) {
                console.error("[PRPL-MSG] Failed to get matrix rooms: %s", e);
            })
        };
    });

    var bridgeController = {
        onUserQuery: function(queriedUser) {
            return {}; // auto-provision users with no additonal data
        },

        onEvent: function(request, context) {
            var event = request.getData();
            var room = context.rooms.matrix;

            // only respond to stuff done by people with accounts configured.
            if (Object.keys(accountsByUserId).indexOf(event.user_id) === -1) {
                return;
            }
            var accountInfo = accountsByUserId[event.user_id];
            
            if (event.type === "m.room.member" && event.content.membership === "invite") {
                // if they are inviting a virtual user, accept it.
                var invitee = context.targets.matrix;
                var account = mapper.getPurpleInfoFromUserLocalpart(invitee.localpart);
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
                    room.set("purple_username", account.username);
                    room.set("purple_protocol", account.protocol);
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
            else if (event.type === "m.room.message" && room.get("purple_username")) {
                var body = event.content.body;
                var prplUsername = room.get("purple_username");
                // send body to purple_user
                console.log("[MSG] Sending message to %s", prplUsername);
                var conv = accountInfo.prpl.getConversation(prplUsername);
                conv.send(body);
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

}

/**
 * Run a Purple instance
 * @param {Object} acc The account object from the config.
 * @return {Purple} An instantiated purple instance
 */
function runPurple(acc) {
    console.log(
        "Run with : %s (%s) on behalf of %s",
        acc.username, acc.protocol, acc.user_id
    );
    var p = new Purple({
        debug: true
    });
    var Status = require('../lib/status');
    var Account = require('../lib/account');
    var Conversation = require("../lib/conversation");
    var account = new Account(acc.username, acc.protocol);
    account.password = acc.password;

    p.enableAccount(account);

    var s = new Status('Testing App', Status.AVAILABLE);
    p.savedstatus = s;

    p.on('create_conversation', function (conversation) {
        console.log(
            "[PRPL-CREATE-CONV] %s %s", conversation.title, conversation.name
        );
    });

    p.on('write_conv', function (conv, who, alias, message, flags, time) {
        try {
            if (alias !== acc.username && who !== acc.username && p.onMessage) {
                p.onMessage(conv, who, alias, message, acc.protocol);
            }
        }
        catch (e) {
            console.error("[PRPL-MSG] write_conv fail", e);
        }
    });

    p.getConversation = function(target) {
        return new Conversation(
            Conversation.IM, account.instance, target
        );
    };

    return p;
}

/**
 * Map a message from one protocol to a matrix message.
 * @param {string} protocol The protocol e.g. prpl-aim
 * @param {string} message The message
 * @return {Object} The matrix message to respond with.
 */
function mapMessageToMatrix(protocol, message) {
    switch (protocol) {
        case "prpl-aim":
            return {
                msgtype: "m.text",
                body: message,
                format: "org.matrix.custom.html",
                formatted_body: message
            };
            break;
    }
    return {
        msgtype: "m.text",
        body: message
    };
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
        var mapper = new UserPrefixMapper(cfg);
        mapper.prefixes.forEach(function(prefix) {
            reg.addRegexPattern("users", "@" + prefix + "_.*", true);
        });
        if (mapper.prefixes.length === 0) {
            throw new Error("You must specify at least one account in the config.");
        }
        callback(reg);
    },
    run: function(port, config) {
        runMatrix(port, config);
    }
})

cli.run();
