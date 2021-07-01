import { expect } from "chai";
import { plugins } from "..";
import { setupPurpleEnv, tearDownPurpleEnv } from "./helper";

describe("Plugins", () => {
    let dir;
    beforeEach(async () => {
        dir = await setupPurpleEnv();
    })
    afterEach(() => {
        tearDownPurpleEnv(dir);
    });
    it("can get a list of supported protocols", () => {
        const protocols = plugins.get_protocols();
        expect(protocols).has.length.greaterThan(0);
    });
});