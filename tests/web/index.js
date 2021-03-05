'use strict';

(async () => {
    console.log('b');

    function oops(s) {
        console.error('oops: ', s);
        throw new Error(s);
    }

    function uint8Array(pos, len) {
        return new Uint8Array(instance.exports.memory.buffer, pos, len);
    }

    function decodeStr(pos, len) {
        return (new TextDecoder()).decode(uint8Array(pos, len));
    }

    let module, instance;
    let consoleBuf = '';
    let objects = [null];

    function addObj(obj) {
        objects.push(obj);
        return objects.length - 1;
    }

    function callback(f) {
        return instance.exports.__indirect_function_table.get(f);
    }

    const imports = {
        // TODO: error handling
        clarion: {
            exit(code) {
                oops('exit: ' + code);
            },

            console(pos, len) {
                const s = consoleBuf + decodeStr(pos, len);
                const l = s.split('\n');
                for (let i = 0; i < l.length - 1; ++i)
                    console.log(l[i]);
                consoleBuf = l[l.length - 1];
            },

            callme_later(delay_ms, p, f) {
                setTimeout(() => callback(f)(p), delay_ms);
            },

            release_object(i) {
                console.log('release_object', i, objects[i]);
                objects[i] = null;
            },

            // TODO: context argument
            open_db(name, len, p, f) {
                const req = indexedDB.open(decodeStr(name, len), 1);
                req.onupgradeneeded = e => req.result.createObjectStore('kv', { keyPath: 'k' });
                req.onsuccess = e => callback(f)(p, addObj(req.result));
            },

            // TODO: automatically abort transactions which aren't committed?
            // TODO: give this an async interface?
            create_transaction(db, writable) {
                const trx = objects[db].transaction('kv', writable ? 'readwrite' : 'readonly');
                const kv = trx.objectStore('kv');
                return addObj({ trx, kv });
            },

            // TODO: give this an async interface?
            abort_transaction(trx) {
                objects[trx].trx.abort();
            },

            // TODO: give this an async interface?
            commit_transaction(trx) {
                objects[trx].trx.commit();
            },

            set_kv(trx, key, keyLen, value, valueLen, p, f) {
                console.log('set_kv', { k: uint8Array(key, keyLen), v: uint8Array(value, valueLen) });
                const req = objects[trx].kv.put({ k: new Uint8Array(uint8Array(key, keyLen)), v: new Uint8Array(uint8Array(value, valueLen)) });
                req.onsuccess = e => callback(f)(p);
                req.onerror = e => console.error(e);
            },

            create_cursor(trx, p, f) {
                const req = objects[trx].kv.openCursor();
                req.onsuccess = e => callback(f)(p, addObj(req));
                req.onerror = e => console.error(e);
            },

            // TODO: remove? Maybe create_cursor and cursor_next should indicate this?
            cursor_has_value(cursor) {
                return objects[cursor].result ? true : false;
            },

            cursor_value(cursor) {
                const c = objects[cursor].result;
                if (!c)
                    oops('cursor has no value');
                return addObj(c.value.v);
            },

            cursor_next(cursor, p, f) {
                const req = objects[cursor];
                if (!req.result)
                    oops('cursor has no value');
                req.onsuccess = e => callback(f)(p);
                req.result.continue();
            }
        },
    };

    const x = await WebAssembly.instantiateStreaming(fetch('../../build/wasm/tests/web/a.wasm'), imports);
    module = x.module;
    instance = x.instance;
    console.log({ module, instance });
    instance.exports._start();
})();
