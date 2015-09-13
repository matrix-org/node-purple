var FFI = require('ffi');
var ref = require('ref');

var uv = require('./uv');
var types = require('./types');

var _timeouts = {};
var _timeoutid = 0;

function handle_timeout(timeout) {
  if (timeout.func(timeout.data))
    timeout.handle = setTimeout(handle_timeout, timeout.interval, timeout);
};

function timeout_add(interval, cb, data) {
  _timeoutid += 1;
  while (_timeouts[_timeoutid])
    _timeoutid += 1; //todo handle overflow (max uint32)

  _timeouts[_timeoutid] = {
    id: _timeoutid,
    interval: interval,
    func: FFI.ForeignFunction(cb, ref.types.uint32, ['pointer']),
    data: data,
  }
  // use setTimeout so common intervals interleave
  _timeouts[_timeoutid].handle = setTimeout(handle_timeout, interval, _timeouts[_timeoutid]);

  return _timeoutid;
}

var timeout_add_fptr = FFI.Callback(ref.types.uint32, [
    ref.types.uint32,
    ref.refType(ref.types.void),
    ref.refType(ref.types.void),
  ], timeout_add);

function timeout_remove(handle) {
  var timeout = _timeouts[handle];
  if (timeout) {
    clearTimeout(timeout.handle);
    delete _timeouts[handle];
    return 1;
  } else {
    return 0;
  }
}

var timeout_remove_fptr = FFI.Callback(ref.types.uint32, [ ref.types.uint32 ], timeout_remove);

var _inputs_by_id = {};
var _inputs_by_poll = {};
var _handles_by_fd = {};

var uv_poll_size = uv.uv_handle_size(8);
var default_loop = uv.uv_default_loop();

function handle_input(poll_t, stat, events) {
  var handle = _inputs_by_poll[poll_t.address()].handle;
  //console.log("handle input on fd " + handle.fd + " with stat=" + stat + " and events=" + events);
  //console.log("readers=" + JSON.stringify(handle.readers) + ", writers=" + JSON.stringify(handle.writers));
  if (handle) {
    if (events & 1) {
      Object.keys(handle.readers).forEach( function(id) {
        var reader = handle.readers[id];
        //console.log("calling back reader " + JSON.stringify(reader));
        reader.func(reader.data, handle.fd, events);
      });
    }
    if (events & 2) {
      Object.keys(handle.writers).forEach( function(id) {
        var writer = handle.writers[id];
        //console.log("calling back writer " + JSON.stringify(writer));
        writer.func(writer.data, handle.fd, events);
      });
    }
  }
};
var handle_input_fptr = FFI.Callback(ref.types.void, ['pointer', ref.types.int32, ref.types.int32], handle_input);

function input_add(fd, cond, func, data) {
  //console.log("input_add fd=" + fd + " cond=" + cond);

  var handle;

  // create or get the handle for this FD that tracks our various callbacks
  var newFd;
  if (_handles_by_fd[fd]) {
    newFd = false;
    handle = _handles_by_fd[fd];
  }
  else {
    newFd = true;
    handle = {
      fd: fd,
      readers: {},
      writers: {},
    };
    handle.poll_t = new Buffer(uv_poll_size);
    handle.poll_t.fill(0);
  }

  // allocate a new input id to return to libpurple
  var inputid = 1;
  while (_inputs_by_id[inputid]) inputid++;

  //console.log("newFd=" + newFd + " and inputid=" + inputid);

  var ffunc = FFI.ForeignFunction(func, ref.types.void, ['pointer', ref.types.int32, ref.types.uint32]);
  if (cond & 1) {
      handle.readers[inputid] = {
        func: ffunc,
        data: data,
      };
  }
  if (cond & 2) {
      handle.writers[inputid] = {
        func: ffunc,
        data: data,
      };
  }

  var input = {
    handle: handle,
    cond: cond,
  };

  _inputs_by_id[inputid] = input;
  _inputs_by_poll[handle.poll_t.address()] = input;
  _handles_by_fd[fd] = handle;

  if (newFd) {
    // only call uv_poll_init if this is a new FD
    // as it's legal for libpurple to call input_add multiple times on the same FD
    // to add both readers and writers.  However, if you call uv_poll_init
    // twice on the same FD (without first calling input_remove), UV
    // crashes with:
    // Assertion failed: (loop->watchers[w->fd] == w), function uv__io_stop
    
    //console.log("uv_poll_init fd=" + fd);
    uv.uv_poll_init(default_loop, handle.poll_t, fd);
  }

  // XXX: this feels a bit evil that we're relying on the fact that UV's uv_poll_event
  // enum has 1=readable, 2=writable, which coincidentally matches libpurple's PurpleInputCondition...
  
  //console.log("uv_poll_start fd=" + fd + " and inputid="+inputid);
  uv.uv_poll_start(handle.poll_t, cond, handle_input_fptr);

  return inputid;
}

var input_add_fptr = FFI.Callback(ref.types.uint32, [
    ref.types.int32,
    ref.types.uint32,
    ref.refType(ref.types.void),
    ref.refType(ref.types.void),
  ], input_add);

function input_remove(id) {
  var input = _inputs_by_id[id];
  //console.log("input_remove id " + id + " fd=" + input.handle.fd);
  if (input) {
    delete _inputs_by_id[id];

    if (input.cond & 1) {
      delete input.handle.readers[id];
    }
    if (input.cond & 2) {
      delete input.handle.writers[id];
    }

    if (!Object.keys(input.handle.readers).length &&
        !Object.keys(input.handle.writers).length)
    {
      uv.uv_poll_stop(input.handle.poll_t);
      delete _inputs_by_poll[input.handle.poll_t.address()];
      delete _handles_by_fd[input.handle.fd];
    }
    else {
      var cond = (Object.keys(input.handle.readers).length ? 1 : 0) |
                 (Object.keys(input.handle.writers).length ? 2 : 0);
      //console.log("reissuing uv_poll_start fd=" + input.handle.fd + " and cond="+cond);
      uv.uv_poll_start(input.handle.poll_t, cond, handle_input_fptr);
    }
    return 1;
  } else {
    return 0;
  }
}

var input_remove_fptr = FFI.Callback(ref.types.uint32, [ ref.types.uint32 ], input_remove);

var event_loop_ui = new types.PurpleEventLoopUiOps();
event_loop_ui.ref().fill(0);
event_loop_ui.timeout_add = timeout_add_fptr;
event_loop_ui.timeout_remove = timeout_remove_fptr;
event_loop_ui.input_add = input_add_fptr;
event_loop_ui.input_remove = input_remove_fptr;
event_loop_ui = event_loop_ui.ref();

module.exports = event_loop_ui;
