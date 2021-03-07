export class Context {
    consoleBuf = "";
    instance?: WebAssembly.Instance;
    module?: WebAssembly.Module;
    objects: any[] = [null];
    db: any = null; // todo: create a clariondb wrapper

    oops(s: any) {
        console.error("oops: ", s);
        throw new Error(s);
    }

    uint8Array(pos: number, len: number) {
        const memory = this.instance!.exports.memory as WebAssembly.Memory;
        return new Uint8Array(memory.buffer, pos, len);
    }

    decodeStr(pos: number, len: number) {
        return new TextDecoder().decode(this.uint8Array(pos, len));
    }

    addObj(obj: any) {
        this.objects.push(obj);
        return this.objects.length - 1;
    }

    wasmCallback(fnIndex: number, ...params: any): any {
        const fnTable = this.instance!.exports
            .__indirect_function_table as WebAssembly.Table;
        return fnTable.get(fnIndex)!(...params);
    }

    imports: any = (() => {
        const context = this;
        return {
            clarion: {
                exit(code: number) {
                    context.oops("exit: " + code);
                },

                console(pos: number, len: number) {
                    const s = context.consoleBuf + context.decodeStr(pos, len);
                    const l = s.split("\n");
                    for (let i = 0; i < l.length - 1; ++i) console.log(l[i]);
                    context.consoleBuf = l[l.length - 1];
                },

                callme_later(delay_ms: number, param: any, fnIndex: number) {
                    setTimeout(
                        () => context.wasmCallback(fnIndex, param),
                        delay_ms
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
                open_db(pos: number, len: number, param: any, fnIndex: number) {
                    const dbName = context.decodeStr(pos, len);

                    const req = context.db.open(dbName, 1);

                    req.onupgradeneeded = (_e: any) =>
                        req.result.createObjectStore("kv", { keyPath: "k" });
                    req.onsuccess = (_e: any) =>
                        context.wasmCallback(
                            fnIndex,
                            param,
                            context.addObj(req.result)
                        );

                    req.onupgradeneeded = (_e: any) =>
                        req.result.createObjectStore("kv", { keyPath: "k" });
                    req.onsuccess = (_e: any) =>
                        context.wasmCallback(
                            fnIndex,
                            param,
                            context.addObj(req.result)
                        );
                },

                // TODO: automatically abort transactions which aren't committed?
                // TODO: give this an async interface?
                create_transaction(dbIndex: number, writable: boolean) {
                    const trx = context.objects[dbIndex].transaction(
                        "kv",
                        writable ? "readwrite" : "readonly"
                    );
                    const kv = trx.objectStore("kv");
                    return context.addObj({ trx, kv });
                },

                // TODO: give this an async interface?
                abort_transaction(trxIndex: number) {
                    context.objects[trxIndex].trx.abort();
                },

                // TODO: give this an async interface?
                commit_transaction(trxIndex: number) {
                    context.objects[trxIndex].trx.commit();
                },

                set_kv(
                    trxIndex: number,
                    key: number,
                    keyLen: number,
                    value: number,
                    valueLen: number,
                    params: any,
                    fnIndex: number
                ) {
                    console.log("set_kv", {
                        k: context.uint8Array(key, keyLen),
                        v: context.uint8Array(value, valueLen),
                    });
                    const req = context.objects[trxIndex].kv.put({
                        k: new Uint8Array(context.uint8Array(key, keyLen)),
                        v: new Uint8Array(context.uint8Array(value, valueLen)),
                    });
                    req.onsuccess = (_e: any) =>
                        context.wasmCallback(fnIndex, params);
                    req.onerror = (e: any) => console.error(e);
                },

                create_cursor(trxIndex: number, params: any, fnIndex: number) {
                    const req = context.objects[trxIndex].kv.openCursor();
                    req.onsuccess = (_e: any) =>
                        context.wasmCallback(
                            fnIndex,
                            params,
                            context.addObj(req)
                        );
                    req.onerror = (e: any) => console.error(e);
                },

                // TODO: remove? Maybe create_cursor and cursor_next should indicate this?
                cursor_has_value(cursorIndex: number) {
                    return context.objects[cursorIndex].result ? true : false;
                },

                cursor_value(cursorIndex: number) {
                    const c = context.objects[cursorIndex].result;
                    if (!c) context.oops("cursor has no value");
                    return context.addObj(c.value.v);
                },

                cursor_next(cursorIndex: number, params: any, fnIndex: number) {
                    const req = context.objects[cursorIndex];
                    if (!req.result) context.oops("cursor has no value");
                    req.onsuccess = (_e: any) =>
                        context.wasmCallback(fnIndex, params);
                    req.result.continue();
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
