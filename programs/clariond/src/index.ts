import * as Level from "level";

import { Context } from "clwasmer";
import { DATABASE, CLARION_WASM_PATH } from "./config";
import { readFileSync } from "fs";

const main = async () => {
  console.info(">>> Initializing Clarion...");
  const db = await initDb();
  await loadClarion(db);
};

const initDb = async () => {
  const db = Level(DATABASE);

  try {
    const existingName = await db.get("name");
    console.info(">>> Existing name: ", existingName);
  } catch (error) {
    console.info(">>> Database is brand new");
  }

  try {
    await db.put("name", "Clarion " + Math.floor(Math.random() * 100000));

    const newName = await db.get("name");
    console.info(">>> Saved New Name: ", newName);
  } catch (error) {
    console.error("!!! Fail to initialize DB: ", error);
  }

  return db;
};

const loadClarion = async (db: any) => {
  const clarionWasm = readFileSync(CLARION_WASM_PATH);
  const context = new Context;
  context.db = db;
  await context.instanciate(clarionWasm);
  (context.instance!.exports as any)._start();
};

main();
