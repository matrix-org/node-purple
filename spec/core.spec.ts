import { expect } from "chai";
import { core } from "..";

// This breaks if we upgrade to a later version of purple, but that's quite rare. 
const MAJOR_LIBPURPLE_VERSION = "2.";

describe("Core", () => {
    it("can get the version of purple in use", () => {
        expect(core.get_version().startsWith(MAJOR_LIBPURPLE_VERSION));
    });
});