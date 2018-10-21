{
    "targets": [{
        "target_name": "module",
        "sources": [ "./src/module.c" ],
        # These are probably ultra-fragile
        'include_dirs': [
           '/usr/include/libpurple',
           '/usr/include/glib-2.0',
           '/usr/lib/x86_64-linux-gnu/glib-2.0/include'
        ],
        "libraries": [ "-lpurple" ]
    }]

}
