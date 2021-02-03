'use strict';

(async () => {
    console.log('b');

    function oops(s) {
        console.error('oops: ', s);
        throw new Error(s);
    }

    let module, instance;
    let consoleBuf = '';
    const imports = {
        clarion: {
            exit(code) {
                oops('exit: ' + code);
            },

            console(pos, len) {
                const s = consoleBuf + (new TextDecoder()).decode(new Uint8Array(instance.exports.memory.buffer, pos, len));
                const l = s.split('\n');
                for (let i = 0; i < l.length - 1; ++i)
                    console.log(l[i]);
                consoleBuf = l[l.length - 1];
            },
        },
    };

    const x = await WebAssembly.instantiateStreaming(fetch('a.wasm'), imports);
    module = x.module;
    instance = x.instance;
    console.log({ module, instance });
    instance.exports._start();
    console.log(consoleBuf);
})();
