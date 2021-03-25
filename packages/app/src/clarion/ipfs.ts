import { importer } from "ipfs-unixfs-importer";
import IPLD from "ipld";
import ipldInMemory from "ipld-in-memory";

// options object to be passed to the IPLD constructor -
// this module sets the 'blockService' property in the options object
// so any passed value for that key will be ignored
// import pull, { once, collect } from "pull-stream";

async function* asAsyncIterable(arr: Uint8Array) {
    yield* [arr];
}

const ipfsQhashFile = async (fileName: string, bytes: Uint8Array) => {
    const ipldOpts = {};
    const ipld = await ipldInMemory(IPLD, ipldOpts);
    const source = {
        path: fileName,
        content: asAsyncIterable(bytes), //AsyncIterable<Uint8Array>
    };

    for await (const entry of (importer as any)(source, ipld)) {
        console.info(">>> entry", entry);
    }

    // console.info(x);

    // pull(
    //     x,
    //     collect((err, files) => {
    //         console.info(err, files);
    //     })
    // );
};

// import IPFS from "ipfs-core";

export const storeFile = async (file: File) => {
    console.info("file", file);
    const bytes = new Uint8Array(await file.arrayBuffer());
    ipfsQhashFile(file.name, bytes);
    // const buffer = await file.arrayBuffer();
    // const ipfs = await IPFS.create();
    // const { cid } = await IPFS.files.add(file, { onlyHash: true });
    // console.info("hashed ipfs file", cid);
    return {};
};
