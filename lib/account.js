var lib = require('./libpurple');
var Connection = require('./connection');

var Account = function(name, prpl) {
  var self = this;

  if (name instanceof Buffer) {
    if (name.isNull())
      throw new Error("Account cannot be null");

    this.instance = name;
  } else {
    this.instance = lib.purple_account_new(name, prpl);
  }

  Object.defineProperty(this, 'username', {
    get: function () {
      return lib.purple_account_get_username(self.instance);
    },
  });

  Object.defineProperty(this, 'password', {
    set: function (value) {
      lib.purple_account_set_password(self.instance, value);
    },
  });

  Object.defineProperty(this, 'enabled', {
    set: function (value) {
      lib.purple_account_set_enabled(self.instance, value);
    },
  });

  Object.defineProperty(this, "alias", {
    get: function() {
      return lib.purple_account_get_alias(self.instance);
    }
  });

  Object.defineProperty(this, 'connection', {
    get: function () {
      return new Connection(lib.purple_account_get_connection(self.instance));
    },
  });

  this.setInt = function(key, value) {
      lib.purple_account_set_int(self.instance, key, value); 
  };
  this.setString = function(key, value) {
      lib.purple_account_set_string(self.instance, key, value); 
  };
  this.setBool = function(key, value) {
      lib.purple_account_set_bool(self.instance, key, value); 
  };
};

Account.find = function (name, prpl) {
  return new Account(lib.purple_accounts_find(name, prpl));
};

Account.all = function () {
  var ret = [];
  var list = lib.purple_accounts_get_all();

  while (!list.isNull()) {
    list = list.deref();

    ret.push(new Account(list.data));

    list = list.next;
  }

  return ret;
};

module.exports = Account;
