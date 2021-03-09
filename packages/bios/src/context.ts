import { ClarionDbAdapter, ClarionDbCursor, ClarionDbTrx } from "./interfaces";

export class Context {
    consoleBuf = "";
    instance?: WebAssembly.Instance;
    module?: WebAssembly.Module;
    objects: any[] = [null];
    args: string[];
    encodedArgs?: Uint8Array[];
    dbAdapter?: ClarionDbAdapter;

    constructor(args: string[], dbAdapter?: ClarionDbAdapter) {
        this.args = args;
        this.dbAdapter = dbAdapter;
    }

    throwError(message: string, e?: Error) {
        console.error(">>> Error:", message);
        if (e) {
            console.error(e);
        }
        throw new Error(message);
    }

    uint8Array(pos: number, len: number) {
        const memory = this.instance!.exports.memory as WebAssembly.Memory;
        return new Uint8Array(memory.buffer, pos, len);
    }

    uint32Array(pos: number, len: number) {
        const memory = this.instance.exports.memory as WebAssembly.Memory;
        return new Uint32Array(memory.buffer, pos, len);
    }

    decodeStr(pos: number, len: number) {
        return new TextDecoder().decode(this.uint8Array(pos, len));
    }

    addObj(obj: any) {
        this.objects.push(obj);
        return this.objects.length - 1;
    }

    getObj<T>(index: number) {
        return this.objects[index] as T;
    }

    wasmCallback(fnIndex: number, ...params: any): any {
        const fnTable = this.instance.exports
            .__indirect_function_table as WebAssembly.Table;
        const fn = fnTable.get(fnIndex);
        if (!fn) {
            return this.throwError("Invalid WASM table function");
        }
        return fn(...params);
    }

