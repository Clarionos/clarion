import { initClarion } from "clwasmer";

const wasmFilePath = "/clarion.wasm";

const init = async () => {
  const response = await fetch(wasmFilePath);
  const wasmBytes = new Uint8Array(await response.arrayBuffer());
  const { instance } = await initClarion(wasmBytes, indexedDB);
  console.info(instance);
  instance.exports._start();
};

init();
