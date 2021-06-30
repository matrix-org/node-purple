// Type definitions for [~THE LIBRARY NAME~] [~OPTIONAL VERSION NUMBER~]
// Project: [~THE PROJECT NAME~]
// Definitions by: [~YOUR NAME~] <[~A URL FOR YOU~]>

/*~ This is the module template file. You should rename it to index.d.ts
 *~ and place it in a folder with the same name as the module.
 *~ For example, if you were writing a file for "super-greeter", this
 *~ file should be 'super-greeter/index.d.ts'
 */

/*~ If this module is a UMD module that exposes a global variable 'myLib' when
 *~ loaded outside a module loader environment, declare that global here.
 *~ Otherwise, delete this declaration.
 */

export as namespace libpurple;

/* Types */
export type Event = {
    eventName: string;
}

export type SetupArgs = {
    debugEnabled: number;
    userDir?: string;
    pluginDir?: string;
}

export type Account = {
    handle: External;
    username: string;
    protocol_id: string;
    password?: string;
    user_info?: string;
    buddy_icon_path?: string;
}

export type Protocol = {
    name: string;
    id: string;
    homepage?: string;
    summary?: string;
}

export type StatusType = {
    id: string;
    name: string;
    saveable: boolean;
    user_settable: boolean;
    independent: boolean;
};

export type Buddy = {
     name: string;
     icon_path?: string;
     nick?: string;
};

export type Conversation = {
    handle: External;
    name: string;
}

/* Sub-modules */
export class core {
    /**
     * Get the version of purple in use.
     */
    static get_version(): string;
    static init();
    static quit();
}

export class debug {
    // Nothing yet
}

export class helper {
    /**
     * Configure purple to start without any UI features. A configuration object
     * should be provided.
     * @param opts 
     */
    static setupPurple(opts: SetupArgs);
    static pollEvents(): Event[];
}

export class plugins {
    static get_protocols(): Protocol[];
}

export class accounts {
    static new(name: string, pluginId: string, password?: string): Account;
    /**
     * Configure an account with extra options.
     * @param handle The account external Handle.
     * @param config A configuration object. Numbers must be integers
     * @throws If the configuration object contains invalid types, or if the handle is invalid.
     */
    static configure(handle: External, config: Record<string, string|boolean|number>);
    static find(name: string, pluginId: string): Account;
    static get_enabled(handle: External): boolean;
    static set_enabled(handle: External, enable: boolean);
    static connect(handle: External);
    static disconnect(handle: External);
    static is_connected(handle: External): boolean;
    static is_connecting(handle: External): boolean;
    static is_disconnected(handle: External): boolean;
    static is_disconnected(handle: External): boolean;
    static get_status_types(handle: External): StatusType[];
    static set_status(handle: External, statusId: string, active: boolean);
}

export class messaging {
    static sendIM(handle: External, name: string, body: string);
    static sendChat(handle: External, name: string, body: string);
    static chatParams(handle: External, protocol: string);
    static joinChat(handle: External, components: {[key: string]:string;});
    static rejectChat(handle: External, components: {[key: string]:string;});
    static getBuddyFromConv(handle: External, buddyName: string);
    static getNickForChat(chatHandle: External);
    static findConversation(acct: External, name: string): Conversation;
}

export class notify {
    static get_user_info(handle: External, who: string);
}

export class buddy {
    /**
     * Find a buddy in the buddylist.
     * @param  handle Account Handle
     * @param  name   [description]
     * @return        [description]
     */
    static find(handle: External, name: string): Buddy;
}