    imports: any = (() => {
        const context = this;
        return {
            clarion: {
                exit(code: number) {
                    context.throwError("exit: " + code);
                },

                console(pos: number, len: number) {
                    const s = context.consoleBuf + context.decodeStr(pos, len);
                    const l = s.split("\n");
                    for (let i = 0; i < l.length - 1; ++i) console.log(l[i]);
                    context.consoleBuf = l[l.length - 1];
                },

                get_arg_counts(argc: number, argv_buf_size: number) {
                    if (!context.encodedArgs) {
                        const encoder = new TextEncoder();
                        context.encodedArgs = context.args.map((s) =>
                            encoder.encode(s)
                        );
                    }
                    let size = 0;
                    for (const a of context.encodedArgs) size += a.length + 1;
                    context.uint32Array(argc, 1)[0] =
                        context.encodedArgs.length;
                    context.uint32Array(argv_buf_size, 1)[0] = size;
                },

                get_args(argv: number, argv_buf: number) {
                    const memory = context.instance!.exports
                        .memory as WebAssembly.Memory;
                    const u8 = new Uint8Array(memory.buffer);
                    const u32 = context.uint32Array(
                        argv,
                        context.encodedArgs!.length
                    );
                    argv = 0;
                    for (const a of context.encodedArgs!) {
                        u32[argv++] = argv_buf;
                        for (const ch of a) u8[argv_buf++] = ch;
                        u8[argv_buf++] = 0;
                    }
                },

                callme_later(
                    delayMs: number,
                    wasmCbPtr: any,
                    wasmCbIndex: number
                ) {
                    setTimeout(
                        () => context.wasmCallback(wasmCbIndex, wasmCbPtr),
                        delayMs
                    );
                },

                release_object(index: number) {
                    console.log(
                        "release_object",
                        index,
                        context.objects[index]
                    );
                    context.objects[index] = null;
                },

                // TODO: context argument
                open_db: async (
                    pos: number,
                    len: number,
                    wasmCbPtr: number,
                    wasmCbIndex: number
                ) => {
                    try {
                        const dbName = context.decodeStr(pos, len);
                        const db = await context.dbAdapter.open(dbName);
                        context.wasmCallback(
                            wasmCbIndex,
                            wasmCbPtr,
                            context.addObj(db)
                        );
                    } catch (e) {
                        context.throwError(e);
                    }
                },

                // TODO: automatically abort transactions which aren't committed?
                // TODO: give this an async interface?
                create_transaction(dbIndex: number, writable: boolean) {
                    const db = context.objects[dbIndex] as ClarionDbAdapter;
                    const trx = context.dbAdapter!.createTransaction(
                        db,
                        writable
                    );
                    return context.addObj(trx);
                },

                // TODO: give this an async interface?
                abort_transaction(trxIndex: number) {
                    context.getObj<ClarionDbTrx>(trxIndex).abort();
                },

                // TODO: give this an async interface?
                commit_transaction(trxIndex: number) {
                    context.getObj<ClarionDbTrx>(trxIndex).commit();
                },

                set_kv: async (
                    trxIndex: number,
                    key: number,
                    keyLen: number,
                    value: number,
                    valueLen: number,
                    wasmCbPtr: any,
                    wasmCbIndex: number
                ) => {
                    const data = {
                        key: new Uint8Array(context.uint8Array(key, keyLen)),
                        value: new Uint8Array(
                            context.uint8Array(value, valueLen)
                        ),
                    };
                    console.log("set_kv", data);
                    context
                        .getObj<ClarionDbTrx>(trxIndex)
                        .put(data.key, data.value);
                    context.wasmCallback(wasmCbIndex, wasmCbPtr);

                    try {
                        const data = {
                            key: new Uint8Array(
                                context.uint8Array(key, keyLen)
                            ),
                            value: new Uint8Array(
                                context.uint8Array(value, valueLen)
                            ),
                        };
                        console.info("set_kv", data);
                        const trx = context.getObj<ClarionDbTrx>(trxIndex);

                        await trx.put(data.key, data.value);
                        context.wasmCallback(wasmCbIndex, wasmCbPtr);
                    } catch (e) {
                        context.throwError("Fail to put value", e);
                    }
                },

                create_cursor: async (
                    trxIndex: number,
                    wasmCbPtr: any,
                    wasmCbIndex: number
                ) => {
                    try {
                        const trx = context.getObj<ClarionDbTrx>(trxIndex);
                        const cursor = await trx.createCursor();

                        if (!cursor.hasValue()) {
                            throw new Error("cursor has no value");
                        }

                        console.info("create_cursor key", cursor.getKey());
                        context.wasmCallback(
                            wasmCbIndex,
                            wasmCbPtr,
                            context.addObj(cursor)
                        );
                    } catch (e) {
                        context.throwError("fail to open cursor", e);
                    }
                },

                // TODO: remove? Maybe create_cursor and cursor_next should indicate this?
                cursor_has_value(cursorIndex: number) {
                    console.info(
                        "cursor_has_value",
                        context.getObj<ClarionDbCursor>(cursorIndex).hasValue()
                    );
                    return context
                        .getObj<ClarionDbCursor>(cursorIndex)
                        .hasValue();
                },

                cursor_value(cursorIndex: number) {
                    const value = context
                        .getObj<ClarionDbCursor>(cursorIndex)
                        .getValue();
                    console.info("cursor_value", value);
                    return context.addObj(value);
                },

                cursor_next: async (
                    cursorIndex: number,
                    wasmCbPtr: any,
                    wasmCbIndex: number
                ) => {
                    const cursor = context.getObj<ClarionDbCursor>(cursorIndex);
                    if (!cursor.hasValue()) {
                        context.throwError("cursor has no value");
                    }

                    try {
                        await cursor.next();
                        context.wasmCallback(wasmCbIndex, wasmCbPtr);
                    } catch (e) {
                        context.throwError(
                            "fail to advance cursor to next position",
                            e
                        );
                    }
                },
            },
        };
    })(); // imports

    async instanciate(wasmBytes: BufferSource) {
        this.module = await WebAssembly.compile(wasmBytes);
        this.instance = await WebAssembly.instantiate(
            this.module,
            this.imports
        );
    }
} // Context
