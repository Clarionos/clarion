import { throwError } from "../error";

export class MemoryHandler {
    instance: WebAssembly.Instance | undefined;
    objects: any[] = [null];

    constructor() {}

    setInstance = (instance: WebAssembly.Instance) => {
        this.instance = instance;
    };

    getMemory = () => {
        return this.instance.exports.memory as WebAssembly.Memory;
    };

    uint8Array = (pos: number, len: number) => {
        return new Uint8Array(this.getMemory().buffer, pos, len);
    };

    uint32Array = (pos: number, len: number) => {
        return new Uint32Array(this.getMemory().buffer, pos, len);
    };

    decodeStr = (pos: number, len: number) => {
        const data = this.uint8Array(pos, len);
        return new TextDecoder().decode(data);
    };

    addObj = (obj: any) => {
        this.objects.push(obj);
        return this.objects.length - 1;
    };

    getObj = <T>(index: number) => {
        return this.objects[index] as T;
    };

    addString = (str: string) => {
        const data = new TextEncoder().encode(str);
        return this.addObj(data);
    };

    wasmCallback = (fnIndex: number, ...params: any): void => {
        const fnTable = this.instance.exports
            .__indirect_function_table as WebAssembly.Table;
        const fn = fnTable.get(fnIndex);
        if (!fn) {
            return throwError("Invalid WASM table function");
        }
        return fn(...params);
    };
}
