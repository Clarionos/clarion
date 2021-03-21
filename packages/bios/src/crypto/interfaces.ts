import { ec } from "elliptic";

export enum KeyType {
    k1 = 0,
    r1 = 1,
}

export interface KeyPair {
    type: KeyType;
    publicKey: Uint8Array;
    privateKey?: Uint8Array;
    ecKeyPair: ec.KeyPair;
}
