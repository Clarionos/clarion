let consoleBuf = "";
let instance: WebAssembly.Instance;
let objects: any[] = [null];
let db: any; // todo: create a clariondb wrapper

const oops = (s: any) => {
  console.error("oops: ", s);
  throw new Error(s);
};

const uint8Array = (pos: number, len: number) => {
  const memory = instance.exports.memory as WebAssembly.Memory;
  return new Uint8Array(memory.buffer, pos, len);
};

const decodeStr = (pos: number, len: number) => {
  return new TextDecoder().decode(uint8Array(pos, len));
};

const addObj = (obj: any) => {
  objects.push(obj);
  return objects.length - 1;
};

const wasmCallback = (fnIndex: number, ...params: any): Function => {
  const fnTable = instance.exports
    .__indirect_function_table as WebAssembly.Table;
  const fn = fnTable.get(fnIndex);
  if (!fn) {
    throw new Error("Invalid function!"); // todo: can we throw?
  }
  return fn(...params);
};

const clarion = {
  exit(code: number) {
    oops("exit: " + code);
  },

  console(pos: number, len: number) {
    const s = consoleBuf + decodeStr(pos, len);
    const l = s.split("\n");
    for (let i = 0; i < l.length - 1; ++i) console.log(l[i]);
    consoleBuf = l[l.length - 1];
  },

  callme_later(delay_ms: number, param: any, fnIndex: number) {
    setTimeout(() => wasmCallback(fnIndex, param), delay_ms);
  },

  release_object(index: number) {
    console.log("release_object", index, objects[index]);
    objects[index] = null;
  },

  // TODO: context argument
  open_db(pos: number, len: number, param: any, fnIndex: number) {
    const dbName = decodeStr(pos, len);

    const req = db.open(dbName, 1);

    req.onupgradeneeded = (_e: any) =>
      req.result.createObjectStore("kv", { keyPath: "k" });
    req.onsuccess = (_e: any) =>
      wasmCallback(fnIndex, param, addObj(req.result));

    req.onupgradeneeded = (_e: any) =>
      req.result.createObjectStore("kv", { keyPath: "k" });
    req.onsuccess = (_e: any) =>
      wasmCallback(fnIndex, param, addObj(req.result));
  },

  // TODO: automatically abort transactions which aren't committed?
  // TODO: give this an async interface?
  create_transaction(dbIndex: number, writable: boolean) {
    const trx = objects[dbIndex].transaction(
      "kv",
      writable ? "readwrite" : "readonly"
    );
    const kv = trx.objectStore("kv");
    return addObj({ trx, kv });
  },

  // TODO: give this an async interface?
  abort_transaction(trxIndex: number) {
    objects[trxIndex].trx.abort();
  },

  // TODO: give this an async interface?
  commit_transaction(trxIndex: number) {
    objects[trxIndex].trx.commit();
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
      k: uint8Array(key, keyLen),
      v: uint8Array(value, valueLen),
    });
    const req = objects[trxIndex].kv.put({
      k: new Uint8Array(uint8Array(key, keyLen)),
      v: new Uint8Array(uint8Array(value, valueLen)),
    });
    req.onsuccess = (_e: any) => wasmCallback(fnIndex, params);
    req.onerror = (e: any) => console.error(e);
  },

  create_cursor(trxIndex: number, params: any, fnIndex: number) {
    const req = objects[trxIndex].kv.openCursor();
    req.onsuccess = (_e: any) => wasmCallback(fnIndex, params, addObj(req));
    req.onerror = (e: any) => console.error(e);
  },

  // TODO: remove? Maybe create_cursor and cursor_next should indicate this?
  cursor_has_value(cursorIndex: number) {
    return objects[cursorIndex].result ? true : false;
  },

  cursor_value(cursorIndex: number) {
    const c = objects[cursorIndex].result;
    if (!c) oops("cursor has no value");
    return addObj(c.value.v);
  },

  cursor_next(cursorIndex: number, params: any, fnIndex: number) {
    const req = objects[cursorIndex];
    if (!req.result) oops("cursor has no value");
    req.onsuccess = (_e: any) => wasmCallback(fnIndex, params);
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
  db = database;
  const module = await WebAssembly.compile(wasmBytes);
  instance = await WebAssembly.instantiate(module, { clarion });
  return { module, instance };
};
