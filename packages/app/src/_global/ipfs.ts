import { importer } from "ipfs-unixfs-importer";
import IPLD from "ipld";
import ipldInMemory from "ipld-in-memory";

// thanks to https://github.com/alanshaw/ipfs-only-hash :)
const ipfsQhashFile = async (content: Uint8Array) => {
    const ipld = await ipldInMemory(IPLD);

    const ipldEntries = [];
    let hash = "";
    for await (const entry of importer([{ content }] as any, ipld, {
        onlyHash: true,
    })) {
        console.info(">>> ipfs entry", `${entry.cid}`, entry);
        ipldEntries.push(entry);
        hash = `${entry.cid}`;
    }

    // last hash is the one that matters
    return { hash, ipldEntries };
};

export const storeFile = async (file: File) => {
    const fileContent = new Uint8Array(await file.arrayBuffer());
    return ipfsQhashFile(fileContent);
};
