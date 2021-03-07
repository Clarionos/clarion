import { initClarion } from "clwasmer";
import level from "level";

const wasmFilePath = "/clarion.wasm";

const init = async () => {
  const response = await fetch(wasmFilePath);
  const wasmBytes = new Uint8Array(await response.arrayBuffer());

  const clarionDb = {
    open(name, callback) {
      console.info("initializing level... ", name);
      level(name, { keyEncoding: "binary", valueEncoding: "binary" }, callback);
    },
  };

  const { instance } = await initClarion(wasmBytes, clarionDb);
  console.info(instance);
  instance.exports._start();
};

init();
