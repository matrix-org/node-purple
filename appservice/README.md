Running
=======


Architecture
============

```
                   config[Account]
                        |
                        V
 Matrix ---ASAPI---> AppService ---libpurple--> Remote Network
```

Flows:
  Matrix-initiated:
   - Matrix user invites `@network_$USERNAME:domain` to a room.
   - Matrix user says something in this room.
   - AppService forwards message to remote user.
   Replies:
    - Remote user responds to message.
    - AppService forwards to matrix room. (which? All of them if >1 matrix user tried to speak to this remote user? Only the latest user (clobbering?))

  Remote network initiated:
   - TODO
   Replies:
    - TODO

Shortcomings:
 - Account is global not per user!
 - Means you can't have >1 matrix user talk to remote user! Doesn't know which
   room to respond to.
