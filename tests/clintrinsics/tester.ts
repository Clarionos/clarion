import { Context } from "clwasmer";
import { readFileSync } from "fs";

const loadClarion = async (path: string) => {
    const context = new Context();
    try {
        const wasm = readFileSync(path);
        await context.instanciate(wasm);
        (context.instance!.exports as any)._start();
    } catch (e) {
        if (context.consoleBuf) console.log(context.consoleBuf);
        console.error(e);
    }
};

loadClarion("wasm/tests/clintrinsics/test-clintrinsics.wasm");
