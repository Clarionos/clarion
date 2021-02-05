'use strict';

(async () => {
    console.log('b');

    function oops(s) {
        console.error('oops: ', s);
        throw new Error(s);
    }

    function decodeStr(pos, len) {
        return (new TextDecoder()).decode(new Uint8Array(instance.exports.memory.buffer, pos, len));
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

            // TODO: error handling
            // TODO: context argument
            open_db(name, len, p, f) {
                const req = indexedDB.open(decodeStr(name, len), 1);
                req.onupgradeneeded = e => req.result.createObjectStore('kv', { keyPath: 'k' });
                req.onsuccess = e => callback(f)(p, addObj(req.result));
            },
        },
    };

    const x = await WebAssembly.instantiateStreaming(fetch('a.wasm'), imports);
    module = x.module;
    instance = x.instance;
    console.log({ module, instance });
    instance.exports._start();
    if (consoleBuf)
        console.log(consoleBuf);
})();
