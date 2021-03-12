import { throwError } from "../error";
import { ClarionDbCursor, ClarionDbManager, ClarionDbTrx } from "../interfaces";
import { MemoryHandler } from "./memory";

export class DatabaseHandler {
    memoryHandler: MemoryHandler;
    dbManager: ClarionDbManager;

    constructor(memoryHandler: MemoryHandler, dbManager: ClarionDbManager) {
        this.memoryHandler = memoryHandler;
        this.dbManager = dbManager;
    }

    openDb = async (
        pos: number,
        len: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const dbName = this.memoryHandler.decodeStr(pos, len);
            const db = await this.dbManager.open(dbName);
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(db)
            );
        } catch (e) {
            throwError(e);
        }
    };

    closeDatabase = async (dbIndex: number) => {
        const db = this.memoryHandler.getObj<any>(dbIndex);
        await this.dbManager.close(db);
    };

    createTransaction = (dbIndex: number, writable: boolean) => {
        const db = this.memoryHandler.getObj<any>(dbIndex);
        const trx = this.dbManager.createTransaction(db, writable);
        return this.memoryHandler.addObj(trx);
    };

    // TODO: give this an async interface?
    abortTransaction = (trxIndex: number) => {
        this.memoryHandler.getObj<ClarionDbTrx>(trxIndex).abort();
    };

    // TODO: give this an async interface?
    commitTransaction = (trxIndex: number) => {
        this.memoryHandler.getObj<ClarionDbTrx>(trxIndex).commit();
    };

    setKV = async (
        trxIndex: number,
        key: number,
        keyLen: number,
        value: number,
        valueLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const data = {
                key: new Uint8Array(this.memoryHandler.uint8Array(key, keyLen)),
                value: new Uint8Array(
                    this.memoryHandler.uint8Array(value, valueLen)
                ),
            };
            console.info("setKV", data);
            const trx = this.memoryHandler.getObj<ClarionDbTrx>(trxIndex);

            await trx.put(data.key, data.value);
            this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr);
        } catch (e) {
            throwError("Fail to put value", e);
        }
    };

    createCursor = async (
        trxIndex: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const trx = this.memoryHandler.getObj<ClarionDbTrx>(trxIndex);
            const cursor = await trx.createCursor();

            if (!cursor.hasValue()) {
                throw new Error("cursor has no value");
            }

            console.info("createCursor key", cursor.getKey());
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(cursor)
            );
        } catch (e) {
            throwError("fail to open cursor", e);
        }
    };

    // TODO: remove? Maybe createCursor and cursorNext should indicate this?
    cursorHasValue = (cursorIndex: number) => {
        return this.memoryHandler
            .getObj<ClarionDbCursor>(cursorIndex)
            .hasValue();
    };

    cursorValue = (cursorIndex: number) => {
        const value = this.memoryHandler
            .getObj<ClarionDbCursor>(cursorIndex)
            .getValue();
        console.info("cursorValue", value);
        return this.memoryHandler.addObj(value);
    };

    cursorNext = async (
        cursorIndex: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        const cursor = this.memoryHandler.getObj<ClarionDbCursor>(cursorIndex);
        if (!cursor.hasValue()) {
            throwError("cursor has no value");
        }

        try {
            await cursor.next();
            this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr);
        } catch (e) {
            throwError("fail to advance cursor to next position", e);
        }
    };

    closeCursor = (cursorIndex: number) => {
        this.memoryHandler.getObj<ClarionDbCursor>(cursorIndex).close();
    };

    imports = {
        openDb: this.openDb.bind(this),
        closeDatabase: this.closeDatabase.bind(this),
        createTransaction: this.createTransaction.bind(this),
        abortTransaction: this.abortTransaction.bind(this),
        commitTransaction: this.commitTransaction.bind(this),
        setKV: this.setKV.bind(this),
        createCursor: this.createCursor.bind(this),
        cursorHasValue: this.cursorHasValue.bind(this),
        cursorValue: this.cursorValue.bind(this),
        cursorNext: this.cursorNext.bind(this),
        closeCursor: this.closeCursor.bind(this),
    };
}
