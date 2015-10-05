var Purple = require('./purple');

var p = new Purple({
  //debug: true,
});

p.protocols.forEach(function(prpl) {
  console.log(prpl.id, prpl.name);
});

// This has to happen AFTER instantiating Purple
var Status = require('./lib/status');
var Account = require('./lib/account');

var account = new Account('someluser', 'prpl-aim');
account.password = 'somepassword';

p.enableAccount(account);

var s = new Status('Testing App', Status.AVAILABLE);
p.savedstatus = s;

p.on('create_conversation', function (conversation) {
  console.log("we have a new conversation", conversation.title, conversation.name);
});

p.on('write_conv', function (conv, who, alias, message, flags, time) {
  if (alias != 'someluser' && who != 'someluser') {
    conv.send('you said "' + message + '"');
  }
});
