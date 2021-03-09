import { Context } from "clwasmer";
import { readFileSync } from "fs";

const runWasm = async (path: string, args: string[]) => {
    const context = new Context(args);
    try {
        const wasm = readFileSync(path);
        await context.instanciate(wasm);
        (context.instance!.exports as any)._start();
    } catch (e) {
        if (context.consoleBuf) console.log(context.consoleBuf);
        console.error(e);
        process.exit(1);
    }
};

if (process.argv.length < 3) {
    console.error(
        `Usage: ${process.argv[0]} ${process.argv[1]} wasm_path wasm_args...`
    );
    process.exit(1);
} else {
    runWasm(process.argv[2], process.argv.slice(2));
}
