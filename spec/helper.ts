import { core, helper } from "..";
import { mkdtemp, rm } from "fs/promises";
import { join } from "path";
import { tmpdir } from "os";

const EXPECTED_PLUGIN_DIR = "/usr/lib/purple-2";

export async function setupPurpleEnv() {
    const dir = await mkdtemp(join(tmpdir(), 'node-purple-test-'));
    helper.setupPurple({
        userDir: dir,
        pluginDir: EXPECTED_PLUGIN_DIR,
        debugEnabled: process.env.PURPLE_LOGGING ? 1 : 0,
    });
    return dir;
}

export function tearDownPurpleEnv(dir) {
    core.quit();
    rm(dir, { recursive: true, force: true });
}