var os = require('os');
var path = require('path');
var util = require('util');
var EventEmitter = require('events').EventEmitter;

var FFI = require('ffi');
var ref = require('ref');

var LIBRARY_PATHS = [];

switch (os.platform()) {
  case 'darwin':
    LIBRARY_PATHS.push('/opt/pidgin-2.10.11/lib/libpurple.0.dylib');
    //LIBRARY_PATHS.push('/Applications/Adium.app/Contents/Frameworks/AdiumLibpurple.framework/AdiumLibpurple');
    //LIBRARY_PATHS.push(path.join(process.env.HOME, 'Applications/Adium.app/Contents/Frameworks/libpurple.framework/libpurple'));
    //LIBRARY_PATHS.push('/Users/tjfontaine/Development/adium/Dependencies/build/lib/libpurple.0.dylib');
    break;
  case 'win32':
    break;
  default:
    LIBRARY_PATHS.push('/usr/lib/libpurple.so.0');
    break;
}

exports.LIBRARY_PATHS = LIBRARY_PATHS;

var lib, Plugin, Conversation;

var types = require('./lib/types');
var eventloop = require('./lib/eventloop');


var Purple = function (args) {
  var uiops, conv_ui_ops, ui_init_ptr;
  var self = this;

  if (!(this instanceof Purple)) {
    return new Purple(args);
  }

  if (!lib) {
    lib = require('./lib/libpurple').initialize(LIBRARY_PATHS);
    Plugin = require('./lib/plugin');
    Conversation = require('./lib/conversation');
  }

  if (!args) {
    args = {};
  }

  lib.purple_debug_set_enabled(args.debug ? 1 : 0);

  //TODO XXX FIXME Darwin only
//  lib.purple_init_ssl_plugin();
//  lib.purple_init_ssl_openssl_plugin();
//  lib.purple_init_ssl_cdsa_plugin();

  conv_ui_ops = new types.PurpleConversationUiOps();
  conv_ui_ops.ref().fill(0);

  function ui_init() {
    lib.purple_conversations_set_ui_ops(conv_ui_ops.ref());
  }
  ui_init_ptr = FFI.Callback(ref.types.void, [], ui_init);

  uiops = new types.PurpleCoreUiOps();
  uiops.ref().fill(0);
  uiops.ui_init = ui_init_ptr;
  lib.purple_core_set_ui_ops(uiops.ref());

  lib.purple_eventloop_set_ui_ops(eventloop);

  this.ui_id = args.ui_id ? args.ui_id : 'NodejsPurple';

  if (!lib.purple_core_init(this.ui_id)) {
    throw new Error("libpurple failed to initialize");
  }

  if (args.custom_user_directory)
    lib.purple_util_set_user_dir(args.custom_user_directory);

  if (args.custom_plugin_path)
    lib.purple_plugins_add_search_path(args.custom_plugin_path);

  lib.purple_set_blist(lib.purple_blist_new());
  lib.purple_blist_load();
  lib.purple_prefs_load();

  if (args.plugin_save_pref)
    lib.purple_plugins_load_saved(args.plugin_save_pref ? 1 : 0);


  //lib.purple_pounces_load();

  Object.defineProperty(this, 'protocols', {
    get: function () {
      var ret = [];
      var list = lib.purple_plugins_get_protocols();
      var data;

      while (!list.isNull()) {
        list = list.deref();

        data = new Plugin(list.data.deref());

        ret.push(data.info);

        list = list.next;
      }
      return ret;
    }
  });

  Object.defineProperty(this, 'savedstatus', {
    set: function(value) {
      lib.purple_savedstatus_activate(value.instance);
    },
  });

  this.enableAccount = function (value) {
    lib.purple_account_set_enabled(value.instance, this.ui_id, 1);
  };

  this.disableAccount = function (value) {
    lib.purple_account_set_enabled(value.instance, this.ui_id, 0);
  };

  function write_conv(conv, who, alias, message, flags, time) {
    self.emit('write_conv',
      new Conversation(conv),
      who,
      alias,
      message,
      flags,
      new Date(time * 1000)
    );
  }

  var write_conv_ptr = FFI.Callback(ref.types.void, [
    ref.refType(ref.types.void),
    ref.types.CString,
    ref.types.CString,
    ref.types.CString,
    ref.types.uint32,
    ref.types.uint64,
  ], write_conv);

  function create_conversation(conv) {
    self.emit('create_conversation', new Conversation(conv));
  }

  var create_conversation_ptr = FFI.Callback(ref.types.void, [
    ref.refType(ref.types.void)
  ], create_conversation);

  function destroy_conversation(conv) {
    self.emit('destroy_conversation', new Conversation(conv));
  }

  var destroy_conversation_ptr = FFI.Callback(ref.types.void, [
    ref.refType(ref.types.void)
  ], destroy_conversation);

  var _on = this.on;
  this.on = function (event, cb) {
    switch (event) {
      case 'create_conversation':
        if (conv_ui_ops.create_conversation.isNull())
          conv_ui_ops.create_conversation = create_conversation_ptr;
        break;
      case 'destroy_conversation':
        if (conv_ui_ops.destroy_conversation.isNull())
          conv_ui_ops.destroy_conversation = destroy_conversation_ptr;
        break;
      case 'write_conv':
        if (conv_ui_ops.write_conv.isNull())
          conv_ui_ops.write_conv = write_conv_ptr;
        break;
      default:
        throw new Error("Unknown event " + event);
        break;
    }
    _on.call(self, event, cb);
  };
};
util.inherits(Purple, EventEmitter);

module.exports = Purple;
