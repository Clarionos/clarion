let consoleBuf = "";
let instance: WebAssembly.Instance;
let objects: any[] = [null];

export interface ClarionDbTrx {
  commit: () => void;
  abort: () => void;
  put: (key: Uint8Array, value: Uint8Array) => void;
  delete: (key: Uint8Array) => void;
  get: (key: Uint8Array) => Uint8Array;
  createCursor: () => any;
}
export interface ClarionDbAdapter {
  open: (name: string, callback: (error?: Error, db?: any) => void) => void;
  close: (db: any) => void;
  createTransaction: (db: any, writable?: boolean) => ClarionDbTrx;
}

let dbAdapter: ClarionDbAdapter;

const throwError = (message: string) => {
  console.error(">>> Error:", message);
  throw new Error(message);
};

const uint8Array = (pos: number, len: number) => {
  const memory = instance.exports.memory as WebAssembly.Memory;
  return new Uint8Array(memory.buffer, pos, len);
};

const decodeStr = (pos: number, len: number) => {
  const data = uint8Array(pos, len);
  return new TextDecoder().decode(data);
};

const addObj = (obj: any) => {
  objects.push(obj);
  return objects.length - 1;
};

const getObj = <T>(index: number): T => {
  return objects[index] as T;
};

const wasmCallback = (fnIndex: number, ...params: any[]): Function => {
  const fnTable = instance.exports
    .__indirect_function_table as WebAssembly.Table;
  const fn = fnTable.get(fnIndex);
  if (!fn) {
    return throwError("Invalid WASM table function");
  }
  return fn(...params);
};

const clarion = {
  exit(code: number) {
    throwError("exit: " + code);
  },

  console(pos: number, len: number) {
    const s = consoleBuf + decodeStr(pos, len);
    const l = s.split("\n");
    for (let i = 0; i < l.length - 1; ++i) console.log(l[i]);
    consoleBuf = l[l.length - 1];
  },

  callme_later(delayMs: number, wasmCbPtr: any, wasmCbIndex: number) {
    setTimeout(() => wasmCallback(wasmCbIndex, wasmCbPtr), delayMs);
  },

  release_object(index: number) {
    console.log("release_object", index, objects[index]);
    objects[index] = null;
  },

  // TODO: context argument
  open_db(pos: number, len: number, wasmCbPtr: any, wasmCbIndex: number) {
    const dbName = decodeStr(pos, len);

    dbAdapter.open(dbName, (error, db) => {
      if (error) throw error;
      wasmCallback(wasmCbIndex, wasmCbPtr, addObj(db));
    });
  },

  // TODO: automatically abort transactions which aren't committed?
  // TODO: give this an async interface?
  create_transaction(dbIndex: number, writable: boolean) {
    const db = objects[dbIndex] as ClarionDbAdapter;
    const trx = dbAdapter.createTransaction(db, writable);
    return addObj(trx);
  },

  // TODO: give this an async interface?
  abort_transaction(trxIndex: number) {
    getObj<ClarionDbTrx>(trxIndex).abort();
  },

  // TODO: give this an async interface?
  commit_transaction(trxIndex: number) {
    getObj<ClarionDbTrx>(trxIndex).commit();
  },

  set_kv(
    trxIndex: number,
    key: number,
    keyLen: number,
    value: number,
    valueLen: number,
    wasmCbPtr: any,
    wasmCbIndex: number
  ) {
    const data = {
      key: new Uint8Array(uint8Array(key, keyLen)),
      value: new Uint8Array(uint8Array(value, valueLen)),
    };
    console.log("set_kv", data);
    getObj<ClarionDbTrx>(trxIndex).put(data.key, data.value);
    wasmCallback(wasmCbIndex, wasmCbPtr);
  },

  create_cursor(trxIndex: number, wasmCbPtr: any, wasmCbIndex: number) {
    const req = objects[trxIndex].kv.openCursor();
    req.onsuccess = (_e: any) =>
      wasmCallback(wasmCbIndex, wasmCbPtr, addObj(req));
    req.onerror = (e: any) => console.error(e);
  },

  // TODO: remove? Maybe create_cursor and cursor_next should indicate this?
  cursor_has_value(cursorIndex: number) {
    return objects[cursorIndex].result ? true : false;
  },

  cursor_value(cursorIndex: number) {
    const c = objects[cursorIndex].result;
    if (!c) throwError("cursor has no value");
    return addObj(c.value.v);
  },

  cursor_next(cursorIndex: number, wasmCbPtr: any, wasmCbIndex: number) {
    const req = objects[cursorIndex];
    if (!req.result) throwError("cursor has no value");
    req.onsuccess = (_e: any) => wasmCallback(wasmCbIndex, wasmCbPtr);
    req.result.continue();
  },
};

export interface ClarionWasm {
  module: WebAssembly.Module;
  instance: WebAssembly.Instance;
}

export const initClarion = async (
  wasmBytes: BufferSource,
  database: any
): Promise<ClarionWasm> => {
  dbAdapter = database;
  const module = await WebAssembly.compile(wasmBytes);
  instance = await WebAssembly.instantiate(module, { clarion });
  return { module, instance };
};
