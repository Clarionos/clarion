import { Context } from "@clarionos/bios";
import { ClarionServer } from "./connection";
import { DbManager } from "./db";

let clarionContext;
const server = new ClarionServer();
const dbManager = new DbManager();

export const initClarion = async (clarionWasm: Buffer) => {
    clarionContext = new Context(["wasm"], dbManager, server);
    await clarionContext.instanciate(clarionWasm);
    clarionContext.instance!.exports.initServer();
    clarionContext.instance!.exports.test();
    // (clarionContext.instance!.exports._start as Function)();
};

export const exitClarion = () => {
    dbManager.closeAll();
};
