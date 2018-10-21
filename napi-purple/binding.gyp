{
    "targets": [{
        "target_name": "module",
        "sources": [
            "./src/module.c",
            "./src/bindings/b_core.c",
            "./src/bindings/b_debug.c",
        ],
        # These are probably ultra-fragile
        'include_dirs': [
           '/usr/include/libpurple',
           "/usr/lib/purple-2/",
           '/usr/include/glib-2.0',
           '/usr/lib/x86_64-linux-gnu/glib-2.0/include',
           './src/',
           './src/bindings'
        ],
        "libraries": [
            "<!@(pkg-config --libs purple)",
        ]
    }]

}
