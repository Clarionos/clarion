import { Context } from "@clarionos/bios";

const wasmFilePath = "/clarion.wasm";

const DATABASE_OBJECTSTORE = "clarion";
const DATABASE_KEYPATH = "k";
const DATABASE_VALUEPATH = "v";

class ClarionCursor {
  cursor: any;

  constructor(objectStore, callback) {
    const req = objectStore.openCursor();
    this.cursor = req;
    req.onsuccess = () => callback(undefined, this);
    req.onerror = (e) => callback(e);
  }

  getKey() {
    return this.cursor.result.key;
  }

  getValue() {
    return this.cursor.result.value[DATABASE_VALUEPATH];
  }

  hasValue() {
    return this.cursor.result ? true : false;
  }

  next(callback) {
    this.cursor.onsuccess = () => callback(undefined);
    this.cursor.onerror = (e) => callback(e);
    this.cursor.result.continue();
  }
}

class ClarionTrx {
  trx: any;
  objectStore: any;

  constructor(db, writable) {
    console.info(db, writable);
    this.trx = db.transaction(
      DATABASE_OBJECTSTORE,
      writable ? "readwrite" : "readonly"
    );
    this.objectStore = this.trx.objectStore(DATABASE_OBJECTSTORE);
  }

  abort() {
    this.trx.abort();
  }
  commit() {
    this.trx.commit();
  }

  put(key, value, callback) {
    const req = this.objectStore.put({
      [DATABASE_KEYPATH]: key,
      [DATABASE_VALUEPATH]: value,
    });
    req.onsuccess = () => callback(undefined);
    req.onerror = (e) => callback(e);
  }

  delete(key, callback) {
    const req = this.objectStore.delete(key);
    req.onsuccess = () => callback(undefined);
    req.onerror = (e) => callback(e);
  }

  get(key, callback) {
    const req = this.objectStore.get(key);
    req.onsuccess = () => callback(undefined, req.result[DATABASE_VALUEPATH]);
    req.onerror = (e) => callback(e);
  }

  createCursor(callback) {
    return new ClarionCursor(this.objectStore, callback);
  }
}

class ClarionDb {
  open(name, callback) {
    const req = indexedDB.open(name, 1);
    req.onupgradeneeded = (e) =>
      req.result.createObjectStore(DATABASE_OBJECTSTORE, {
        keyPath: DATABASE_KEYPATH,
      });
    req.onsuccess = (e) => callback(undefined, req.result);
  }

  createTransaction(db, writable = false) {
    return new ClarionTrx(db, writable);
  }
}

const init = async (): Promise<void> => {
  const response = await fetch(wasmFilePath);
  const wasmBytes = new Uint8Array(await response.arrayBuffer());

  const clarionDb = new ClarionDb();
  const context = new Context(["clarion.wasm"], clarionDb as any);

  await context.instanciate(wasmBytes);
  (context.instance!.exports._start as Function)();
};

init();
