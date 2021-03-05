import * as Level from "level";

import { DATABASE } from "./config";

const main = () => {
  console.info(">>> Initializing Clarion...");
  initDb();
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
};

main();
