import { throwError } from "../error";
import { MemoryHandler } from "./memory";

export class BasicHandler {
    memoryHandler: MemoryHandler;
    consoleBuf = "";

    constructor(memoryHandler: MemoryHandler) {
        this.memoryHandler = memoryHandler;
    }

    console = (pos: number, len: number) => {
        const s = this.consoleBuf + this.memoryHandler.decodeStr(pos, len);
        const l = s.split("\n");
        for (let i = 0; i < l.length - 1; ++i) console.log(l[i]);
        this.consoleBuf = l[l.length - 1];
    };

    getObjSize = (index: number) => {
        try {
            return this.memoryHandler.getObj<Uint8Array>(index).length;
        } catch (e) {
            throwError(e);
        }
    };

    getObjData = (index: number, dest: number) => {
        const memory = this.memoryHandler.instance!.exports
            .memory as WebAssembly.Memory;
        const buffer = new Uint8Array(memory.buffer);

        const obj = this.memoryHandler.getObj<Uint8Array>(index);
        for (const byte of obj) {
            buffer[dest++] = byte;
        }
    };

    releaseObject = (index: number) => {
        console.log("releaseObject", index, this.memoryHandler.objects[index]);
        this.memoryHandler.objects[index] = null;
    };

    callmeLater = (delayMs: number, wasmCbPtr: number, wasmCbIndex: number) => {
        setTimeout(
            () => this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr),
            delayMs
        );
    };

    exit = (code: number) => {
        throwError("exit: " + code);
    };

    imports = {
        console: this.console.bind(this),
        getObjSize: this.getObjSize.bind(this),
        getObjData: this.getObjData.bind(this),
        releaseObject: this.releaseObject.bind(this),
        callmeLater: this.callmeLater.bind(this),
        exit: this.exit.bind(this),
    };
}
